#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

typedef unsigned int u32;

int dummyBegin();
void sayHello();
int dummyEnd();

int writeMem(HANDLE proc, void *mem, void *buf, int len) {
	SIZE_T copied = 0;
	int res = WriteProcessMemory(proc, mem, buf, (SIZE_T)len, &copied);

	int success = copied == len;
	if (!success)
		printf("Incomplete copy: %d/%d bytes were written\n");

	return success;
}

int main(int argc, char **argv) {
	if (argc < 2) {
		printf("Invalid arguments\n"
		       "Usage: %s <PID>\n",
		       argv[0]);
		return 0;
	}

	int pid = strtol(argv[1], NULL, 0);

	HANDLE proc = OpenProcess(PROCESS_ALL_ACCESS, 0, pid);
	if (!proc) {
		printf("Could not open process %d\n", pid);
		return 3;
	}

	HMODULE kernel32 = GetModuleHandleA("kernel32");
	if (!kernel32) {
		printf("Could not manually find kernel32\n");
		return 1;
	}

	LPVOID __stdcall (*alloc)(
		HANDLE, LPVOID, SIZE_T, DWORD, DWORD
	) = GetProcAddress(kernel32, "VirtualAllocEx");

	if (!alloc) {
		printf("Could not retrieve VirtualAllocEx\n");
		return 2;
	}

	void *addr = (void*)0x20000000;
	int size = 0x1000;

	void *mem = alloc(
		proc,
		addr,
		size,
		MEM_COMMIT | MEM_RESERVE,
		PAGE_EXECUTE_READWRITE
	);
	if (mem != addr) {
		printf("Could not create virtual memory page\n"
		       "Intended address = %p, returned address = %p\n", addr, mem);
		return 4;
	}

	if (!writeMem(proc, mem, sayHello, dummyEnd - dummyBegin))
		return 5;

	FlushInstructionCache(proc, mem, size);

	BOOL __stdcall (*protect)(
		HANDLE, LPVOID, SIZE_T, DWORD, PDWORD
	) = GetProcAddress(kernel32, "VirtualProtectEx");
	
	if (!protect) {
		printf("Could not retrieve VirtualProtectEx\n");
		CloseHandle(proc);
		return 6;
	}

	DWORD old_prot = 0;
	void *text = (void*)0x401000;
	int text_sz = 0x8b000;

	if (!protect(proc, text, text_sz, PAGE_EXECUTE_READWRITE, &old_prot)) {
		printf("VirtualProtectEx failed\n");
		CloseHandle(proc);
		return 7;
	}

	void *call_addr = (void*)0x419939;
	if (!writeMem(proc, call_addr, "\xe8\xc2\x66\xbe\x1f", 5))
		return 8;

	FlushInstructionCache(proc, text, text_sz);

	CloseHandle(proc);
	return 0;
}