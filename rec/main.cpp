#include <iostream>

#include <Windows.h>

#include "rawinput.h"

int main() {
	// open file
	int argc;
	wchar_t** argv = CommandLineToArgvW(GetCommandLineW(), &argc);
	if (argc == 1) {
		std::wcerr << L"Usage:\n" << argv[0] << L" [FILE]\n";
		LocalFree(argv);
		return 1;
	}
	else if (argc != 2) {
		std::wcerr << L"Output needs to be written to a single file." << std::endl;
		LocalFree(argv);
		return 1;
	}
	HANDLE file = CreateFileW(argv[1], GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (!file) {
		std::wcerr << L"Cannot open file " << argv[1] << ": " << GetLastError() << std::endl;
		LocalFree(argv);
		return 1;
	}
	LocalFree(argv);

	CaptureRawInput(file);

	CloseHandle(file);
	return 0;
}