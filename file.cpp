#include <iostream>
#include <cstdio>
#include <vector>
#include <queue>

#include <Windows.h>

#include "file.h"

#include "scancode.h"

extern bool append(HANDLE file, std::vector<INPUT> *input, std::vector<LARGE_INTEGER> *wait) {
	HANDLE mapping = CreateFileMappingW(file, NULL, PAGE_READONLY, 0, 0, NULL);
	if (!mapping) {
		std::wcerr << L"Could not create file mapping object: " << GetLastError() << std::endl;
		return false;
	}
	char* p = (char*)MapViewOfFile(mapping, FILE_MAP_READ, 0, 0, 0);
	if (!p) {
		std::wcerr << L"Could not map view of file mapping object: " << GetLastError() << std::endl;
		CloseHandle(mapping);
		return false;
	}
	// how large of a file did we get?
	MEMORY_BASIC_INFORMATION meminfo;
	LARGE_INTEGER fsize;
	if (VirtualQuery((LPCVOID)p, &meminfo, sizeof(MEMORY_BASIC_INFORMATION)) != sizeof(MEMORY_BASIC_INFORMATION) || !GetFileSizeEx(file, &fsize)) {
		std::wcerr << L"Could not get the size of the mapped region." << std::endl;
		UnmapViewOfFile((LPCVOID)p);
		CloseHandle(mapping);
		return false;
	}
	else if ((LONGLONG) meminfo.RegionSize <= fsize.QuadPart) {
		std::wcerr << L"Could not completely map file." << std::endl;
		UnmapViewOfFile((LPCVOID)p);
		CloseHandle(mapping);
		return false;
	}
	size_t nbytes = (size_t)fsize.QuadPart;
	bool success = false;
	size_t line = 0;
	for (size_t i = 0; i < nbytes;) {
		line++;
		LARGE_INTEGER l;
		INPUT in = {};
		// Read device character
		bool keyboard = false;
		switch (p[i++]) {
		case '#':
			while (i < nbytes && p[i++] != '\n');
			continue;
		case 'K':
			keyboard = true;
		case 'M':
			break;
		default:
			std::wcerr << L"Error in line " << line << L": Unexpected device character." << std::endl;
			goto append_fail;
		}
		// Read amout of time to wait
		if (i >= nbytes || p[i] < '0' || p[i] > '9') {
			std::wcerr << L"Error in line " << line << L": Expected a decimal number specifying time interval after device character." << std::endl;
			goto append_fail;
		}
		uint64_t t = 0;
		while (i < nbytes && p[i] >= '0' && p[i] <= '9') {
			t *= 10;
			t += (uint64_t)p[i++] - (uint64_t)'0';
		}
		l.QuadPart = t;
		// Read ':'
		if (i >= nbytes || p[i++] != ':') {
			std::wcerr << L"Error in line " << line << L": Expected ':' after time interval." << std::endl;
			goto append_fail;
		}
		if (keyboard) {
			// Current line is keyboard input
			WORD vk = 0;
			if (i >= nbytes || (p[i] < '0' || p[i] > '9') && (p[i] < 'a' || p[i] > 'f') && (p[i] < 'A' || p[i] > 'F')) {
				std::wcerr << L"Error in line " << line << L": Expected a hexdecimal number specifying the virtual-key code." << std::endl;
				goto append_fail;
			}
			while (i < nbytes && (p[i] >= '0' && p[i] <= '9' || p[i] >= 'a' && p[i] <= 'f' || p[i] >= 'A' && p[i] <= 'F')) {
				vk *= 16;
				if (p[i] >= '0' && p[i] <= '9') {
					vk += p[i++] - '0';
				}
				else if (p[i] >= 'a' && p[i] <= 'f') {
					vk += p[i++] - 'a' + 10;
				}
				else if (p[i] >= 'A' && p[i] <= 'F') {
					vk += p[i++] - 'A' + 10;
				}
			}
			if (vk < 1 || vk > 254) {
				std::wcerr << L"Error in line " << line << L": virtual-key code must be in the range 1 to 254." << std::endl;
				goto append_fail;
			}
			bool press = false;
			if (i >= nbytes || (p[i] != 'P' && p[i] != 'R')) {
				std::wcerr << L"Error in line " << line << L": Expected key state indicator." << std::endl;
				goto append_fail;
			}
			else if (p[i] == 'P') {
				press = true;
			}
			if (++i < nbytes && p[i] == ';') {
				// eat any trailing semicolon
				i++;
			}
			WORD scancode = vk2scan(vk, press) & (WORD)0xFF7F; // ignore key release bit

			if (vk == VK_F12 && press) {
				std::wcerr << L"Warning in line " << line << L": F12 will interfere and cancel the replay." << std::endl;
				goto append_fail;
			}
			in.type = INPUT_KEYBOARD;
			in.ki = {
				.wVk = vk,
				.wScan = scancode,
				.dwFlags = (press ? (DWORD)0 : (DWORD)KEYEVENTF_KEYUP) | (scancode & (WORD)0xE000 ? (DWORD)KEYEVENTF_EXTENDEDKEY : (DWORD)0),
				.time = 0,
				.dwExtraInfo = 0,
			};
		}
		else {
			// current line is mouse input
			LONG x = 0;
			LONG y = 0;
			DWORD wheel = 0;
			DWORD xbutflag = 0;
			DWORD flags = 0;
			while (i < nbytes && ((p[i] >= '0' && p[i] <= '9') || p[i] == '-')) {
				// read one of the semicolon-seperated tokens
				DWORD sign = 1;
				if (i < nbytes && p[i] == '-') {
					sign = -1;
					if (++i >= nbytes || p[i] < '0' || p[i] > '9') {
						std::wcerr << L"Error in line " << line << L": Expected a number after minus sign." << std::endl;
						goto append_fail;
					}
				}
				DWORD number = 0;
				while (i < nbytes && (p[i] >= '0' && p[i] <= '9')) {
					number *= 10;
					number += p[i++] - '0';
				}
				number *= sign;
				if (i < nbytes && p[i] == 'P') {
					// button press
					switch (number) {
					case 1:
						flags |= MOUSEEVENTF_LEFTDOWN;
						break;
					case 2:
						flags |= MOUSEEVENTF_RIGHTDOWN;
						break;
					case 3:
						flags |= MOUSEEVENTF_MIDDLEDOWN;
						break;
					case 4:
						flags |= MOUSEEVENTF_XDOWN;
						xbutflag |= XBUTTON1;
						break;
					case 5:
						flags |= MOUSEEVENTF_XDOWN;
						xbutflag |= XBUTTON2;
						break;
					default:
						std::wcerr << L"Error in line " << line << L": Invalid button number." << std::endl;
						goto append_fail;
					}
					i++; // eat 'P'
				}
				else if (i < nbytes && p[i] == 'R') {
					// button release
					switch (number) {
					case 1:
						flags |= MOUSEEVENTF_LEFTUP;
						break;
					case 2:
						flags |= MOUSEEVENTF_RIGHTUP;
						break;
					case 3:
						flags |= MOUSEEVENTF_MIDDLEUP;
						break;
					case 4:
						flags |= MOUSEEVENTF_XUP;
						xbutflag |= XBUTTON1;
						break;
					case 5:
						flags |= MOUSEEVENTF_XUP;
						xbutflag |= XBUTTON2;
						break;
					default:
						std::wcerr << L"Error in line " << line << L": Invalid button number." << std::endl;
						goto append_fail;
					}
					i++; // eat 'R'
				}
				else if (i < nbytes && p[i] == ',') {
					// mouse move
					i++; // eat ','
					x = number;
					// read another number
					sign = 1;
					if (i < nbytes && p[i] == '-') {
						sign = -1;
						if (++i >= nbytes || p[i] < '0' || p[i] > '9') {
							std::wcerr << L"Error in line " << line << L": Expected a number after minus sign." << std::endl;
							goto append_fail;
						}
					}
					number = 0;
					while (i < nbytes && (p[i] >= '0' && p[i] <= '9')) {
						number *= 10;
						number += p[i++] - '0';
					}
					number *= sign;
					y = number;
					flags |= MOUSEEVENTF_MOVE | MOUSEEVENTF_MOVE_NOCOALESCE;
				}
				else {
					// must be wheel delta
					wheel = number;
					flags |= MOUSEEVENTF_WHEEL;
				}
				// possibly consume a semicolon
				if (i < nbytes && p[i] == ';') {
					i++;
				}
			}
			if (wheel && xbutflag) {
				std::wcerr << L"Error in line " << line << L": Wheel delta and X buttons cannot be specified together." << std::endl;
				goto append_fail;
			}
			if (flags & MOUSEEVENTF_XDOWN && flags & MOUSEEVENTF_XUP) {
				std::wcerr << L"Warning in line " << line << L": State of X buttons has become ambiguous." << std::endl;
			}
			in.type = INPUT_MOUSE;
			in.mi = {
				.dx = x,
				.dy = y,
				.mouseData = wheel | xbutflag,
				.dwFlags = flags,
				.time = 0,
				.dwExtraInfo = 0,
			};
		}
		// consume the rest of the current line
		while (i < nbytes && (p[i] == ' ' || p[i] == '\t' || p[i] == '\r')) {
			// eat spaces
			i++;
		}
		if (i < nbytes && p[i] == '\n') {
			// eat newline
			i++;
		}
		else if (i < nbytes && p[i] == '#') {
			// eat comment (and newline)
			while (i < nbytes && p[i++] != '\n');
		}
		else if (i < nbytes) {
			// error if we did not arrive at newline
			std::wcerr << L"Error in line " << line << L": Exraneous character encountered." << std::endl;
			goto append_fail;
		}
		// Append the input form the current line.
		input->push_back(in);
		wait->push_back(l);
	}
	success = true;
append_fail:
	UnmapViewOfFile((LPCVOID)p);
	CloseHandle(mapping);
	return success;
}

extern bool write(HANDLE file, std::queue<RAWINPUT>* in, std::queue<LARGE_INTEGER>* qpc) {
	LARGE_INTEGER freq;
	QueryPerformanceFrequency(&freq);
	while (!in->empty()) {
		char buf[104]; // (theoretically) max size is 104 (including '\0')
		int i = 0; // current offset in the buffer

		RAWINPUT& input = in->front();
		LARGE_INTEGER now = qpc->front();
		qpc->pop();
		LARGE_INTEGER& next = qpc->front();
		long long t = (long long)((next.QuadPart - now.QuadPart) * (LONGLONG)10000000ll / freq.QuadPart);
		if (input.header.dwType & RIM_TYPEKEYBOARD) {
			// keyboard
			i += snprintf(&buf[i], 104 - i, "K%lld:", t);
			i += snprintf(&buf[i], 104 - i, "%02hX", (unsigned short)input.data.keyboard.VKey);
			if (input.data.keyboard.Flags & RI_KEY_BREAK) {
				// key release
				i += snprintf(&buf[i], 104 - i, "R");
			}
			else {
				// key press
				i += snprintf(&buf[i], 104 - i, "P");
			}
		}
		else {
			// mouse
			i += snprintf(&buf[i], 104 - i, "M%lld:", t);
			if (input.data.mouse.usFlags & MOUSE_MOVE_ABSOLUTE) {
				std::wcerr << L"Error: Cannot handle absolute mouse movements." << std::endl;
				goto write_fail;
			}
			if (input.data.mouse.lLastX != 0l || input.data.mouse.lLastY != 0l) {
				i += snprintf(&buf[i], 104 - i, "%ld,%ld;", (long)input.data.mouse.lLastX, (long)input.data.mouse.lLastY);
			}
			if (input.data.mouse.usButtonFlags & RI_MOUSE_WHEEL) {
				i += snprintf(&buf[i], 104 - i, "%hd;", (short)input.data.mouse.usButtonData);
			}
			if (input.data.mouse.usButtonFlags & RI_MOUSE_BUTTON_1_DOWN) {
				i += snprintf(&buf[i], 104 - i, "1P;");
			}
			if (input.data.mouse.usButtonFlags & RI_MOUSE_BUTTON_1_UP) {
				i += snprintf(&buf[i], 104 - i, "1R;");
			}
			if (input.data.mouse.usButtonFlags & RI_MOUSE_BUTTON_2_DOWN) {
				i += snprintf(&buf[i], 104 - i, "2P;");
			}
			if (input.data.mouse.usButtonFlags & RI_MOUSE_BUTTON_2_UP) {
				i += snprintf(&buf[i], 104 - i, "2R;");
			}
			if (input.data.mouse.usButtonFlags & RI_MOUSE_BUTTON_3_DOWN) {
				i += snprintf(&buf[i], 104 - i, "3P;");
			}
			if (input.data.mouse.usButtonFlags & RI_MOUSE_BUTTON_3_UP) {
				i += snprintf(&buf[i], 104 - i, "3R;");
			}
			if (input.data.mouse.usButtonFlags & RI_MOUSE_BUTTON_4_DOWN) {
				i += snprintf(&buf[i], 104 - i, "4P;");
			}
			if (input.data.mouse.usButtonFlags & RI_MOUSE_BUTTON_4_UP) {
				i += snprintf(&buf[i], 104 - i, "4R;");
			}
			if (input.data.mouse.usButtonFlags & RI_MOUSE_BUTTON_5_DOWN) {
				i += snprintf(&buf[i], 104 - i, "5P;");
			}
			if (input.data.mouse.usButtonFlags & RI_MOUSE_BUTTON_5_UP) {
				i += snprintf(&buf[i], 104 - i, "5R;");
			}
		}
		i += snprintf(&buf[i], 104 - i, "\r\n");

		DWORD written;
		if (!WriteFile(file, (LPVOID)buf, i, &written, NULL) || written != i) {
			std::wcerr << L"Error: Cannot handle absolute mouse movements." << std::endl;
			goto write_fail;
		}

		// Remove processed elements
		in->pop();
		// qpc already popped to reveal next timestamp
	}
	qpc->pop(); // remove the excess element in qpc
	return true;
write_fail:
	while (!in->empty()) {
		in->pop();
	}
	while (!qpc->empty()) {
		qpc->pop();
	}
	return false;
}