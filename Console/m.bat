@echo off

REM   Support file for building a Qt project on Windows
REM
REM   Author: Dennis W. Jackson
REM
REM   Copyright (c) 2009-2013 Avinger, Inc.

REM  set commands in this script do not live beyond run-time
setlocal

set INSTALLER_EXE=installer\OCT-HS-Console-setup.exe
set INSTALLER_ARCHIVE_DIR="\\aspen\data0\R&D\Software\SW\Internal_Release\OCT_HS_Console\v%2"

if "%1" == "" goto Help
if "%1" == "help" goto Help
if "%1" == "--help" goto Help
if "%1" == "installer" goto CreateInstaller
if "%1" == "installer-release" goto CopyInstaller
if "%1" == "cleanall" goto CleanAll
if "%1" == "buildall" goto BuildAll

goto EOF


:CreateInstaller
REM  Default to Win 7, 64-bit location
set NSIS="C:\Program Files (x86)\NSIS\makensis.exe"

REM  Check for WinXP. Set the path appropriately
ver | %SystemRoot%\system32\find "XP" > nul
if %ERRORLEVEL% == 0 set NSIS="C:\Program Files\NSIS\makensis.exe"

%NSIS% installer\OCTConsole.nsi
goto EOF


:CopyInstaller
if "%2" == "" goto Error_MissingVersion

if NOT EXIST %INSTALLER_EXE% goto Error_MissingInstaller

mkdir %INSTALLER_ARCHIVE_DIR%
if %ERRORLEVEL% == 1 goto Error_Mkdir

echo Copying the installer...
copy  %INSTALLER_EXE%   %INSTALLER_ARCHIVE_DIR%

echo Copying map and symbol files...
copy  homescreen\release\homescreen.map     %INSTALLER_ARCHIVE_DIR%
copy  homescreen\release\homescreen.pdb     %INSTALLER_ARCHIVE_DIR%
copy  consoleApp\release\octConsole.map     %INSTALLER_ARCHIVE_DIR%
copy  consoleApp\release\octConsole.pdb     %INSTALLER_ARCHIVE_DIR%
  
@echo off
goto EOF


:Error_MissingVersion
echo ERROR: You did not specify a version number for this release;
echo        e.g., m installer-release 0.0.1
echo        No files were copied. Try again!
echo.
goto EOF


:Error_MissingInstaller
echo ERROR: The installer, %INSTALLER_EXE%, is missing!
echo.
goto EOF


:Error_Mkdir
echo ERROR: Could not make the Installer Archive directory (%INSTALLER_ARCHIVE_DIR%)
echo.
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
echo           installer          Create Windows Installer (output in ./Installer)
echo           installer-release  Copy installer to network folder
echo           cleanall           Clean all projects (target is distclean) and the installer
echo           buildall           Build the release targets in all projects and the installer
echo.
goto EOF



:CleanAll
for %%i in ( homescreen consoleApp ) do (
   echo.
   pushd %%i
   echo In %%i...
   call m.bat distclean
   popd
)
del Installer\OCT-HS-Console-setup.exe
goto EOF


:BuildAll
for %%i in ( homescreen consoleApp ) do (
   echo.
   pushd %%i
   echo In %%i...
   call m.bat release
   popd
)
call m.bat installer
goto EOF


:EOF
endlocal
