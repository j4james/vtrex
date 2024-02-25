// VT-Rex
// Copyright (c) 2024 James Holderness
// Distributed under the MIT License

#pragma once

class options {
public:
    options(const int argc, const char* argv[]);

    bool color = true;
    bool sound = true;
    bool blink = true;
    bool yolo = false;
    bool exit = false;
    int fps = 15;
};
