cmake_minimum_required (VERSION 3.1)
set(CMAKE_CXX_STANDARD 11)
set(CXX_STANDARD_REQUIRED)

set(EXECUTABLE_NAME "chip8")
project(chip8-emulator)

if(NOT CMAKE_BUILD_TYPE) # If build type isn't already specified
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING
        "Choose the type of build, options are: Release, Debug."
        FORCE)
endif()

# Add directory containing FindSFML.cmake to module path
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake;${CMAKE_MODULE_PATH};${CMAKE_SOURCE_DIR}")

# Add project files
include_directories("${PROJECT_SOURCE_DIR}/include")
set(SOURCE_FILES src/main.cpp
                 src/chip8.cpp
                 src/emulator.cpp
                 include/chip8.hpp
                 include/emulator.hpp)
add_executable(${EXECUTABLE_NAME} ${SOURCE_FILES})

# Add SFML
set(SFML_ROOT CACHE PATH "Set SFML_ROOT to SFML's top-level path (containing \"include\" and \"lib\" directories).\nSFML_INCLUDE_DIR will also be inferred from this.")
find_package(SFML 2 COMPONENTS audio window system REQUIRED)
if(SFML_FOUND)
    include_directories(${SFML_INCLUDE_DIR})
    target_link_libraries(${EXECUTABLE_NAME} ${SFML_LIBRARIES} ${SFML_DEPENDENCIES})
endif()

# Add OpenGL
find_package(OpenGL REQUIRED)
if (OPENGL_FOUND)
    include_directories(${OPENGL_INCLUDE_DIR})
    target_link_libraries(${EXECUTABLE_NAME} ${OPENGL_LIBRARIES})
endif()

# Copy assets to binary directory after building
add_custom_command(
    TARGET ${EXECUTABLE_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/assets/
            $<TARGET_FILE_DIR:${EXECUTABLE_NAME}>/assets)

# Copy dll files to target directory, if the current OS is Windows
if (WIN32)
    add_custom_command(
        TARGET ${EXECUTABLE_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                ${SFML_ROOT}/bin/openal32.dll
                ${SFML_ROOT}/bin/sfml-audio-2.dll
                ${SFML_ROOT}/bin/sfml-system-2.dll
                ${SFML_ROOT}/bin/sfml-window-2.dll
                $<TARGET_FILE_DIR:${EXECUTABLE_NAME}>)
endif()