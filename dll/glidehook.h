#define __WIN32__
#define __MSC__

#include "3dfx.h"
#include "glide.h"

extern void HookGlideLibs(HMODULE);

typedef void (WINAPI *grGlideInit_Type)(void);
typedef void (WINAPI *grGlideShutdown_Type)(void);
typedef void (WINAPI *grClipWindow_Type)(FxU32, FxU32, FxU32, FxU32);
typedef GrContext_t (WINAPI *grSstWinOpen_Type)(FxU32, GrScreenResolution_t, GrScreenRefresh_t, GrColorFormat_t, GrOriginLocation_t, int, int);
typedef FxI32 (WINAPI *grQueryResolutions_Type)(const GrResolution *, GrResolution *);

