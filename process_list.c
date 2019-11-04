#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "header.h"

#define WIDTH  300
#define HEIGHT 300

#define BTN_WIDTH 60
#define BTN_HEIGHT 28

#define IDC_OPENBTN   190
#define IDC_CANCELBTN 191

#define IDC_LISTVIEW 200

#define MAX_PROCS 1024

char *className = "openProcDlg";

ProcessInfo *processes = NULL;
ProcessInfo *selectedProc = NULL;

HWND procWnd = NULL, listWnd = NULL;
HWND openBtn = NULL, cancelBtn = NULL;
WNDCLASSEXA procWc = {0};

void getProcesses() {
	DWORD len = 0;
	DWORD *pid_list = malloc(MAX_PROCS * sizeof(DWORD));
	EnumProcesses(pid_list, MAX_PROCS, &len);

	processes = calloc(len, sizeof(ProcessInfo));
	int idx = 0;

	const int proc_path_len = 0x1000;
	char *proc_path = malloc(proc_path_len);

	for (int i = 0; i < len; i++) {
		// Many of the PIDs provided by EnumProcesses can't be opened, so we filter them out here
		int pid = pid_list[i];
		processes[i].pid = pid;
		HANDLE proc = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, 0, pid);
		if (!proc)
			continue;

		LVITEMA cell = {0};
		cell.iItem = idx++;                  // row index
		cell.mask = LVIF_TEXT | LVIF_PARAM;  // enables strings and the ability to sort
		cell.lParam = (LPARAM)&processes[i]; // this is what ties a listview item to its sorting information

		cell.pszText = processes[i].pid_str; // the first cell will contain a string version of a PID
		cell.cchTextMax = snprintf(cell.pszText, PI_PID_LEN, "%d", pid);

		SendMessage(listWnd, LVM_INSERTITEM, 0, (LPARAM)&cell);

		// At this point, 'cell' still contains its previous values.
		cell.iSubItem = 1;                // column index
		cell.mask = LVIF_TEXT;            // we disable LVIF_PARAM as it seems it should only be used one per row
		cell.pszText = processes[i].name; // name of the process
		cell.cchTextMax = PI_NAME_LEN;

		// gets us the full name of the current process
		if (!GetProcessImageFileNameA(proc, proc_path, proc_path_len))
			errorMessage(__FILE__, __LINE__);

		CloseHandle(proc);

		// The name in 'proc_path' is currently represented as a path to the file,
		//  so we find the last backslash and use the string from that point
		char *p = strrchr(proc_path, '\\');
		if (!p)
			p = proc_path;
		else
			p++;

		strncpy(cell.pszText, p, PI_NAME_LEN-1);
		SendMessage(listWnd, LVM_SETITEM, 0, (LPARAM)&cell);
	}

	free(proc_path);
	free(pid_list);
}

#define SORT_COLUMN 1
#define SORT_PIDS   0
#define SORT_NAMES  1
#define SORT_DIR   2
#define SORT_ASC   0
#define SORT_DESC  2

int CALLBACK sortItems(LPARAM lp1, LPARAM lp2, LPARAM type) {
	ProcessInfo *item1 = (ProcessInfo*)lp1;
	ProcessInfo *item2 = (ProcessInfo*)lp2;

	int diff = 0;
	if ((type & SORT_COLUMN) == SORT_NAMES) {
		diff = strcmp(item1->name, item2->name);
	}
	else {
		diff = item1->pid - item2->pid;
	}

	if ((type & SORT_DIR) == SORT_DESC)
		diff = -diff;

	return diff;
}

int sortDir[2] = {SORT_DESC};

LRESULT CALLBACK openProcWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	int res = 0;

	switch (uMsg) {
		case WM_CREATE:
		{
			HINSTANCE inst = GetModuleHandle(NULL);

			openBtn = CreateWindowA(
				"BUTTON", "Open",
				WS_VISIBLE | WS_CHILD,
				10, 10, BTN_WIDTH, BTN_HEIGHT,
				hwnd, (HMENU)IDC_OPENBTN,
				inst, NULL
			);
			applyNiceFont(openBtn);
			EnableWindow(openBtn, 0);

			cancelBtn = CreateWindowA(
				"BUTTON", "Cancel",
				WS_VISIBLE | WS_CHILD,
				10, 10, BTN_WIDTH, BTN_HEIGHT,
				hwnd, (HMENU)IDC_CANCELBTN,
				inst, NULL
			);
			applyNiceFont(cancelBtn);

			listWnd = CreateWindowA(
				WC_LISTVIEW, "",
				WS_VISIBLE | WS_CHILD | LVS_REPORT,
				0, 0, WIDTH, HEIGHT,
				hwnd, (HMENU)IDC_LISTVIEW,
				inst, NULL
			);
			applyNiceFont(listWnd);
			SendMessage(
				listWnd,
			   LVM_SETEXTENDEDLISTVIEWSTYLE,
			   LVS_EX_FULLROWSELECT,
			   LVS_EX_FULLROWSELECT
			);

			insertLVColumn(listWnd, 0, "PID", LVCFMT_RIGHT, 50);
			insertLVColumn(listWnd, 1, "Name", LVCFMT_LEFT, 150);

			getProcesses();
			break;
		}
		case WM_SIZE:
		{
			RECT r = {0};
			GetClientRect(hwnd, &r);

			const int bdr = 5;
			int listW = r.right - bdr*2;
			int listH = r.bottom - bdr*4 - BTN_HEIGHT;

			int btnY = listH + bdr * 3;
			int cancelX = listW - BTN_WIDTH;
			int openX = cancelX - BTN_WIDTH - bdr;

			SetWindowPos(openBtn, NULL, openX, btnY, BTN_WIDTH, BTN_HEIGHT, SWP_NOZORDER);
			SetWindowPos(cancelBtn, NULL, cancelX, btnY, BTN_WIDTH, BTN_HEIGHT, SWP_NOZORDER);

			SetWindowPos(listWnd, NULL, bdr, bdr, listW, listH, SWP_NOZORDER);

			int pidW = SendMessage(listWnd, LVM_GETCOLUMNWIDTH, 0, 0);
			SendMessage(listWnd, LVM_SETCOLUMNWIDTH, 1, listW - pidW - 20);
			break;
		}
		case WM_COMMAND:
		{
			int id = wParam & 0xffff;
			if (id == IDC_OPENBTN && selectedProc)
				useProcess(selectedProc);

			if (id == IDC_CANCELBTN || id == IDC_OPENBTN)
				closeProcessDialog();

			break;
		}
		case WM_NOTIFY:
		{
			if (wParam != IDC_LISTVIEW)
				return DefWindowProc(hwnd, uMsg, wParam, lParam);

			int code = ((NMHDR*)lParam)->code;
			switch (code) {
				case NM_CLICK:
				{
					int row = ((NMITEMACTIVATE*)lParam)->iItem;

					LVITEMA item = {0};
					item.iItem = row;
					item.mask = LVIF_TEXT | LVIF_PARAM;
					SendMessage(listWnd, LVM_GETITEM, 0, (LPARAM)&item);

					selectedProc = (ProcessInfo*)item.lParam;
					EnableWindow(openBtn, 1);
					break;
				}
				case LVN_COLUMNCLICK:
				{
					NMLISTVIEW *item = (NMLISTVIEW*)lParam;
					int dir = (sortDir[item->iSubItem] ^= SORT_DIR); // flip the sorting direction
					int type = dir | (item->iSubItem == 1);          // combine with the column (as a bool)

					SendMessage(listWnd, LVM_SORTITEMS, type, (LPARAM)sortItems);
					break;
				}
			}

			break;
		}
		case WM_CLOSE:
			procWnd = NULL;
			DestroyWindow(listWnd);
			DestroyWindow(hwnd);
			break;
		default:
			res = DefWindowProc(hwnd, uMsg, wParam, lParam);
	}

	return res;
}

void openProcessDialog(HWND mainWnd) {
	if (procWnd)
		return;

	HINSTANCE thisMod = GetModuleHandle(NULL);

	if (!procWc.cbSize) {
		procWc.cbSize        = sizeof(WNDCLASSEXA);
		procWc.style         = CS_GLOBALCLASS | CS_HREDRAW | CS_VREDRAW;
		procWc.lpfnWndProc   = openProcWndProc;
		procWc.hInstance     = thisMod;
		procWc.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
		procWc.hCursor       = LoadCursor(NULL, IDC_ARROW);
		procWc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
		procWc.lpszMenuName  = NULL;
		procWc.lpszClassName = className;
		procWc.hIconSm       = LoadIcon(NULL, IDI_APPLICATION);

		RegisterClassExA(&procWc);
	}

	procWnd = CreateWindowA(
		/*WS_EX_CLIENTEDGE,*/
		className, "Open Process",
		WS_OVERLAPPEDWINDOW | WS_VISIBLE,
		CW_USEDEFAULT, CW_USEDEFAULT, WIDTH, HEIGHT,
		mainWnd, NULL, thisMod, NULL
	);

	ShowWindow(procWnd, SW_SHOW);
	UpdateWindow(procWnd);
}

void closeProcessDialog() {
	if (processes) {
		free(processes);
		processes = NULL;
	}
	if (procWnd)
		SendMessage(procWnd, WM_CLOSE, 0, 0);
}