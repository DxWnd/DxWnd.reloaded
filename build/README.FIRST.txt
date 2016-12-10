Readme First release notes - October 10th, 2016

DxWnd is a project still in development, so new releases are not always fully compatible with previous ones.
Compatibility issues may easily happen while trying to use old configurations (the dxwnd.ini file) or exported files (*.dxw files located in the export folder) with new releases of the program.
In particular, dxw exported files are not always checked and updated before releasing new versions of the program, since that activity would steal precious time for the program enhancements, so I apologize but this is the current scenario.
This file will try to provide a quick guide to the resolution of the most common portability problems, listing the known cases that can be easily fixed.

==============================================================================================

Flip emulation: 
the DirectX/"Flip emulation" flag is required for most of DirectDraw games and is now set by default, but old exported files may not have this flag set. The sympthom generally is an error in DirectDraw calls or a black screen. In case the flag is not checked, just check it.

==============================================================================================

Optimize for AERO mode:
same as above, the flag is set by default, but some old exported files may not have it. 
The flag activated a blitting schema that is more compatible with AERO and desktop composition that is now mandatory on recent OS (Win8 and above). Better check it in case of problems.

==============================================================================================

Final note:
If all else fails do not despair! Defaulted values are pretty good in most cases, so better start from scratch without using an old .dxw export file.
Or, even better, create a new defauled entry, import the old .dxw file and start comparing the two configurations: it is a hell of a job, but likely you can pick something good from the old file that will make the new entry working.
Finally, share your success with the community! Please, post on the SF board any working and non trivial configuration to share it with other users, and notify residual problems that I could work about.
Please, consider this as being part of the fun!
yours faithfully
GHO