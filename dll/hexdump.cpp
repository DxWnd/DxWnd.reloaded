#define  _CRT_SECURE_NO_WARNINGS
#include "stdio.h"
#include "string.h"

#define MAX_HEXDUMP 512

char *hexdump(unsigned char *Buffer, int len)
{
	static char sHexBuffer[3*MAX_HEXDUMP+12];
	char *s;
	int iMaxI;
	if(!Buffer) return "(NULL)";
	s=sHexBuffer;
	iMaxI = len;
	if(iMaxI > MAX_HEXDUMP) iMaxI = MAX_HEXDUMP;
	for (int i=0; i<iMaxI; i++){
		sprintf(s, "%02.2X,", Buffer[i]); 
		s += 3;
	}
	*(--s)=0; // eliminate last comma
	if(len > iMaxI) strcpy(s, ",...");
	return sHexBuffer;
}