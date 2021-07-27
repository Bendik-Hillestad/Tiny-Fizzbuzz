@echo off

REM You will need Build Tools for Visual Studio 2019 (or a full installation of Visual Studio 2019)
REM Link: https://visualstudio.microsoft.com/downloads/#build-tools-for-visual-studio-2019
REM Make sure you're in a Developer Command Prompt for VS 2019, or that you have found the well-hidden vcvarsall.bat
REM and executed it from a regular command prompt (not PowerShell) before running this script. Make sure you're running
REM the 32-bit toolchain as well.

echo - Build started...

REM Setup temporary directory and output directory if needed

if not exist "tmp\" mkdir "tmp\"
if not exist "tmp\obj\" mkdir "tmp\obj\"
if not exist "tmp\exe\" mkdir "tmp\exe\"
if not exist "out\" mkdir "out\"

REM Clear temporary directory and output directory before starting

del "tmp\*.*" /s /f /q >nul
del "out\*.*" /f /q >nul

REM Compile the program

echo - Compiling

cl.exe /nologo /std:c++17 /permissive- /Zc:inline /Zc:threadSafeInit- /Zc:forScope /O1 /Oi /Oy- /GR- /GS- /Gs9999999 /EHa- /MD /W4 /WX /Zl /D"WIN32" /D"_HAS_EXCEPTIONS=0" /D"NDEBUG" /Fo"tmp/obj/fizzbuzz.obj" /c src/main.cpp

if %errorlevel% neq 0 exit /b %errorlevel%

REM Link the program

echo - Linking

link.exe /nologo /nodefaultlib /entry:_main /machine:x86 /stack:0x100000,0x100000 /largeaddressaware /incremental:no /opt:ref /opt:icf /manifest:no /dynamicbase:no /fixed /safeseh:no  /subsystem:console /align:16 /emitpogophaseinfo /out:tmp/exe/fizzbuzz.exe tmp/obj/fizzbuzz.obj 1>nul

if %errorlevel% neq 0 exit /b %errorlevel%

REM Move executable to the out directory

move /Y "tmp\exe\fizzbuzz.exe" "out\fizzbuzz.exe" >nul
echo -^> %~dp0out\fizzbuzz.exe

REM Check for crinkler

if exist "crinkler.exe" (
    REM Compile the program

    echo - Compiling for crinkler

    cl.exe /nologo /std:c++17 /permissive- /Zc:inline /Zc:threadSafeInit- /Zc:forScope /O1 /Oi /Oy- /GR- /GS- /Gs9999999 /EHa- /MD /W4 /WX /Zl /D"CRINKLER" /D"WIN32" /D"_HAS_EXCEPTIONS=0" /D"NDEBUG" /Fo"tmp/obj/fizzbuzz_cr.obj" /c src/main.cpp

    if %errorlevel% neq 0 exit /b %errorlevel%
    
    REM Link the program

    echo - Linking with crinkler

    crinkler.exe /nodefaultlib /entry:_main /largeaddressaware /subsystem:console /out:tmp/exe/fizzbuzz_cr.exe /CRINKLER /TINYHEADER /TINYIMPORT /UNALIGNCODE tmp/obj/fizzbuzz_cr.obj kernel32.lib

    if %errorlevel% neq 0 exit /b %errorlevel%
    
    REM Move executable to the out directory
    
    move /Y "tmp\exe\fizzbuzz_cr.exe" "out\fizzbuzz_cr.exe" >nul
    echo -^> %~dp0out\fizzbuzz_cr.exe
)
