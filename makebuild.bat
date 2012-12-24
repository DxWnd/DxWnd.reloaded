del dll\dxwnd.vcproj.*.user
del dll\dxwnd.ncb

del host\dxwndhost.vcproj.*.user
del host\dxwndhost.ncb

copy Release\dxwnd.exe build
copy Release\dxwnd.dll build

del dll\Debug\*.*
del dll\Release\*.*

del host\Debug\*.*
del host\Release\*.*

echo build done
pause
