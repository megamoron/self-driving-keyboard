#pragma once

#include <Windows.h>

// Translates the virtual key code to scancode
WORD vk2scan(WORD vk, BOOL press);