#include "stdafx.h"
#include "bmpext.h"

//////////////////////////////////////////////////////////////////////////
// DIB support defines
#define BMIH_SIZE sizeof BITMAPINFOHEADER
#define BMIF_SIZE sizeof BITMAPFILEHEADER

/* DIB constants */
#define PALVERSION   0x300

/* Dib Header Marker - used in writing DIBs to files */
#define DIB_HEADER_MARKER   ((WORD) ('M' << 8) | 'B')

// WIDTHBYTES performs DWORD-aligning of DIB scanlines.  The "bits"
// parameter is the bit count for the scanline (biWidth * biBitCount),
// and this macro returns the number of DWORD-aligned bytes needed
// to hold those bits.
#define WIDTHBYTES(bits)    (((bits) + 31) / 32 * 4)

/////////////////////////////////////////////////////////////////////////////
//CDib implementation

CDib::CDib() : m_pBMI(0), m_pBits(0), m_pPalette(0)
{}

CDib::~CDib()
{
	Free();
}

DWORD CDib::Width() const
{
	if (!m_pBMI)
		return 0;
	
	/* return the DIB width */
	return m_pBMI->bmiHeader.biWidth;
}

DWORD CDib::Height() const
{
	if (!m_pBMI)
		return 0;
	
	/* return the DIB height */
	return m_pBMI->bmiHeader.biHeight;
}

WORD  CDib::NumColors(  BITMAPINFOHEADER& bmiHeader ) const
{
	if ( bmiHeader.biClrUsed != 0)
		return (WORD)bmiHeader.biClrUsed;
	
	switch ( bmiHeader.biBitCount )
	{
	case 1:
		return 2;
	case 4:
		return 16;
	case 8:
		return 256;
	default:
		return 0;
	}
}

BOOL  CDib::IsValid()   const
{
	return (m_pBits != NULL);
}

DWORD CDib::PaletteSize() const
{
	return NumColors( m_pBMI->bmiHeader ) * sizeof(RGBQUAD);
}

BOOL  CDib::Draw(CDC* pDC, CRect& rectDC, CRect& rectDIB) const
{
	if ( !IsValid() )
		return FALSE;
	
	CPalette* pOldPal = NULL;        // Previous palette
	
	// Get the DIB's palette, then select it into DC
	if (m_pPalette != NULL)
	{
		// Select as background since we have
		// already realized in forground if needed
		pOldPal = pDC->SelectPalette( m_pPalette, TRUE);
	}
	
	/* Make sure to use the stretching mode best for color pictures */
	pDC->SetStretchBltMode( COLORONCOLOR );
	
	/* Determine whether to call StretchDIBits() or SetDIBitsToDevice() */
	BOOL bSuccess;
	if( ( rectDC.Width() == rectDIB.Width() ) &&
		( rectDC.Height() == rectDIB.Height() ) )
		bSuccess = ::SetDIBitsToDevice(pDC->m_hDC,				// hDC
		rectDC.left,         // DestX
		rectDC.top,          // DestY
		rectDC.Width(),      // nDestWidth
		rectDC.Height(),     // nDestHeight
		rectDIB.left,	    // SrcX
		(int)Height() -
		rectDIB.top -
		rectDIB.Height(),		   // SrcY
		0,                          // nStartScan
		(WORD)Height(),             // nNumScans
		m_pBits,                    // lpBits
		m_pBMI,                     // lpBitsInfo
		DIB_RGB_COLORS);            // wUsage
	else
		bSuccess = ::StretchDIBits(pDC->m_hDC,						// hDC
		rectDC.left,					// DestX
		rectDC.top,					// DestY
		rectDC.Width(),				// nDestWidth
		rectDC.Height(),				// nDestHeight
		rectDIB.left,				// SrcX
		rectDIB.top,					// SrcY
		rectDIB.Width(),				// wSrcWidth
		rectDIB.Height(),			// wSrcHeight
		m_pBits,                      // lpBits
		m_pBMI,                       // lpBitsInfo
		DIB_RGB_COLORS,               // wUsage
		SRCCOPY);                     // dwROP
	
	/* Reselect old palette */
	if (pOldPal != NULL)
	{
		pDC->SelectPalette( pOldPal, TRUE);
	}
	
	return bSuccess;
}


void CDib::AssertPosition(int iX, int iY)
{
	if( (iX < 0) || (iX > m_pBMI->bmiHeader.biWidth - 1) ||
		(iY < 0) || (iY > m_pBMI->bmiHeader.biHeight - 1) )
	{
		//invalid image pixel position
		CDibException::Throw( CDibException::E_INVPOS );
	}
}


RGBQUAD CDib::GetPixel(int iX, int iY)
{
 	RGBQUAD rgbResult;
 	WORD wDummy;
 	
	//takeinto account that DIBit raws are reversed vertically
	iY = (m_pBMI->bmiHeader.biHeight - 1) - iY; // GHO fix
	//iY = m_pBMI->bmiHeader.biHeight - iY;
	
	//assert pixel position
	AssertPosition( iX, iY );

 	//access the destination pixel
 	int nRowBytes = m_pBMI->bmiHeader.biWidth * m_pBMI->bmiHeader.biBitCount;
     nRowBytes = ( (nRowBytes + 31) & (~31) ) / 8;
 	
 	switch( m_pBMI->bmiHeader.biBitCount )
 	{
 		case 1:		//Monochrome
 			rgbResult = m_pBMI->bmiColors[ *(m_pBits + nRowBytes*iY + iX/8) & (0x80 >> iX%8) ];
 			break;
 		case 4:
 			rgbResult = m_pBMI->bmiColors[ *(m_pBits + nRowBytes*iY + iX/2) & ((iX&1) ? 0x0f : 0xf0) ];
 			break;
 		case 8:
 			rgbResult = m_pBMI->bmiColors[ *(m_pBits + nRowBytes*iY + iX) ];
 			break;
 		case 16:
 			wDummy = *(LPWORD)(m_pBits + nRowBytes*iY + iX*2);
 
 			rgbResult.rgbBlue = (BYTE)(0x001F & wDummy);
 			rgbResult.rgbGreen = (BYTE)(0x001F & (wDummy >> 5));
 			rgbResult.rgbRed = (BYTE)(0x001F & wDummy >> 10 );
 			break;
 		case 24:
 			rgbResult = *(LPRGBQUAD)(m_pBits + nRowBytes*iY + iX*3);
 			break;
 		case 32:
 			rgbResult = *(LPRGBQUAD)(m_pBits + nRowBytes*iY + iX*4);
 			break;
 	}
 
 	return rgbResult;
}

void CDib::SetPixel(int iX, int iY, RGBQUAD &rgbPixel)
{
	WORD wDummy;

	//takeinto account that DIBit raws are reversed vertically
	iY = (m_pBMI->bmiHeader.biHeight - 1) - iY; // GHO fix
	//iY = m_pBMI->bmiHeader.biHeight - iY;
	
	//assert pixel position
	AssertPosition( iX, iY );

	//access the destination pixel
	int nRowBytes = m_pBMI->bmiHeader.biWidth * m_pBMI->bmiHeader.biBitCount;
    nRowBytes = ( (nRowBytes + 31) & (~31) ) / 8;
	
	switch( m_pBMI->bmiHeader.biBitCount )
	{
		case 1:		
		case 4:
		case 8:
			//do not support this operation;
			CDibException::Throw( CDibException::E_NOTSUPP );
			break;
		case 16:
			wDummy = rgbPixel.rgbRed;
			wDummy = wDummy << 5;
			wDummy |= rgbPixel.rgbGreen;
			wDummy = wDummy << 5;
			wDummy |= rgbPixel.rgbBlue;

			*(LPWORD)(m_pBits + nRowBytes*iY + iX*2) = wDummy;
			break;
		case 24:
			*(LPRGBQUAD)(m_pBits + nRowBytes*iY + iX*3) = rgbPixel;
			break;
		case 32:
			*(LPRGBQUAD)(m_pBits + nRowBytes*iY + iX*4) = rgbPixel;
			break;
	}
}

DWORD CDib::Save(CFile& file) const
{
	BITMAPFILEHEADER bmfHdr; // Header for Bitmap file
	DWORD dwDIBSize;
	
	if (m_pBMI == NULL)
		return 0;
	
	// Fill in the fields of the file header
	
	// Fill in file type (first 2 bytes must be "BM" for a bitmap)
	bmfHdr.bfType = DIB_HEADER_MARKER;  // "BM"
	
	// Calculating the size of the DIB is a bit tricky (if we want to
	// do it right).  The easiest way to do this is to call GlobalSize()
	// on our global handle, but since the size of our global memory may have
	// been padded a few bytes, we may end up writing out a few too
	// many bytes to the file (which may cause problems with some apps).
	//
	// So, instead let's calculate the size manually (if we can)
	//
	// First, find size of header plus size of color table.  Since the
	// first DWORD in both BITMAPINFOHEADER and BITMAPCOREHEADER conains
	// the size of the structure, let's use this.
	dwDIBSize = m_pBMI->bmiHeader.biSize + PaletteSize();  // Partial Calculation
	
	// Now calculate the size of the image
	if ((m_pBMI->bmiHeader.biCompression == BI_RLE8) || (m_pBMI->bmiHeader.biCompression == BI_RLE4))
	{
		// It's an RLE bitmap, we can't calculate size, so trust the
		// biSizeImage field
		dwDIBSize += m_pBMI->bmiHeader.biSizeImage;
	}
	else
	{
		DWORD dwBmBitsSize;  // Size of Bitmap Bits only
		
		// It's not RLE, so size is Width (DWORD aligned) * Height
		dwBmBitsSize = WIDTHBYTES((m_pBMI->bmiHeader.biWidth)*((DWORD)m_pBMI->bmiHeader.biBitCount)) * m_pBMI->bmiHeader.biHeight;
		dwDIBSize += dwBmBitsSize;
		
		// Now, since we have calculated the correct size, why don't we
		// fill in the biSizeImage field (this will fix any .BMP files which
		// have this field incorrect).
		m_pBMI->bmiHeader.biSizeImage = dwBmBitsSize;
	}
	
	// Calculate the file size by adding the DIB size to sizeof(BITMAPFILEHEADER)
	bmfHdr.bfSize = dwDIBSize + BMIF_SIZE;
	bmfHdr.bfReserved1 = 0;
	bmfHdr.bfReserved2 = 0;
	
	/*
	* Now, calculate the offset the actual bitmap bits will be in
	* the file -- It's the Bitmap file header plus the DIB header,
	* plus the size of the color table.
	*/
	bmfHdr.bfOffBits = BMIF_SIZE + m_pBMI->bmiHeader.biSize + PaletteSize();
	
	// Write the file header
	file.Write( (LPSTR)&bmfHdr, BMIF_SIZE );
	DWORD dwBytesSaved = BMIF_SIZE; 
	
	// Write the DIB header
	UINT nCount = sizeof(BITMAPINFO) + PaletteSize();
	dwBytesSaved += nCount; 
	file.Write(m_pBMI, nCount);
	
	// Write the DIB bits
	DWORD dwBytes = m_pBMI->bmiHeader.biBitCount * Width();
	// Calculate the number of bytes per line
	if (dwBytes%32 == 0)
		dwBytes /= 8;
	else
		dwBytes = dwBytes/8 + (32-dwBytes%32)/8 + (((32-dwBytes%32)%8 > 0) ? 1 : 0); 
	nCount = dwBytes * Height();
	dwBytesSaved += nCount; 
	//file.WriteHuge(m_pBits, nCount);
	file.Write(m_pBits, nCount);
	
	return dwBytesSaved;
}

DWORD CDib::Read(CFile& file, BOOL bFromResource)
{
	DWORD dwReadBytes = 0;
	DWORD dwLength = (DWORD)file.GetLength();
	
	// Ensures no memory leaks will occur
	Free();
	
	BITMAPFILEHEADER bmfHeader;
	BITMAPINFOHEADER bmiHeader;
	
	if( !bFromResource )
	{
		// Go read the DIB file header and check if it's valid.
		if( (dwReadBytes = file.Read((LPSTR)&bmfHeader, BMIF_SIZE)) != BMIF_SIZE)
			return 0;
		if(bmfHeader.bfType != DIB_HEADER_MARKER)
			return 0;
	}
	
	// Read DIB header.
	if( file.Read( &bmiHeader, BMIH_SIZE ) != BMIH_SIZE )
		return 0;
	dwReadBytes += BMIH_SIZE;
	
	DWORD dwPalSize = NumColors( bmiHeader ) * sizeof RGBQUAD;
	m_pBMI = (LPBITMAPINFO) new BYTE[BMIH_SIZE + dwPalSize];
	memcpy( m_pBMI, &bmiHeader, BMIH_SIZE );
	// read palette data
	if( file.Read( m_pBMI->bmiColors, dwPalSize ) != dwPalSize )
		return 0;
	dwReadBytes += dwPalSize;
	CreatePalette();
	
	// Go read the bits.
	m_pBits = new BYTE[ dwLength - dwReadBytes + 0x200]; // GHO fix: you need some more space who knows why...? 
	if (m_pBits == 0)
		return 0;
	
	if (file.Read( m_pBits, dwLength - dwReadBytes ) != (dwLength - dwReadBytes))
	{
		delete m_pBMI;
		m_pBMI = NULL;
		
		delete m_pBits;
		m_pBits = NULL;
		
		return 0;
	}
	dwReadBytes = dwLength;
	
	return dwReadBytes;
}

DWORD CDib::ReadFromResource(UINT nResID)
{
	DWORD	dwResult = 0;
	// Load	from resource
	HRSRC	hbmres = FindResource( NULL, MAKEINTRESOURCE(nResID), RT_BITMAP );
	CMemFile file; 
	HGLOBAL	hGlob;
	
	if (hbmres)
	{
		DWORD	dwResSize = SizeofResource( NULL, hbmres );
		file.Attach( (LPBYTE)LockResource( hGlob = LoadResource(NULL, hbmres) ), dwResSize );
		dwResult = Read(file, TRUE);
		file.Detach();
		DeleteObject( hGlob );
	}
	
	return dwResult;
}

void CDib::Invalidate()
{ 
	Free();
}

BOOL  CDib::CreatePalette()
{
	if ( !IsValid() )
		return FALSE;
	
	//get the number of colors in the DIB
	WORD wNumColors = NumColors( m_pBMI->bmiHeader );
	BOOL bResult = TRUE;
	
	if (wNumColors != 0)
	{
		// allocate memory block for logical palette
		LPLOGPALETTE pLogPal = (LPLOGPALETTE) new BYTE[ sizeof(LOGPALETTE) +
			sizeof(PALETTEENTRY)*wNumColors ];
		
		// if not enough memory, clean up and return NULL
		if( pLogPal == 0 )
			return FALSE;
		
		// set version and number of palette entries
		pLogPal->palVersion = PALVERSION;
		pLogPal->palNumEntries = wNumColors;
		
		for (WORD i = 0; i < wNumColors; i++)
		{
			pLogPal->palPalEntry[i].peRed = m_pBMI->bmiColors[i].rgbRed;
			pLogPal->palPalEntry[i].peGreen = m_pBMI->bmiColors[i].rgbGreen;
			pLogPal->palPalEntry[i].peBlue = m_pBMI->bmiColors[i].rgbBlue;
			pLogPal->palPalEntry[i].peFlags = 0;
		}
		
		// create the palette and get handle to it 
		if (m_pPalette)
		{
			m_pPalette->DeleteObject();
			delete m_pPalette;
		}
		
		m_pPalette = new CPalette;
		bResult = m_pPalette->CreatePalette( pLogPal );
		delete pLogPal;
	}
	
	return bResult;
}

void CDib::Free()
{
	// Make sure all member data that might have been allocated is freed.
	if(m_pBMI)
	{
		delete m_pBMI;
		m_pBMI = NULL;
	}
	
	if(m_pBits)
	{
		delete m_pBits;
		m_pBits = NULL;
	}
	
	if(m_pPalette)
	{
		m_pPalette->DeleteObject();
		delete m_pPalette;
		m_pPalette = NULL;
	}
}

HBITMAP CDib::CreateDDBitmap( HDC hDC )
{
	HBITMAP hBitmap = ::CreateDIBitmap( hDC, &m_pBMI->bmiHeader,
		CBM_INIT, m_pBits, (LPBITMAPINFO)m_pBMI, DIB_RGB_COLORS);
	ASSERT(hBitmap);
	return hBitmap;
}

HBITMAP	CDib::CreateDDBitmap(CDC* pDC)
{	
	return CreateDDBitmap( pDC->GetSafeHdc() );
}

BOOL CDib::Compress(CDC* pDC, BOOL bCompress )
{
	// 1. makes GDI bitmap from existing DIB
	// 2. makes a new DIB from GDI bitmap with compression
	// 3. cleans up the original DIB
	// 4. puts the new DIB in the object
	if((m_pBMI->bmiHeader.biBitCount != 4) && (m_pBMI->bmiHeader.biBitCount != 8)) return FALSE;
	// compression supported only for 4 bpp and 8 bpp DIBs
	TRACE(_T("Compress: original palette size = %d\n"), NumColors(m_pBMI->bmiHeader) ); 
	HDC hdc = pDC->GetSafeHdc();
	CPalette* pOldPalette = pDC->SelectPalette( m_pPalette, TRUE);
	HBITMAP hBitmap;  // temporary
	if((hBitmap = CreateDDBitmap(pDC)) == NULL) return FALSE;
	int nSize = BMIF_SIZE + PaletteSize();
	LPBITMAPINFO pBMI = (LPBITMAPINFO) new char[nSize];
	memcpy(pBMI, &m_pBMI->bmiHeader, nSize);  // new header
	if(bCompress) {
		switch (pBMI->bmiHeader.biBitCount) {
		case 4:
			pBMI->bmiHeader.biCompression = BI_RLE4;
			break;
		case 8:
			pBMI->bmiHeader.biCompression = BI_RLE8;
			break;
		default:
			ASSERT(FALSE);
		}
		// calls GetDIBits with null data pointer to get size of compressed DIB
		if(!::GetDIBits(pDC->m_hDC, hBitmap, 0, (UINT) pBMI->bmiHeader.biHeight,
			NULL, pBMI, DIB_RGB_COLORS)) {
			AfxMessageBox(_T("Unable to compress this DIB"));
			// probably a problem with the color table
			::DeleteObject(hBitmap);
			delete[] pBMI;
			pDC->SelectPalette( pOldPalette, TRUE);
			return FALSE; 
		}
		if (pBMI->bmiHeader.biSizeImage == 0) {
			AfxMessageBox(_T("Driver can't do compression"));
			::DeleteObject(hBitmap);
			delete[] pBMI;
			pDC->SelectPalette( pOldPalette, TRUE);
			return FALSE; 
		}
		else {
			m_pBMI->bmiHeader.biSizeImage = pBMI->bmiHeader.biSizeImage;
		}
	}
		else {
			pBMI->bmiHeader.biCompression = BI_RGB; // decompress
			// figure the image size from the bitmap width and height
			DWORD dwBytes = ((DWORD) pBMI->bmiHeader.biWidth * pBMI->bmiHeader.biBitCount) / 32;
			if(((DWORD) pBMI->bmiHeader.biWidth * pBMI->bmiHeader.biBitCount) % 32) {
				dwBytes++;
			}
			dwBytes *= 4;
			m_pBMI->bmiHeader.biSizeImage = dwBytes * pBMI->bmiHeader.biHeight; // no compression
			pBMI->bmiHeader.biSizeImage = m_pBMI->bmiHeader.biSizeImage;
		} 
		// second GetDIBits call to make DIB
		LPBYTE lpImage = (LPBYTE) new char[m_pBMI->bmiHeader.biSizeImage];
		VERIFY(::GetDIBits(pDC->m_hDC, hBitmap, 0, (UINT) pBMI->bmiHeader.biHeight,
			lpImage, pBMI, DIB_RGB_COLORS));
		TRACE(_T("dib successfully created - height = %d\n"), pBMI->bmiHeader.biHeight);
		::DeleteObject(hBitmap);
		Free();
		m_pBMI = pBMI;
		m_pBits = lpImage;
		CreatePalette();
		pDC->SelectPalette( pOldPalette, TRUE );
		TRACE(_T("Compress: new palette size = %d\n"), NumColors(m_pBMI->bmiHeader) ); 
		return TRUE;
	}
