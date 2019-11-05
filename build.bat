@echo off

set dir=%CD:~0,2%\Tools\tcc
set files=list_view.c processes.c detours.c main.c

%dir%\tcc32 -mwindows -lpsapi -lcomctl32 %files% -o detour.exe

pause