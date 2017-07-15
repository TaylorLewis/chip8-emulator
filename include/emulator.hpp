#ifndef EMULATOR_HPP
#define EMULATOR_HPP

#include <stdint.h>
#include <unordered_map>
#include <SFML/OpenGL.hpp>
#include <SFML/Window.hpp>
#include <SFML/Audio.hpp>

#include "chip8.hpp"

// This class basically runs everything, once the command line arguments are handled in 'main.cpp',
// running the Chip-8 object, and handling its input/output.
class Emulator {
public:
    Emulator();
    // The main emulation loop.
    // Takes input, executes the next instruction, updates screen, plays sound,
    // and then sleeps a short period (to avoid running far too fast).
    void run();

    int window_width;
    int window_height;
    std::string rom_path;

private:
    // Default window size parameters.
    static constexpr int WINDOW_WIDTH_DEFAULT  = 1024,
                         WINDOW_HEIGHT_DEFAULT =  512;

    // Mapping from keyboard to virtual hex keypad.
    static const std::unordered_map<int, uint8_t> keypad_map;

    // Sets up graphics and sound, and loads the Chip-8 program into memory.
    void startup();
    void setupWindow();
    // Adjusts viewport (and 'pixel_size') so a disproportionate window doesn't stretch the picture.
    void setAspect();
    void setupOpenGLContext();
    // This method may fail to load the sound (1), but continues execution regardless.
    // I rather like the little boop; but it's really not essential.
    void setupSound();

    // Handles events like window focus, resizing, or closing.
    // Updates 'chip8.keys_pressed' with the state of corresponding keys in 'keypad_map'.
    void handleInput();
    // Renders the Chip-8's virtual screen, and then displays it.
    void updateScreen();
    // Plays a sound if the Chip-8 sound timer signals it.
    void handleSound();
    // Used to slow down emulation to maintain humane pace.
    // TODO: Make speed more consistent across faster/slower hardware.
    // TODO: Make speed customizable.
    void sleep();

    int pixel_size;
    sf::Window window;
    sf::SoundBuffer sound_buffer;
    sf::Sound sound;

    Chip8 chip8;

    // State flags
    bool running,    // Whether the emulator is running.
         paused,     // Toggled by the pause key ('Pause' or 'P')
         have_focus; // Whether the window has focus
};

#endif