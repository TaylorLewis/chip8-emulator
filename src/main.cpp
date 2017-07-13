#include <iostream>
#include <string>
#include <limits>
#include <stdexcept>

#include "emulator.hpp"

// Returns true if the argument can be converted to an integer,
// and the result is non-negative and no larger than 'INT_MAX'.
bool verifySizeInput(const std::string& inputStr, const std::string& dimension) {
    int inputInt;

    try {
        inputInt = std::stoi(inputStr); }
    catch (std::invalid_argument&) {
        std::cerr << "Custom " << dimension << " setting failed (Invalid argument; not a number). Argument: " << inputStr << std::endl;
        return false;
    }
    catch (std::out_of_range&) {
        std::cerr << "Custom " << dimension << " setting failed (Out of range; number is too large). Argument: " << inputStr << std::endl;
        return false;
    }

    if (inputInt < 0) {
        std::cerr << "Custom " << dimension << " setting failed (Negative number). Argument: " << inputInt << std::endl;
        return false;
    }
    else {
        return true; }
}

// Parses command line arguments and passes appropriate arguments to 'emulator' if they are valid.
// The final argument, if it doesn't match any option, is simply passed as the file path and tried later.
void handleArguments(const int& argc, char* argv[], Emulator& emulator) {
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        if (arg == "-h"
             || arg == "--help") {
            std::cout
                << "A Chip-8 emulator.\n\n"
                << "Usage: chip8 [options] <path-to-rom-file>\n\n"
                << "Options:\n"
                << "\t-h  --help\n"
                << "\t\tPrint this help text and exit.\n\n"
                << "\t-w <int>, --width <int>\n"
                << "\t\tSet the window width.  (Default: 1024)\n\n"
                << "\t-H <int>, --height <int>\n"
                << "\t\tSet the window height. (Default:  512)"
                // TODO: Add more options, like color.
                << std::endl;
            exit(EXIT_SUCCESS);
        }
        else if (arg == "-w"
                  || arg == "--width") {
            if (i + 1 < argc) {
                const std::string inputStr = argv[i + 1];
                if (verifySizeInput(inputStr, "width")) {
                    emulator.window_width = std::stoi(inputStr); }
            }
            else {
                std::cerr << "Custom width setting failed (No number specified)." << std::endl; }
            ++i;
        }
        else if (arg == "-H"
                  || arg == "--height") {
            if (i + 1 < argc) {
                const std::string inputStr = argv[i + 1];
                if (verifySizeInput(inputStr, "height")) {
                    emulator.window_height = std::stoi(inputStr); }
            }
            else {
                std::cerr << "Custom height setting failed (No number specified)." << std::endl; }
            ++i;
        }
        else if (i == argc - 1) {
            emulator.rom_path = arg;
        }
        else {
            std::cerr << "Unrecognized argument: " << arg << std::endl; }
    }
}

int main(const int argc, char* argv[]) {
    Emulator emulator;
    handleArguments(argc, argv, emulator);

    try {
        emulator.run(); }
    catch (const std::runtime_error& e) {
        std::cerr << "\nFailed to run (" << e.what() <<  "). Shutting down." << std::endl;
        // The pause here is to make sure the error can be read.
        std::cout << "Press Enter to exit . . . ";
        // Waits until Enter ('\n') is pressed. It turns out to be simpler
        // to write this portably, if we wait for Enter, rather than "any key".
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        return EXIT_FAILURE;
    }
}