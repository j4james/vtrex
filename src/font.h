// VT-Rex
// Copyright (c) 2024 James Holderness
// Distributed under the MIT License

#pragma once

class capabilities;

class soft_font {
public:
    soft_font(const capabilities& caps);
    ~soft_font();

private:
    bool _has_soft_fonts;
};
