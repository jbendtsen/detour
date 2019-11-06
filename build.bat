@echo off

set dir=%CD:~0,2%\Tools\tcc
set files=src\utils.c src\processes.c src\add_detour.c src\main.c
set output=detour.exe

taskkill /im %output% > nul 2>&1
%dir%\tcc32 -mwindows -lpsapi -lcomctl32 %files% -o %output%

pause