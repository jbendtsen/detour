#ifndef HEADER_H
#define HEADER_H

#include <windows.h>
#include <psapi.h>

void applyNiceFont(HWND hwnd);

void openProcessDialog(HWND mainWnd);
void closeProcessDialog(void);

#endif
