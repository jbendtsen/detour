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
#define ID_ADD_DLLCB       161
#define ID_ADD_METHODCB    162
#define ID_ADD_ADDRSEDIT   163
#define ID_ADD_ADDBTN      164
#define ID_ADD_CANCELBTN   165
#define ID_ADD_DLLBTN      166

#define SHOW_LBL 1
#define HIDE_LBL 0

#define BTN_WIDTH 60
#define BTN_HEIGHT 28

#define METHOD_NAME_LEN 124
#define DLL_NAME_LEN 104

typedef struct {
	int offset;
	char name[METHOD_NAME_LEN];
} DllMethod;

struct dllinformation {
	struct dllinformation *next, *prev;
	void *base_ptr;
	DllMethod *methods;
	char name[DLL_NAME_LEN];
};
typedef struct dllinformation DllInfo;

#define MAX_COLS 10

#define LV_PID_LEN  8
#define LV_PROC_LEN 200

typedef struct {
	DWORD pid;
	char pid_str[LV_PID_LEN];
	char name[LV_PROC_LEN];
} ProcessInfo;

#define LV_ENBL_LEN   4
#define LV_ADDR_LEN   20
#define LV_METHOD_LEN 100
#define LV_INVOC_LEN  8

typedef struct {
	u64 addr;
	void *func;
	HINSTANCE mod;
	int invoc;

	char enable_str[LV_ENBL_LEN];
	char addr_str[LV_ADDR_LEN];
	char method_str[LV_METHOD_LEN];
	char invoc_str[LV_INVOC_LEN];
} DetourInfo;

typedef struct {
	int row_info_size;
	LVCOLUMNA columns[MAX_COLS];
	int field_offs[MAX_COLS];
	int n_cols;
	int cur_col;
	int sort_dir;
} ListDesc;

// inject.c
DllInfo *refreshDllList(void);
DllInfo *insertDll(char *path);

// main.c
void useProcess(ProcessInfo *proc);

// processes.c
void openProcessDialog(HWND mainWnd);
void closeProcessDialog(void);

// add_detour.c
void openDetourDialog(HWND mainWnd);
void closeDetourDialog(void);

// utils.c
ULONG_PTR enableVisualStyles(void);
void applyNiceFont(HWND hwnd);
void errorMessage(const char *file, int line);
void showVar(const char *name, void *var, int elems);
HWND createLabel(HWND mainWnd, int visible, char *text, int x, int y, int w, int h);
HWND createButton(LONG_PTR id, HWND mainWnd, char *text, int x, int y);
HWND createButtonEx(LONG_PTR id, HWND mainWnd, char *text, int x, int y, int w, int h);
void createColumn(LVCOLUMNA *col, int idx, char *name);
void createTable(HWND listWnd, ListDesc *desc, void *items, int nRows);
void sortListView(HWND listWnd, ListDesc *desc, int col);
void *rowFromEvent(HWND listWnd, NMITEMACTIVATE *item);
HWND createComboBox(LONG_PTR id, HWND mainWnd, int x, int y, int w, int h);

#endif
