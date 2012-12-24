#include <windows.h>

#define CLIP_TOLERANCE 4

class dxwCore
{
// Construction/destruction
public:
    dxwCore();
    virtual ~dxwCore();

// Operations
public:
	void SethWnd(HWND);
public:
	POINT FixCursorPos(POINT);

// Implementation
protected:
	HWND hWnd;
	int dwScreenWidth;
	int dwScreenHeight;
	BOOL IsFullScreen;
};
