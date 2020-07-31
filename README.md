# self-driving-keyboard
Record and replay keyboard (and mouse) events on Microsoft Windows. The events are recorded using [Raw Input](https://docs.microsoft.com/en-us/windows/win32/inputdev/raw-input) and replayed using [`SendInput`](https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-sendinput).
## Usage
To record keyboard and mouse input to FILE, run:
```
rec.exe [FILE]
```
To replay a recorded file, run:
```
rep.exe [FILE]...
```

**HINT:** You might need to run the these commands as administrator to record/replay for processes with elevated privileges.

## File format
The file is an ordinary text file. Each line is of the form `[DEVICE][TIME]:[INPUT]`, where
```
[DEVICE] is either the character 'M' (for mouse) or 'K' (for keyboard)
[TIME] is a non-negative amount of time to wait before preceeding to the next line (in units of 100 nanoseconds)
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
````
You can find the virtual-key codes for keyboard input on the [Windows Dev Center](https://docs.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes).

Comments start with #.

## Building
###### Visual Studio
As Microsoft Visual Studio is unable to create a solution in an existing directory, the solution/projects need to be created first after which the code can be cloned into the solution directory. Like so:
1. Create a new solution consisting of two projects called `rec` and `rep`. (Make sure "Place solution and project in the same directory" is unticked.)
2. Clone this repository into the solution directory, as described [here](https://stackoverflow.com/questions/2411031/how-do-i-clone-into-a-non-empty-directory).
3. Add to projects the existing files from their respective directories. Also add the files in the root of the repository to both projects. 
4. Append $(SolutionDir) to the include directories in both projects.
