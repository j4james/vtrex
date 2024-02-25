// VT-Rex
// Copyright (c) 2024 James Holderness
// Distributed under the MIT License

#pragma once

#include <array>
#include <chrono>

class macro_manager;
class options;

class engine {
public:
    static constexpr int width = 30;
    static constexpr int height = 10;

    engine(const macro_manager& macros, const options& options);
    bool run();

private:
    void _render_landscape();
    void _render_ground();
    void _render_clouds();
    void _render_trex();
    void _render_score();
    void _render_high_score();
    void _play_sound_effects();

    const macro_manager& _macros;
    const options& _options;

    int _distance = 0;
    bool _game_over = false;
    volatile bool _jump_pressed = false;
    bool _jump_required = false;
    int _jump_time = 0;
    std::chrono::milliseconds _frame_len;

    template <class _Ty, int _Size>
    class buffer {
    public:
        bool empty() const;
        void push_back(const _Ty value);
        _Ty pop_front();
        _Ty operator[](const int offset) const;

    private:
        std::array<_Ty, _Size> _values = {};
        int _front = 0;
        int _back = 0;
    };

    buffer<char, width> _ground_buffer;
    buffer<int, 10> _cloud_buffer;
    buffer<int, width * 2> _cactus_buffer;
    int _last_cloud_pos = 0;
    int _last_cactus_pos = 0;
};
