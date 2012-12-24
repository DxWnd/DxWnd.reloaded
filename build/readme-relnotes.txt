V2.01.48:

Implemented proper handling of blitting from on-screen primary surfaces. 
Now DxWnd supports un-emulated resized "The Sims" and "Warhammer 40K Rites of War".
Improved handling of attached backbuffers, even dynamically created ones.
Improved handling of GDI calls using HDC. "688I Hunter Killer" partially working. "Black Moon Chronicles" working.

V2.01.50:

Fixed bug on attached surface handling in non-emulated mode ("Alien vs. Predator" now supported)
Added Restore Screen Mode flag ("Army Men" OK)

V2.01.52:

Added handling of screen color setting to 16BPP color depth (partial).
Handling of games that check and require 16BPP color depth: "Resident Evil" now supported.
Fixed problem about "Turok 2", now supported

V2.01.53:

Major code reworking for emulation mode blitting rendering functions
Fixed GetPalette hook: "uprising" and "uprising 2" now supported

V2.01.54:

Fixed PeekMessage/GetMessage hook: "uprising" and "uprising 2" mouse handling now working
Implemented "Refresh on win resize" flag: useful for "uprising"
Fixed bug on new flags handling (not defaulted to OFF).

V2.01.56:

Implemented DirectDraw full proxy for method logging. 
Big code reorganization: new files hddproxy.cpp hddproxy.h
fixed hooking to IDirectDrawSurface::GetCaps, IDirectDrawSurface::GetAttachedSurface, IDirectDrawSurface::DeleteAttachedSurface
fixed hooking of extDirectDrawCreateEx
improved logging (DumpSurfaceAttributes)
BackBuffer created only when necessary

V2.01.62:

Fully revised COM hooking method
Warlord series now fully working
Implemented 24BPP color depth (Delta Force)
Started handling of backbuffer array for flipping chain emulation (unfinished)
Added handling of DDERR_NOCLIPPINGLIST error
Added "Simulate 8BPP desktop" mode
Added preliminary implementation of "Automatic Emulation mode"
Syberia & Syberia 2 now working
GUI: fixed game list sort command
More games, more stable, less parameters....

V2.01.63:

Implemented INIT16BPP flag
hdinput.cpp: Fixed DirectInput GetMousePosition() call
u32lib.cpp: Fixed FixWindowFrame() to avoid messing with windows when not in fullscreen mode
More code for handling of dynamically created flipping chains (unfinished)
Hooked address for dynamically loaded ChangeDisplaySettingsA
New supported games: Monster Truck Madness 1 & 2, Mortal Kombat 4

V2.01.64:

Hooked & neutralized IDirect3DDevice8->Reset() method (preliminary implementation)
New supported games: Virtua Tennis, Crazy Taxi

V2.01.65:
Fixed handling of inherited backbuffers from previous and closed primary surface in no-emulated mode. Fixes "Warhammer 40k Rites of War" display problems.
Hook for DIRECTDRAW object creation through CoCreateInstance: new supported game "Axis & Allies"

V2.01.66/67
Added wrapping for BeginPaint() in "Map GDI HDC to Primary DC" mode, returning the virtual primary surface DC: makes "Deadlock 2" playable and stretchable.
Deleted the "Mouse to Screen Fix" option (useless).
Fixed bug in palette handling in desktop 16BPP color depth: Fixes colors in Age of Empires, Enemy Infestation.

V2.01.68
Fixed mouse message processing to alter ALL mouse events: This makes "Uprising 2" handling the mouse correctly.
Aligned the hooking logic in proxy mode to the new logic in normal mode. 
Inserted the Diablo FIX: this is odd! Apparently, and perhaps for an internal game bug, the game works only if you create an extra surface after the primary one, maybe to insulate some memory corruption. As a matter of fact, doing so, you have an useless and unused extra surface, but old Diablo works pretty well.
Some unused code and comments cleanup.
GUI major changes:
- Added system tray handling
- Added control about maximum number of entries reached
- Added controls about allowed GUI commands
- Eliminated several useless commands
- cleanup of unused resources & code 
- Added control to avoid multiple task activation

V2.01.68.FIX
Fixed a new bug that prevented execution of ANY multi-threaded game!!!!

V2.01.69
CORE:
Fixed bug: "GP500" is now working
Inserted "Keep cursor fixed" flag: "Necrodrome" is now working
Fixed window message handling for "Fix window style", "Prevent Maximize" and "Lock win properties" flags
Optimized IPC shared space, now the supported entries are 256 again.
Extra special key: Alt-F9 toggle window position locking
Extra special key: Alt-F10 toggle logging
Added "Blit using NULL rect spec." to handle some odd cases: makes "Railroad Tycoon II" working.
Fixed "Remap GDI Client rect" option: now sets the LPPAINTSTRUCT returned by BeginPaint. This makes "Railroad Tycoon II" working with no need of the emergency "Blit using NULL rect spec." flag.
Fixed dx_ScreenRefresh() internal routine: more accurate screen refreshing with "Refresh on win resize" mode.
Hooked InvalidateRect() to invalidate whole window: improves "Railroad Tycoon II" accuracy.
Some improvement in Diablo menu screens (character selection menu)
Some more logging and error detection.
Hook API code revision: HookSysLibs takes care of both HookSysLibsByAddr and HookSysLibsByName. No more similar routines to keep aligned!
GUI: 
Added command line arguments: /I (start idle), /T (start into tray)
added save changes confirmation message.

V2.01.70
CORE:
Fixed "Force cursor clipping" in non emulated mode.
Added retry in timed out surface lock operation. Fixes sporadic "Ancient Evil" crash in emulated mode.
Entirely rewritten the "Pitch Bug Fix" mode: now it uses more a service surface in memory to write to, and then the surface is blitted to the emulated primary surface, using more performant dx blit routines and allowing for surface scaling.
GUI:
The "Pitch Bug Fix" has been renamed to "Emulate Primary Buffer".

V2.01.71
CORE:
Added hooking for MapWindowPoints: makes "Commandos 2" working.
fixed windows message handling procedure: avoid calling fake address in the form of 0xFFFFnnnn returned by GetWindowLong (necessary for "Commandos 2")
Improved DirectDrawCreate/DirectDrawCreateEx hooking: now auto mode should work in more cases
Fixed isFullScreen toggle: now it handles both entering and exiting from fullscreen state
Fixed PITCHBUGFIX handling
introduced experimental ONEPIXELFIX flag
Fixed GetSurfaceDesc returned values for primary surface: needed for "Commandos 2".
Fixed PeekMessage/GetMessage whnd handling.

V2.01.72
CORE:
Fixed a regression bug on child windows handling
Added hook for DeferWindowPos() - to test
cleaned up a DEBUG message

V2.01.73
CORE:
Completely revised the logging code.
Added Import table entry logging
Added separate flags for mouse/cursor events
Added timestamp log banner
Fixed crash caused by attempt to write log on error condition (e.g. trying to write on a game CD)
Fixed GetDesktopWindow() and SetCooperativeLevel() to handle operations on desktop window: GP500 and Microsoft Midtown Madness are now working better.
hanling COLOR operations on blitting/flip: nor Pacific General half-vertical-lines movies don't show the previous screen any more.
By default, don't alter the window properties. Some games will lack the frame border: the "Fix Window Frame Style" should be checked.

V2.01.74
added bidirectional info exchange between GUI & DLL: now GUI can show internal emulation status.
GUI:
Added DxWnd status to trayicon tooltip
Fixed "Kill" menu command logic to act on the specific & correct task
Fixed bug that prevented asking for save configuration when task were added / deleted / sorted.
Fixed bug that disaligned task name field when deleting a task in the list.
CORE:
Updated GetHookStatus() arguments & behaviour
Fixed double-typing bug when "Process Messages" option is set.

V2.01.75
added "Handle Exceptions" flag. Applied to IDIV and CLI opcodes (exceptions 0xc0000095 divide by zero and 0xc0000096 priviliged instruction): now legacy "SonicR" and "Resident Evil" are working. 
fixed GetClientRect logic to handle desktop special case: this fixes "AfterLife".
revised VirtualScr Height & Width settings: now, they can't possibly be 0 ever, they're defaulted to 800x600.
fixed BlitError logging routine
added hooking to GDI32 palette API: CreatePalette, SelectPalette, RealizePalette. Fixed display for "Dementia".
fixed import table dump output: forwarderchain field is numeric
fixed a regression bug about the window style - now default style is successfully set to WS_OVERLAPPEDWINDOW (the windows default).

V2.01.76
added hook for MoveWindow API: fixed Lionheart positioning problem.

V2.01.77,78
added hook for GetSystemPaletteEntries - useless for now.....
fixed/improved several logs
fixed WS_EX_CONTROLPARENT exstyle windows handling: now "Diablo" works fine, and also Worms series shows some improvement.
added EnumDisplayModes hook with setting of emulated color depth. This makes "Grim Fandango" working with no need to set RGB565 encoding.
eliminated NULLRECTBLIT, TRANSPARENTALL and TRANSPARENTWIN handling, no longer useful
fixed bug in SetClipper handling for NULL lpddc: fixes "Gangsters" ingame screen.
added FIWPARENTWIN flag: fixes "Sid Meier's Golf" position and improves "Sleepwalker" behaviour
GUI:
game properties tab layout revised
eliminated useless options
View Status dialog
Recover desktop settings when changed on task kill or DxWnd exit

V2.01.79/80
fixed/improved some more log messages (fixed BltFast flags dump, added ROP dump, added ColorKey dump for blit operations).
fixed reverse blitting algorythm from 16/32BPP to palitezed 8BPP chosing a best matching index dinamically: that fixed several color problems in "Beasts & Bumpkins" and "Dink Smallwood".
code cleanup in GetDC hook function.
updated EnumDisplayModes hook function to avoid showing hi-res screen modes when PREVENTMAXIMIZE is set (useful?).
revised dxwFixWindowPos and AdjustWindowPos: code merged into CalculateWindowPos.
fixed bug in window positioning while toggling with ALT-F9.

V2.01.81
Eliminated useless "Reset PRIMARY surface" opt.
added 0xC0000094 exception code ("Ultim@te race pro" - untested)
added handling of fake backbuffer attached surfaces - should fix "Silver"
fixed missing hook to LoadLibrary & LoadLibraryEx - caused several oddities. Should allow more games be handled by default hook mode.
fixed some positioning logic in Fix Parent Window mode
fixed the "Video -> System surface on fail" behaviour: switch is tried not only on DDERR_OUTOFVIDEOMEMORY error condition, but also on DDERR_INVALIDPIXELFORMAT. This makes "The Sims" playable also in non-emulated mode.

V2.01.82
Project's folders reorganization, some better logging.
Fixed SetCooperativeLevel with NULL surface handler and "FixParent Window" option: makes Tomb Raider IV playable

V2.01.82
Fixed DDSURFACE::GetCaps adding DDSCAPS_FLIP capability in emulated mode. Makes "Funtracks" visible.
Eliminated useless "Use absolute coord" flag
fixed bug in Alt-F10 log toggle command

V2.01.83
Added "Blit from backbuffer" option - improves "The Sims"...
Eliminated useless "Automatic Emulation"
GUI: emulation modes (none, emulate buffer, emulate surface) are now mutually exclusive
minor fixes

V2.01.84/85/86
minor fixes on the GUI
fixed BackBuffer creation with an initial refcount=1 to simulate its initial reference by the frontbuffer. This makes "Monopoly 3D" working.
added "Suppress clipping" surface option: eliminates interference with primary surface blittin on "Monopoly 3D".
fixed bug in Alt-F10 log toggle command (I know I said that before....)
added logging status in status window (just to be sure the previous bug is FIXED!)
log fixes
experimental: PrimarySurface created with 1 backbuffer when no backbuffercount option is specified
fixed primary surface refcount in SetPalette, when a palette is attached to the primary surface
deleted the EXTRASURFACEFIX code prtion: now Diablo works without this trick, thanks to the proper refcount to avoid deleting a closed surface.
regression: Dungeon Keeper II no longer works.
Added sys libraries hooking to LoadLibrary and LoadLibraryEx hookers
fixed window coordinates in MoveWindow hooker
major: revised the sys libraries hooking mechanism. So far, nothing good happens, but ....

V2.01.87
new window option: "Modal style"
fixed dd::EnumDisplayModes hook with separate handling for dd interface 1 and 4
fixed handling of primary dd object (the one that owns the primary surface and the service objects). Makes Dungeon Keeper II playable again.
new games tested & working: eXpendable, Rally Championship 2000, Thief the Dark Project, In the Hunt, M1 Tank Platoon II, ...

V2.01.88
GUI: layout reorganization
new window option: "Keep aspect ratio" - to lock aspect ratio when resizing the game window
added "Limit available resources" option (void - to be implemented)

V2.01.89
new window option: "Force win resize" - experimental
revised GetDC reverse blitting algorithm - should now handle surfaces bigger than the primary (ref. Warlords III)
logging: added result of WM_NCHITTEST processing
added Alt-F8 toggle key for "Handle DC" option to speed up operations when accuracy can be sacrified
Direct3D preliminary hooks for display modes query
Directx9: hooked Reset method
fixed bug in ChangeDisplaySettings: now it saves current size also in emulated mode

v2.01.90
fully revised GetDC/ReleaseDC handling: Warlords III seems playable
revised mouse and clipping logic to fix some visibility and control problems in window resizing
project adapted and compiled with VS2008. Eliminated references to undefined ColorCorrectPalette API.

v2.01.91
Hyperblade fixes:
Wrapping of GDI PatBlt API to scale rect coordinates
fixing of wrong Blt destrect coordinates when blitting to primary surface: Hyperblade fails using the proper screen size 640x480 and uses an invalid 800x600 size.
again: revised GetDC/ReleaseDC handling to fix regression problems with other games
again: revised mouse and clipping logic to improve previous fixes. Now clipping mode is restored only after clicking on the window client area.

v2.01.92
Road Rash special edition:
several fixes in WS_CHLD windows handling: Child window process routine, MoveWindow and GetWindowPos hook routines.
fixed BIG window detection: now every BIG window (that is (0,0)-(ScreenX,ScreenY) in position) is detected and mapped on the main window coordinate system
fixed a log bug in extMoveWindow()
now "Road Rash" is fully playable, apart from the game movies that can't be resized and are correctly placed only with 640x480 window size.
known problem: the game window can be resized, but it returns back to the original size every time you get back to the menus.
loggin routine fix: now, if it can't open the logfile (ROADRASH.EXE stays on CD) tries to write on %TEMP% folder. Don't forget to clean it up!
GUI: added delete of %TEMP%\dxwnd.log file in "Clear all logs" command.

v2.01.93
Emergency special edition:
fully revised the GDI ClipCursor and GetClipCursor handling, to ensure that the cursor stays confined within the clipping region: now Emergency crashes no more when you attempt to move the cursor close to the window borders.
Introduced a "tolerance" interval to avoid unreacheable clip/window borders and difficulty to scroll the game area.
fixed "Keep aspect ratio" option to take in account the inner client area instead of the outer window size.

v2.01.94
fixed d3d EnumAdapterModes proxy hook that has different prototype for version 8 and 9: apps calling this method crash no more.
fixed GDI GetDC/GetWindowDC hooks: Tipically, you'd better prevent the app to access the desktop window, where it can retrieve info about the actual screen size and color depth, so the call is redirected to the main window. But, in certain circumstances, the window handle may become invalid, so that it's better to let the desktop hanlde pass. The fix makes "Halo: Combat Evolved" working.
fixed CreateWindowEx hook, preventing the creation of zero-sized windows. Makes "Star Wars Jedi Knight II: Jedi Outcast" at least visible...
added "Disable setting gamma ramp" flag: avoid changing screen brightness on the whole desktop surface, as "Star Wars Jedi Knight II: Jedi Outcast" may do.

v2.01.95
very preliminary support for OpenGL games. See "Star Wars Jedi Knight II: Jedi Outcast"....

v2.01.96/97
better support for OpenGL games (still preliminary...): Hexen II, American McGee Alice, ...
fixed "Simulate 8/16BPP desktop" flag: "Moto Racer" is now working on 32BPP desktop.
various improvement and log changes.

v2.01.98/99
wrapped SendMessage to revert fix to X,Y mouse coordinates: fixes "Pax Imparia Eminent Domain" right mouse problem
added dxwnd version and flags configuration to log for better diagnostic
updated the manual (at last!)
 
v2.1.100
double project definition, for vs2005 and vs2008
fixed PeekMessage logic to avoid message regeneration
added "none/OpenGL" emulation mode to avoid interfering with ddraw/d3d when unnecessary
added preliminary DirectDrawEnumerate hooking - just proxed so far
fixed crash when DirectDrawCreate/Ex was called with DDCREATE_HARDWAREONLY or DDCREATE_EMULATIONONLY flags
some code cleanup
fixed WM_NCHITEST handling to translate mouse coordinates properly: now "7th Legion" finally works.
fixed LoadLibrary logic to ignore full path when searching for specific libraries
fixes CreateWindowEx hook to handle the special case of "big" windows created with CW_USEDEFAULT position or bigger size: the fixes improve "The Grinch" window handling.
experimental changes/fixes to message processing
