@echo off
REM   dsptest-setup.bat : copies IPP/IPPS dll's into debug and release directories
REM
REM   Author: Consultant - Jim Smith
REM
REM   Copyright (c) 2010 Avinger, Inc.

set IPP_DIR=..\..\..\..\..\lib\win32\ipp\ia32\bin

@echo on
copy %IPP_DIR%\ippcore-6.0.dll debug
copy %IPP_DIR%\ipps-6.0.dll debug
copy %IPP_DIR%\ippsp8-6.0.dll debug
copy %IPP_DIR%\ippspx-6.0.dll debug
copy %IPP_DIR%\ippss8-6.0.dll debug
copy %IPP_DIR%\ippst7-6.0.dll debug
copy %IPP_DIR%\ippsv8-6.0.dll debug
copy %IPP_DIR%\ippsw7-6.0.dll debug
copy %IPP_DIR%\libiomp5md.dll debug

copy %IPP_DIR%\ippcore-6.0.dll release
copy %IPP_DIR%\ipps-6.0.dll release
copy %IPP_DIR%\ippsp8-6.0.dll release
copy %IPP_DIR%\ippspx-6.0.dll release
copy %IPP_DIR%\ippss8-6.0.dll release
copy %IPP_DIR%\ippst7-6.0.dll release
copy %IPP_DIR%\ippsv8-6.0.dll release
copy %IPP_DIR%\ippsw7-6.0.dll release
copy %IPP_DIR%\libiomp5md.dll release
