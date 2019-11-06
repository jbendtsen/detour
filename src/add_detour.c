#include "header.h"

#define ADDDETOUR_WIDTH  300
#define ADDDETOUR_HEIGHT 300

#define N_ADV_WNDS 7

#define EXPAND_H 80

char *addDetourTitle = "Add Detour(s)";
char *advancedTitle = "Advanced";

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
static HWND methodCb = NULL;
static HWND invocCb = NULL;

static HWND beforeEd = NULL;
static HWND afterEd = NULL;
static HWND addrsEd = NULL;

static HWND addBtn = NULL;
static HWND cancelBtn = NULL;

static HWND advancedList[N_ADV_WNDS] = {NULL};

static int expandH = 0;

void tryExpand(HWND hwnd) {
	if (expandH)
		return;

	for (int i = 0; i < N_ADV_WNDS; i++)
		ShowWindow(advancedList[i], SW_SHOW);

	expandH = EXPAND_H;

	RECT r = {0};
	GetWindowRect(dlgWnd, &r);
	r.bottom += expandH;
	resizeWindow(dlgWnd, &r);
}

void tryCollapse(HWND hwnd) {
	if (!expandH)
		return;

	for (int i = 0; i < N_ADV_WNDS; i++)
		ShowWindow(advancedList[i], SW_HIDE);

	RECT r = {0};
	GetWindowRect(dlgWnd, &r);
	r.bottom -= expandH;
	resizeWindow(dlgWnd, &r);

	expandH = 0;
}

LRESULT CALLBACK dlgWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	int res = 0;

	switch (uMsg) {
		case WM_CREATE:
		{
			HINSTANCE inst = GetModuleHandle(NULL);

			dllLbl    = createLabel(hwnd, SHOW_LBL, "DLL:",         10, 10, 30, 20);
			methodLbl = createLabel(hwnd, SHOW_LBL, "Method:",      10, 30, 50, 20);
			addrsLbl  = createLabel(hwnd, SHOW_LBL, "Address(es):", 10, 50, 70, 20);

			dllCb = createComboBox(hwnd, 50, 10, 70, 20);

			invocLbl  = createLabel(hwnd, HIDE_LBL, "Invocation:",   10, 100, 70, 20);
			sbLbl     = createLabel(hwnd, HIDE_LBL, "Stack Balance", 10, 120, 80, 20);
			beforeLbl = createLabel(hwnd, HIDE_LBL, "Before:",       20, 140, 50, 20);
			afterLbl  = createLabel(hwnd, HIDE_LBL, "After:",        20, 160, 50, 20);

			advancedBtn = CreateWindowA(
				"BUTTON", "Advanced",
				WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
				10, 70, 70, 20,
				hwnd, (HMENU)ID_ADD_ADVANCEDBTN,
				inst, NULL
			);
			applyNiceFont(advancedBtn);

			advancedList[0] = invocLbl;
			advancedList[1] = invocCb;
			advancedList[2] = sbLbl;
			advancedList[3] = beforeLbl;
			advancedList[4] = beforeEd;
			advancedList[5] = afterLbl;
			advancedList[6] = afterEd;

			break;
		}
		case WM_SIZE:
		{
			
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
			break;
		}
		case WM_CLOSE:
			dlgWnd = NULL;
			expandH = 0;
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

	ShowWindow(dlgWnd, SW_SHOW);
	UpdateWindow(dlgWnd);
}

void closeDetourDialog() {
	if (dlgWnd)
		SendMessage(dlgWnd, WM_CLOSE, 0, 0);
}