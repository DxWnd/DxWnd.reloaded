Readme First release notes - November 7th, 2016

DxWnd is a project still in development, so new releases are not always fully compatible with previous ones.
Compatibility issues may easily happen while trying to use old configurations (the dxwnd.ini file) or exported files (*.dxw files located in the export folder) with new releases of the program.
In particular, dxw exported files are not always checked and updated before releasing new versions of the program, since that activity would steal precious time for the program enhancements, so I apologize but this is the current scenario.
This file will try to provide a quick guide to the resolution of the most common portability problems, listing the known cases that can be easily fixed.

==============================================================================================
DxWnd does nothing!

If it seems that DxWnd is perfectly useless, the programs starting in fullscreen mode exactly as if DxWnd was not running, you could likely have a capability problem caused either from lack of permissions or some compatibility setting. 
You can try these two simple steps:

1) Assign to DxWnd.exe administrator privileges (right-click on DxWnd.exe, Properties->Compatibility->check "run this program as administrator" or whatever is your locale translated interface). I always try to set this flag in the DxWnd distribution, but a simple file copy can clear the privilege.

2) Clear compatibility settings on the target program (again, "Properties"->"Compatibility"->uncheck all compatibility flags). Maybe you'd better use the dedicated DxWnd command ("Tools"->"Clear compatibility flags", apparently this one goes a little deeper and clears some possibly hidden flags). If you clear compatibility flags it is likely that the game will no longer run in fullscreen without DxWnd, so maybe you can perform the operation on a copy of the executable file.
==============================================================================================
Window size & position: 

Old DxWnd releases had a different setting for locking the window size and position.
If the game jumps in some unexpected position of the screen or changes its window size, head to the Video / Window size & position panel and try some option different from "Free".

==============================================================================================
Flip emulation: 

the DirectX/"Flip emulation" flag is required for most of DirectDraw games and is now set by default, but old exported files may not have this flag set. The sympthom generally is an error in DirectDraw calls or a black screen. In case the flag is not checked, just check it.

==============================================================================================
Optimize for AERO mode:

same as above, the flag is set by default, but some old exported files may not have it. 
The flag activated a blitting schema that is more compatible with AERO and desktop composition that is now mandatory on recent OS (Win8 and above). Better check it in case of problems.

==============================================================================================
Force clipper:

Setting a clipper on the primary surface is something that sometimes is good, sometimes is bad. For this reason, so far the "DirectX" / "Force clipper" is unchecked by default. 
But a side effect of the lack of a clipping region is the rupture of the desktop composition! So, if that happens (the desktop blinks and the AERO transparency effects disappear) you can try to set the flag and see if there are changes for the better.

============================================================================================
Final note:

If all else fails do not despair! Defaulted values are pretty good in most cases, so better start from scratch without using an old .dxw export file.
Or, even better, create a new defauled entry, import the old .dxw file and start comparing the two configurations: it is a hell of a job, but likely you can pick something good from the old file that will make the new entry working.
Finally, share your success with the community! Please, post on the SF board any working and non trivial configuration to share it with other users, and notify residual problems that I could work about.
Please, consider this as being part of the fun!

yours faithfully
GHO



