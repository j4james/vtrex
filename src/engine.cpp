// VT-Rex
// Copyright (c) 2024 James Holderness
// Distributed under the MIT License

#include "engine.h"

#include "macros.h"
#include "options.h"
#include "os.h"

#include <iomanip>
#include <iostream>
#include <random>
#include <thread>
#include <vector>

using namespace std::string_literals;
using namespace std::chrono_literals;

using std::chrono::duration_cast;
using std::chrono::milliseconds;
using std::chrono::seconds;

static auto rand_seed = std::random_device{};
static auto rand_engine = std::mt19937{rand_seed()};

engine::engine(const macro_manager& macros, const options& options)
    : _macros{macros}, _options{options}
{
}

bool engine::run()
{
    volatile auto exit_requested = false;
    volatile auto keyboard_shutdown = false;
    auto keyboard_thread = std::thread([&]() {
        while (!keyboard_shutdown && !exit_requested) {
            const auto ch = os::getch();
            if (ch == 32) {
                _jump_pressed = true;
            } else if (ch == 27 || ch == 3) {
                exit_requested = true;
            }
        }
    });

    _render_high_score();

    // We need to clear out pages 2 and 3 at the start of each run. On some
    // terminals (like PowerTerm and RLogin) this must be done with ED2 for
    // it to work on a background page. We also designate the soft font on
    // these two pages - it shouldn't be necessary, but RLogin requires it.
    std::cout << "\033[3 P\033[2J\033( @";
    std::cout << "\033[2 P\033[2J\033( @";

    // RLogin also requires that the origin mode is set on the the specific
    // page where it's needed, which for us is page 2.
    std::cout << "\033[?6h";

    // We start by rendering the ground for the full width of the game area.
    std::cout << "\033[10H";
    for (auto i = 0; i < width; i++)
        _render_ground();

    const auto start_time = std::chrono::steady_clock::now();
    const auto start_frame_len = 1000ms / _options.fps;
    auto frame_end = start_time + 1000ms;
    for (_distance = 0; !exit_requested; _distance++) {
        // We speed up over time by shortening the frame length by 250us every 1s.
        const auto elapsed = duration_cast<seconds>(frame_end - start_time);
        _frame_len = duration_cast<milliseconds>(start_frame_len - 250us * elapsed.count());
        _frame_len = std::max(_frame_len, 33ms);

        // Every frame we scroll the landscape left by one column, and add a
        // new piece of ground, but the clouds move at a slower rate, so we
        // only scroll them on every second frame. However, there are two
        // versions of the cloud layer, one of which is offset by a half a
        // column, and we swap between these two renditions on every frame.
        // So this way they are actually moving every frame, but with a half
        // column step each time.
        if ((_distance & 1) == 0) {
            _macros.scroll_start.run();
            _render_landscape();
            _macros.scroll_end.run();
        } else {
            _macros.scroll_start_with_clouds.run();
            _render_landscape();
            _render_clouds();
            _macros.scroll_end_with_clouds.run();
        }

        // The landscape scrolling takes place on page 2, but once it's done
        // the content is copied onto page 3, so we can render the dinosaur
        // on top of that.
        _render_trex();

        // Once that's done, we'll copy the final composited frame back to
        // page 1 (the visible page), and add update the current score.
        _macros.frame_complete.run();
        _render_score();

        // Any sound effects must be output as the last step in this sequence,
        // because they'll block further output until they're complete.
        _play_sound_effects();
        std::cout.flush();
        if (_game_over) break;

        std::this_thread::sleep_until(frame_end);
        frame_end += _frame_len;
    }

    if (!exit_requested) {
        _macros.game_over_banner.run();
        _render_high_score();
        _macros.game_over_sound.run();
        std::cout.flush();
        std::this_thread::sleep_for(500ms);
    }

    keyboard_shutdown = true;
    keyboard_thread.join();
    return !exit_requested;
}

void engine::_render_landscape()
{
    static const auto cactus_types = std::array<std::vector<size_t>, 6>{{
        {1},
        {1, 2},
        {1, 3, 4},
        {5, 6},
        {5, 7, 8},
        {5, 9, 10, 11},
    }};
    static auto rand_cactus_type = std::uniform_int_distribution<>{0, 60};
    static auto last_type = -1;

    const auto render_cactus = [&]() {
        if (_cactus_buffer.empty()) {
            if (_distance - _last_cactus_pos < 15) return false;
            auto type = rand_cactus_type(rand_engine);
            if (_distance - _last_cactus_pos >= width + 4) type %= 6;
            if (type == last_type) type = (type + 1) % 6;
            if (type >= 6) return false;
            _last_cactus_pos = _distance;
            last_type = type;
            for (auto cactus_part : cactus_types[type])
                _cactus_buffer.push_back(cactus_part);
        }
        _macros.cactus_parts[_cactus_buffer.pop_front()].run();
        return true;
    };

    if (!render_cactus()) {
        _cactus_buffer.push_back(0);
        _cactus_buffer.pop_front();
        _render_ground();
    }

    const auto cactus_present = [&](const auto distance_from_left) {
        const auto distance_from_right = width - distance_from_left;
        return _cactus_buffer[-distance_from_right] > 0;
    };
    _jump_required = cactus_present(3) || cactus_present(4);
}

void engine::_render_ground()
{
    static const auto flat_ground = "=-~_~-_-=-_-_~_-_~_=~-_-~-=-_-"s;
    static const auto bumpy_ground = "=-~_~-#$%-_-_~_-_~_=~-*+~-=-_-"s;
    static auto rand_ground = std::uniform_int_distribution<>{0, 3};
    if (_ground_buffer.empty()) {
        const auto& ground = rand_ground(rand_engine) ? flat_ground : bumpy_ground;
        for (auto ch : ground)
            _ground_buffer.push_back(ch);
    }
    std::cout << _ground_buffer.pop_front();
}

void engine::_render_clouds()
{
    static auto rand_cloud_height = std::uniform_int_distribution<>{0, 30};
    static auto last_height = -1;
    if (_cloud_buffer.empty()) {
        auto height = rand_cloud_height(rand_engine);
        if (_distance - _last_cloud_pos >= width) height %= 3;
        if (height == last_height) height = (height + 1) % 3;
        if (height > 2) return;
        _last_cloud_pos = _distance;
        last_height = height;
        for (auto i = 0; i < 3; i++)
            _cloud_buffer.push_back(height * 3 + i);
    }
    _macros.cloud_parts[_cloud_buffer.pop_front()].run();
}

void engine::_render_trex()
{
    static constexpr auto jump_heights = std::array{0, 2, 4, 6, 7, 8, 8, 7, 6, 4, 2, 0};
    auto height = 0;
    auto next_height = 0;
    if (_jump_pressed) {
        _jump_time++;
        height = jump_heights[_jump_time];
        if (_jump_time + 1 >= jump_heights.size()) {
            _jump_time = 0;
            _jump_pressed = false;
        } else {
            next_height = jump_heights[_jump_time + 1];
        }
    }

    _game_over = _jump_required && next_height < 4;

    if (height > 0)
        _macros.trex_jumping[height].run();
    else if (_distance == 0 || _game_over)
        _macros.trex_standing.run();
    else
        _macros.trex_running[(_distance >> 1) & 1].run();

    if (_game_over)
        _macros.trex_dead[height >> 1].run();
}

void engine::_render_score()
{
    const auto score = _distance >> 1;
    const auto frame_units = _distance % 200;
    const auto blink_segment = std::max<int>(500ms / _frame_len, 1);
    const auto blink_duration = 1700ms / _frame_len;
    const auto blink_visible = (frame_units % blink_segment) >= (blink_segment >> 1);
    if (_game_over || score < 100 || frame_units > blink_duration)
        std::cout << std::setfill('0') << std::setw(5) << (score % 100000);
    else if (blink_visible || !_options.blink)
        std::cout << std::setfill('0') << std::setw(5) << (score % 100000 - score % 100);
    else
        std::cout << "     ";
}

void engine::_render_high_score()
{
    static auto high_score = 0;
    const auto score = _distance >> 1;
    high_score = std::max(high_score, score);
    if (high_score > 0) {
        _macros.high_score_label.run();
        std::cout << std::setfill('0') << std::setw(5) << (high_score % 100000);
    }
}

void engine::_play_sound_effects()
{
    if (_options.sound && !_game_over) {
        const auto score = _distance >> 1;
        const auto score_unit = score % 100;
        if (score_unit < 2 && score >= 100 && (_distance & 1) == 0)
            _macros.score_sound[score_unit].run();
        else if (_jump_time == 1)
            _macros.jump_sound.run();
    }
}

template <class _Ty, int _Size>
bool engine::buffer<_Ty, _Size>::empty() const
{
    return _front >= _back;
}

template <class _Ty, int _Size>
void engine::buffer<_Ty, _Size>::push_back(const _Ty value)
{
    _values[_back++ % _Size] = value;
}

template <class _Ty, int _Size>
_Ty engine::buffer<_Ty, _Size>::pop_front()
{
    return _values[_front++ % _Size];
}

template <class _Ty, int _Size>
_Ty engine::buffer<_Ty, _Size>::operator[](const int offset) const
{
    return _values[(_front + offset + _Size) % _Size];
}
