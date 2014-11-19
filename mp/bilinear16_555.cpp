#include <windows.h>
#include <xmmintrin.h>
#include <math.h>

static int* g_px1a    = NULL;
static int  g_px1a_w  = 0;
static int* g_px1ab   = NULL;
static int  g_px1ab_w = 0;

void WINAPI Resize_HQ_2ch555( unsigned char* src, RECT *srcrect, int srcpitch,
                    unsigned char* dest, RECT *destrect, int destpitch)
{
    // Both buffers must be in RGB 565 format.

	int w1, w2, h1, h2;
	w1 = srcrect->right - srcrect->left;
	h1 = srcrect->bottom - srcrect->top;
	w2 = destrect->right - destrect->left;
	h2 = destrect->bottom - destrect->top;

	if(!srcpitch) srcpitch=w1<<1;
	if(!destpitch) destpitch=w1<<1;

	// GHO addiction: new variables
	// p1, p2: pitch offsets of source and dest surfaces in DWORD offset, that is pitch / sizeof(DWORD)
	// beware: current version can operate on displaced source rect, but assumes the dest rect is always the full surface!!
	USHORT p1 = srcpitch >> 1; 
	USHORT p2 = destpitch >> 1; 
	USHORT *dsrc  = (USHORT *)src + (srcrect->top * p1) + srcrect->left;
    USHORT *ddest = (USHORT *)dest;

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

            USHORT *ddest = &((USHORT *)dest)[y2*p2 + 0];

            for (int x2=0; x2<w2; x2++)
            {
                // find the x-range of input pixels that will contribute:
                int x1a = g_px1a[x2];//(int)(x2*fw); 
                int x1c = x1a >> 8;

                USHORT *dsrc2 = &dsrc[y1c*p1 + x1c]; // GHO

                // PERFORM BILINEAR INTERPOLATION on 2x2 pixels
                UINT r=0, g=0, b=0, a=0;
                UINT weight_x = 256 - (x1a & 0xFF);
                UINT weight_y = 256 - (y1a & 0xFF);
                for (int y=0; y<2; y++)
                {
                    for (int x=0; x<2; x++)
                    {
                        UINT c = (UINT)dsrc2[x + y*p1]; // GHO
                        UINT r_src = (c    ) & 0x1F;
                        UINT g_src = (c>> 5) & 0x1F;
                        UINT b_src = (c>>10) & 0x1F;
                        UINT w = (weight_x * weight_y) >> weight_shift;
                        r += r_src * w;
                        g += g_src * w;
                        b += b_src * w;
                        weight_x = 256 - weight_x;
                    }
                    weight_y = 256 - weight_y;
                }

				UINT c = ((r>>16) & 0x1F) | ((g>>(16-5)) & 0x3E0) | ((b>>(16-10)) & 0x7C00);
				*ddest++ = (USHORT)c;
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

			ddest = &((USHORT *)dest)[y2*p2 + 0];

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

                    USHORT *dsrc2 = &dsrc[y*p1 + x1c]; // GHO
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

                        UINT c = dsrc[y*p1 + x];
                        UINT r_src = (c    ) & 0x1F;
                        UINT g_src = (c>> 5) & 0x1F;
                        UINT b_src = (c>>10) & 0x1F;
                        UINT w = (weight_x * weight_y) >> weight_shift;
                        r += r_src * w;
                        g += g_src * w;
                        b += b_src * w;
                        a += w;
                    }
                }

                // write results
				UINT c = ((r/a) & 0x1F) | (((g/a) << 5) & 0x3E0) | (((b/a) << 10) & 0x7C00);
				*ddest++ = c;
            }
        }
    }
}