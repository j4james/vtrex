// VT-Rex
// Copyright (c) 2024 James Holderness
// Distributed under the MIT License

#include "macros.h"

#include "capabilities.h"
#include "engine.h"
#include "options.h"

#include <cstdarg>
#include <iostream>

macro::macro(const std::string content)
    : _content{content}
{
}

void macro::run() const
{
    std::cout << _content;
}

macro_manager::macro_manager(const capabilities& caps, const options& options)
    : _caps{caps}, _options{options}
{
    // Clear existing macros first to make sure we have space.
    if (_caps.has_macros)
        std::cout << "\033P0;1;0!z\033\\";
    const auto x_indent = std::max((caps.width - engine::width * 2) / 4, 0);
    const auto y_indent = std::max((caps.height - engine::height) / 2, 1);
    _init_scrollers(x_indent, y_indent);
    _init_trex(x_indent, y_indent);
    _init_game_over_banner(x_indent, y_indent);
    _init_high_score_label(x_indent, y_indent);
    _init_double_width(y_indent);
    _init_clouds();
    _init_cactus();
    _init_sounds();
}

macro_manager::~macro_manager()
{
    // Clean out our macros on exit.
    if (_caps.has_macros)
        std::cout << "\033P0;1;0!z\033\\";
}

macro macro_manager::create(std::function<void(builder&)> callback)
{
    auto content = builder{};
    callback(content);
    return create(content);
}

macro macro_manager::create(const std::string_view text)
{
    if (text.length() <= 5 || !_caps.has_macros) {
        return std::string{text};
    } else {
        auto encoded = std::string(text.length() * 2, ' ');
        for (auto i = 0, offset = 0; i < text.length(); i++) {
            static constexpr auto hex = "0123456789ABCDEF";
            encoded[offset++] = hex[(text[i] >> 4) & 0x0F];
            encoded[offset++] = hex[text[i] & 0x0F];
        }
        const auto id = _next_id++;
        std::cout << "\033P" << id << ";0;1!z" << encoded << "\033\\";
        return "\033[" + std::to_string(id) + "*z";
    }
}

void macro_manager::_init_scrollers(const int x_indent, const int y_indent)
{
    const auto top = y_indent + 2;
    const auto bottom = y_indent + 8;
    const auto left = x_indent + 1;
    const auto right = x_indent + engine::width;

    scroll_start = create([&](auto& builder) {
        builder.add("\033[2 P");
        builder.add("\033[8;10r");
        builder.add("\033['~");
        builder.add("\033[r");
        builder.add("\033[10;%dH", engine::width);
    });
    scroll_end = create([&](auto& builder) {
        builder.add("\033[1;1;3;%d;2;%d;%d;3$v", engine::width, top, left);
        builder.add("\033[7;1;10;%d;2;%d;%d;3$v", engine::width, top + 3, left);
        builder.add("\033[3 P");
    });

    scroll_start_with_clouds = create([&](auto& builder) {
        builder.add("\033[2 P");
        builder.add("\033[1;10r");
        builder.add("\033['~");
        builder.add("\033[r");
        builder.add("\033[10;%dH", engine::width);
    });
    scroll_end_with_clouds = create([&](auto& builder) {
        builder.add("\033[4;1;10;%d;2;%d;%d;3$v", engine::width, top, left);
        builder.add("\033[3 P");
    });

    frame_complete = create([&](auto& builder) {
        builder.add("\033[1 P");
        builder.add("\033[%d;%d;%d;%d;3;%d;%d;1$v", top, left, bottom, right, top, left);
        builder.add("\033[%d;%dH", y_indent + 1, (x_indent + engine::width) * 2 - 6);
    });
}

void macro_manager::_init_trex(const int x_indent, const int y_indent)
{
    auto create_trex = [&](const auto x, const auto y, const auto sprite) {
        return create([&](auto& builder) {
            builder.add("\033[%d;%dH", y_indent + 7 - y, x_indent + x);
            builder.add(sprite);
        });
    };

    trex_running[0] = create_trex(3, 0, ":<\b\b\v/`");
    trex_running[1] = create_trex(3, 0, ":<\b\b\v^\\");
    trex_jumping[2] = create_trex(3, 1, ":<\b\b\v!|");
    trex_jumping[4] = create_trex(3, 2, ":<\b\b\v!|");
    trex_jumping[6] = create_trex(3, 3, ":<\b\b\v!|");
    trex_jumping[7] = create_trex(3, 4, "\033[C,\b\b\v;K\b\b\v'\"");
    trex_jumping[8] = create_trex(3, 4, ":<\b\b\v!|");
    trex_dead[0] = create_trex(4, 0, "&");
    trex_dead[1] = create_trex(4, 1, "&");
    trex_dead[2] = create_trex(4, 2, "&");
    trex_standing = create_trex(3, 0, ":<\b\b\v/\\");
}

void macro_manager::_init_game_over_banner(const int x_indent, const int y_indent)
{
    game_over_banner = create([&](auto& builder) {
        const auto x = (engine::width - 10) / 2 + x_indent + 1;
        const auto y = y_indent + 3;
        builder.add("\033[%d;%dHGAME  OVER", y, x);
        builder.add("\033[%d;%dHST", y + 2, x + 4);
    });
}

void macro_manager::_init_high_score_label(const int x_indent, const int y_indent)
{
    high_score_label = create([&](auto& builder) {
        builder.add("\033[%d;%dH", y_indent + 1, (x_indent + engine::width) * 2 - 15);
        builder.add("HI ");
    });
}

void macro_manager::_init_double_width(const int y_indent)
{
    double_width = create([&](auto& builder) {
        builder.add("\033[%dH", y_indent + 2);
        for (auto i = 0; i < 7; i++)
            builder.add("\033#6\n");
    });
}

void macro_manager::_init_clouds()
{
    const auto using_color = _options.color && _caps.has_color;
    for (auto cloud_height = 0; cloud_height < 3; cloud_height++) {
        for (auto cloud_type = 0; cloud_type < 3; cloud_type++) {
            const auto index = cloud_height * 3 + cloud_type;
            const auto ch1 = "{@}"[cloud_type];
            const auto ch2 = "(?)"[cloud_type];
            cloud_parts[index] = create([&](auto& builder) {
                if (using_color) builder.add("\033[44m");
                builder.add("\033[%d;%dH%c\b\033M\033M\033M%c", 6 - cloud_height, engine::width, ch1, ch2);
                if (using_color) builder.add("\033[m");
            });
        }
    }
}

void macro_manager::_init_cactus()
{
    cactus_parts[1] = create("w\b\033MW");
    cactus_parts[2] = create("x\b\033MX");
    cactus_parts[3] = create("y\b\033MY");
    cactus_parts[4] = create("z\b\033MZ");
    cactus_parts[5] = create("n\b\033Mg\b\033Ma");
    cactus_parts[6] = create("-\b\033Mh");
    cactus_parts[7] = create("o\b\033Mi\b\033Mb");
    cactus_parts[8] = create("p\b\033Mj\b\033Mc");
    cactus_parts[9] = create("q\b\033Mk\b\033Md");
    cactus_parts[10] = create("r\b\033Ml\b\033Me");
    cactus_parts[11] = create("s\b\033Mm\b\033Mf");
}

void macro_manager::_init_sounds()
{
    if (_options.sound) {
        game_over_sound = create("\033[4;1;1,~\033[4;1;0,~\033[4;1;1,~");
        jump_sound = create("\033[2;1;3,~");
        score_sound[0] = create("\033[4;1;3,~");
        score_sound[1] = create("\033[4;2;10,~");
        // We play a mute sound on startup to preinitialize the audio, otherwise
        // you can get a stutter when the first sound effect is triggered.
        std::cout << "\033[0;1;1,~";
    }
}

void macro_manager::builder::add(const char* fmt...)
{
    va_list args;
    va_start(args, fmt);
    char text[1024];
    vsnprintf(text, sizeof text, fmt, args);
    va_end(args);
    _buffer.append(text);
}

macro_manager::builder::operator std::string_view() const
{
    return _buffer;
}
