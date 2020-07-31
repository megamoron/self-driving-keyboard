#pragma once
#include <vector>
#include <queue>

#include <Windows.h>

/*
FILE FORMAT: Each line is of the form "[DEVICE][TIME]:[INPUT]", where

[DEVICE] is either the character 'M' (for mouse) or 'K' (for keyboard)
[TIME] is a non-negative amount of time to wait before preceeding after the command has been issued (in units of 100 nanoseconds)
[INPUT] is the input to send:
    (*) For keyboard input, [INPUT] must be either "[??]P" (press) or "[??]R" (release), where [??] is the virtual-key code in hex.
    (*) For mouse input, [INPUT] must be a semicolon seperated list of the following tokens:
        (-) 1P (press primary mouse button)
        (-) 1R (release primary mouse button)
        (-) 2P (press secondary mouse button)
        (-) 2R (release secondary mouse button)
        (-) 3P (press middle mouse button)
        (-) 3R (release middle mouse button)
        (-) 4P (press XBUTTON1)
        (-) 4R (release XBUTTON1)
        (-) 5P (press XBUTTON2)
        (-) 5R (release XBUTTON2)
        (-) [X],[Y] (where [X] and [Y] are decimal numbers specifying relative mouse movement (can be negative))
        (-) [D] (where [D] is a decimal number specifying the wheel delta (can be negative))

You can find the virtual-key codes for keyboard input here:
https://docs.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes

Comments start with #.
*/

/*
Reads file (in the above format) and builds the two vectors:
    input: argument to be passed to SendInput
    wait: number of time units (100ns) to wait after the corresponding call to SendInput
*/
extern bool append(HANDLE file, std::vector<INPUT>* input, std::vector<LARGE_INTEGER>* wait);

/*
Writes the captured input data into file (of the above format) and empties the queues:
    input: events obtained by raw input
    qpc: processor specific timestamps of the events (obtained by QueryPerformanceCounter)
Note: the time interval elapesed after the i-th input is the difference of the i-th and i+1-th timestamp.
Hence, the queue qpc must be one element larger in length.
*/
extern bool write(HANDLE file, std::queue<RAWINPUT>* input, std::queue<LARGE_INTEGER>* qpc);