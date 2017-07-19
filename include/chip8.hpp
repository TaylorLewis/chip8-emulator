#ifndef CHIP8_HPP
#define CHIP8_HPP

#include <stdint.h>
#include <array>

// Models a Chip-8 CPU. Also contains the memory.
// Interprets Chip-8 instructions.
//
// I'll explain some things, but for more detailed specification, read:
// https://en.wikipedia.org/wiki/CHIP-8
// http://mattmik.com/files/chip8/mastering/chip8.html
// which are the specifications that this program mostly draws from.
class Chip8 {
public:
    // Internal resolution.
    static constexpr int WIDTH = 64,
                         HEIGHT = 32,
                         ASPECT_RATIO = 2; // Ratio of width to height

    Chip8();

    // Load Chip-8 program data into memory.
    void load(const char rom_buffer[], const int& rom_size);

    // Excecutes the next instruction and decrements timers.
    void step();

    // Signals when it's time to play a sound.
    bool soundReady();

    bool getPixelAt(const int& x, const int& y);

    void setOldInstructions(const bool& value);

    void setKey(const uint8_t& key, const bool& value);

    // Indicates that the screen has changed, and should be redrawn.
    bool draw_flag;

private:
    static constexpr int MEM_SIZE      = 0x1000, // RAM size, in bytes. 0x1000 = 4096.
                         PROGRAM_START =  0x200; // Location in 'memory' where program data begins. 0x200 = 512.

    // Value of 'sound_timer' that indicates a sound should be played.
    // Documentation seems to differ on whether the threshold is 0 or 1.
    static constexpr int SOUND_TIMER_THRESHOLD = 1;

    // Reads the next opcode (2 bytes) and executes it.
    void executeNextInstruction();

    // Decrements the timer variables if they are greater than zero.
    void decrementTimers();

    // The program is stored directly into here, as well as fontset.
    // The program data is big endian.
    std::array<uint8_t, MEM_SIZE> memory;

    // Registers (V0-VF)
    std::array<uint8_t, 0x10> V;

    // The current address is stored in the stack when a subroutine is called.
    std::array<uint16_t, 0x10> stack;
    // Stack pointer
    uint8_t sp;

    // Program Counter; index for 'memory'.
    // Increments in steps of 2 (the length of an opcode).
    uint16_t pc;
    // Current opcode. Every opcode is 2 bytes long.
    uint16_t opcode;
    // Index register. Stores memory addresses,
    // which the main registers can't because they're only 1 byte large.
    // This is used to point to sprite data locations. 
    uint16_t I;

    // These count down from values set by the program, decrementing each instruction step.
    uint8_t delay_timer, // This is used by the program, like a register, but constantly decrementing.
            sound_timer; // Indicates a sound should be made, on any value greater than the 'SOUND_TIMER_THRESHOLD'.

    // Some instructions have changed slightly from the originals.
    // This toggles the use of the old instructions rather than their contemporary versions.
    // Some ROMs require this.
    bool old_instructions;

    // Intermediary representation of the screen to be displayed.
    // Pixels are represented as bools, as color depth is only 1-bit.
    // Coordinates originate at the upper-left corner, and positive directions are right and down.
    // Letting x be the left-to-right axis (width), and y be the up-to-down axis (height),
    // the position (x,y) corresponds to screen[y][x].
    class VirtualScreen {
        public:
            VirtualScreen();

            // Takes non-negative coordinates.
            // If an argument exceeds the screen bounds, the remainder will be used.
            void setPixel(const int& x, const int& y, const bool& value);
            bool getPixel(const int& x, const int& y);

            // Sets all pixels to false.
            void clear();

        private:
            std::array<std::array<bool, WIDTH>, HEIGHT> virtual_screen;
    };

    VirtualScreen screen;

    // The Chip-8 originally took input through a hex keypad., which was arranged as such:
    // 123C
    // 456D
    // 789E
    // A0BF
    // The keys are indexed in the array by their literal value in hex.
    std::array<bool, 0x10> keys_pressed;

public:
    // Maximum size for Chip-8 ROM files.
    static constexpr int ROM_SIZE_MAX = MEM_SIZE - PROGRAM_START;
};

#endif