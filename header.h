#ifndef HEADER_H
#define HEADER_H

#include <windows.h>
#include <commctrl.h>
#include <psapi.h>

#define PI_PID_LEN  8
#define PI_NAME_LEN 200

typedef struct {
	DWORD pid;
	char pid_str[PI_PID_LEN];
	char name[PI_NAME_LEN];
} ProcessInfo;

void applyNiceFont(HWND hwnd);
void errorMessage(const char *file, int line);
void insertLVColumn(HWND listWnd, int idx, char *name, int fmt, int width);
void useProcess(ProcessInfo *proc);

void openProcessDialog(HWND mainWnd);
void closeProcessDialog(void);

#endif
