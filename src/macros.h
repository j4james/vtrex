// VT-Rex
// Copyright (c) 2024 James Holderness
// Distributed under the MIT License

#pragma once

#include <array>
#include <functional>
#include <string>
#include <string_view>

class capabilities;
class options;

class macro {
public:
    macro() = default;
    macro(const std::string content);
    void run() const;

private:
    std::string _content;
};

class macro_manager {
public:
    class builder;
    macro_manager(const capabilities& caps, const options& options);
    ~macro_manager();
    macro create(std::function<void(builder&)> callback);
    macro create(const std::string_view text);

    macro scroll_start;
    macro scroll_end;
    macro scroll_start_with_clouds;
    macro scroll_end_with_clouds;
    macro frame_complete;
    std::array<macro, 2> trex_running;
    std::array<macro, 9> trex_jumping;
    std::array<macro, 3> trex_dead;
    macro trex_standing;
    macro game_over_banner;
    macro high_score_label;
    macro double_width;
    std::array<macro, 9> cloud_parts;
    std::array<macro, 12> cactus_parts;
    macro game_over_sound;
    macro jump_sound;
    std::array<macro, 2> score_sound;

private:
    void _init_scrollers(const int x_indent, const int y_indent);
    void _init_trex(const int x_indent, const int y_indent);
    void _init_game_over_banner(const int x_indent, const int y_indent);
    void _init_high_score_label(const int x_indent, const int y_indent);
    void _init_double_width(const int y_indent);
    void _init_clouds();
    void _init_cactus();
    void _init_sounds();

    const capabilities& _caps;
    const options& _options;
    int _next_id = 0;
};

class macro_manager::builder {
public:
    void add(const char* fmt...);
    operator std::string_view() const;

private:
    std::string _buffer;
};
