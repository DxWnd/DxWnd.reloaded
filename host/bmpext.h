#pragma once

#include "afxtempl.h"

//MakeRgn helper data definition
typedef CTypedPtrList< CPtrList, LPRECT > RECTLIST;

//////////////////////////////////////////////////////////////////////////
// DD Bitmap draw extensions class
class AFX_EXT_CLASS CDDBDrawEx
{
public:
	CDDBDrawEx(CDC* pDC, CBitmap* pbmSrc, CBitmap* pbmBack = NULL);
	virtual ~CDDBDrawEx();

	void Fill(CRect& rDest);
	void Draw(CRect& rDest, CPoint& pntSrc);
	void DrawTransparent(CRect& rDest, CPoint& pntSrc, COLORREF crMask );
	HRGN MakeRgn(COLORREF cTransparentColor = 0, COLORREF cTolerance = 0x101010);

private:
	CDC* m_pDC;
	CBitmap* m_pbmSrc;
	CBitmap* m_pbmBack;

	LPRGNDATA RectList2RGNDATA( RECTLIST& rl, RECT& rBound, int& iByteCount );
};

///////////////////////////////////////////////////////////////////////////
// DIB support classes
class CDibException
{
public:
	//exception types: not enough memory, operation not supported, invalid pixel position
	enum {E_NOMEM, E_NOTSUPP, E_INVPOS};
	CDibException( int iReason ) : m_iReason(iReason) {}

	static void Throw( int iReason ){ throw new CDibException(iReason); }

public:
	int m_iReason;
};

class AFX_EXT_CLASS CDib
{
// Constructors
public:
	CDib();
	virtual ~CDib();

// Attributes
protected:
	LPBYTE m_pBits;
	LPBITMAPINFO m_pBMI;

public:	
	CPalette* m_pPalette;

public:
	DWORD Width() const;
	DWORD Height() const;
	WORD  NumColors(  BITMAPINFOHEADER& bmiHeader ) const;
	BOOL  IsValid() const;

	void SetPixel( int iX, int iY, RGBQUAD& rgbPixel );
	RGBQUAD GetPixel(int iX, int iY);

private:
	void AssertPosition( int iX, int iY );
	DWORD PaletteSize() const;

// Operations
public:
	BOOL  Draw(CDC*, CRect& rectDC, CRect& rectDIB) const;
	
	DWORD Save(CFile& file) const;
	DWORD Read(CFile& file, BOOL bFromResource = FALSE );
	DWORD ReadFromResource(UINT nResID);

	void Invalidate();
	
// Implementation
protected:
	BOOL  CreatePalette();

	void Free();

public:
	HBITMAP CreateDDBitmap(CDC* pDC);
	HBITMAP	CreateDDBitmap( HDC hDC );
	BOOL Compress(CDC* pDC, BOOL bCompress );

protected:
	CDib& operator = (CDib& dib);
};

