#include <iostream>
#include <fstream>
#include <stdexcept>

#include "emulator.hpp"

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
    setupOpenGLContext();
    setupSound();
}

void Emulator::setupWindow() {
    window.create(sf::VideoMode(window_width, window_height), "Chip-8 Emulator");
    have_focus = true;
}

void Emulator::setAspect() {
    int view_width,
        view_height;
    // Size parameters proportionate to each other.
    const int proportionate_width = window_height * Chip8::ASPECT_RATIO,
              proportionate_height = window_width / Chip8::ASPECT_RATIO;

    if (proportionate_height > window_height) {
        view_width = proportionate_width;
        view_height = window_height;
        pixel_size = proportionate_width / Chip8::WIDTH;
    }
    else { // (proportionate_width >= window_width)
        view_width = window_width;
        view_height = proportionate_height;
        pixel_size = proportionate_height / Chip8::HEIGHT;
    }

    // Coordinates of the lower-left corner of the viewport
    const int coord_x =  (window_width - view_width)  / 2,
              coord_y = (window_height - view_height) / 2;

    glViewport(coord_x, coord_y, view_width, view_height);
}

void Emulator::setupOpenGLContext() {
    setAspect();
    // Change the coordinate scale from (-1 - +1) to (0 - window.getSize().[x/y])
    glOrtho(
        0,                  // left
        window.getSize().x, // right
        window.getSize().y, // bottom
        0,                  // top
        0,                  // zNear
        1                   // zFar
    );
    // Offset to translate coordinate arguments. Needed to center points.
    // TODO: This isn't quite right for different aspect ratios.
    //       To be fixed when the graphics are overhauled.
    const GLfloat offset = pixel_size / 2;
    glTranslatef(offset, offset, 0);

    // Set scale to multiply coordinate arguments. Needed to fill the context.
    const int proportionate_width = window_height * Chip8::ASPECT_RATIO,
              proportionate_height = window_width / Chip8::ASPECT_RATIO;
    glScalef(proportionate_height / Chip8::HEIGHT,
             proportionate_width  / Chip8::WIDTH,
             1);

    // Set colors.
    // TODO: Make customizable.
    // TODO: Implement letter/pillar-boxing (black bars).
    //glColor3ub(0, 0xFF, 0); // Sprite color
    glClearColor(0x1c / 255.0, 0x28 / 255.0, 0x41 / 255.0, 1); // Background color. #1c2841, a pleasant shade of blue.
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

// TODO!: This should be overhauled; points turned out to be the wrong choice.
//        Being square, they don't fill the screen properly (stretch) when using a different aspect ratio.
//        The right approach appears to be rectangles after all.
void Emulator::updateScreen() {
    if (chip8.draw_flag) {
        glClear(GL_COLOR_BUFFER_BIT); // Clear screen.
        glPointSize(pixel_size); // Expanded points here are square, by default.
        glBegin(GL_POINTS); // Set vertices whereupon points are drawn.
        for (int row = 0; row < Chip8::HEIGHT; ++row) {
            for (int col = 0; col < Chip8::WIDTH; ++col) {
                if (chip8.getPixelAt(col, row)) {
                    glVertex2f(col, row); }
            }
        }
        glEnd();
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