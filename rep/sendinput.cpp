#include <iostream>
#include <vector>

#include <Windows.h>

#include "file.h"

std::vector<INPUT> in;
std::vector<LARGE_INTEGER> wait;

static bool setup(); // reads args and sets up the vector in and the vector wait
static bool mainloop();

DWORD WINAPI ReplaySendInput(_In_ LPVOID lpParameter) {
	DWORD ret = -1;
	HWND w = FindWindowExW(NULL, NULL, L"input replay controller", NULL);
	HANDLE ready = OpenEventW(EVENT_MODIFY_STATE, FALSE, L"sender ready");

	// Setup the data structures
	if (!setup()) {
		SetEvent(ready);
		goto fail;
	} 
	if (!in.size()) {
		// nothing to replay
		ret = 0;
		SetEvent(ready);
		goto fail;
	}
	std::wcerr << L"Press F12 to start/cancel replay." << std::endl;
	
	// Signal done and sleep untill start message
	{
		HANDLE start = CreateEventExW(NULL, L"start sending", CREATE_EVENT_MANUAL_RESET, SYNCHRONIZE);
		SetEvent(ready);
		WaitForSingleObject(start, INFINITE);
		CloseHandle(start);
	}

	// run the main loop
	ret = !mainloop();
	
fail:
	CloseHandle(ready);
	PostMessageW(w, WM_CLOSE, 0, 0);
	return ret;
}


static bool setup() {
	// open files
	int argc;
	wchar_t** argv = CommandLineToArgvW(GetCommandLineW(), &argc);
	if (argc == 1) {
		std::wcerr << L"Usage:\n" << argv[0] << L" [FILE]...\n";
		LocalFree(argv);
		return true;
	}
	HANDLE* file = new HANDLE[argc - (size_t)1];
	uint64_t tsize = 0ll;
	for (int i = 1; i < argc; i++) {
		file[i - 1] = CreateFileW(argv[i], GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
		if (file[i - 1] == INVALID_HANDLE_VALUE) {
			std::wcerr << L"Cannot open file " << argv[i] << ": " << GetLastError() << std::endl;
			for (int j = 1; j < i; j++) {
				CloseHandle(file[j - 1]);
			}
			delete[] file;
			LocalFree(argv);
			return false;
		}
		LARGE_INTEGER l;
		GetFileSizeEx(file[i - 1], &l);
		if (!l.QuadPart) {
			// file empty (cannot map empty files)
			CloseHandle(file[i - 1]);
			file[i - 1] = NULL;
		}
		tsize += l.QuadPart;
	}

	// build datastructure
	// on average: approx 13 bytes on one line
	in.reserve((size_t)(tsize / 13ll));
	wait.reserve((size_t)(tsize / 13ll));
	for (int i = 1; i < argc; i++) {
		std::wcerr << argv[i] << L": ";
		if (!file[i - 1]) {
			std::wcerr << L"Empty." << std::endl;
			continue;
		}
		if (!append(file[i - 1], &in, &wait)) {
			for (int j = i; j < argc; j++) {
				CloseHandle(file[j - 1]);
			}
			delete[] file;
			LocalFree(argv);
			return false;
		}
		CloseHandle(file[i - 1]);
		std::wcerr << L"Ok." << std::endl;
	}
	delete[] file;
	LocalFree(argv);
	return true;
}


static bool mainloop() {
	size_t vec_size = in.size();
	LARGE_INTEGER freq;
	QueryPerformanceFrequency(&freq);
	HANDLE timer = CreateWaitableTimerExW(NULL, NULL, 0, TIMER_MODIFY_STATE | SYNCHRONIZE);
	LARGE_INTEGER base_ft, base_qpc;
	GetSystemTimePreciseAsFileTime((LPFILETIME)&base_ft);
	QueryPerformanceCounter(&base_qpc);
	LONGLONG elapsed = 0;
	for (size_t i = 0; i < vec_size; i++) {
		// Send Input
		if (!SendInput(1, &in[i], sizeof(INPUT))) {
			std::wcerr << L"Sending input failed: " << GetLastError() << std::endl;
			CloseHandle(timer);
			return false;
		}
		// Prepare timestamps
		elapsed += wait[i].QuadPart;
		LARGE_INTEGER next_approx = { .QuadPart = base_ft.QuadPart + elapsed - 10000ll }; // wakeup takes approx <= 1ms.
		LARGE_INTEGER next = { .QuadPart = base_qpc.QuadPart + (elapsed * freq.QuadPart) / 10000000ll };
		// Wait
		LARGE_INTEGER now;
		QueryPerformanceCounter(&now);
		if (now.QuadPart + freq.QuadPart / 1000 < next.QuadPart) {
			// more time left than 1ms.
			SetWaitableTimer(timer, &next_approx, 0, NULL, NULL, false);
			WaitForSingleObject(timer, INFINITE);
			QueryPerformanceCounter(&now);
		}
		while (now.QuadPart < next.QuadPart) {
			// busy wait
			QueryPerformanceCounter(&now);
		}
	}
	CloseHandle(timer);
	return true;
}