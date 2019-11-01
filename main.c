#include "header.h"

#define IDM_FILE_OPEN 1

char *progName = "Detour!";

void applyNiceFont(HWND hwnd) {
	SendMessage(hwnd, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), 1);
}

void createMenu(HWND mainWnd) {
	HMENU mainMenu = CreateMenu();
	HMENU fileMenu = CreateMenu();

	AppendMenuA(fileMenu, MF_STRING, IDM_FILE_OPEN, "&Open Process");
	AppendMenuA(mainMenu, MF_POPUP, (UINT_PTR)fileMenu, "&File");

	SetMenu(mainWnd, mainMenu);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	int res = 0;

	switch (uMsg) {
		case WM_CREATE:
			createMenu(hwnd);
			break;
		case WM_SIZE:
			//SetWindowPos(hwnd, NULL, 0, 0, w, h, SWP_NOZORDER);
			break;
		case WM_COMMAND:
		{
			int cmd = wParam & 0xffff;
			if (cmd == IDM_FILE_OPEN) {
				openProcessDialog(hwnd);
				res = 1;
			}
			break;
		}
		case WM_CLOSE:
			closeProcessDialog();
			DestroyWindow(hwnd);
			break;
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		case WM_NCHITTEST:
			return HTCLIENT;
		default:
			res = DefWindowProc(hwnd, uMsg, wParam, lParam);
	}

	return res;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	WNDCLASSEX wc;
	wc.cbSize        = sizeof(WNDCLASSEX);
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

	RegisterClassEx(&wc);

	int width = 600;
	int height = 400;

	HWND hwnd = CreateWindowEx(
		WS_EX_CLIENTEDGE,
		progName, progName,
		WS_OVERLAPPEDWINDOW | WS_VISIBLE,
		CW_USEDEFAULT, CW_USEDEFAULT, width, height,
		NULL, NULL, hInstance, NULL
	);

	ShowWindow(hwnd, nCmdShow);
	UpdateWindow(hwnd);

	ACCEL openProcAccel = {FVIRTKEY | FCONTROL, 'O', IDM_FILE_OPEN};
	HACCEL accels = CreateAcceleratorTable(&openProcAccel, 1);

	MSG Msg;
	while (GetMessage(&Msg, NULL, 0, 0) > 0) {
		if (!TranslateAccelerator(hwnd, accels, &Msg)) {
			TranslateMessage(&Msg);
			DispatchMessage(&Msg);
		}
	}

	return Msg.wParam;
}
