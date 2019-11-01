#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "header.h"

#define IDC_LISTBOX 200

#define MAX_PROCS 1024
#define LB_ITEM_SIZE 48

char *className = "openProcDlg";

DWORD processList[MAX_PROCS] = {0};
int selectedProcIdx = -1;

HWND procDlg = NULL, procDlgList = NULL;
WNDCLASSEX procWc = {0};

void getProcesses() {
	DWORD len = 0;
	EnumProcesses(&processList[0], MAX_PROCS, &len);

	//SendMessage(procDlgList, LB_INITSTORAGE, len, len * LB_ITEM_SIZE); // maybe unnecessary?
	char str[LB_ITEM_SIZE];
	int idx = 0;

	for (int i = 0; i < len; i++) {
		int pid = processList[i];
		if (pid <= 0)
			continue;

		sprintf(str, "%d\t", pid);
		int len = strlen(str);

		HANDLE proc = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, 0, pid);
		GetModuleBaseName(proc, NULL, str + len, LB_ITEM_SIZE - len - 2);
		CloseHandle(proc);

		int pos = SendMessage(procDlgList, LB_ADDSTRING, 0, (LPARAM)str);
		SendMessage(procDlgList, LB_SETITEMDATA, pos, idx);
		idx++;
	}
}

LRESULT CALLBACK openProcWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	int res = 0;

	switch (uMsg) {
		case WM_CREATE:
		{
			procDlgList = CreateWindow(
				"LISTBOX", NULL,
				WS_VISIBLE | WS_CHILD | LBS_STANDARD | LBS_NOTIFY,
				0, 0, 200, 200,
				hwnd, (HMENU)IDC_LISTBOX,
				GetModuleHandle(NULL), NULL
			);
			applyNiceFont(procDlgList);
			
			getProcesses();
			break;
		}
		case WM_SIZE:
		{
			RECT r = {0};
			GetClientRect(hwnd, &r);
			const int bdr = 5;

			SetWindowPos(
				procDlgList, NULL,
				bdr, bdr,
				r.right - bdr*2, r.bottom - bdr*2,
				SWP_NOZORDER
			);
			break;
		}
		/*
		case WM_INITDIALOG:
		{
			getProcesses();
			res = 1;
			break;
		}
		*/
		case WM_COMMAND:
		{
			int id = wParam >> 16;
			if (id == IDC_LISTBOX) {
				int item = SendMessage(procDlgList, LB_GETCURSEL, 0, 0);
				selectedProcIdx = SendMessage(procDlgList, LB_GETITEMDATA, item, 0);

				char title[40];
				sprintf(title, "PID: %d", selectedProcIdx);
				SetWindowText(procDlg, title);
			}

			res = 1;
			break;
		}
		case WM_CLOSE:
			procDlg = NULL;
			DestroyWindow(procDlgList);
			DestroyWindow(hwnd);
			break;
		default:
			res = DefWindowProc(hwnd, uMsg, wParam, lParam);
	}

	return res;
}

void openProcessDialog(HWND mainWnd) {
	if (procDlg)
		return;

	HINSTANCE thisMod = GetModuleHandle(NULL);

	if (!procWc.cbSize) {
		procWc.cbSize        = sizeof(WNDCLASSEX);
		procWc.style         = CS_GLOBALCLASS | CS_HREDRAW | CS_VREDRAW;
		procWc.lpfnWndProc   = openProcWndProc;
		procWc.hInstance     = thisMod;
		procWc.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
		procWc.hCursor       = LoadCursor(NULL, IDC_ARROW);
		procWc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
		procWc.lpszMenuName  = NULL;
		procWc.lpszClassName = className;
		procWc.hIconSm       = LoadIcon(NULL, IDI_APPLICATION);

		RegisterClassEx(&procWc);
	}

	int width = 200, height = 200;

	procDlg = CreateWindowEx(
		WS_EX_CLIENTEDGE,
		className, "Open Process",
		WS_OVERLAPPEDWINDOW | WS_VISIBLE,
		CW_USEDEFAULT, CW_USEDEFAULT, width, height,
		mainWnd, NULL, thisMod, NULL
	);

	ShowWindow(procDlg, SW_SHOW);
	UpdateWindow(procDlg);
}

void closeProcessDialog() {
	if (procDlg)
		SendMessage(procDlg, WM_CLOSE, 0, 0);
}
