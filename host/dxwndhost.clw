; CLW ファイルは MFC ClassWizard の情報を含んでいます。

[General Info]
Version=1
LastClass=CTargetDlg
LastTemplate=CDialog
NewFileInclude1=#include "stdafx.h"
NewFileInclude2=#include "dxwndhost.h"
LastPage=0

ClassCount=6
Class1=CDxwndhostApp
Class2=CDxwndhostDoc
Class3=CDxwndhostView
Class4=CMainFrame

ResourceCount=4
Resource1=IDD_ABOUTBOX
Resource2=IDR_MENU_POPUP
Class5=CAboutDlg
Class6=CTargetDlg
Resource3=IDR_MAINFRAME
Resource4=IDD_TARGET

[CLS:CDxwndhostApp]
Type=0
HeaderFile=dxwndhost.h
ImplementationFile=dxwndhost.cpp
Filter=N

[CLS:CDxwndhostDoc]
Type=0
HeaderFile=dxwndhostDoc.h
ImplementationFile=dxwndhostDoc.cpp
Filter=N

[CLS:CDxwndhostView]
Type=0
HeaderFile=dxwndhostView.h
ImplementationFile=dxwndhostView.cpp
Filter=C
BaseClass=CListView
VirtualFilter=VWC
LastObject=ID_RUN


[CLS:CMainFrame]
Type=0
HeaderFile=MainFrm.h
ImplementationFile=MainFrm.cpp
Filter=T
LastObject=ID_MENUITEM32774




[CLS:CAboutDlg]
Type=0
HeaderFile=dxwndhost.cpp
ImplementationFile=dxwndhost.cpp
Filter=D
LastObject=IDC_VERSION
BaseClass=CDialog
VirtualFilter=dWC

[DLG:IDD_ABOUTBOX]
Type=1
Class=CAboutDlg
ControlCount=4
Control1=IDC_STATIC,static,1342177283
Control2=IDC_VERSION,static,1342308480
Control3=IDC_STATIC,static,1342308352
Control4=IDOK,button,1342373889

[MNU:IDR_MAINFRAME]
Type=1
Class=CMainFrame
Command1=ID_APP_EXIT
Command2=ID_RUN
Command3=ID_MODIFY
Command4=ID_DELETE
Command5=ID_ADD
Command6=ID_APP_ABOUT
CommandCount=6

[ACL:IDR_MAINFRAME]
Type=1
Class=CMainFrame
Command1=ID_FILE_NEW
Command2=ID_FILE_OPEN
Command3=ID_FILE_SAVE
Command4=ID_EDIT_UNDO
Command5=ID_EDIT_CUT
Command6=ID_EDIT_COPY
Command7=ID_EDIT_PASTE
Command8=ID_EDIT_UNDO
Command9=ID_EDIT_CUT
Command10=ID_EDIT_COPY
Command11=ID_EDIT_PASTE
Command12=ID_NEXT_PANE
Command13=ID_PREV_PANE
CommandCount=13

[DLG:IDD_TARGET]
Type=1
Class=CTargetDlg
ControlCount=30
Control1=IDC_FILE,edit,1350631552
Control2=IDC_OPEN,button,1342242816
Control3=IDC_AUTO,button,1342308361
Control4=IDC_DIRECTX1,button,1342177289
Control5=IDC_DIRECTX7,button,1342177289
Control6=IDC_DIRECTX8,button,1342177289
Control7=IDC_DIRECTX9,button,1342177289
Control8=IDC_UNNOTIFY,button,1342242819
Control9=IDC_EMULATEPAL,button,1342242819
Control10=IDC_HOOKDI,button,1342242819
Control11=IDC_INITX,edit,1350639744
Control12=IDC_INITY,edit,1350639744
Control13=IDC_MINX,edit,1350631552
Control14=IDC_MINY,edit,1350631552
Control15=IDC_MAXX,edit,1350631552
Control16=IDC_MAXY,edit,1350631552
Control17=IDC_MODIFYMOUSE,button,1342242819
Control18=IDC_OUTTRACE,button,1342242819
Control19=IDC_SAVELOAD,button,1342242819
Control20=IDOK,button,1342242817
Control21=IDCANCEL,button,1342242816
Control22=IDC_STATIC,button,1342308359
Control23=IDC_STATIC,static,1342308352
Control24=IDC_STATIC,static,1342308352
Control25=IDC_STATIC,static,1342308352
Control26=IDC_STATIC,static,1342308352
Control27=IDC_STATIC,static,1342308352
Control28=IDC_STATIC,static,1342308352
Control29=IDC_STATIC,static,1342308352
Control30=IDC_STATIC,static,1342308352

[CLS:CTargetDlg]
Type=0
HeaderFile=TargetDlg.h
ImplementationFile=TargetDlg.cpp
BaseClass=CDialog
Filter=D
LastObject=IDC_FILE
VirtualFilter=dWC

[MNU:IDR_MENU_POPUP]
Type=1
Class=CDxwndhostView
Command1=ID_PRUN
Command2=ID_PMODIFY
Command3=ID_PDELETE
Command4=ID_PADD
CommandCount=4

