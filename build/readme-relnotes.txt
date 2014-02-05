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

v2.02.31
CORE:
disabled annoying compilation security warnings
improved advapi32.dll hooking and logging
handling of complex primary surfaces with backbuffer surface inherited from one directdraw session to the following and with different interfaces
fixed SetCooperativeLevel handling in case of WINDOWED mode against desktop hwnd==NULL
fixed GetGDISurface in EMULATED mode 
hooked gdi32 API set for Imperialism I & II and 688 Hunter Killer:GetClipBox, Polyline, PolyBezierTo, PolylineTo, PolyDraw, MoveToEx, ArcTo, LineTo, StretchDIBits, SetDIBitsToDevice, SetPixel, Ellipse, Polygon, Arc, CreateEllipticRgn, CreateEllipticRgnIndirect, CreateRectRgn, CreateRectRgnIndirect, CreatePolygonRgn, DrawTextA, DrawTextExA.
fixed gdi32.dll hooking for TextOutA, TabbedTextOutA, Rectangle, BitBlt, PatBlt, StretchBlt, CreateFont, CreateFontIndirect
improved ddraw proxy logging
added / fixed user32.dll API FrameRect, TabbedTextOutA, CloseWindow, DestroyWindow, SendMessageW
opengl: fixed glCreateContext, wglMakeCurrent to make Descent III playable in opengl video mode.
fixed DeferWindowPos hook to make Imperialism II working
fixed SM_CXVIRTUALSCREEN, SM_CYVIRTUALSCREEN properties
fixed window class logging causing program crash
GUI:
added "GDI/Scale font parameters" & "DirectX/Disable HAL support" options

v2.02.32
CORE: 
added new GDI handling mode, "Emulated devce context". Currently tested successfully on "G-Nome" only.
fixed "Prevent maximize" windows handling avoiding to disable topmost style for child windows
fixed handling of DDSCAPS_3DDEVICE surfaces in emulated mode: "Nightmare Ned" is working ok, "The Sims" works better (still crashing..)
fixed banner handling in case of device context remmapped coordinates
fixed memory leakage in SetDIBitsToDevice hooker
GUI
added radiobutton group to set the preferred GDI emulation mode

v2.02.33
CORE:
experimental attempt to emulate the ZBUFFER attach to the BACKBUFFER surface
improved log to detail all ddraw object properties
experimental emulation of RGB to YUV conversion to investigate "duckman" problems on Win7
improved grayscale algorythm
fixed missing DDPF_ALPHAPIXELS property causing several blit incompabilities
fixed surface handling for "Submarine Titans"
fixed mousewheel handling
fixed hooking bug causing several problems (i.e. Age of Empires III)
Added FULLSCREENONLY option: fixes "Submarine Titans" intro movies
Added "Bypass font unsupported api" compatibility flag: makes "Stratego" working
fixed several d3d8/9 hooking problems
GUI
added controls for new core features
defaulted ddraw emulation mode to surface emulation

v2.02.34
CORE:
Much better surface description in log
Completely revised CreateSurface hook: emulated and direct code are merged as much as possible, and reference counter are kept accurate. Now most games can work both in emulated and direct mode.
Fixed surface capabilities for "Vangers", "The Sims" (now working in emulated mode again)
Updated "Fixed aspect ratio" option: now uses the x,y size declared in the configuration instead of the fixed 800 x 600 standard one.
Added virtual fullscreen "Desktop" mode.
Completely revised the Peek/GetMessage handling: now it uses the standard SetWindowHook API instead of the address redirection, making the handling more reliable and accurate: now "Age of Empires III" works at last!
GUI:
Added virtual fullscreen "Desktop" mode.
Added "Fix buffered IO after Win98" flag: this is meant to fix an incompatibility in the ReadFile kernel32 API that must read from block boundaries, where a block was smaller up to Win98 and bigger after it.
As a result, games like "Dylan Dog Horror Luna Park" show IO errors while reading data from the CD. Unfortunately, this is not the only problem of this game, so Dylan Dog fans will have to wait further.
Maybe some other programs suffer of this problem, and I will strongly appreciate if anyone finds some.

v2.02.35
fixed BACKBUFFER surface attributes in direct (not emulated) mode: Rayman 2 playable again
added FILTERMESSAGES flag ("filter offending messages") to eliminate some problems to games not developed to work windowized (Rayman 2, Mirror's Edge ...)
fixed bug crashing the program when "keep aspect ratio" is selected with window size set to 0

v2.02.36
hooked GetAttachedSurface to non-PRIMARY surface to track ZBUFFER attach to BACKBUFFER. Useless, so far...
revised capability handling in CreateSurface 
hook trace is now activated from a separate log flag to reduce debug log size
added "Peek all messages in queue" to avoid queue saturation and automatic task kill in Win7 (thank to P K help)
fixed message handling in the case messages are passed to a routine handle

v2.02.37
code reorganization
GUI:
removed useless flags and moved debug options in a hidden tab

v2.02.38-40
Fixed coordinate calculation for blit operations when keeping aspect ratio.
Fixed window coordinate placement in desktop / client area mode for programs with a visible menu bar.
Changed "Lock win coordinates" flag: now it allows window movement / resizing when driven by mouse input.
Fixed a bug in GDI "Map DC to primary surface"mode.
Added palette display window to GUI

v2.02.41
Added "suppress child process creation"flag. This flag is introduced to manage games such as "Heart of Iron 2" that start intro movie by means of a separate process. HoI2 is starting the binkplay.exe program in the game's avi subfolder. This way you don't get a windowed movie, but completely suppress it.

v2.02.42
added hook to kernel32.dll QueryPerformanceCounter API to enable time stretching to Rayman 2
handled GetAttachedSurface emulation for FLIP capability on primary surface - that makes the intro movies of Empire Earth visible.
Some code cleaning
Cleared invisible debug flags ...

v2.02.43
Several changes for Empire Earth:
fix: restore 16BPP color mode after directdraw session termination
fix: recovered window destruction
fix: proper handling of MIPMAP and LOCALVIDMEM surfaces 
fix: color depth handling - now when the program terminates the desktop is brought to the original color depth

v2.02.44
Improved debug logging for all palette operations
fixed a bugged log causing the crash of Empire Earth in debug mode
added show time stretching flag - preliminary version. Some code cleaning as well.
change in automatic screen refresh - should make the "palette update don't blit" flag obsolete.....
added a static definition for the default system palette
added interception for CoCreateInstance following cases:
	case 0xe436ebb3: Module="quartz"; Class="CLSID_FilterGraph";
	case 0x4fd2a832: Module="ddrawex"; Class="CLSID_DirectDrawEx";
	case 0x49c47ce5: Module="amstream"; Class="CLSID_AMMultiMediaStream";
preliminary (proxed) interception for "GetActiveWindow" and "GetForegroundWindow"

v2.02.45
A small fix for ddraw 7 games that makes Praetorians playable, though with some problems!

v2.02.46
Fixed handling of user32 CreateDialogIndirectParam call to avoid processing in non fullscreen mode - fix necessary to start "Crimson Skies".

v2.02.47
GUI:
Configuration tabs reorganization: now some tabs have merged making it easier to reach all options
core:
improved d3d wrapping and sdded ZBUFFER cleanup flags to fix some d3d games. In particular:
Clean ZBUFFER @1.0 fix: to fix "Star Wars Episode I Racer"
Clean ZBUFFER @0.0 fix: to fix "Crimson Skies" black blocks bug on ATI cards

v2.02.48
New options: "Disable fogging", "Textures not power of 2 fix"
A few fix to prevent game crashes

v2.02.49
Separated ddraw, d3d and dxwnd log messages
Hooked GetAvailableVidMem method to limit memory values
Hooked ChangeDisplaySettings call in both versions ANSI and WIDECHAR, to prevent messing up with the screen
Added HookWindowProc() subroutine, called at dxhook.c@1534
Added "Add proxy libs" flag - so far to copy a d3d9.dll proxy lib to fix d3d9 games
Hooked d3d8/9 SetCursorPosition method
Hooked d3d8/9 AddRef / release methods (for logging)
Fixed D3DDevice8/9 hooking
Hooked QueryPerformanceFrequency
Fixed a bug in d3d7 hooking
GUI: added desktop color setting

v2.02.50
Added "Intercept RDTSC" option: so far, tested on Unreal Tournament only.
To implement RDTSC detection, I owe a big thank you to Olly who shared the disasm lib used to browse the assembly code.

v2.02.51/52
Hooked window timers: now "Ed Hunter" can be time stretched
added NOFILLRECT debug option
fixed WM_DISPLAYCHANGE handling (x,y swapped coordinates?)
fixed int64 arithmetic for performance counters
Added (untested !!!) handling for RDTSCP opcode with "Intercept RDTSC" option
Added "Limit screen resolution" option: seems necessary for "Sid Meyer's Civilization III" to work
Improved initial window coordinate & style handling
fixed virtual screen size showing into status panel
hooked SetPixelFormat, GetPixelFormat, ChoosePixelFormat and DescribePixelFormat to redirect desktop hwnd and make wglCreateContext work (needed for Civ III)
fixed log message for TextOut parameters
hooked DisableD3DSpy (invoked by The Bard's Tale)
fixed extglViewport coordinate remapping when invoked with CW_USEDEFAULT values (Civ III)
fixed bug in DirectDarawCreate/Ex hooking with wrong module handle

v2.02.53
Handling of ddraw screen color depth 15BPP - in "Hesperian Wars"
fixed IAT scanning: now function addresses for remapping are searched in possibly multiple instances of dll text segment - in "Aaron Hall's Dungeon Odissey"
hooked SystemParametersInfo call
fixed window size logic to check for window menu bar, considering also the WS_CHILD case.
fixed window move/resize when message processing is enabled: coordinates outside the child area or within ENTER/EXITSIZEMOVE must NOT be altered.
expanded timer processing to include user32 Set/KillTimer - in "Aaron Hall's Dungeon Odissey"
fixed possible recursion while hooking child window procedure identical to parent's one
fixed gdi emulation on top of ddraw surface, enough to play "Sid Meyer's Civilization III".

v2.02.54
fixed a bug in d3d GetAvailableVidMem that was causing too many troubles...
fixed a bug in Suppress IME option
fixed a bug in handled library list, possibly affecting d3d/d3d7 games
avoid fixing window style for non-desktop windows (fixed a Port Royale 2 bug)
eliminated "Disable HAL support", no longer necessary
some incomplete work on gdi game handling
added preliminary, incomplete (not working) glide handling

v2.02.55
added "Hide desktop background" video option

v2.02.56
hooked RegisterClassA API (useless so far...)
fixed bug in SendMessageW hooking: caused truncated text in window titles and form captions
fixed several d3d1-7 hooks: prevented some d3d games to properly work, i.e. "Thief the Dark Project"
fixed d3d GetDisplayMode to return virtual desktop size - fixed "Affari Tuoi", an italian game.
fixed some log messages

v2.02.57
fixed WS_NCCALCSIZE & WM_NCPAINT messages to ensure a proper window's border and size - fixed Black Thorn window problems
fixed "Hide desktop background" flag 

v2.02.58
Added Automatic DirectX mode: not workink always, but should make it much easier to guess a proper surface setting.
Automatic mode allows dynamic mode change depending on the situation: it is necessary for Populous 3 D3D version.
fixed BltFast operation with SRC color key (often used to draw cursor from texures) directly to primary surface: this makes the cursor visible in Populous 3.
minor fixes on logging

v2.02.59
fixed IDIrectDraw reference counter for ddraw release 1 session: fixes "Warhammer 40K Rites Of War" first screen
fixed QueryPerformanceCounter handling: fixes "New York Racer" time stretching problems

v2.02.60
fixed time stretching to handle Total Soccer 2000
fixed ZBUFFER surface creation to handle GOG Incoming troubles on some cards
fixed handling of special effects (wireframe, no fogging, zbuffer always) to be valid on all d3d versions

v2.02.61
added directx "Compensate Flip emulation" flag: used primarily to get rid of mouse artifacts due to emulated Flip rpocedure in windowed mode. Fixes Gruntz issues and some other games...

v2.02.62
fixed d3d7 CreateDevice hook and d3d7 rendering options (wireframe, disablefogging, zbufferalways)
fixed doublebuffer emulation - avoid returning the backbuffer when zbuffer is requested. Fixes rendering problems of many games!

v2.02.63
fixed d3d hooking for Reset and GetDirect3D methods: now Jumpgate works at any available resolution, and the "Add proxy libs" option is no longer necessary
fixed "Compensate Flip emulation" option for non emulated surface mode: fixes mouse artifacts for "Rogue Spear Black Thorn" 
added preliminary handling for "Disable Textures" option

v2.02.64
GUI: implemented multiple file import
DLL:
fixed a ddraw session reference count error that prevented "Jet Moto" to start
fixed "Compensate Flip emulation" for ddraw7 games: now "Empire Earth" supports this flag.
fixed CloseWindow hook: now games that minimize/restore the main window on task switch can possibly recover (e.g. Hundred Swords)
fixed process hook: now it should be a little more robust and efficient. Hopefully should fix some Macromedia Flash problems.
fixed IDDrawSurface::Lock() prototype. Not gameplay improvements, though...
added DirectX "Return 0 ref counter" option as quick & dirty solution to many reference counter problems!
fixed DirectDrawCreateEx failing to register the main ddraw session handle
fixed palette problem in emulated mode: palette must be applied to backbuffer surface as well. Now "Duckman" and "Total Soccer 2000" show perfect colors.

v2.02.65
added "Fine time adjust" flag to finely set timeslider by 10% steps ranging from about :2 to x2
fixed a bug in DWORD timers introduced in v2.02.60: now "Warcraft 2" timing works again
revised logit to calculate delays to emulate VSync timing - now should be more accurate and depending on actual screen refresh rate

v2.02.66
fixed palette bug: "Virtua Fighter PC" now shows correct colors.

v2.02.67
fixed log for D3D CreateDevice method, missing in some D3D interface versions
added wildcarded program path: now you needn't set the full pathname, but it is sufficient to specify the rightmost part, e.g. the executable filename.
added emulated mode color conversion from 32BPP to 16BPP. Despite the fact that the best way to run a 32BPP game is against a 32BPP desktop, now it is possible to downgrade 32BPP colors to 16BPP. This fixed the fact that 32BPP games run on a 16BPP desktop showed simply black screens (e.g. Baldur's Gate II)
fixed logic for EnumDisplayModes implementation: now screen resolutions and, for emulated mode only, also color depth, are generated by DxWnd and not derived directly from the real ones. This make it possible to fake support for very low screen resolutions (e.g. 320x200) on modern screens that support them no longer. This make "Genocide" running on both supported resolutions 320x200 and 320x240.

v2.02.68
added screen resolution choice: either a set of SVGA resolutions (mostly for 3:4 monitors), HDTV resolutions (mostly for 16:9 monitors) or the native set of resolutions offered by your monitor / video card.

v2.02.69
improved exception catching to intercept memory violation exceptions and to neutralize offending code. Thank again to olly didasm lib, now the assembly instruction length is automatically determined so that the opcode can be replaced by the correct number of NOP instructions. If this doesn't mean much for you, just consider that this makes playable the Win95 (patched) release of "Star Wars Tie Fighter".
fixed FillRect user32 call - brings some improvements to Imperialism (still far from being acceptable).

v2.02.70
GUI:
fixed saving wrong coordinates when dxwnd is terminated while minimized
added check for adminstrative capabilities on startup
DLL:
fix: moved DrawText/Ex api hooks into right library
fix: corrected child window procedure handling - fixes "Imperialism" child window closing 
fixed FillRect coordinate handling - fixes "Imperialism" menus
fixed SetWindowPlacement handling

v2.02.71
fix: Set/GetWindowLongA/W are always hooked.
fix: added user32 GetDCEx hook for GDI Emulation & Directraw mode
fix: hooked "FrameRect", "TabbedTextOutA", "DrawTextA", "DrawTextExA", "FillRect" in scaled mode only
fix: FIXNCHITTEST mode
fix: when main win is closed, blit area is made null to avoid messing with a wrong screen area
added "Release mouse outside window" option. This option causes the get cursor position to detect a centered mouse position when the cursor is moved outside the window, allowing interaction with other windows without scrolling ot the windowed program. Mainly, this option is meant to help people with a physical disability to use other programs (e. g. the virtual keyboard) to play games.
added "Launch" field (optional) to start the program with arguments or a separate task

v2.02.72
fix: fixed Launch field used with "Use DLL injection" flag
fix: somehow improved "Use DLL injection" to avoid blocked tasks and allow exception handling

v2.02.73
to be written ....