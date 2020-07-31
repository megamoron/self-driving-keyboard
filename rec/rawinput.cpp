#include <iostream>
#include <queue>

#include <Windows.h>

#include "file.h"

#ifndef HID_USAGE_PAGE_GENERIC
#define HID_USAGE_PAGE_GENERIC (USHORT)(0x01)
#endif
#ifndef HID_USAGE_GENERIC_MOUSE
#define HID_USAGE_GENERIC_MOUSE (USHORT)(0x02)
#endif
#ifndef HID_USAGE_GENERIC_KEYBOARD
#define HID_USAGE_GENERIC_KEYBOARD (USHORT)(0x06)
#endif

static LRESULT CALLBACK PreRecordProc(_In_ HWND, _In_ UINT, _In_ WPARAM, _In_ LPARAM);
static LRESULT CALLBACK RecordProc(_In_ HWND, _In_ UINT, _In_ WPARAM, _In_ LPARAM);

static std::queue<RAWINPUT> input;
static std::queue<LARGE_INTEGER> qpc;

bool CaptureRawInput(HANDLE file) {
	// create window
	HINSTANCE me;
	GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, NULL, (HINSTANCE*)&me);
	WNDCLASSEXW wc = {
		.cbSize = sizeof(WNDCLASSEXW),
		.lpfnWndProc = PreRecordProc,
		.hInstance = me,
		.lpszClassName = L"raw input listener",
	};
	ATOM a = RegisterClassExW(&wc);
	HWND w = CreateWindowExW(0, (LPCWSTR)a, NULL, (DWORD)0, 0, 0, 0, 0, HWND_MESSAGE, NULL, me, NULL);
	if (!w) {
		std::wcerr << L"Creating message-only window failed: " << GetLastError() << std::endl;
		return false;
	}

	// subscribe to raw input
	RAWINPUTDEVICE dev[]{
		{
			.usUsagePage = HID_USAGE_PAGE_GENERIC,
			.usUsage = HID_USAGE_GENERIC_MOUSE,
			.dwFlags = RIDEV_NOLEGACY | RIDEV_INPUTSINK,
			.hwndTarget = w,
		},
		{
			.usUsagePage = HID_USAGE_PAGE_GENERIC,
			.usUsage = HID_USAGE_GENERIC_KEYBOARD,
			.dwFlags = RIDEV_NOLEGACY | RIDEV_INPUTSINK,
			.hwndTarget = w,
		},
	};
	if (!RegisterRawInputDevices(dev, 2, sizeof(RAWINPUTDEVICE))) {
		std::wcerr << L"Registering for raw input failed: " << GetLastError() << std::endl;
		return false;
	}

	// pump messages
	std::wcerr << L"Press F12 to start or stop capturing." << std::endl;
	MSG m = {};
	while (GetMessageW(&m, NULL, 0, 0)) {
		DispatchMessageW(&m);
	}
	if (m.wParam) {
		// There was an error
		return false;
	}

	// data to file
	return write(file, &input, &qpc);
}

static LRESULT CALLBACK PreRecordProc(_In_ HWND hwnd, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam) {
	if (uMsg == WM_INPUT) {
		UINT size = sizeof(RAWINPUT);
		RAWINPUT r;
		UINT bytes = GetRawInputData((HRAWINPUT)lParam, RID_INPUT, (LPVOID)&r, &size, sizeof(RAWINPUTHEADER));
		// Start recording?
		if (r.header.dwType == RIM_TYPEKEYBOARD && r.data.keyboard.VKey == VK_F12 && r.data.keyboard.Flags & RI_KEY_BREAK) {
			// Replace window procedure with RecordProc
			if (SetWindowLongPtrW(hwnd, GWLP_WNDPROC, (LONG_PTR)RecordProc) != (LONG_PTR) PreRecordProc) {
				std::wcerr << L"Unable to register recording procedure: " << GetLastError() << std::endl;
				PostQuitMessage(-1);
			}
		}
		return 0;
	}
	return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}
static LRESULT CALLBACK RecordProc(_In_ HWND hwnd, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam) {
	if (uMsg == WM_INPUT) {
		// push timestamp
		LARGE_INTEGER l;
		QueryPerformanceCounter(&l);

		// push event (if not stop signal)
		// TODO: is a buffered read (GetRawInputBuffer) faster/more accurate?
		UINT size = sizeof(RAWINPUT);
		RAWINPUT r;
		UINT bytes = GetRawInputData((HRAWINPUT)lParam, RID_INPUT, (LPVOID)&r, &size, sizeof(RAWINPUTHEADER));
		if (r.header.dwType == RIM_TYPEKEYBOARD && r.data.keyboard.VKey == 0xFF) {
			// Ignore this (invalid?) key
			return S_OK;
		}
		if (r.header.dwType == RIM_TYPEKEYBOARD && r.data.keyboard.VKey == VK_F12 && !(r.data.keyboard.Flags & RI_KEY_BREAK)) {
			// Replace window procedure with DefWindowProcW (prevents further pending messages from being recorded)
			qpc.push(l);
			if (SetWindowLongPtrW(hwnd, GWLP_WNDPROC, (LONG_PTR)DefWindowProcW) != (LONG_PTR)RecordProc) {
				std::wcerr << L"Unable to deregister recording procedure: " << GetLastError() << std::endl;
				PostQuitMessage(-1);
			}
			else {
				PostQuitMessage(0);
			}
		}
		else {
			qpc.push(l);
			input.push(r);
		}
	}
	return S_OK;
}