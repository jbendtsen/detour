#include "header.h"
#include <stdlib.h>

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

void createColumn(LVCOLUMNA *col, int idx, char *name) {
	col->iSubItem = idx;
	col->mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	col->pszText = name;
	col->cchTextMax = strlen(name);
	col->cx = 50;
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