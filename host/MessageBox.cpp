#include "stdafx.h"
#include <stdio.h>
#include <stdarg.h>

int MessageBoxLang(UINT iText, UINT iCaption, UINT uType)
{
	WCHAR sBuffer[1024+1];
	WCHAR sCaption[48+1];
	LoadStringW(AfxGetResourceHandle(), iText, (LPWSTR)&sBuffer, sizeof(sBuffer));
	LoadStringW(AfxGetResourceHandle(), iCaption, (LPWSTR)&sCaption, sizeof(sCaption));

	return MessageBoxExW(NULL, sBuffer, sCaption, uType, NULL);
}

int MessageBoxLangArg(UINT iText, UINT iCaption, UINT uType, ...)
{
	va_list al;
	WCHAR sBuffer[1024+1];
	WCHAR sFormat[1024+1];
	WCHAR sCaption[48+1];

	LoadStringW(AfxGetResourceHandle(), iText, (LPWSTR)&sFormat, sizeof(sFormat));
	LoadStringW(AfxGetResourceHandle(), iCaption, (LPWSTR)&sCaption, sizeof(sCaption));

	va_start(al, uType);
	vswprintf(sBuffer, 1024, sFormat, al);
	va_end(al);

	return MessageBoxExW(NULL, sBuffer, sCaption, uType, NULL);
}

