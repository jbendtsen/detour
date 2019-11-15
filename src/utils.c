#include <stdio.h>
#include <stdlib.h>
#include "header.h"

#define RESID_SHELL32 124

void applyNiceFont(HWND hwnd) {
	SendMessage(hwnd, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), 1);
}

ULONG_PTR enableVisualStyles() {
	ACTCTXA ctx = {0};
	ctx.cbSize  = sizeof(ACTCTXA);
	ctx.dwFlags = ACTCTX_FLAG_RESOURCE_NAME_VALID | ACTCTX_FLAG_SET_PROCESS_DEFAULT | ACTCTX_FLAG_ASSEMBLY_DIRECTORY_VALID;
	ctx.lpSource = "shell32.dll";
	ctx.lpAssemblyDirectory = "C:\\Windows\\System32";
	ctx.lpResourceName = (LPCSTR)RESID_SHELL32;

	HANDLE active = CreateActCtxA(&ctx);

	ULONG_PTR cookie = 0;
	ActivateActCtx(active, &cookie);

	InitCommonControls();
	return cookie;
}

void errorMessage(const char *file, int line) {
	DWORD error = GetLastError();
	char buf[128];
	sprintf(buf, "Error %#x (in %s:%d)\n", error, file, line);
	MessageBoxA(NULL, buf, "WinAPI Error", MB_ICONWARNING);
	return;
}

// messy yet juicy
void showVar(const char *name, void *var, int elems) {
	if (elems < 1) elems = 1;
	int name_len = strlen(name);
	char *msg = malloc(name_len + 20 * elems);
	if (!var)
		sprintf(msg, "%s = NULL", name);
	else if ((ULONG_PTR)var < 0x1000)
		sprintf(msg, "%s = %#x", name, (ULONG_PTR)var);
	else {
		u32 *p = (u32*)var;
		int len = sprintf(msg, "%s = %#x", name, *p++);
		for (int i = 1; i < elems; i++)
			len += sprintf(msg + len, ", %#x", *p++);
	}
	MessageBoxA(NULL, msg, name, MB_OK);
	free(msg);
}

HWND createLabel(HWND mainWnd, int visible, char *text, int x, int y, int w, int h) {
	int style = WS_CHILD | SS_LEFT;
	if (visible) style |= WS_VISIBLE;

	HWND lbl = CreateWindowA(
		"STATIC", text,
		style,
		x, y, w, h,
		mainWnd, NULL,
		GetModuleHandle(NULL), NULL
	);

	applyNiceFont(lbl);
	return lbl;
}

HWND createButton(LONG_PTR id, HWND mainWnd, char *text, int x, int y) {
	HWND btn = CreateWindowA(
		"BUTTON", text,
		WS_VISIBLE | WS_CHILD,
		x, y, BTN_WIDTH, BTN_HEIGHT,
		mainWnd, (HMENU)id,
		GetModuleHandle(NULL), NULL
	);

	applyNiceFont(btn);
	return btn;
}

HWND createButtonEx(LONG_PTR id, HWND mainWnd, char *text, int x, int y, int w, int h) {
	HWND btn = CreateWindowA(
		"BUTTON", text,
		WS_VISIBLE | WS_CHILD,
		x, y, w, h,
		mainWnd, (HMENU)id,
		GetModuleHandle(NULL), NULL
	);

	applyNiceFont(btn);
	return btn;
}

// List-View helper functions

void createColumn(LVCOLUMNA *col, int idx, char *name) {
	col->iSubItem = idx;
	col->mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	col->pszText = name;
	col->cchTextMax = strlen(name);
	col->cx = 50;
}

void createTable(HWND listWnd, ListDesc *desc, void *items, int nRows) {
	for (int i = 0; i < desc->n_cols; i++)
		SendMessage(listWnd, LVM_INSERTCOLUMN, i, (LPARAM)&desc->columns[i]);

	if (nRows <= 0)
		return;

	LVITEMA cell = {0};
	for (int i = 0; i < nRows; i++) {
		cell.iItem = i;
		cell.lParam = (LPARAM)(items + i * desc->row_info_size);
		for (int j = 0; j < desc->n_cols; j++) {
			cell.iSubItem = j;
			cell.pszText = (char*)cell.lParam + desc->field_offs[j];
			cell.cchTextMax = strlen(cell.pszText);

			if (j == 0) {
				cell.mask = LVIF_TEXT | LVIF_PARAM; // LVIF_PARAM only works for one sub-item per row
				SendMessage(listWnd, LVM_INSERTITEM, 0, (LPARAM)&cell);
			}
			else {
				cell.mask = LVIF_TEXT;
				SendMessage(listWnd, LVM_SETITEM, 0, (LPARAM)&cell);
			}
		}
	}
}

#define SORT_ASC   0
#define SORT_DESC  1

int CALLBACK sort_items(LPARAM lp1, LPARAM lp2, LPARAM d) {
	ListDesc *desc = (ListDesc*)d;
	int offset = desc->field_offs[desc->cur_col];
	char *item1 = (char*)(lp1 + offset);
	char *item2 = (char*)(lp2 + offset);

	int diff = 0;
	if (item1[0] >= '0' && item1[0] <= '9' &&
		item2[0] >= '0' && item2[0] <= '9')
			diff = atoi(item1) - atoi(item2);
	else
			diff = strcmp(item1, item2);

	if (desc->sort_dir == SORT_DESC)
		diff = -diff;

	return diff;
}

void sortListView(HWND listWnd, ListDesc *desc, int col) {
	if (desc->cur_col == col)
		desc->sort_dir ^= 1;
	else
		desc->cur_col = col;

	SendMessage(listWnd, LVM_SORTITEMS, (WPARAM)desc, (LPARAM)sort_items);
}

void *rowFromEvent(HWND listWnd, NMITEMACTIVATE *item) {
	LVITEMA row = {0};
	row.iItem = item->iItem;
	row.mask = LVIF_TEXT | LVIF_PARAM;
	SendMessage(listWnd, LVM_GETITEM, 0, (LPARAM)&row);

	return (void*)row.lParam;
}

// Combo-Box helper functions

HWND createComboBox(LONG_PTR id, HWND mainWnd, int x, int y, int w, int h) {
	HWND cbWnd = CreateWindowA(
		WC_COMBOBOX, "",
		WS_VISIBLE | WS_CHILD | CBS_DROPDOWN | CBS_HASSTRINGS,
		x, y, w, h,
		mainWnd, (HMENU)id,
		GetModuleHandle(NULL), NULL
	);
	applyNiceFont(cbWnd);
	return cbWnd;
}
