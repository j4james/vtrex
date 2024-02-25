// VT-Rex
// Copyright (c) 2024 James Holderness
// Distributed under the MIT License

#pragma once

#include <string>

class capabilities;
class options;

class coloring {
public:
    coloring(const capabilities& caps, const options& options);
    ~coloring();

private:
    bool _using_colors;
    std::string _color_assignment;
    std::string _color_table;
};
