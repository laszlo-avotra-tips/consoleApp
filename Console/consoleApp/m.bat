@echo off

REM   Support file for building a Qt project on Windows
REM
REM   Author: Dennis W. Jackson
REM
REM   Copyright (c) 2009-2018 Avinger, Inc.

REM  set commands in this script do not live beyond run-time
setlocal

set TARGET=octConsole.pro

if not exist Makefile qmake %TARGET%

if "%1" == "VC" goto MakeVC
if "%1" == "VC-clean" goto CleanVCFiles
if "%1" == "help" goto Help
if "%1" == "--help" goto Help
if "%1" == "setup" goto MakeSetup

REM Use Qt's tool for kicking off multiprocessor builds
..\..\bin\jom.exe %*

REM If this was a release build, build the check sum file
if "%1"  == "release" goto CheckSumRelease

goto EOF


:CheckSumRelease
REM  Copy the OpenCL files here for processing. Key the EXE and all of the
REM  OpenCL files.  These are checked by the initialization class at start-up.
pushd release
copy ..\Backend\OpenCL\*.cl .
..\..\..\bin\sha1sum -t octConsole.exe > octConsole.key
..\..\..\bin\sha1sum -t *.cl >> octConsole.key
popd
goto EOF


:MakeVC
qmake -tp vc %TARGET%
goto EOF


:CleanVCFiles
del *.ncb vc* *vcproj* *.suo *.sln
goto EOF


:Help
REM           1         2         3         4         5         6         7         8
REM  12345678901234567890123456789012345678901234567890123456789012345678901234567890 
echo Support file for building a Qt project on Windows
echo.
echo Usage: "m <target | option>"
echo.
echo target  Any target from Makefile
echo option  Batch file options
echo           --help, help       This text
echo           VC                 Create a Visual Studio project file
echo           setup              Copy DLLs required for running the EXE
echo.
goto EOF


:MakeSetup
REM Set relative locations
set ALAZAR_DIR=..\..\lib\win32\AlazarTech\bin
set AMD_DIR=..\..\lib\win32\AMD\clAmdFft\bin
set FFMPEG_DIR=..\..\lib\win32\ffmpeg\bin
   REM move FFMPEG to Common if we keep direct to video
set IPP_DIR=..\..\lib\win32\ipp\ia32\bin
set QWT_DIR=..\..\lib\win32\qwt-6.0.1\lib
set QXT_DIR=..\..\lib\win32\Qxt-0.6.0\lib
set QSERIAL_DIR=..\..\lib\win32\qserialport\lib
set VLD_DIR=..\..\lib\win32\vld\bin
set FTDI_DIR=..\..\lib\win32\FTDI

mkdir release
mkdir release\sqldrivers
mkdir release\imageformats
copy %AMD_DIR%\clAmdFft.Runtime.dll release
copy %FFMPEG_DIR%\avcodec-54.dll release
copy %FFMPEG_DIR%\avformat-54.dll release
copy %FFMPEG_DIR%\avutil-51.dll release
copy %FFMPEG_DIR%\swscale-2.dll release
copy %QTDIR%\bin\phonon4.dll release
copy %QTDIR%\bin\QtCore4.dll release
copy %QTDIR%\bin\QtGui4.dll release
copy %QTDIR%\bin\QtSql4.dll release
copy %QTDIR%\bin\QtSvg4.dll release
copy %QTDIR%\bin\QtXml4.dll release
copy %QTDIR%\plugins\sqldrivers\qsqlite4.dll release\sqldrivers
copy %QTDIR%\plugins\imageformats\qjpeg4.dll release\imageformats
copy %IPP_DIR%\ippcore-6.0.dll release
copy %IPP_DIR%\ipps-6.0.dll release
copy %IPP_DIR%\ippsp8-6.0.dll release
copy %IPP_DIR%\ippspx-6.0.dll release
copy %IPP_DIR%\ippss8-6.0.dll release
copy %IPP_DIR%\ippst7-6.0.dll release
copy %IPP_DIR%\ippsv8-6.0.dll release
copy %IPP_DIR%\ippsw7-6.0.dll release
copy %IPP_DIR%\libiomp5md.dll release
copy %QWT_DIR%\qwt.dll release
copy %QXT_DIR%\QxtCore.dll release
copy %QXT_DIR%\QxtGui.dll release
copy %QSERIAL_DIR%\QtSerialPort1.dll release
copy %FTDI_DIR%\ftd2xx.dll release
REM   copy the openCL files for running under Qt Creator and from the command line
copy Backend\OpenCL\*.cl release
copy Backend\OpenCL\*.cl .

mkdir debug
mkdir debug\sqldrivers
mkdir debug\imageformats
copy %ALAZAR_DIR%\ATSApi.dll debug
copy %AMD_DIR%\clAmdFft.Runtime.dll debug
copy %FFMPEG_DIR%\avcodec-54.dll debug
copy %FFMPEG_DIR%\avformat-54.dll debug
copy %FFMPEG_DIR%\avutil-51.dll debug
copy %FFMPEG_DIR%\swscale-2.dll debug
copy %QTDIR%\bin\phonond4.dll debug
copy %QTDIR%\bin\QtCored4.dll debug
copy %QTDIR%\bin\QtGuid4.dll debug
copy %QTDIR%\bin\QtSqld4.dll debug
copy %QTDIR%\bin\QtXmld4.dll debug
copy %QTDIR%\plugins\sqldrivers\qsqlited4.dll debug\sqldrivers
copy %QTDIR%\plugins\imageformats\qjpegd4.dll debug\imageformats
copy %IPP_DIR%\ippcore-6.0.dll debug
copy %IPP_DIR%\ipps-6.0.dll debug
copy %IPP_DIR%\ippsp8-6.0.dll debug
copy %IPP_DIR%\ippspx-6.0.dll debug
copy %IPP_DIR%\ippss8-6.0.dll debug
copy %IPP_DIR%\ippst7-6.0.dll debug
copy %IPP_DIR%\ippsv8-6.0.dll debug
copy %IPP_DIR%\ippsw7-6.0.dll debug
copy %IPP_DIR%\libiomp5md.dll debug
copy %QWT_DIR%\qwtd.dll debug
copy %QXT_DIR%\QxtCored.dll debug
copy %QXT_DIR%\QxtGuid.dll debug
copy %QSERIAL_DIR%\QtSerialPort1.dll debug
copy %FTDI_DIR%\ftd2xx.dll debug
copy %VLD_DIR%\* debug
goto EOF


:EOF
endlocal
