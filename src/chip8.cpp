#include <iostream>
#include "time.h"

#include "chip8.hpp"

bool Chip8::soundReady() {
    return sound_timer > SOUND_TIMER_THRESHOLD;
}

bool Chip8::getPixelAt(const int& x, const int& y) {
    return screen.getPixel(x, y); }

void Chip8::setKey(const uint8_t& key, const bool& value) {
    keys_pressed[key] = value;
}

Chip8::Chip8() {
    srand((unsigned int)time(NULL)); // Seed RNG

    sp = 0;
    pc = PROGRAM_START;
    opcode = 0;
    I = 0;
    draw_flag = false;

    delay_timer = 0;
    sound_timer = 0;

    for (uint8_t &element : memory) {
        element = 0;
    }

    for (int i = 0; i < 0x10; ++i) {
        V[i] = 0;
        stack[i] = 0;
    }

    for (bool &key : keys_pressed) {
        key = false;
    }

    // This sprite data is used for graphics in Chip-8 programs.
    // This is loaded into the first 80 bytes of 'memory' (program data starts at 0x200=512).
    // Each sprite is a hex digit character, from 0x0 to 0xF, and is represented by five bytes.
    // Each byte, in binary, represents a row of pixels for a given character.
    //     e.g. Looking at the first line, a 0 is represented with:
    //     0xF0 = 1111 0000
    //     0x90 = 1001 0000
    //     0x90 = 1001 0000
    //     0x90 = 1001 0000
    //     0xF0 = 1111 0000
    //     Ignoring the empty second nibble, the shape of the ones forms a zero.
    constexpr std::array<uint8_t, 16 * 5> sprites{
        0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
        0x20, 0x60, 0x20, 0x20, 0x70, // 1
        0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
        0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
        0x90, 0x90, 0xF0, 0x10, 0x10, // 4
        0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
        0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
        0xF0, 0x10, 0x20, 0x40, 0x40, // 7
        0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
        0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
        0xF0, 0x90, 0xF0, 0x90, 0x90, // A
        0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
        0xF0, 0x80, 0x80, 0x80, 0xF0, // C
        0xE0, 0x90, 0x90, 0x90, 0xE0, // D
        0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
        0xF0, 0x80, 0xF0, 0x80, 0x80  // F
    };
    // Load sprite data into memory
    for (size_t i = 0; i < sprites.size(); ++i) {
        memory[i] = sprites[i];
    }
}

void Chip8::load(const char rom_buffer[], const int& rom_size) {
    for (int i = 0; i < rom_size; ++i) {
        memory[i + PROGRAM_START] = (uint8_t)rom_buffer[i];
    }
}

void Chip8::step() {
    executeNextInstruction();
    decrementTimers();
}

void Chip8::executeNextInstruction() {
    opcode = (memory[pc] << 8) | memory[pc + 1]; // Big endian

    // These correspond to particular hex digits in the opcode.
    const int X   = (opcode & 0x0F00) >> 8; // .X..
    const int Y   = (opcode & 0x00F0) >> 4; // ..Y.
    const int N   =  opcode & 0x000F;       // ...N
    const int NN  =  opcode & 0x00FF;       // ..NN
    const int NNN =  opcode & 0x0FFF;       // .NNN

    switch (opcode & 0xF000) {
    // 0???
    case 0x0000:
        switch (opcode) {
        // 00E0: Clear the screen.
        case 0x00E0:
            screen.clear();
            draw_flag = true;
            pc += 2;
            break;

        // 00EE: Return from a subroutine.
        case 0x00EE:
            --sp;
            pc = stack[sp];
            pc += 2;
            break;

        default:
            std::cerr << "Unknown opcode (0???): " << std::hex << std::uppercase << opcode << std::endl;
            break;
        }
        break;

    // 1NNN: Jump to address NNN.
    case 0x1000:
        pc = NNN;
        break;

    // 2NNN: Execute subroutine starting at address NNN.
    case 0x2000:
        stack[sp] = pc;
        ++sp;
        pc = NNN;
        break;

    // 3XNN: Skip the following instruction if the value of register VX equals NN.
    case 0x3000:
        if (V[X] == NN) {
            pc += 4; }
        else {
            pc += 2; }
        break;

    // 4XNN: Skip the following instruction if the value of register VX is not equal to NN.
    case 0x4000:
        if (V[X] != NN) {
            pc += 4; }
        else {
            pc += 2; }
        break;

    // 5XY0: Skip the following instruction if the value of register VX is equal to the value of register VY.
    case 0x5000:
        if (V[X] == V[Y]) {
            pc += 4; }
        else {
            pc += 2; }
        break;

    // 6XNN: Store number NN in register VX.
    case 0x6000:
        V[X] = NN;
        pc += 2;
        break;

    // 7XNN: Add the value NN to register VX.
    case 0x7000:
        V[X] += NN;
        pc += 2;
        break;

    // 8XY?
    case 0x8000:
        switch (opcode & 0xF00F) {
        // 8XY0: Store the value of register VY in register VX.
        case 0x8000:
            V[X] = V[Y];
            pc += 2;
            break;

        // 8XY1: Set VX to VX OR VY.
        case 0x8001:
            V[X] |= V[Y];
            pc += 2;
            break;

        // 8XY2: Set VX to VX AND VY.
        case 0x8002:
            V[X] &= V[Y];
            pc += 2;
            break;

        // 8XY3: Set VX to VX XOR VY.
        case 0x8003:
            V[X] ^= V[Y];
            pc += 2;
            break;

        // 8XY4: Add the value of register VY to register VX.
        //       Set VF to 1 if a carry occurs.
        //       Set VF to 0 if a carry does not occur.
        case 0x8004:
            if (V[X] + V[Y] > UINT8_MAX) {
                V[0xF] = 1; }
            else {
                V[0xF] = 0; }
            V[X] += V[Y];
            pc += 2;
            break;

        // 8XY5: Subtract the value of register VY from register VX.
        //       Set VF to 0 if a borrow occurs.
        //       Set VF to 1 if a borrow does not occur.
        case 0x8005:
            if (V[X] - V[Y] < 0) {
                V[0xF] = 0; }
            else {
                V[0xF] = 1; }
            V[X] -= V[Y];
            pc += 2;
            break;

        // 8XY6: Store the value of register VY shifted right one bit in register VX.
        //       Set register VF to the least significant bit prior to the shift
        //       (the bit that fell off).
        case 0x8006:
            V[X] = V[Y] >> 1;
            V[0xF] = V[Y] & 1;
            pc += 2;
            break;

        // 8XY7: Set register VX to the value of VY minus VX.
        //       Set VF to 0 if a borrow occurs.
        //       Set VF to 1 if a borrow does not occur.
        case 0x8007:
            if (V[Y] < V[X]) {
                V[0xF] = 0; }
            else {
                V[0xF] = 1; }
            V[X] = V[Y] - V[X];
            pc += 2;
            break;

        // 8XYE: Store the value of register VY shifted left one bit in register VX.
        //       Set register VF to the most significant bit prior to the shift
        //       (the bit that fell off).
        case 0x800E:
            V[X] = V[Y] << 1;
            V[0xF] = V[Y] >> 7;
            pc += 2;
            break;

        // Error, or possibly SCHIP opcode.
        default:
            std::cerr << "Unknown opcode (8XY?): " << std::hex << std::uppercase << opcode << std::endl;
            break;
        }
        break;

    // 9XY0: Skip the following instruction if the value of register VX is not equal to the value of register VY.
    case 0x9000:
        if (V[X] != V[Y]) {
            pc += 4; }
        else {
            pc += 2; }
        break;

    // ANNN: Store memory address NNN in register I.
    case 0xA000:
        I = NNN;
        pc += 2;
        break;

    // BNNN: Jump to address NNN + V0.
    case 0xB000:
        pc = NNN + V[0];
        break;

    // CXNN: Set VX to a random number (Typically: 0 to 255) with a bitwise AND mask of NN.
    case 0xC000:
        V[X] = (rand() % (UINT8_MAX + 1))  &  NN;
        pc += 2;
        break;

    // DXYN: Draw a sprite at position VX, VY with N bytes of sprite data starting at the address stored in I.
    //       Set VF to 1 if any set pixels are changed to unset, and 0 otherwise.
    case 0xD000: {
        V[0xF] = 0;
        for (int row = 0; row < N; ++row) {
            const uint8_t sprite_row = memory[I + row];

            for (int col = 0; col < 8; ++col) {
                const bool sprite_pixel = (sprite_row & (0x80 >> col));

                if (sprite_pixel) {
                    const bool current_pixel = screen.getPixel(col + V[X], row + V[Y]);
                    
                    if (current_pixel) {
                        V[0xF] = 1; }
                    screen.setPixel(col + V[X], row + V[Y], !current_pixel);
                }
            }
        }

        draw_flag = true;
        pc += 2;
        break;
    }

    // EX??
    case 0xE000:
        switch (opcode & 0xF0FF) {
        // EX9E: Skip the following instruction if the key corresponding to the hex value currently stored in register VX is pressed.
        case 0xE09E:
            if (keys_pressed[V[X]]) {
                pc += 4; }
            else {
                pc += 2; }
            break;

        // EXA1: Skip the following instruction if the key corresponding to the hex value currently stored in register VX is not pressed.
        case 0xE0A1:
            if (!keys_pressed[V[X]]) {
                pc += 4; }
            else {
                pc += 2; }
            break;

        // Error, or possibly SCHIP opcode.
        default:
            std::cerr << "Unknown opcode (EX??): " << std::hex << std::uppercase << opcode << std::endl;
            break;
        }
        break;

    // FX??
    case 0xF000:
        switch (opcode & 0xF0FF) {
        // FX07: Store the current value of the delay timer in register VX.
        case 0xF007:
            V[X] = delay_timer;
            pc += 2;
            break;

        // FX0A: Wait for a keypress and store the result in register VX.
        case 0xF00A: {
            bool key_pressed = false;

            for (size_t i = 0; i < keys_pressed.size(); ++i) {
                if (keys_pressed[i]) {
                    V[X] = (uint8_t)i;
                    key_pressed = true;
                }
            }
            if (!key_pressed) {
                return; }
            pc += 2;
            break;
        }

        // FX15: Set the delay timer to the value of register VX.
        // Undocumented feature: VF is set to 1 when there is a range overflow(I + VX>0xFFF), and to 0 when there isn't.
        case 0xF015:
            if (I + V[X] > 0xFFF) {
                V[0xF] = 1; }
            else {
                V[0xF] = 0; }
            delay_timer = V[X];
            pc += 2;
            break;

        // FX18: Set the sound timer to the value of register VX.
        case 0xF018:
            sound_timer = V[X];
            pc += 2;
            break;

        // FX1E: Add the value stored in register VX to register I.
        case 0xF01E:
            I += V[X];
            pc += 2;
            break;

        // FX29: Set I to the memory address of the sprite data corresponding to the hexadecimal digit stored in register VX.
        case 0xF029:
            I = V[X] * 5;
            pc += 2;
            break;

        // FX33: Store the binary-coded decimal equivalent of the value stored in register VX at addresses I, I+1, and I+2
        //       (hundreds digit in memory at location in I, the tens digit at location I + 1, and the ones digit at location I + 2).
        case 0xF033:
            static const uint8_t VX = V[X];
            memory[I] =      V[X] / 100; // Given that the maximum value is 255, taking a remainder should be unnecessary here.
            memory[I + 1] = (V[X] / 10) % 10;
            memory[I + 2] =        V[X] % 10;
            pc += 2;
            break;

        // FX55: Store the values of registers V0 to VX inclusive in memory starting at address I.
        //       I is set to I + X + 1 after operation.
        case 0xF055:
            for (int i = 0; i <= X; ++i) {
                memory[I + i] = V[i];
            }
            I = I + X + 1;
            pc += 2;
            break;

        // FX65: Fill registers V0 to VX inclusive with the values stored in memory starting at address I.
        //       I is set to I + X + 1 after operation.
        case 0xF065:
            for (int i = 0; i <= X; ++i) {
                V[i] = memory[I + i];
            }
            I = I + X + 1;
            pc += 2;
            break;

        // Error, or possibly SCHIP opcode.
        default:
            std::cerr << "Unknown opcode (FX??): " << std::hex << std::uppercase << opcode << std::endl;
            break;
        }
        break;

    // Error, or possibly SCHIP opcode.
    default:
        std::cerr << "Unknown opcode (????): " << std::hex << std::uppercase << opcode << std::endl;
        break;
    }
}

void Chip8::decrementTimers() {
    if (delay_timer > 0) {
        --delay_timer; }
    if (sound_timer > 0) {
        --sound_timer; }
}



Chip8::VirtualScreen::VirtualScreen() {
    clear();
}

void Chip8::VirtualScreen::setPixel(const int& x, const int& y, const bool& value) {
    virtual_screen[y % HEIGHT][x % WIDTH] = value;
}
bool Chip8::VirtualScreen::getPixel(const int& x, const int& y) {
    return virtual_screen[y % HEIGHT][x % WIDTH];
}

void Chip8::VirtualScreen::clear() {
    for (auto &rows : virtual_screen) {
        for (bool &pixel : rows) {
            pixel = false;
        }
    }
}