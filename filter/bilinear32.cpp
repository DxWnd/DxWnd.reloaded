/* =============================================================================

Source Code for Smooth Image Resampling (Resizing) in C/C++ (Visual Studio)
by Ryan Geiss - 3 July 2008
  
  The code below performs a fairly-well-optimized high-quality resample 
  (smooth resize) of a 3-channel image that is padded to 4 bytes per 
  pixel.  The pixel format is assumed to be ARGB.  If you want to make 
  it handle an alpha channel, the changes should be very straightforward.
  
  In general, if the image is being enlarged, bilinear interpolation
  is used; if the image is being downsized, all input pixels are weighed
  appropriately to produce the correct result.
  
  In order to be efficient, it actually performs 1 of 4 routines.  First, 
  if you are cutting the image size *exactly* in half (common when generating 
  mipmap levels), it will use a specialized routine to do just that.  There
  are actually two versions of this routine - an MMX one and a non-MMX one.
  It detects if MMX is present and chooses the right one.
  
  If you're not cutting the image perfectly in half, it executes one
  of two general resize routines.  If upsampling (increasing width and height)
  on both X and Y, then it executes a faster resize algorithm that just performs
  a 2x2 bilinear interpolation of the appropriate input pixels, for each output 
  pixel.  
  
  If downsampling on either X or Y (or both), though, the general-purpose 
  routine gets run.  It iterates over every output pixel, and for each one, it 
  iterates over the input pixels that map to that output pixel [there will 
  usually be more than 1 in this case].  Each input pixel is properly weighted
  to produce exactly the right image.  There's a little bit of extra bookkeeping,
  but in general, it's pretty efficient.
  
  Note that on extreme downsizing (2,800 x 2,800 -> 1x1 or greater ratio),
  the colors can overflow.  If you want to fix this lazily, just break
  your resize into two passes.
  
  Also note that when your program exits, or when you are done using this 
  function, you should delete [] g_px1a and g_px1ab if they have been 
  allocated.
  
  I posted this here because this is pretty common code that is a bit of
  a pain to write; I've written it several times over the years, and I really
  don't feel like writing it again.  So - here it is - for my reference, and
  for yours.  Enjoy!
/* ========================================================================== */

#include <windows.h>
#include <xmmintrin.h>
#include <math.h>

static int* g_px1a    = NULL;
static int  g_px1a_w  = 0;
static int* g_px1ab   = NULL;
static int  g_px1ab_w = 0;

void WINAPI Resize_HQ_4ch( unsigned char* src, RECT *srcrect, int srcpitch,
                    unsigned char* dest, RECT *destrect, int destpitch)
{
    // Both buffers must be in ARGB format, and a scanline should be w*4 bytes.

    // NOTE: THIS WILL OVERFLOW for really major downsizing (2800x2800 to 1x1 or more) 
    // (2800 ~ sqrt(2^23)) - for a lazy fix, just call this in two passes.

	int w1, w2, h1, h2;
	w1 = srcrect->right - srcrect->left;
	h1 = srcrect->bottom - srcrect->top;
	w2 = destrect->right - destrect->left;
	h2 = destrect->bottom - destrect->top;

	if(!srcpitch) srcpitch=w1<<2;
	if(!destpitch) destpitch=w1<<2;

	// GHO addiction: new variables
	// p1, p2: pitch offsets of source and dest surfaces in DWORD offset, that is pitch / sizeof(DWORD)
	// beware: current version can operate on displaced source rect, but assumes the dest rect is always the full surface!!
	UINT p1 = srcpitch >> 2; 
	UINT p2 = destpitch >> 2; 
	UINT *dsrc  = (UINT *)src + (srcrect->top * p1) + srcrect->left;
    UINT *ddest = (UINT *)dest;

    // arbitrary resize.

    bool bUpsampleX = (w1 < w2);
    bool bUpsampleY = (h1 < h2);

    // If too many input pixels map to one output pixel, our 32-bit accumulation values
    // could overflow - so, if we have huge mappings like that, cut down the weights:
    //    256 max color value
    //   *256 weight_x
    //   *256 weight_y
    //   *256 (16*16) maximum # of input pixels (x,y) - unless we cut the weights down...
    int weight_shift = 0;

	//gsky916: weight_shift calculation in bUpsampleX && bUpsampleY cases are not necessary.
	//Move to else block to reduce floating point calculations.

    float fh = 256*h1/(float)h2;
    float fw = 256*w1/(float)w2;

    if (bUpsampleX && bUpsampleY)
    {
        // faster to just do 2x2 bilinear interp here

        // cache x1a, x1b for all the columns:
        // ...and your OS better have garbage collection on process exit :)
        if (g_px1a_w < w2)
        {
            if (g_px1a) delete [] g_px1a;
            g_px1a = new int[w2*2 * 1];
            g_px1a_w = w2*2;
        }
        for (int x2=0; x2<w2; x2++)
        {
            // find the x-range of input pixels that will contribute:
            int x1a = (int)(x2*fw);
            x1a = min(x1a, 256*(w1-1) - 1);
            g_px1a[x2] = x1a;
        }

        // FOR EVERY OUTPUT PIXEL
		// gsky916: Use OpenMP to speed up nested for loops (Enable OpenMP support in compiler).
		#pragma omp parallel for schedule(dynamic)
        for (int y2=0; y2<h2; y2++)
        {   
            // find the y-range of input pixels that will contribute:
            int y1a = (int)(y2*fh);
            y1a = min(y1a, 256*(h1-1) - 1);
            int y1c = y1a >> 8;

            UINT *ddest = &((UINT *)dest)[y2*p2 + 0];

            for (int x2=0; x2<w2; x2++)
            {
                // find the x-range of input pixels that will contribute:
                int x1a = g_px1a[x2];//(int)(x2*fw); 
                int x1c = x1a >> 8;

                UINT *dsrc2 = &dsrc[y1c*p1 + x1c]; // GHO

                // PERFORM BILINEAR INTERPOLATION on 2x2 pixels
                UINT r=0, g=0, b=0, a=0;
                UINT weight_x = 256 - (x1a & 0xFF);
                UINT weight_y = 256 - (y1a & 0xFF);
                for (int y=0; y<2; y++)
                {
                    for (int x=0; x<2; x++)
                    {
                        //UINT c = dsrc2[x + y*w1];
                        UINT c = dsrc2[x + y*p1]; // GHO
                        UINT r_src = (c    ) & 0xFF;
                        UINT g_src = (c>> 8) & 0xFF;
                        UINT b_src = (c>>16) & 0xFF;
                        UINT w = (weight_x * weight_y) >> weight_shift;
                        r += r_src * w;
                        g += g_src * w;
                        b += b_src * w;
                        weight_x = 256 - weight_x;
                    }
                    weight_y = 256 - weight_y;
                }

                UINT c = ((r>>16)) | ((g>>8) & 0xFF00) | (b & 0xFF0000);
				*ddest++ = c;
            }
        }
    }
    else // either downscale on vertical or horizontal direction ...
    {
		//gsky916: weight_shift calculation moved here.
        float source_texels_per_out_pixel = (   (w1/(float)w2 + 1) 
                                              * (h1/(float)h2 + 1)
                                            );
        float weight_per_pixel = source_texels_per_out_pixel * 256 * 256;  //weight_x * weight_y
        float accum_per_pixel = weight_per_pixel*256; //color value is 0-255
        float weight_div = accum_per_pixel / 4294967000.0f;
        if (weight_div > 1)
            weight_shift = (int)ceilf( logf((float)weight_div)/logf(2.0f) );
        weight_shift = min(15, weight_shift);  // this could go to 15 and still be ok.

        // cache x1a, x1b for all the columns:
        // ...and your OS better have garbage collection on process exit :)
        if (g_px1ab_w < w2)
        {
            if (g_px1ab) delete [] g_px1ab;
            g_px1ab = new int[w2*2 * 2];
            g_px1ab_w = w2*2;
        }
        for (int x2=0; x2<w2; x2++)
        {
            // find the x-range of input pixels that will contribute:
            int x1a = (int)((x2  )*fw); 
            int x1b = (int)((x2+1)*fw); 
            if (bUpsampleX) // map to same pixel -> we want to interpolate between two pixels!
                x1b = x1a + 256;
            x1b = min(x1b, 256*w1 - 1);
            g_px1ab[x2*2+0] = x1a;
            g_px1ab[x2*2+1] = x1b;
        }

        // FOR EVERY OUTPUT PIXEL
        for (int y2=0; y2<h2; y2++)
        {   
            // find the y-range of input pixels that will contribute:
            int y1a = (int)((y2  )*fh); 
            int y1b = (int)((y2+1)*fh); 
            if (bUpsampleY) // map to same pixel -> we want to interpolate between two pixels!
                y1b = y1a + 256;
            y1b = min(y1b, 256*h1 - 1);
            int y1c = y1a >> 8;
            int y1d = y1b >> 8;

			ddest = &((UINT *)dest)[y2*p2 + 0];

            for (int x2=0; x2<w2; x2++)
            {
                // find the x-range of input pixels that will contribute:
                int x1a = g_px1ab[x2*2+0];    // (computed earlier)
                int x1b = g_px1ab[x2*2+1];    // (computed earlier)
                int x1c = x1a >> 8;
                int x1d = x1b >> 8;

                // ADD UP ALL INPUT PIXELS CONTRIBUTING TO THIS OUTPUT PIXEL:
                UINT r=0, g=0, b=0, a=0;
                for (int y=y1c; y<=y1d; y++)
                {
                    UINT weight_y = 256;
                    if (y1c != y1d) 
                    {
                        if (y==y1c)
                            weight_y = 256 - (y1a & 0xFF);
                        else if (y==y1d)
                            weight_y = (y1b & 0xFF);
                    }

                    //UINT *dsrc2 = &dsrc[y*w1 + x1c];
                    UINT *dsrc2 = &dsrc[y*p1 + x1c]; // GHO
                    for (int x=x1c; x<=x1d; x++)
                    {
                        UINT weight_x = 256;
                        if (x1c != x1d) 
                        {
                            if (x==x1c)
                                weight_x = 256 - (x1a & 0xFF);
                            else if (x==x1d)
                                weight_x = (x1b & 0xFF);
                        }

                        //UINT c = *dsrc2++;//dsrc[y*w1 + x];
                        UINT c = dsrc[y*p1 + x];
                        UINT r_src = (c    ) & 0xFF;
                        UINT g_src = (c>> 8) & 0xFF;
                        UINT b_src = (c>>16) & 0xFF;
                        UINT w = (weight_x * weight_y) >> weight_shift;
                        r += r_src * w;
                        g += g_src * w;
                        b += b_src * w;
                        a += w;
                    }
                }

                // write results
                UINT c = ((r/a)) | ((g/a)<<8) | ((b/a)<<16);
                //*ddest++ = c;//ddest[y2*w2 + x2] = c;
				*ddest++ = c;
				//ddest+=(w2-p2);
            }
        }
    }
}

