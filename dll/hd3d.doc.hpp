#if 0
from http://msdn.microsoft.com/en-us/library/windows/desktop/bb172588%28v=vs.85%29.aspx

D3DPRESENT_PARAMETERS structure

Describes the presentation parameters.
Syntax
C++

typedef struct D3DPRESENT_PARAMETERS {
  UINT                BackBufferWidth;
  UINT                BackBufferHeight;
  D3DFORMAT           BackBufferFormat;
  UINT                BackBufferCount;
  D3DMULTISAMPLE_TYPE MultiSampleType;
  DWORD               MultiSampleQuality;
  D3DSWAPEFFECT       SwapEffect;
  HWND                hDeviceWindow;
  BOOL                Windowed;
  BOOL                EnableAutoDepthStencil;
  D3DFORMAT           AutoDepthStencilFormat;
  DWORD               Flags;
  UINT                FullScreen_RefreshRateInHz;
  UINT                PresentationInterval;
} D3DPRESENT_PARAMETERS, *LPD3DPRESENT_PARAMETERS;

BackBufferFormat
    Type: D3DFORMAT
    The back buffer format. For more information about formats, see D3DFORMAT. 
	This value must be one of the render-target formats as validated by CheckDeviceType. 
	You can use GetDisplayMode to obtain the current format.
    In fact, D3DFMT_UNKNOWN can be specified for the BackBufferFormat while in windowed mode. 
	This tells the runtime to use the current display-mode format and eliminates the need to call GetDisplayMode.
    For windowed applications, the back buffer format no longer needs to match the display-mode format because 
	color conversion can now be done by the hardware (if the hardware supports color conversion). 
	The set of possible back buffer formats is constrained, but the runtime will allow any valid back buffer format 
	to be presented to any desktop format. (There is the additional requirement that the device be operable in the
	desktop; devices typically do not operate in 8 bits per pixel modes.)

    Full-screen applications cannot do color conversion.
#endif