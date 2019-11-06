#ifndef HEADER_H
#define HEADER_H

#include <windows.h>
#include <commctrl.h>
#include <psapi.h>

typedef unsigned int u32;
typedef unsigned long long int u64;

#define FIELD_BYTEPOS(templ, field) (u64)&templ.field - (u64)&templ

#define ID_FILE_OPEN       101
#define ID_MAIN_LV         120
#define ID_MAIN_ADDBTN     130
#define ID_PROC_LV         140
#define ID_PROC_OPENBTN    150
#define ID_PROC_CANCELBTN  151
#define ID_ADD_ADVANCEDBTN 160

#define SHOW_LBL 1
#define HIDE_LBL 0

#define BTN_WIDTH 60
#define BTN_HEIGHT 28

#define MAX_COLS 10

#define PI_PID_LEN  8
#define PI_NAME_LEN 200

typedef struct {
	DWORD pid;
	char pid_str[PI_PID_LEN];
	char name[PI_NAME_LEN];
} ProcessInfo;

#define DI_ENBL_LEN   4
#define DI_ADDR_LEN   20
#define DI_METHOD_LEN 100
#define DI_INVOC_LEN  8

typedef struct {
	u64 addr;
	void *func;
	HINSTANCE mod;
	int invoc;

	char enable_str[DI_ENBL_LEN];
	char addr_str[DI_ADDR_LEN];
	char method_str[DI_METHOD_LEN];
	char invoc_str[DI_INVOC_LEN];
} DetourInfo;

typedef struct {
	int row_info_size;
	LVCOLUMNA columns[MAX_COLS];
	int field_offs[MAX_COLS];
	int n_cols;
	int cur_col;
	int sort_dir;
} ListDesc;

// main.c
void useProcess(ProcessInfo *proc);

// processes.c
void openProcessDialog(HWND mainWnd);
void closeProcessDialog(void);

// add_detour.c
void openDetourDialog(HWND mainWnd);
void closeDetourDialog(void);

// utils.c
void applyNiceFont(HWND hwnd);
int resizeWindow(HWND hwnd, RECT *r);
void errorMessage(const char *file, int line);
void showVar(const char *name, void *var, int elems);
HWND createLabel(HWND mainWnd, int visible, char *text, int x, int y, int w, int h);
void createColumn(LVCOLUMNA *col, int idx, char *name);
void createTable(HWND listWnd, ListDesc *desc, void *items, int nRows);
void sortListView(HWND listWnd, ListDesc *desc, int col);
void *rowFromEvent(HWND listWnd, NMITEMACTIVATE *item);
HWND createComboBox(HWND mainWnd, int x, int y, int w, int h);

#endif
