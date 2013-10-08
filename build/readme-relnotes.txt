v2.2.00/01
major code rewriting - introduced dxwCore class
preliminary FPS handling: Limit, Skip & Count with configurable delay
Hide Multi Monitor configuration flag - used for "Dream Acquarium" on multimonitor PC.

v2.2.02
preliminary time stretching: so far applies to GetTickCount() only, and is controlled by Alt-F5 / F6 keys.
Fixed bug in GDI BitBlt call: stretching must be made on screen DC only, or it's made twice.

v2.02.03/04:
code cleanup - see syslib calls hooking
new configuration flags: Hook GDI and Hook OpenGL
OpenGL custom library field
API hooking fix with module specification
Time stretching by keyboard control (Alt F5/F6) and/or Time Slider dialog

v2.02.05:
hooked winmm timeGetTime() API: makes time stretching work for Age of Empires series
changed time stretching grain: now it's not the coarse grained 2x, 4x,... series in 9 possible values but the fine grained series 1.5x, 2x, 3x,.... in 17 possible values
added status and time stretching view panels to tray icon menu

v2.02.06:
preliminary FPS counter overlapped on game screen, Alt-F7 to toggle display on/off.
fixed buf on time stretch logging (and possible game crash).
revised GetDC handling with 8BPP paletized surfaces: avoided need to emulate reverse-blitting and got an impressive speed improvement for games such as Age of Empires I & II and Hyperblade.

v2.02.07:
many fixes on the FPS and time control features.

v2.02.08:
fixed some errors in the main directdraw palette descriptor. That gives better compatibility and less complicated source code.
added Fake Version feature: now Dungeon Keeper II (original version, not GOG hack) can detect a fake Win2000 / WinXP environment on Win7 and newer. Tested and working on Win7. Many thanks to Maxim for pushing me hard to win my lazyness and implement this new feature.

v2.02.09:
Fixed some x,y window coordinates bugs
Fixed some proxy log messages (missing \n line terminator)
Fixed Trace "DirectX" flag.
improved GetDC handling in 8BPP palette mode: AddPalette called on demand, and on any surface (including backbuffers): makes Emergency work with no "Handle DC" flag set. Beware: this may affect the "Map GDI HDC on Primary DC" flag causing surface locks.
Fixed limit FPS timing issues: now the max possible FPS is 1000/delay.
Fixed EndPaint bug causing HDC lock in "Map GDI HDC to Primary DC" mode.

v2.02.10:
Added "Full RECT Blit" mode: may be useful to handle problematic situations (e.g. "Urban Assault" intro movies)
Fixed ClientToScreen and ScreenToClient hookers to properly handle scaled windows. This makes "Postal" working.
Fixed global palette reference count (??) in DirectDraw::Release hook
Fixed Window messages handling for SWP_NOMOVE, SWP_NOSIZE modes.

v2.02.11:
Added debug messages for GetSystemMetrics() modes, MapWindowPoints() points, DirectDrawEnumerate/Ex() devices.
ompiled with #define _WIN32_WINNT 0x0600 -> handles Vista modes
Added NOPALETTEUPDATE ("Palette update don't Blit" flag) to eliminate flickering when ddraw and GDI methods conflict
Hooked all LoadLibraryA/W and LoadLibraryExA/W calls
Hooked 
extDirectDrawEnumerate/Ex ddraw calls to handle Hide multi-monitor option.
Detected directshow activation through CoCreateInstance and hooked quartz.dll segment: now Urban Assault movies don't require "Full RECT Blit" option to be set.
Updated DDSurface::Release hook 

v2.02.12
GUI: Tabbed setup panel. More space for more future options.
DLL: (optional) splash screen 
probably, some regression bugs....
 
v2.02.13
Added decoding of WINDOWPOS.flags field in trace log
revised whole hooking procedures to use HMODULE handle instead of module name
Added WM_GETMINMAXINFO and WM_NCCALCSIZE handling in WinProcess hook
Attempt to handle double buffering through surface attach to backbuffer (???)
Fixed CHILD window positioning for BIG windows in CreateWindowExA hook
Added GlobalMemoryStatus hook to fix huge values when value exceeds DWORD range. Fixes Nocturne intro warning message.

V2.02.14
Started dll injection to handle startup code
Fixed directx CreateSurface hook to fix "Wargames" error in emulated mode
Fixed directx SetClipper hook to properly handle backbuffer clipping and fix "Wargames" clipping problems

v2.02.15
Fixed clipping handling of primary/backbuffer surfaces 
Added option to set AERO compatibility for Vista/Win7/Win8 platforms. Need to call a undocumented ddraw API. Thanks to Vovchik that discovered it. Use at your own risk!

v2.02.16
DLL injection finally working! Quake 2 is supported.
Added Wireframe option for OpenGL games
Improved fix for clipper handling of primary/backbuffer surfaces 

v2.02.18
Improved LoadLibrary family hook and other improvements to let DxWnd better retrieve OpenGL libs:  now Homeworld 2 is working
Fixed a bug in backbuffer ZBUFFER attach emulation. Now Dungeon Keeper 2 crashes no more.
Moved "Remap client rect" option from directx tab to main program tab
Added the "Force Hook" in the OpenGL tab. Useless for now.
Preliminary work for d3d10/d3d11 wrapping. Unfinished and not working so far.
Added LoadLibraryEx flags explaination in log
Added support for mouse X,Y coordinates display in status window
OpenGL hooking: fixed bug to prevent hooking same call twice
OpenGL hooking: added hook for wglMakeCurrent call to keep track of rendered window
OpenGL hooking: fixed glViewport and extglScissor hook coordinates handling
D3D hooking: Added Wireframe option for D3D games
Added hooking of CLSID_DxDiagProvider through CoCreateInstance
Fixed (further simplified) clipping handling of primary/backbuffer surfaces 
ChangeDisplaySettings hook: fixed x,y coordinate inversion in log message

v2.02.19
Saves GUI coordinates
Fixed "Remap client rect" option for Diablo's windows and Premier Manager 98 mouse movements
Added "Highlight blit to primary" option to draw a yellow bounding box around blits to primary surface
Fixed some exception conditions when closing the programs
Fixed CreateSurface handling to allow Premier Manager 98 start in emulated mode
Fixed ONEPIXELFIX handling
Fixed BIG WIN handling for Diablo's windows
Fixed FillRect hook to prevent filling outside virtual desktop
Disabled hooking of system libraries
Fixed a nasty bug that caused your desktop to freeze until shutdown !!!
Fixed GetWindowRect handling for windows not created by the task: Diablo queries the explorer window size! Now the retrieved RECT can't be larger than the virtual desktop

v2.02.20
Fixed an error in D3D10/11 preliminary code: AoE III working again
Added "Hook all DLLs" option to simplify congiguration: see new Diablo setup
Added screen coordinates to log
Some code rearrangement

v2.02.21
it's a w.i.p. release, wait 'till finished....

v2.02.22
Major code rearrangement in API hooking. Expect some troubles...
Added desktop workarea and centered coordinate settings
Fixed imelib hooking
Added HOOKENABLED flag
Fixed ChangeDisplaySettings 
GUI: added ListView icons
GUI: added pause command
GUI: updated commands layout

v2.02.23/24
Fixed "disable setting gamma ramp" flag to intercept both GDI and D3D calls
Fixed client workarea setting to occupy the whole client area even when preserving aspect ratio (it draws black rectangles to the left/right or top/bottom side)
Added DisableThreadLibraryCalls optimization
Added B&W screen simulation (for primary emulation only)
Improved the primary emulation capacity to handle 3D games
Added the "preserve surface capabilities" flag
Fixed an hooking bug for user32.dll

v2.02.25
CORE:
Added 'Single CPU Process Affinity' flag to let old games run on a single CPU core.
Fixed bug causing possible crash using 'preserve surface caps' flag
Switched lpDDSBack reference when backbuffer is upgraded through QueryInterface
Eliminated lpDDSBack clearing when refcount is zero - why is this working better? Needs further investigation
Added DDSCAPS_3DDEVICE capability when in EMULATED mode
GUI:
Added Kill process by name functionality (right click menu on program's list)
Fixed GUI initial position: now checks for desktop size to fit GUI within visible borders
Added single cpu process affinity checkbox in compatibility tab

v2.02.26
CORE:
Fixed Black&White mode for 16BPP color depth
Revised hooking code, now more compact and clear....
Fixed DxWnd splash screen, for those who love it
Increased child win table - now 688 hunter killer works perfectly 
GUI:
Added /debug flag to enable debugging options
Revised hooking code, now more compact and clear....
Restored Hook child win option

v2.02.27
CORE:
fixed GetDC/ReleaseDC ddraw implementation to refresh GDI operation on primary surface. Warlords 3 text is now visible.
preliminary implementation of MapWindowPoints - to be tested
GUI:
Fixed log flags wrong initialization

v2.02.28
CORE:
eliminated experimental ICSendMessage and ICOpen hooks, preventing some games (Alien Nations) to work
added mciSendCommand hook to fix video playback
fixed MoveWindow bug - still, it's unknown the reason why some programs (Emergency) tries to MoveWindow upon the hWnd=0 desktop!
fixed MapWindowPoints hook (Alien Nations)
fixed desktop detections in some places, now correctly using dxw.IsDesktop() method
Now "Alien Nations" and "Emergency" working (almost) perfectly.
fixed d3d8/9 hook to Get/SetGammaRamp. This should make a great number of recent games playable again.

v2.02.29
CORE:
hooked DirectDrawSurface::Lock method to center primary surface memory updates centered into the window (in no emulated mode)

v2.02.30
CORE:
Some ddraw::GetDC log message fixes
added preliminary registry emulation - tested ok with "duckman"
added separate flag for preliminary registry operation trace
added CDROM drive type emulation - tested ok with "Fighting Forces" RIP
fixed FixCursorPos routine: cursor x,y compensation must always use pseudo-fullscreen window parameters - fixes Imperialism II mouse problems
fixed Desktop Workarea position mode for ddraw games
added CoCreateInstanceEx hook - "Final Fighter" seems to be using it, but who knows why...?
added "Don't move D3D Rendering Window" window option to make "Fable Lost Chapters" working  
GUI:
updated default values on new entry creation to map most-likely-to-work parameters
