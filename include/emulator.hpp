#ifndef EMULATOR_HPP
#define EMULATOR_HPP

#include <stdint.h>
#include <unordered_map>
#include <string>
#include <chrono>
#include <SFML/OpenGL.hpp>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>

#include "chip8.hpp"

// This class basically runs everything, once the command line arguments are handled in 'main.cpp',
// running the Chip-8 object, and handling its input/output.
class Emulator {
public:
    Emulator();
    // The main emulation loop.
    // Takes input, executes the next instruction, updates screen, and plays sound.
    // The next Chip-8 cycle will only occur if the emulator is on time or behind.
    // Extra cycles will occur if the emulator is behind by a whole period ('TIME_PER_STEP') or more.
    void run();

    // Sets 'old_instructions' in Chip-8,
    // which alters some instructions back to their old versions.
    void setOldInstructions(const bool& value);

    int window_width;
    int window_height;
    sf::Color color_sprite;
    sf::Color color_background;
    std::string rom_path;

private:
    // Default window size parameters.
    static constexpr int WINDOW_WIDTH_DEFAULT  = 1024,
                         WINDOW_HEIGHT_DEFAULT =  512;

    static const sf::Color COLOR_SPRITE_DEFAULT;
    static const sf::Color COLOR_BACKGROUND_DEFAULT;

    // Mapping from keyboard to virtual hex keypad.
    static const std::unordered_map<int, uint8_t> keypad_map;

    // Sets up graphics and sound, and loads the Chip-8 program into memory.
    void startup();
    void setupWindow();
    // Adjusts viewport so a disproportionate window doesn't stretch the picture.
    void setAspect();
    // This method may fail to load the sound (1), but continues execution regardless.
    // I rather like the little boop; but it's really not essential.
    void setupSound();

    // Loads file into buffer and passes it to the Chip-8. If loading fails, tries to load "./assets/roms/PONG".
    // Should that fail, or if the file is too large to be a Chip-8 ROM, then an exception is thrown.
    //
    // To my knowledge, there's no general way to detect whether a file is a Chip-8 ROM;
    // there is no standard file extension or header. As such, any other input,
    // that doesn't exceed the size limit, results in undefined behavior.
    void loadFile();

    // Handles events like window focus, resizing, or closing.
    // Updates 'chip8.keys_pressed' with the state of corresponding keys in 'keypad_map'.
    void handleInput();
    // Renders the Chip-8's virtual screen, and then displays it.
    void updateScreen();
    // Plays a sound if the Chip-8 sound timer signals it.
    void handleSound();

    sf::RenderWindow window;
    // Sets the part of the window that actually gets drawn to. Used here to maintain aspect ratio.
    sf::View view;
    // Pixels are drawn to this.
    sf::Texture texture;
    // Takes texture, scales it, and sends to window.
    sf::Sprite sprite;

    sf::SoundBuffer sound_buffer;
    sf::Sound sound;

    Chip8 chip8;

    // State flags
    bool running,    // Whether the emulator is running.
         paused,     // Toggled by the pause key ('Pause' or 'P')
         have_focus; // Whether the window has focus

    // Tracks time elapsed between calls to update().
    class Timer {
        public:
            Timer();
            // Updates 'elapsed'
            void update();
            std::chrono::high_resolution_clock::duration getElapsed();
        private:
            // Difference between last two calls to update()
            std::chrono::high_resolution_clock::duration elapsed;
            // Time of last call to update()
            std::chrono::high_resolution_clock::time_point previous;
    };
};

#endif