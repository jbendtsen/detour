@echo off

set compiler=%CD:~0,2%\Tools\tcc\tcc32
set libs=-mwindows -lpsapi -lcomctl32
set output=detour.exe
set cfiles=src\add_detour.c src\inject.c src\main.c src\manage_dlls.c src\processes.c src\utils.c

taskkill /im %output% > nul 2>&1
%compiler% %libs% %cfiles% -o %output%

if %ERRORLEVEL% == 0 (
	start %output%
	exit /b 0
)

pause