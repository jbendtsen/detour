#include <stdio.h>
#include <stdlib.h>
#include "header.h"

#define MAIN_WIDTH  480
#define MAIN_HEIGHT 360

#define ENABLE_W 20
#define ADDR_W   70
#define METHOD_W 80
#define INVOC_W  70

char *progName = "Detour!";
HWND mainWnd = NULL;

static HWND listWnd = NULL;
static HWND addBtn = NULL;

ListDesc detoursDesc = {0};

void useProcess(ProcessInfo *proc) {
	char title[100];
	sprintf(title, "%s - %s", progName, proc->name);
	SetWindowTextA(mainWnd, title);
	EnableWindow(addBtn, 1);
}

void setupDetoursTable() {
	detoursDesc.row_info_size = sizeof(DetourInfo);
	detoursDesc.n_cols = 4;

	DetourInfo template = {0};
	detoursDesc.field_offs[0] = FIELD_BYTEPOS(template, enable_str);
	detoursDesc.field_offs[1] = FIELD_BYTEPOS(template, addr_str);
	detoursDesc.field_offs[2] = FIELD_BYTEPOS(template, method_str);
	detoursDesc.field_offs[3] = FIELD_BYTEPOS(template, invoc_str);

	char *col_names[] = {"E", "Address", "DLL Method", "Invocation"};
	int col_widths[] = {ENABLE_W, ADDR_W, METHOD_W, INVOC_W};
	LVCOLUMNA *col = &detoursDesc.columns[0];
	for (int i = 0; i < 4; i++, col++) {
		createColumn(col, i, col_names[i]);
		col->cx = col_widths[i];
	}

	createTable(listWnd, &detoursDesc, NULL, 0);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	int res = 0;

	switch (uMsg) {
		case WM_CREATE:
		{
			HMENU mainMenu = CreateMenu();
			HMENU fileMenu = CreatePopupMenu();

			AppendMenuA(fileMenu, 0, ID_FILE_OPEN, "&Open Process");
			AppendMenuA(mainMenu, MF_POPUP, (UINT_PTR)fileMenu, "&File");

			SetMenu(hwnd, mainMenu);

			HINSTANCE inst = GetModuleHandle(NULL);

			listWnd = CreateWindowA(
				WC_LISTVIEW, "",
				WS_VISIBLE | WS_CHILD | LVS_REPORT,
				0, 0, 200, 200,
				hwnd, (HMENU)ID_MAIN_LV,
				inst, NULL
			);
			applyNiceFont(listWnd);

			addBtn = CreateWindowA(
				"BUTTON", "Add",
				WS_VISIBLE | WS_CHILD,
				10, 10, BTN_WIDTH, BTN_HEIGHT,
				hwnd, (HMENU)ID_MAIN_ADDBTN,
				inst, NULL
			);
			applyNiceFont(addBtn);
			EnableWindow(addBtn, 0);

			setupDetoursTable();
			break;
		}
		case WM_SIZE:
		{
			RECT r = {0};
			GetClientRect(hwnd, &r);

			const int bdr = 5;
			int tableW = r.right - bdr*2;
			int tableH = r.bottom - bdr*4 - BTN_HEIGHT;

			int addY = tableH + bdr * 3;
			int addX = tableW - BTN_WIDTH;

			SetWindowPos(addBtn, NULL, addX, addY, BTN_WIDTH, BTN_HEIGHT, SWP_NOZORDER);

			SetWindowPos(listWnd, NULL, bdr, bdr, tableW, tableH, SWP_NOZORDER);

			int minTotalW = ENABLE_W + ADDR_W + METHOD_W + INVOC_W;
			int methodW = SendMessage(listWnd, LVM_GETCOLUMNWIDTH, 2, 0);
			if (methodW < METHOD_W)
				SendMessage(listWnd, LVM_SETCOLUMNWIDTH, 2, METHOD_W);
			else if (tableW > minTotalW)
				SendMessage(listWnd, LVM_SETCOLUMNWIDTH, 2, METHOD_W + tableW - minTotalW);

			break;
		}
		case WM_COMMAND:
		{
			int cmd = wParam & 0xffff;
			if (cmd == ID_FILE_OPEN) {
				openProcessDialog(hwnd);
				res = 1;
				break;
			}
			if (cmd == ID_MAIN_ADDBTN) {
				openDetourDialog(hwnd);
				res = 1;
				break;
			}

			return DefWindowProc(hwnd, uMsg, wParam, lParam);
		}
		case WM_CLOSE:
			closeProcessDialog();
			closeDetourDialog();
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
	ctrls.dwICC = ICC_LISTVIEW_CLASSES | ICC_TREEVIEW_CLASSES;
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

	// Using CreateWindowEx with WS_EX_CLIENTEDGE here would create an ugly border underneath the menu
	mainWnd = CreateWindowA(
		progName, progName,
		WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
		200, 200, MAIN_WIDTH, MAIN_HEIGHT,
		NULL, NULL, hInstance, NULL
	);

	ShowWindow(mainWnd, nCmdShow);
	UpdateWindow(mainWnd);

	ACCEL openProcAccel = {FVIRTKEY | FCONTROL, 'O', ID_FILE_OPEN};
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
