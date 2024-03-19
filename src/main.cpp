// VT-Rex
// Copyright (c) 2024 James Holderness
// Distributed under the MIT License

#include "capabilities.h"
#include "coloring.h"
#include "engine.h"
#include "font.h"
#include "macros.h"
#include "options.h"
#include "os.h"

#include <iostream>

bool check_compatibility(const capabilities& caps, const options& options)
{
    const auto compatible =
        caps.has_soft_fonts &&
        caps.has_horizontal_scrolling &&
        caps.has_rectangle_ops &&
        caps.has_pages;
    if (!compatible && !options.yolo) {
        std::cout << "VT-Rex requires a VT420-compatible terminal or better.\n";
        std::cout << "Try 'vtrex --yolo' to bypass the compatibility checks.\n";
        return false;
    }
    if (caps.height < engine::height) {
        std::cout << "VT-Rex requires a minimum screen height of " << engine::height << ".\n";
        return false;
    }
    if (caps.width < engine::width * 2) {
        std::cout << "VT-Rex requires a minimum screen width of " << engine::width * 2 << ".\n";
        return false;
    }
    return true;
}

int main(const int argc, const char* argv[])
{
    os os;

    options options(argc, argv);
    if (options.exit)
        return 1;

    capabilities caps;
    if (!check_compatibility(caps, options))
        return 1;

    // Set the window title.
    std::cout << "\033]21;VT-Rex\033\\";
    // Load the soft font.
    const auto font = soft_font{caps};
    // Initialize the macros.
    const auto macros = macro_manager{caps, options};
    // Setup the color assignment and palette.
    const auto colors = coloring{caps, options};
    // Save the modes and settings that we're going to change.
    const auto original_decscnm = caps.query_mode(5);
    const auto original_decawm = caps.query_mode(7);
    const auto original_decssdt = caps.query_setting("$~");
    // Reverse screen attributes so it's effectively black on white.
    std::cout << "\033[?5h";
    // Disable line wrapping.
    std::cout << "\033[?7l";
    // Disable page cursor coupling.
    std::cout << "\033[?64l";
    // Hide the status line.
    std::cout << "\033[0$~";
    // Clear margins.
    std::cout << "\033[r";
    // Set default attributes.
    std::cout << "\033[m";
    // Clear the screen.
    std::cout << "\033[2J";
    // Hide the cursor.
    std::cout << "\033[?25l";
    // Make the play area double width
    macros.double_width.run();

    while (true) {
        auto game_engine = engine{macros, options};
        if (!game_engine.run()) break;
    }

    // Clear the window title.
    std::cout << "\033]21;\033\\";
    // Set default attributes.
    std::cout << "\033[m";
    // Clear all pages.
    std::cout << "\033[3 P\033[2J";
    std::cout << "\033[2 P\033[2J";
    std::cout << "\033[1 P\033[H\033[J";
    // Make sure page 1 is visible.
    std::cout << "\033[?64h";
    // Reset reverse screen attributes if not originally set.
    if (original_decscnm != true)
        std::cout << "\033[?5l";
    // Reapply line wrapping if not originally reset.
    if (original_decawm != false)
        std::cout << "\033[?7h";
    // Restore the original status display type.
    if (!original_decssdt.empty())
        std::cout << "\033[" << original_decssdt;
    // Show the cursor.
    std::cout << "\033[?25h";

    return 0;
}
