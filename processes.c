#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "header.h"

#define WIDTH  300
#define HEIGHT 300

#define IDC_OPENBTN   190
#define IDC_CANCELBTN 191

#define IDC_LISTVIEW 200

#define MAX_PROCS 1024

char *className = "openProcDlg";

ListDesc format = {0};

ProcessInfo *processes = NULL;
ProcessInfo *selectedProc = NULL;

HWND procWnd = NULL, listWnd = NULL;
HWND openBtn = NULL, cancelBtn = NULL;
WNDCLASSEXA procWc = {0};

int getProcesses() {
	DWORD len = 0;
	DWORD *pid_list = malloc(MAX_PROCS * sizeof(DWORD));
	EnumProcesses(pid_list, MAX_PROCS, &len);

	processes = calloc(len, sizeof(ProcessInfo));

	const int proc_path_len = 0x1000;
	char *proc_path = malloc(proc_path_len);

	int idx = 0;
	for (int i = 0; i < len; i++) {
		// Many of the PIDs provided by EnumProcesses can't be opened, so we filter them out here
		int pid = pid_list[i];
		HANDLE proc = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, 0, pid);
		if (!proc)
			continue;

		// gets us the full name of the current process
		int res = GetProcessImageFileNameA(proc, proc_path, proc_path_len);
		CloseHandle(proc);

		if (!res) {
			errorMessage(__FILE__, __LINE__);
			continue;
		}

		// The name in 'proc_path' is currently represented as a path to the file,
		//  so we find the last backslash and use the string from that point
		char *p = strrchr(proc_path, '\\');
		if (!p)
			p = proc_path;
		else
			p++;

		processes[idx].pid = pid;
		snprintf(processes[idx].pid_str, PI_PID_LEN, "%d", pid);
		strncpy(processes[idx].name, p, PI_NAME_LEN-1);
		idx++;
	}

	free(proc_path);
	free(pid_list);

	return idx;
}

void setupListView(int nRows) {
	format.row_info_size = sizeof(ProcessInfo);
	format.n_cols = 2;

	ProcessInfo template = {0};
	format.field_offs[0] = FIELD_BYTEPOS(template, pid_str);
	format.field_offs[1] = FIELD_BYTEPOS(template, name);

	createColumn(&format.columns[0], 0, "PID");
	createColumn(&format.columns[1], 1, "Name");
	createTable(listWnd, &format, processes, nRows);
}

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

			int nRows = getProcesses();
			setupListView(nRows);
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
					selectedProc = rowFromEvent(listWnd, (NMITEMACTIVATE*)lParam);
					EnableWindow(openBtn, 1);
					break;
				}
				case NM_DBLCLK:
					if (selectedProc) {
						useProcess(selectedProc);
						closeProcessDialog();
					}
					break;
				case LVN_COLUMNCLICK:
				{
					int col = ((NMLISTVIEW*)lParam)->iSubItem;
					sortListView(listWnd, &format, col);
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
