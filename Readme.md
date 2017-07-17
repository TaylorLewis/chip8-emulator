# Chip8 Emulator

![Brix](/docs/screenshots/BRIX.gif)

I wrote this as a stepping stone to programming a full-fledged NES emulator, as many in the emulation scene do. I hope this is a useful example to anyone else writing their first emulator.

## Building

This project requires **[SFML](https://www.sfml-dev.org/)**, and uses **[CMake](https://cmake.org/)** to build. A **C++11** compliant compiler is also required to build.

### Windows

*This example uses Visual Studio.*

1. Install **[CMake](https://cmake.org/download/)**.
2. Download **[SFML](https://www.sfml-dev.org/download.php)**, and place directory wherever *(there's no standard installation directory for SFML in Windows)*.
3. Open **Command Prompt**, and type the following commands in it.
4. If **[Git](https://git-scm.com/download/win)** is installed, simply type: `git clone https://github.com/TaylorLewis/chip8-emulator`. Otherwise download this repository in GitHub, unzip files, and direct the Command Prompt to the repository folder with `cd`.
5. `mkdir build`
6. `cd build`
7. `cmake .. -DSFML_ROOT=<sfml-path>`, where *\<sfml-path>* is the path to your SFML directory.
8. `chip8-emulator.sln`

This will open the project in Visual Studio. Then:

9. Make sure _**Release**_ is selected (under the menu bar), rather than _**Debug**_.
10. Press Ctrl+Shift+B to build the solution, or in the menu bar click *Build > Build Solution*.


### macOS

1. Open the **Terminal**, and type the following commands:
2. If you don't have a package manager installed, like **[Homebrew](https://brew.sh/)**, type:
`/usr/bin/ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"`
3. `brew install cmake sfml`
4. `git clone https://github.com/TaylorLewis/chip8-emulator`
5. `mkdir build`
6. `cd build`
7. `cmake ..`
8. `make`

### Linux

In the terminal, type:

1. `sudo apt-get install cmake libsfml-dev`
2. `git clone https://github.com/TaylorLewis/chip8-emulator`
3. `mkdir build`
4. `cd build`
5. `cmake ..`
6. `make`

## Usage


`chip8 [options]... <path-to-rom-file>`

Options:
```
    -h, --help 
    	Print this help text and exit.
        
    -w <int>, --width <int>
    	Set the window width.  Default: 1024
        
    -H <int>, --height <int>
    	Set the window height. Default:  512
```
    
Example : `chip8 -w 512 -H 256 ./assets/roms/BRIX`


## Controls

The Chip-8 originally took input through a hex keypad. The default keyboard mapping is arranged as follows:

| Keyboard | Hex Keypad |
| :---:|:---: |
| `1` `2` `3` `4`<br>`Q` `W` `E` `R`<br>`A` `S` `D` `F`<br>`Z` `X` `C` `V` | `1` `2` `3` `C`<br>`4` `5` `6` `D`<br>`7` `8` `9` `E`<br>`A` `0` `B` `F` |

`Pause` or `P` to pause.

`Escape` to quit.

## Upcoming Features

* Graphics overhaul
* GUI
* Save/load state
* Debugger
* More robust timing
* Customizable color / speed / controls
* Save configuration

## Legal

This project is licensed under the terms of the [MIT license](https://tldrlegal.com/license/mit-license).

The ROMs included are generally considered to be in the public domain. For all intents and purposes, this appears to be a safe enough assumption.

## References

The Chip-8 specification used was largely derived from:
* http://mattmik.com/files/chip8/mastering/chip8.html

![Pong](/docs/screenshots/PONG.gif)
