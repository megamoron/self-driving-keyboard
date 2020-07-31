#include <iostream>

#include <Windows.h>

#include "sendinput.h"

#ifndef HID_USAGE_PAGE_GENERIC
#define HID_USAGE_PAGE_GENERIC (USHORT)(0x01)
#endif
#ifndef HID_USAGE_GENERIC_KEYBOARD
#define HID_USAGE_GENERIC_KEYBOARD (USHORT)(0x06)
#endif

static LRESULT CALLBACK EventListener(_In_ HWND, _In_ UINT, _In_ WPARAM, _In_ LPARAM);

int main() {
	// create msg window and subscribe to raw input
	HINSTANCE me;
	GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, NULL, (HMODULE*)&me);
	WNDCLASSEXW c = {
		.cbSize = sizeof(WNDCLASSEX),
		.lpfnWndProc = EventListener,
		.hInstance = me,
		.lpszClassName = L"input replay controller",
	};
	ATOM a = RegisterClassExW(&c);
	HWND w = CreateWindowExW((DWORD)0, (LPCWSTR)a, NULL, (DWORD)0, 0, 0, 0, 0, HWND_MESSAGE, NULL, me, NULL);
	if (!w) {
		std::wcerr << L"Creating message-only window failed: " << GetLastError() << std::endl;
		return 1;
	}
	RAWINPUTDEVICE d = {
		.usUsagePage = HID_USAGE_PAGE_GENERIC,
		.usUsage = HID_USAGE_GENERIC_KEYBOARD,
		.dwFlags = RIDEV_INPUTSINK | RIDEV_NOLEGACY,
		.hwndTarget = w,
	};
	if (!RegisterRawInputDevices(&d, 1, sizeof(RAWINPUTDEVICE))) {
		std::wcerr << L"Failed to register for start/stop commands: " << GetLastError() << std::endl;
		return 1;
	}

	// create thread and wait until ready
	HANDLE ready = CreateEventExW(NULL, L"sender ready", CREATE_EVENT_MANUAL_RESET, SYNCHRONIZE);
	HANDLE worker = CreateThread(NULL, 0, ReplaySendInput, NULL, 0, NULL);
	if (!worker) {
		std::wcerr << L"Unable to create sending thread: " << GetLastError() << std::endl;
		return 1;
	}
	WaitForSingleObject(ready, INFINITE);
	CloseHandle(ready);

	// Pump messages
	MSG m = {};
	while (GetMessageW(&m, NULL, 0, 0)) {
		DispatchMessageW(&m);
	}
	
	if (m.wParam) {
		// thread did not finish (error or cancelled by user)
		TerminateThread(worker, (DWORD)m.wParam);
	}
	else {
		WaitForSingleObject(worker, INFINITE);
	}
	DWORD exit;
	GetExitCodeThread(worker, &exit);
	CloseHandle(worker);
	return (int)m.wParam;
}

static LRESULT CALLBACK EventListener(_In_ HWND hwnd, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam) {
	static bool running = false;
	if (uMsg == WM_INPUT) {
		UINT size = sizeof(RAWINPUT);
		RAWINPUT r;
		if (GetRawInputData((HRAWINPUT)lParam, RID_INPUT, (LPVOID)&r, &size, sizeof(RAWINPUTHEADER)) == -1) {
			std::wcerr << L"Failed to get start/stop commands." << std::endl;
			PostQuitMessage(2);
		}
		if (!running && r.data.keyboard.VKey == VK_F12 && r.data.keyboard.Flags & RI_KEY_BREAK) {
			// Start replay
			std::wcerr << L"Starting replay." << std::endl;
			HANDLE start = OpenEventW(EVENT_MODIFY_STATE, FALSE, L"start sending");;
			SetEvent(start);
			CloseHandle(start);
			running = true;
		}
		else if (running && r.data.keyboard.VKey == VK_F12 && !(r.data.keyboard.Flags & RI_KEY_BREAK)) {
			std::wcerr << L"Replay canceled." << std::endl;
			PostQuitMessage(1);
		}
		return 0;
	}
	else if (uMsg == WM_CLOSE) {
		// Worker thread exited
		PostQuitMessage(0);
	}
	return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}
