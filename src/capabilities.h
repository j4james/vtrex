// VT-Rex
// Copyright (c) 2024 James Holderness
// Distributed under the MIT License

#pragma once

#include <optional>
#include <regex>
#include <string>
#include <string_view>

class capabilities {
public:
    capabilities();
    ~capabilities();
    std::optional<bool> query_mode(const int mode) const;
    std::string query_setting(const std::string_view setting) const;
    std::string query_color_table() const;

    int width = 80;
    int height = 24;
    bool has_soft_fonts = false;
    bool has_horizontal_scrolling = false;
    bool has_color = false;
    bool has_rectangle_ops = false;
    bool has_macros = false;
    bool has_pages = false;

private:
    void _query_device_attributes();
    static std::smatch _query(const char* pattern, const bool may_not_work);

    std::optional<bool> _original_decrpl;
    std::optional<bool> _original_decpccm;
};
