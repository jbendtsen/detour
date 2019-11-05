#ifndef HEADER_H
#define HEADER_H

#include <windows.h>
#include <commctrl.h>
#include <psapi.h>

#define WRAP(x) x
#define FIELD_OF(x, y) x.y
#define FIELD_BYTEPOS(templ, field) (u64)&FIELD_OF(templ, field) - (u64)&WRAP(templ)

#define BTN_WIDTH 60
#define BTN_HEIGHT 28

#define MAX_COLS 10

typedef unsigned long long int u64;

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

void applyNiceFont(HWND hwnd);
void errorMessage(const char *file, int line);
void useProcess(ProcessInfo *proc);

void openProcessDialog(HWND mainWnd);
void closeProcessDialog(void);

void createColumn(LVCOLUMNA *col, int idx, char *name);
void createTable(HWND listWnd, ListDesc *desc, void *items, int nRows);
void sortListView(HWND listWnd, ListDesc *desc, int col);
void *rowFromEvent(HWND listWnd, NMITEMACTIVATE *item);

#endif
