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
Time stratching by keyboard control (Alt F5/F6) and/or Time Slider dialog