#include <stdio.h>
#include "header.h"

#define IDM_MAIN_MENU 150
#define IDM_FILE_OPEN 101

char *progName = "Detour!";
HWND mainWnd = NULL;

void applyNiceFont(HWND hwnd) {
	SendMessage(hwnd, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), 1);
}

void errorMessage(const char *file, int line) {
	DWORD error = GetLastError();
	char buf[128];
	sprintf(buf, "Error %#x in %s at line %d\n", error, file, line);
	MessageBoxA(NULL, buf, "WinAPI Error", MB_ICONWARNING);
	return;
}

void insertLVColumn(HWND listWnd, int idx, char *name, int fmt, int width) {
	LVCOLUMNA col;
	col.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	col.iSubItem = idx;
	col.pszText = name;
	col.cchTextMax = strlen(name);
	col.fmt = fmt;
	col.cx = width;

	SendMessage(listWnd, LVM_INSERTCOLUMN, idx, (LPARAM)&col);
}

void useProcess(ProcessInfo *proc) {
	SetWindowTextA(mainWnd, proc->name);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	int res = 0;

	switch (uMsg) {
		case WM_CREATE:
		{
			HMENU mainMenu = CreateMenu();
			HMENU fileMenu = CreatePopupMenu();

			AppendMenuA(fileMenu, 0, IDM_FILE_OPEN, "&Open Process");
			AppendMenuA(mainMenu, MF_POPUP, (UINT_PTR)fileMenu, "&File");

			SetMenu(hwnd, mainMenu);
			break;
		}
		case WM_COMMAND:
		{
			int cmd = wParam & 0xffff;
			if (cmd == IDM_FILE_OPEN) {
				openProcessDialog(hwnd);
				res = 1;
			}
			else
				return DefWindowProc(hwnd, uMsg, wParam, lParam);

			break;
		}
		case WM_CLOSE:
			closeProcessDialog();
			DestroyWindow(hwnd);
			break;
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		// I used to respond to WM_NCCLIENT here and it stuffed up menu clicks somehow
		default:
			res = DefWindowProc(hwnd, uMsg, wParam, lParam);
	}

	return res;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	INITCOMMONCONTROLSEX ctrls = {0};
	ctrls.dwICC = ICC_LISTVIEW_CLASSES;
    ctrls.dwSize = sizeof(INITCOMMONCONTROLSEX);
	InitCommonControlsEx(&ctrls);

	WNDCLASSEXA wc;
	wc.cbSize        = sizeof(WNDCLASSEXA);
	wc.style         = CS_GLOBALCLASS | CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc   = WndProc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 0;
	wc.hInstance     = hInstance;
	wc.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
	wc.lpszClassName = progName;
	wc.hIconSm       = LoadIcon(NULL, IDI_APPLICATION);

	RegisterClassExA(&wc);

	int width = 600;
	int height = 400;

	// Using CreateWindowEx with WS_EX_CLIENTEDGE here would create an ugly border underneath the menu
	mainWnd = CreateWindowA(
		progName, progName,
		WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
		CW_USEDEFAULT, CW_USEDEFAULT, width, height,
		NULL, NULL, hInstance, NULL
	);

	ShowWindow(mainWnd, nCmdShow);
	UpdateWindow(mainWnd);

	ACCEL openProcAccel = {FVIRTKEY | FCONTROL, 'O', IDM_FILE_OPEN};
	HACCEL accels = CreateAcceleratorTable(&openProcAccel, 1);

	MSG Msg;
	while (GetMessage(&Msg, NULL, 0, 0) > 0) {
		if (!TranslateAccelerator(mainWnd, accels, &Msg)) {
			TranslateMessage(&Msg);
			DispatchMessage(&Msg);
		}
	}

	return Msg.wParam;
}
