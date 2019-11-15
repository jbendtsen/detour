#include "header.h"

#define ADDDETOUR_WIDTH  226
#define ADDDETOUR_HEIGHT 380

#define N_ADV_WNDS 7

#define EXPAND_W 90

char *addDetourTitle = "Add Detour(s)";
char *advancedTitle = "Advanced";

static HBRUSH backBrush = NULL;

static HWND dlgWnd = NULL;
static WNDCLASSEXA wc = {0};

static HWND dllLbl = NULL;
static HWND methodLbl = NULL;
static HWND addrsLbl = NULL;

static HWND advancedBtn = NULL;

static HWND invocLbl = NULL;
static HWND sbLbl = NULL;
static HWND beforeLbl = NULL;
static HWND afterLbl = NULL;

static HWND dllCb = NULL;
static HWND dllBtn = NULL;
static HWND methodCb = NULL;
static HWND invocCb = NULL;

static HWND beforeEd = NULL;
static HWND afterEd = NULL;
static HWND addrsEd = NULL;

static HWND addBtn = NULL;
static HWND cancelBtn = NULL;

static HWND advancedList[N_ADV_WNDS] = {NULL};

static int expandW = 0;

void shiftWndLeftRight(HWND hwnd, int displ) {
	RECT r = {0};
	GetWindowRect(hwnd, &r);

	r.right += displ;

	SetWindowPos(
		hwnd, NULL,
		r.left, r.top,
		r.right - r.left, r.bottom - r.top,
		SWP_NOZORDER
	);
}

void tryExpand(HWND hwnd) {
	if (expandW)
		return;

	expandW = EXPAND_W;

	shiftWndLeftRight(dlgWnd, expandW);

	for (int i = 0; i < N_ADV_WNDS; i++)
		ShowWindow(advancedList[i], SW_SHOW);
}

void tryCollapse(HWND hwnd) {
	if (!expandW)
		return;

	int disp = -expandW;
	expandW = 0;

	for (int i = 0; i < N_ADV_WNDS; i++)
		ShowWindow(advancedList[i], SW_HIDE);

	shiftWndLeftRight(dlgWnd, disp);
}

LRESULT CALLBACK dlgWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	int res = 0;

	switch (uMsg) {
		case WM_CREATE:
		{
			HINSTANCE inst = GetModuleHandle(NULL);

			dllLbl    = createLabel(hwnd, SHOW_LBL, "DLL",         10,  10, 30, 20);
			methodLbl = createLabel(hwnd, SHOW_LBL, "Method",      10,  60, 50, 20);
			addrsLbl  = createLabel(hwnd, SHOW_LBL, "Address(es)", 10, 110, 70, 20);

			dllCb = createComboBox(ID_ADD_DLLCB, hwnd, 10, 30, 155, 150);
			DllInfo *dll = refreshDllList();
			while (dll) {
				SendMessage(dllCb, CB_ADDSTRING, 0, (LPARAM)dll->name);
				dll = dll->next;
			}
			//SendMessage(dllCb, CB_ADDSTRING, 0, (LPARAM)"Add New...");

			dllBtn = createButtonEx(ID_ADD_DLLBTN, hwnd, "...", 170, 30, 30, 21);

			methodCb = createComboBox(ID_ADD_METHODCB, hwnd, 10, 80, 190, 200);

			addrsEd = CreateWindowA(
				"EDIT", "",
				WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE,
				10, 130, 190, 140,
				hwnd, (HMENU)ID_ADD_ADDRSEDIT,
				inst, NULL
			);
			applyNiceFont(addrsEd);

			cancelBtn = createButton(ID_ADD_CANCELBTN, hwnd, "Cancel", 220, 200);
			addBtn    = createButton(ID_ADD_ADDBTN, hwnd, "Add", 150, 200);
			EnableWindow(addBtn, 0);

			advancedBtn = CreateWindowA(
				"BUTTON", "Advanced",
				WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
				10, 280, 70, 20,
				hwnd, (HMENU)ID_ADD_ADVANCEDBTN,
				inst, NULL
			);
			applyNiceFont(advancedBtn);

			invocLbl  = createLabel(hwnd, HIDE_LBL, "Invocation",    210, 20, 70, 20);
			sbLbl     = createLabel(hwnd, HIDE_LBL, "Stack Balance", 210, 40, 80, 20);
			beforeLbl = createLabel(hwnd, HIDE_LBL, "Before",        220, 60, 50, 20);
			afterLbl  = createLabel(hwnd, HIDE_LBL, "After",         220, 80, 50, 20);

			advancedList[0] = invocLbl;
			advancedList[1] = invocCb;
			advancedList[2] = sbLbl;
			advancedList[3] = beforeLbl;
			advancedList[4] = beforeEd;
			advancedList[5] = afterLbl;
			advancedList[6] = afterEd;

			break;
		}
		case WM_COMMAND:
		{
			int code = wParam >> 16;
			int wndId = wParam & 0xffff;
			if (wndId == ID_ADD_ADVANCEDBTN && code == BN_CLICKED) {
				int state = SendMessage(advancedBtn, BM_GETCHECK, 0, 0);
				if (state)
					tryExpand(hwnd);
				else
					tryCollapse(hwnd);
			}

			/*
			if (wndId == ID_ADD_DLLCB && code == CBN_SELCHANGE) {
				int idx = SendMessage(dllCb, CB_GETCURSEL, 0, 0);
				int n_items = SendMessage(dllCb, CB_GETCOUNT, 0, 0);
			}
			*/
			if (wndId == ID_ADD_DLLBTN) {
				/*
				if (idx == n_items-1) {
					char *dll_file = openFileDialog();
					if (dll_file != NULL) {
						insertDll(dll_file);
						refreshDllCombobox();
						// set selected item to be of the 2nd last index
					}
					else {
						// revert selection
					}
				}
				*/
			}

			if (wndId == ID_ADD_CANCELBTN) {
				closeDetourDialog();
				break;
			}

			return DefWindowProc(hwnd, uMsg, wParam, lParam);
		}
		case WM_SIZE:
		{
			RECT r = {0};
			GetClientRect(hwnd, &r);
			SetWindowPos(cancelBtn, NULL, r.right -  72, r.bottom - 35, BTN_WIDTH, BTN_HEIGHT, SWP_NOZORDER);
			SetWindowPos(addBtn,    NULL, r.right - 142, r.bottom - 35, BTN_WIDTH, BTN_HEIGHT, SWP_NOZORDER);
			break;
		}
		case WM_GETMINMAXINFO:
		{
			LPMINMAXINFO lpMMI = (LPMINMAXINFO)lParam;
			lpMMI->ptMinTrackSize.x = ADDDETOUR_WIDTH + expandW;
			lpMMI->ptMinTrackSize.y = ADDDETOUR_HEIGHT;
			return 0;
		}
		case WM_CTLCOLORSTATIC:
		{
			return (LRESULT)backBrush;
		}
		case WM_CLOSE:
			dlgWnd = NULL;
			expandW = 0;
			DeleteObject(backBrush);
			backBrush = NULL;
			DestroyWindow(hwnd);
			break;
		default:
			res = DefWindowProc(hwnd, uMsg, wParam, lParam);
	}

	return res;
}

void openDetourDialog(HWND mainWnd) {
	if (dlgWnd)
		return;

	HINSTANCE thisMod = GetModuleHandle(NULL);

	if (!wc.cbSize) {
		wc.cbSize        = sizeof(WNDCLASSEXA);
		wc.style         = CS_GLOBALCLASS | CS_HREDRAW | CS_VREDRAW;
		wc.lpfnWndProc   = dlgWndProc;
		wc.hInstance     = thisMod;
		wc.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
		wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
		wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
		wc.lpszMenuName  = NULL;
		wc.lpszClassName = addDetourTitle;
		wc.hIconSm       = LoadIcon(NULL, IDI_APPLICATION);

		RegisterClassExA(&wc);
	}

	dlgWnd = CreateWindowA(
		addDetourTitle, addDetourTitle,
		WS_OVERLAPPEDWINDOW | WS_VISIBLE,
		CW_USEDEFAULT, CW_USEDEFAULT, ADDDETOUR_WIDTH, ADDDETOUR_HEIGHT,
		mainWnd, NULL, thisMod, NULL
	);

	if (!backBrush)
		backBrush = CreateSolidBrush(GetBkColor(GetDC(NULL)));

	ShowWindow(dlgWnd, SW_SHOW);
	UpdateWindow(dlgWnd);
}

void closeDetourDialog() {
	if (dlgWnd)
		SendMessage(dlgWnd, WM_CLOSE, 0, 0);
}
