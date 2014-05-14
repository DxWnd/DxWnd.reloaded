// hotpatch compiled system dlls come with Windows XP SP2 or later

// return: 
// 0 = patch failed
// 1 = already patched
// addr = address of the original function

#include <windows.h>
#include "dxwnd.h"
#include "dxwcore.hpp"

void *HotPatch(void *apiproc, const char *apiname, void *hookproc)
{
	DWORD dwPrevProtect;
	BYTE* patch_address;
	void *orig_address;

	OutTraceH("HotPatch: api=%s addr=%x hook=%x\n", apiname, apiproc, hookproc);

	if(!strcmp(apiname, "GetProcAddress")) return 0; // do not mess with this one!

	patch_address = ((BYTE *)apiproc) - 5;
	orig_address = (BYTE *)apiproc + 2;

	// entry point could be at the top of a page? so VirtualProtect first to make sure patch_address is readable
	//if(!VirtualProtect(patch_address, 7, PAGE_EXECUTE_READWRITE, &dwPrevProtect)){
	if(!VirtualProtect(patch_address, 7, PAGE_EXECUTE_WRITECOPY, &dwPrevProtect)){
		OutTraceH("HotPatch: access denied. err=%x\n", GetLastError());
		return (void *)0; // access denied
	}

	// make sure it is a hotpatchable image... check for 5 nops followed by mov edi,edi
	if(memcmp( "\x90\x90\x90\x90\x90\x8B\xFF", patch_address, 7) && memcmp( "\x90\x90\x90\x90\x90\x89\xFF", patch_address, 7)){
		VirtualProtect( patch_address, 7, dwPrevProtect, &dwPrevProtect ); // restore protection
		// check it wasn't patched already
		if((*patch_address==0xE9) && (*(WORD *)apiproc == 0xF9EB)){
			// should never go through here ...
			OutTraceH("HotPatch: patched already\n");
			return (void *)1;
		}
		else{
			OutTraceH("HotPatch: not patch aware.\n");
			return (void *)0; // not hot patch "aware"
		}
	}
	
	*patch_address = 0xE9; // jmp (4-byte relative)
	*((DWORD *)(patch_address + 1)) = (DWORD)hookproc - (DWORD)patch_address - 5; // relative address
	*((WORD *)apiproc) = 0xF9EB; // should be atomic write (jmp $-5)
	
	VirtualProtect( patch_address, 7, dwPrevProtect, &dwPrevProtect ); // restore protection
	OutTrace("HotPatch: api=%s addr=%x->%x hook=%x\n", apiname, apiproc, orig_address, hookproc);
	return orig_address;
}