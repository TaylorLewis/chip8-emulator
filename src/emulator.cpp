#include <iostream>
#include <fstream>
#include <stdexcept>

#include "emulator.hpp"

const sf::Color Emulator::COLOR_SPRITE_DEFAULT = sf::Color(0xFF, 0xFF, 0xFF); // #ffffff: white.
const sf::Color Emulator::COLOR_BACKGROUND_DEFAULT = sf::Color(0x1c, 0x28, 0x41); // #1c2841: a pleasant shade of blue.

// Default keyboard mapping.
// (Keyboard -> Hex Keypad)
//      1234 -> 123C
//      QWER -> 456D
//      ASDF -> 789E
//      ZXCV -> A0BF
// TODO: Add customization (Note: don't forget other keys like Escape and Pause).
const std::unordered_map<int, uint8_t> Emulator::keypad_map = {
    { sf::Keyboard::Num1, 0x1 },{ sf::Keyboard::Num2, 0x2 },{ sf::Keyboard::Num3, 0x3 },{ sf::Keyboard::Num4, 0xC },
    { sf::Keyboard::Q,    0x4 },{ sf::Keyboard::W,    0x5 },{ sf::Keyboard::E,    0x6 },{ sf::Keyboard::R,    0xD },
    { sf::Keyboard::A,    0x7 },{ sf::Keyboard::S,    0x8 },{ sf::Keyboard::D,    0x9 },{ sf::Keyboard::F,    0xE },
    { sf::Keyboard::Z,    0xA },{ sf::Keyboard::X,    0x0 },{ sf::Keyboard::C,    0xB },{ sf::Keyboard::V,    0xF }
};

Emulator::Emulator() {
    running    = false;
    paused     = false;
    have_focus = false;

    window_width  = WINDOW_WIDTH_DEFAULT;
    window_height = WINDOW_HEIGHT_DEFAULT;

    color_sprite = COLOR_SPRITE_DEFAULT;
    color_background = COLOR_BACKGROUND_DEFAULT;
}

void Emulator::run() {
    startup();

    running = true;
    // Time Chip-8 is considered to have been running (doesn't include paused and unfocused time).
    auto run_time = std::chrono::high_resolution_clock::duration::zero();
    Timer timer;

    while (running) {
        timer.update();

        handleInput();
        if (have_focus && !paused) {
            run_time += timer.getElapsed();
            // This loop is for "catching up" to target rate, skipping input, screen, and sound updates to get there
            while (run_time >= Chip8::TIME_PER_STEP) {
                chip8.step();
                run_time -= Chip8::TIME_PER_STEP;
            }
            updateScreen();
            handleSound();
        }
    }
}

void Emulator::startup() {
    loadFile();

    setupWindow();
    setupSound();
}

void Emulator::setupWindow() {
    window.create(sf::VideoMode(window_width, window_height), "Chip-8 Emulator");
    view.setSize(window_width, window_height);
    view.setCenter(view.getSize().x / 2, view.getSize().y / 2);
    setAspect();

    texture.create(Chip8::WIDTH, Chip8::HEIGHT);
    sprite.setTexture(texture);
    sprite.setScale(sf::Vector2f(view.getSize().x / (float)texture.getSize().x,
                                 view.getSize().y / (float)texture.getSize().y));
    have_focus = true;
}

void Emulator::setAspect() {
    float window_ratio = window.getSize().x / (float)window.getSize().y;
    float view_ratio = view.getSize().x / (float)view.getSize().y;
    float sizeX = 1;
    float sizeY = 1;
    float posX = 0;
    float posY = 0;

    if (window_ratio >= view_ratio) { // Window is too wide relative to aspect ratio.
        // Pillarbox
        sizeX = view_ratio / window_ratio;
        posX = (1 - sizeX) / 2.f;
    }
    else { // Window is too tall relative to aspect ratio.
        // Letterbox
        sizeY = window_ratio / view_ratio;
        posY = (1 - sizeY) / 2.f;
    }

    view.setViewport(sf::FloatRect(posX,
                                   posY,
                                   sizeX,
                                   sizeY));
    window.setView(view);
}

void Emulator::setupSound() {
    sound_buffer.loadFromFile("assets/sound/boop.wav"); // Produces an error message if it fails
    sound.setBuffer(sound_buffer);
}

void Emulator::loadFile() {
    std::ifstream rom(rom_path, std::ios::binary);

    if (rom.fail()) {
        std::cerr
            << "Couldn't load file at designated path: " << rom_path << '\n'
            << "Usage: chip8 [OPTION]... [FILE]\n\n"
            << "Trying default: ./assets/roms/PONG" << std::endl;
        rom.open("./assets/roms/PONG", std::ios::binary);
        if (rom.fail()) {
            std::cerr << "Couldn't load file at default path either." << std::endl;
            throw std::runtime_error("Failed ifstream");
        }
    }
    if (rom.bad()) {
        std::cerr << "Unknown file error." << std::endl;
        throw std::runtime_error("Bad ifstream");
    }

    // Get length of file
    rom.seekg(0, std::ios::end);
    const int rom_size = (int)rom.tellg();
    rom.seekg(0, std::ios::beg);

    if (rom_size > Chip8::ROM_SIZE_MAX) {
        std::cerr << "File too large to fit into memory."
            << " Filesize/max: " << rom_size << "/" << Chip8::ROM_SIZE_MAX << " bytes." << std::endl;
        throw std::runtime_error("File too large");
    }

    // Copy ROM into buffer
    char rom_buffer[Chip8::ROM_SIZE_MAX];
    rom.read(rom_buffer, rom_size);

    chip8.load(rom_buffer, rom_size);

    std::cout << "File loaded." << std::endl;
}

void Emulator::handleInput() {
    sf::Event event;

    while (window.pollEvent(event)) {
        switch (event.type) {
            case sf::Event::LostFocus:
                have_focus = false;
                break;

            case sf::Event::GainedFocus:
                have_focus = true;
                break;

            case sf::Event::Closed:
                running = false;
                break;

            case sf::Event::Resized: {
                window_width  = window.getSize().x;
                window_height = window.getSize().y;
                setAspect();
                break;
            }

            case sf::Event::KeyPressed: {
                if (event.key.code == sf::Keyboard::P
                     || event.key.code == sf::Keyboard::Pause) {
                    paused = !paused;
                    if (paused) {
                        std::cout << "Paused" << std::endl; }
                    else {
                        std::cout << "Unpaused" << std::endl; }
                }

                if (event.key.code == sf::Keyboard::Escape) {
                    running = false; }

                auto key = keypad_map.find(event.key.code);
                if (key != keypad_map.end()) {
                    chip8.setKey(key->second, true); }
                break;
            }

            case sf::Event::KeyReleased: {
                auto key = keypad_map.find(event.key.code);
                if (key != keypad_map.end()) {
                    chip8.setKey(key->second, false); }
                break;
            }
        }
    }
}

void Emulator::updateScreen() {
    if (chip8.draw_flag) {
        window.clear(sf::Color::Black);

        // SFML forces the use of a C style array here.
        // Each set of 4 is read as a 32-bit RGBA pixel.
        sf::Uint8 pixels[Chip8::HEIGHT * Chip8::WIDTH * 4];

        for (int row = 0; row < Chip8::HEIGHT; ++row) {
            for (int col = 0; col < Chip8::WIDTH; ++col) {
                const int i = (col + row*Chip8::WIDTH) * 4;

                if (chip8.getPixelAt(col, row)) {
                    // Write sprite color.
                    pixels[i]     = color_sprite.r;
                    pixels[i + 1] = color_sprite.g;
                    pixels[i + 2] = color_sprite.b;
                    pixels[i + 3] = color_sprite.a;
                }
                else {
                    // Write background color.
                    pixels[i]     = color_background.r;
                    pixels[i + 1] = color_background.g;
                    pixels[i + 2] = color_background.b;
                    pixels[i + 3] = color_background.a;
                }
            }
        }

        texture.update(pixels);
        window.draw(sprite);
        window.display();

        chip8.draw_flag = false;
    }
}

void Emulator::handleSound() {
    if (chip8.soundReady()) {
        sound.play(); }
}

void Emulator::setOldInstructions(const bool& value) {
    chip8.setOldInstructions(value); }



Emulator::Timer::Timer() {
    previous = std::chrono::high_resolution_clock::now();
    update();
}

void Emulator::Timer::update() {
    auto current = std::chrono::high_resolution_clock::now();
    elapsed = current - previous;
    previous = current;
}

std::chrono::high_resolution_clock::duration Emulator::Timer::getElapsed() {
    return elapsed;
}