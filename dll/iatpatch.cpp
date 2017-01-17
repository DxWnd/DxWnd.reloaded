#include <windows.h>
#include "dxwnd.h"
#include "dxwcore.hpp"

void *IATPatchDefault(HMODULE module, DWORD ordinal, char *dll, void *apiproc, const char *apiname, void *hookproc)
{
	PIMAGE_NT_HEADERS pnth;
	PIMAGE_IMPORT_DESCRIPTOR pidesc;
	DWORD base, rva;
	PSTR impmodule;
	PIMAGE_THUNK_DATA ptaddr;
	PIMAGE_THUNK_DATA ptname;
	PIMAGE_IMPORT_BY_NAME piname;
	DWORD oldprotect;
	void *org;
	OutTraceH("IATPatch: module=%x ordinal=%x name=%s dll=%s\n", module, ordinal, apiname, dll);

	base = (DWORD)module;
	org = 0; // by default, ret = 0 => API not found

	__try{
		pnth = PIMAGE_NT_HEADERS(PBYTE(base) + PIMAGE_DOS_HEADER(base)->e_lfanew);
		if(!pnth) {
			OutTraceH("IATPatch: ERROR no PNTH at %d\n", __LINE__);
			return 0;
		}
		rva = pnth->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;
		if(!rva) {
			OutTraceH("IATPatch: ERROR no RVA at %d\n", __LINE__);
			return 0;
		}
		pidesc = (PIMAGE_IMPORT_DESCRIPTOR)(base + rva);

		while(pidesc->FirstThunk){
			impmodule = (PSTR)(base + pidesc->Name);
			//OutTraceH("IATPatch: analyze impmodule=%s\n", impmodule);
			char *fname = impmodule;
			for(; *fname; fname++); for(; !*fname; fname++);

			if(!lstrcmpi(dll, impmodule)) {
				OutTraceH("IATPatch: dll=%s found at %x\n", dll, impmodule);

				ptaddr = (PIMAGE_THUNK_DATA)(base + (DWORD)pidesc->FirstThunk);
				ptname = (pidesc->OriginalFirstThunk) ? (PIMAGE_THUNK_DATA)(base + (DWORD)pidesc->OriginalFirstThunk) : NULL;

				while(ptaddr->u1.Function){
					// OutTraceH("IATPatch: address=%x ptname=%x\n", ptaddr->u1.AddressOfData, ptname);
					
					if (ptname){
						// examining by function name
						if(!IMAGE_SNAP_BY_ORDINAL(ptname->u1.Ordinal)){
							piname = (PIMAGE_IMPORT_BY_NAME)(base + (DWORD)ptname->u1.AddressOfData);
							OutTraceH("IATPatch: BYNAME ordinal=%x address=%x name=%s hint=%x\n", ptaddr->u1.Ordinal, ptaddr->u1.AddressOfData, (char *)piname->Name, piname->Hint);
							if(!lstrcmpi(apiname, (char *)piname->Name)) break;
						}
						else{
							// OutTraceH("IATPatch: BYORD target=%x ord=%x\n", ordinal, IMAGE_ORDINAL32(ptname->u1.Ordinal));
							if(ordinal && (IMAGE_ORDINAL32(ptname->u1.Ordinal) == ordinal)) { // skip unknow ordinal 0
							OutTraceH("IATPatch: BYORD ordinal=%x addr=%x\n", ptname->u1.Ordinal, ptaddr->u1.Function);
							//OutTraceH("IATPatch: BYORD GetProcAddress=%x\n", GetProcAddress(GetModuleHandle(dll), MAKEINTRESOURCE(IMAGE_ORDINAL32(ptname->u1.Ordinal))));	
								break;
							}
						}

					}
					else {
						// OutTraceH("IATPatch: fname=%s\n", fname);
						if(!lstrcmpi(apiname, fname)) {
							OutTraceH("IATPatch: BYSCAN ordinal=%x address=%x name=%s\n", ptaddr->u1.Ordinal, ptaddr->u1.AddressOfData, fname);
							break;
						}
						for(; *fname; fname++); for(; !*fname; fname++);
					}

					if (apiproc){
						// examining by function addr
						if(ptaddr->u1.Function == (DWORD)apiproc) break;
					}
					ptaddr ++;
					if (ptname) ptname ++;
				}

				if(ptaddr->u1.Function) {
					org = (void *)ptaddr->u1.Function;
					if(org == hookproc) return 0; // already hooked
						
					if(!VirtualProtect(&ptaddr->u1.Function, 4, PAGE_EXECUTE_READWRITE, &oldprotect)) {
						OutTraceDW("IATPatch: VirtualProtect error %d at %d\n", GetLastError(), __LINE__);
						return 0;
					}
					ptaddr->u1.Function = (DWORD)hookproc;
					if(!VirtualProtect(&ptaddr->u1.Function, 4, oldprotect, &oldprotect)) {
						OutTraceDW("IATPatch: VirtualProtect error %d at %d\n", GetLastError(), __LINE__);
						return 0;
					}
					if (!FlushInstructionCache(GetCurrentProcess(), &ptaddr->u1.Function, 4)) {
						OutTraceDW("IATPatch: FlushInstructionCache error %d at %d\n", GetLastError(), __LINE__);
						return 0;
					}
					OutTraceH("IATPatch hook=%s address=%x->%x\n", apiname, org, hookproc);

					return org;
				}
			}
			pidesc ++;
		}
		if(!pidesc->FirstThunk) {
			OutTraceH("IATPatch: PE unreferenced function %s:%s\n", dll, apiname);
			return 0;
		}

	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{       
		OutTraceH("IATPatch: EXCEPTION hook=%s:%s Hook Failed.\n", dll, apiname);
	}
	return org;
}

void *IATPatchSequential(HMODULE module, DWORD ordinal, char *dll, void *apiproc, const char *apiname, void *hookproc)
{
	PIMAGE_NT_HEADERS pnth;
	PIMAGE_IMPORT_DESCRIPTOR pidesc;
	DWORD base, rva;
	PSTR impmodule;
	PIMAGE_THUNK_DATA ptaddr;
	PIMAGE_THUNK_DATA ptname;
	PIMAGE_IMPORT_BY_NAME piname;
	DWORD oldprotect;
	void *org;
	OutTraceH("IATPatch: module=%x ordinal=%x name=%s dll=%s\n", module, ordinal, apiname, dll);

	base = (DWORD)module;
	org = 0; // by default, ret = 0 => API not found

	__try{
		pnth = PIMAGE_NT_HEADERS(PBYTE(base) + PIMAGE_DOS_HEADER(base)->e_lfanew);
		if(!pnth) {
			OutTraceH("IATPatch: ERROR no PNTH at %d\n", __LINE__);
			return 0;
		}
		rva = pnth->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;
		if(!rva) {
			OutTraceH("IATPatch: ERROR no RVA at %d\n", __LINE__);
			return 0;
		}
		pidesc = (PIMAGE_IMPORT_DESCRIPTOR)(base + rva);

		// move fname pointer to first API name
		char *fname;
		while(pidesc->FirstThunk){
			impmodule = (PSTR)(base + pidesc->Name);
			fname  = impmodule;
			for(; *fname; fname++); for(; !*fname; fname++);
			pidesc ++;
		}
		pidesc = (PIMAGE_IMPORT_DESCRIPTOR)(base + rva);
		//OutTraceB("IATPatch: first call=%s\n", fname);

		while(pidesc->FirstThunk){
			impmodule = (PSTR)(base + pidesc->Name);

			if(!lstrcmpi(dll, impmodule)) {
				OutTraceH("IATPatch: dll=%s found at %x\n", dll, impmodule);
				//OutTraceH("IATPatch: first call=%s\n", fname);

				ptaddr = (PIMAGE_THUNK_DATA)(base + (DWORD)pidesc->FirstThunk);
				ptname = (pidesc->OriginalFirstThunk) ? (PIMAGE_THUNK_DATA)(base + (DWORD)pidesc->OriginalFirstThunk) : NULL;

				while(ptaddr->u1.Function){
					// OutTraceH("IATPatch: address=%x ptname=%x\n", ptaddr->u1.AddressOfData, ptname);
					
					if (ptname){
						// examining by function name
						if(!IMAGE_SNAP_BY_ORDINAL(ptname->u1.Ordinal)){
							piname = (PIMAGE_IMPORT_BY_NAME)(base + (DWORD)ptname->u1.AddressOfData);
							OutTraceH("IATPatch: BYNAME ordinal=%x address=%x name=%s hint=%x\n", ptaddr->u1.Ordinal, ptaddr->u1.AddressOfData, (char *)piname->Name, piname->Hint);
							if(!lstrcmpi(apiname, (char *)piname->Name)) break;
						}
						else{
							// OutTraceH("IATPatch: BYORD target=%x ord=%x\n", ordinal, IMAGE_ORDINAL32(ptname->u1.Ordinal));
							if(ordinal && (IMAGE_ORDINAL32(ptname->u1.Ordinal) == ordinal)) { // skip unknow ordinal 0
							OutTraceH("IATPatch: BYORD ordinal=%x addr=%x\n", ptname->u1.Ordinal, ptaddr->u1.Function);
							//OutTraceH("IATPatch: BYORD GetProcAddress=%x\n", GetProcAddress(GetModuleHandle(dll), MAKEINTRESOURCE(IMAGE_ORDINAL32(ptname->u1.Ordinal))));	
								break;
							}
						}

					}
					else {
						// OutTraceH("IATPatch: fname=%s\n", fname);
						if(!lstrcmpi(apiname, fname)) {
							OutTraceH("IATPatch: BYSCAN ordinal=%x address=%x name=%s\n", ptaddr->u1.Ordinal, ptaddr->u1.AddressOfData, fname);
							break;
						}
						for(; *fname; fname++); for(; !*fname; fname++);
					}

					if (apiproc){
						// examining by function addr
						if(ptaddr->u1.Function == (DWORD)apiproc) break;
					}
					ptaddr ++;
					if (ptname) ptname ++;
				}

				if(ptaddr->u1.Function) {
					OutTraceDW("IATPatch: hooking %s\n", apiname);
					org = (void *)ptaddr->u1.Function;
					if(org == hookproc) return 0; // already hooked
						
					if(!VirtualProtect(&ptaddr->u1.Function, 4, PAGE_EXECUTE_READWRITE, &oldprotect)) {
						OutTraceDW("IATPatch: VirtualProtect error %d at %d\n", GetLastError(), __LINE__);
						return 0;
					}
					ptaddr->u1.Function = (DWORD)hookproc;
					if(!VirtualProtect(&ptaddr->u1.Function, 4, oldprotect, &oldprotect)) {
						OutTraceDW("IATPatch: VirtualProtect error %d at %d\n", GetLastError(), __LINE__);
						return 0;
					}
					if (!FlushInstructionCache(GetCurrentProcess(), &ptaddr->u1.Function, 4)) {
						OutTraceDW("IATPatch: FlushInstructionCache error %d at %d\n", GetLastError(), __LINE__);
						return 0;
					}
					OutTraceH("IATPatch hook=%s address=%x->%x\n", apiname, org, hookproc);

					return org;
				}
			}
			else{
				//OutTraceDW("IATPatch: skip dll=%s first call=%s\n", impmodule, fname);
				// skip dll fnames ...
				ptaddr = (PIMAGE_THUNK_DATA)(base + (DWORD)pidesc->FirstThunk);
				//ptname = (pidesc->OriginalFirstThunk) ? (PIMAGE_THUNK_DATA)(base + (DWORD)pidesc->OriginalFirstThunk) : NULL;
				while(ptaddr->u1.Function){
					ptaddr ++;
					for(; *fname; fname++); for(; !*fname; fname++);
				}
			}
			pidesc ++;
		}
		if(!pidesc->FirstThunk) {
			OutTraceH("IATPatch: PE unreferenced function %s:%s\n", dll, apiname);
			return 0;
		}

	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{       
		OutTraceH("IATPatch: EXCEPTION hook=%s:%s Hook Failed.\n", dll, apiname);
	}
	return org;
}

// Note: when pidesc->OriginalFirstThunk is NULL, the pidesc->FirstThunk points to an array of 
// RVA for imported function names in the PE file, but when the loader loads the program these
// values gets replaced by the function addresses. The only way to retrieve the function names 
// after that event is to point to the dll name and get the list of the following strings sequentially
// taking in account that the function names have variable length and are aligned to a DWORD
// boundary, so that a practical way to retrieve the next name is this piece of code:
// for(; *fname; fname++); for(; !*fname; fname++);

// Note (2): the above condition is not always true. The original version of "Al Unser Jr Arcade Racing" 
// executable ALUNSER.EXE seems to have all dll names first, then followed by the names of all the dll
// entries, so that it is still possible to retrieve the function name, but a different schema must be used.


void DumpImportTableDefault(HMODULE module)
{
	PIMAGE_NT_HEADERS pnth;
	PIMAGE_IMPORT_DESCRIPTOR pidesc;
	DWORD base, rva;
	PSTR impmodule;
	PIMAGE_THUNK_DATA ptaddr;
	PIMAGE_THUNK_DATA ptname;
	PIMAGE_IMPORT_BY_NAME piname;

	base=(DWORD)module;
	// OutTrace("DumpImportTable: base=%x\n", base);
	__try{
		pnth = PIMAGE_NT_HEADERS(PBYTE(base) + PIMAGE_DOS_HEADER(base)->e_lfanew);
		if(!pnth) {
			OutTrace("DumpImportTable: ERROR no pnth at %d\n", __LINE__);
			return;
		}
		rva = pnth->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;
		if(!rva) {
			OutTrace("DumpImportTable: ERROR no rva at %d\n", __LINE__);
			return;
		}
		pidesc = (PIMAGE_IMPORT_DESCRIPTOR)(base + rva);

		while(pidesc->FirstThunk){
			char *fname;
			impmodule = (PSTR)(base + pidesc->Name);
			OutTrace("DumpImportTable: ENTRY timestamp=%x module=%s forwarderchain=%x\n", 
				pidesc->TimeDateStamp, impmodule, pidesc->ForwarderChain);
			if(pidesc->OriginalFirstThunk) {
				ptname = (PIMAGE_THUNK_DATA)(base + (DWORD)pidesc->OriginalFirstThunk);
			}
			else{
				ptname = 0;
				fname = impmodule;
				for(; *fname; fname++); for(; !*fname; fname++);
				OutTrace("DumpImportTable: no PE OFTs - stripped module=%s\n", impmodule);
			}
			ptaddr = (PIMAGE_THUNK_DATA)(base + (DWORD)pidesc->FirstThunk);
			while(ptaddr->u1.Function){
				OutTrace("addr=%x", ptaddr->u1.Function); 
				ptaddr ++;
				if(ptname){
					if(!IMAGE_SNAP_BY_ORDINAL(ptname->u1.Ordinal)){
						piname = (PIMAGE_IMPORT_BY_NAME)(base + (DWORD)ptname->u1.AddressOfData);
						OutTrace(" hint=%x name=%s", piname->Hint, piname->Name);
						ptname ++;
					}
				}
				else {
					OutTrace(" name=%s", fname);
					for(; *fname; fname++); for(; !*fname; fname++);
				}
				OutTrace("\n");
			}
			OutTrace("*** EOT ***\n", ptaddr->u1.Function); 
			pidesc ++;
		}
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{       
		OutTraceDW("DumpImportTable: EXCEPTION\n");
	}
	return;
}

void DumpImportTableSequential(HMODULE module)
{
	PIMAGE_NT_HEADERS pnth;
	PIMAGE_IMPORT_DESCRIPTOR pidesc;
	DWORD base, rva;
	PSTR impmodule;
	PIMAGE_THUNK_DATA ptaddr;
	PIMAGE_THUNK_DATA ptname;
	PIMAGE_IMPORT_BY_NAME piname;

	base=(DWORD)module;
	// OutTrace("DumpImportTable: base=%x\n", base);
	__try{
		pnth = PIMAGE_NT_HEADERS(PBYTE(base) + PIMAGE_DOS_HEADER(base)->e_lfanew);
		if(!pnth) {
			OutTrace("DumpImportTable: ERROR no pnth at %d\n", __LINE__);
			return;
		}
		rva = pnth->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;
		if(!rva) {
			OutTrace("DumpImportTable: ERROR no rva at %d\n", __LINE__);
			return;
		}
		pidesc = (PIMAGE_IMPORT_DESCRIPTOR)(base + rva);
		OutTrace("DumpImportTable: pidesc=%x\n", pidesc);

		char *fname;
		PIMAGE_IMPORT_DESCRIPTOR savepidesc = pidesc;
		while(pidesc->FirstThunk){
			impmodule = (PSTR)(base + pidesc->Name);
			fname  = impmodule;
			for(; *fname; fname++); for(; !*fname; fname++);
			pidesc ++;
		}
		pidesc = savepidesc;
		OutTrace("DumpImportTable: no PE OFTs - first call=%s\n", fname);

		while(pidesc->FirstThunk){
			//char *fname;
			impmodule = (PSTR)(base + pidesc->Name);
			OutTrace("DumpImportTable: ENTRY timestamp=%x module=%s forwarderchain=%x\n", 
				pidesc->TimeDateStamp, impmodule, pidesc->ForwarderChain);
			if(pidesc->OriginalFirstThunk) {
				ptname = (PIMAGE_THUNK_DATA)(base + (DWORD)pidesc->OriginalFirstThunk);
			}
			else{
				ptname = 0;
				//fname = impmodule;
				//for(; *fname; fname++); for(; !*fname; fname++);
				OutTrace("DumpImportTable: no PE OFTs - stripped module=%s\n", impmodule);
			}
			ptaddr = (PIMAGE_THUNK_DATA)(base + (DWORD)pidesc->FirstThunk);
			while(ptaddr->u1.Function){
				OutTrace("addr=%x", ptaddr->u1.Function); 
				ptaddr ++;
				if(ptname){
					if(!IMAGE_SNAP_BY_ORDINAL(ptname->u1.Ordinal)){
						piname = (PIMAGE_IMPORT_BY_NAME)(base + (DWORD)ptname->u1.AddressOfData);
						OutTrace(" hint=%x name=%s", piname->Hint, piname->Name);
						ptname ++;
					}
				}
				else {
					OutTrace(" name=%s", fname);
					for(; *fname; fname++); for(; !*fname; fname++);
				}
				OutTrace("\n");
			}
			OutTrace("*** EOT ***\n", ptaddr->u1.Function); 
			pidesc ++;
		}
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{       
		OutTraceDW("DumpImportTable: EXCEPTION\n");
	}
	return;
}

static char* stristr(char* str1, const char* str2)
{
    char* p1 = str1 ;
    const char* p2 = str2 ;
    char* r = *p2 == 0 ? str1 : 0 ;

    while(*p1 != 0 && *p2 != 0) {
        if(tolower(*p1) == tolower(*p2)){
            if(r == 0) r = p1;
            p2++ ;
        }
        else {
            p2 = str2;
            if(tolower(*p1) == tolower(*p2)){
                r = p1;
                p2++ ;
            }
            else{
                r = 0;
            }
        }
        p1++ ;
    }
    return *p2 == 0 ? r : 0 ;
}

BOOL IsIATSequential(HMODULE module)
{
	PIMAGE_NT_HEADERS pnth;
	PIMAGE_IMPORT_DESCRIPTOR pidesc;
	DWORD base, rva;

	base=(DWORD)module;
	__try{
		pnth = PIMAGE_NT_HEADERS(PBYTE(base) + PIMAGE_DOS_HEADER(base)->e_lfanew);
		if(!pnth) {
			OutTrace("IsIATSequential: ERROR no pnth at %d\n", __LINE__);
			return FALSE;
		}
		rva = pnth->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;
		if(!rva) {
			OutTrace("IsIATSequential: ERROR no rva at %d\n", __LINE__);
			return FALSE;
		}
		pidesc = (PIMAGE_IMPORT_DESCRIPTOR)(base + rva);
		//OutTrace("IsIATSequential: pidesc=%x\n", pidesc);

		char *fname = (PSTR)(base + pidesc->Name);
		// first string should be a DLL in both cases
		if(!stristr(fname, ".DLL")) {
			OutTrace("IsIATSequential: ERROR no .DLL at %d\n", __LINE__);
			return FALSE;
		}
		// skip first string
		for(; *fname; fname++); for(; !*fname; fname++);
		// if second string is another DLL it is sequential, otherwise not.
		//OutTraceB("IsIATSequential: second entry=%s\n", fname);
		return (BOOL)stristr(fname, ".DLL");
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{       
		OutTraceDW("IsIATSequential: EXCEPTION\n");
	}
	return FALSE;
}
