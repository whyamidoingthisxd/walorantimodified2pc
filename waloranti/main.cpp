#include "control_mouse/control_mouse.hpp"
#include "enemy_scanner/enemy_scanner.hpp"
#include "stopwatch/stopwatch.hpp"
#include "utilities/utilities.hpp"
#include "utilities/config.hpp"
#include "utilities/skcrypt.hpp"
#include "render_menu/render_menu.hpp"
#include <thread>
#include <chrono>

void handle_triggerbot(control_mouse& mouse, enemy_scanner& enemy, stopwatch& timer) {
    if (!cfg::triggerbot_enabled || !utilities::is_pressed(cfg::triggerbot_key)) return;

    // Check if triggerbot detects a target and delay has passed
    if (enemy.triggerbot_logic() && timer.get_elapsed() > cfg::triggerbot_delay) {
        mouse.click(); // Send "km.click" command
        timer.update(); // Reset the timer
    }
}

// Recoil control logic
void recoil_control() {
    utilities::set_thread_priority(THREAD_PRIORITY_TIME_CRITICAL);
    stopwatch timer;

    while (true) {
        if (utilities::is_pressed(cfg::aimbot_key)) {
            if (timer.get_elapsed() > 90.0) {
                if (cfg::recoil_offset < cfg::recoil_length) {
                    cfg::recoil_offset += 0.125;
                }
                else {
                    cfg::recoil_offset = cfg::recoil_length;
                }
            }
        }
        else {
            if (cfg::recoil_offset > 0) {
                cfg::recoil_offset -= 0.125;
            }
            else {
                // Not shooting and recoil reset
                cfg::recoil_offset = 0;
                timer.update();
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

// Flick logic function
void handle_flickbot(control_mouse& mouse, enemy_scanner& enemy, stopwatch& timer) {
    if (!cfg::flickbot_enabled || !utilities::is_pressed(cfg::flick_key)) return;

    // Find the closest enemy within the flick FOV
    std::vector<int> enemy_head = enemy.find_flick_target(cfg::flick_fov_x, cfg::flick_fov_y);

    if (enemy_head[0] == 0 && enemy_head[1] == 0) return; // No enemy found

    // Check if the delay between flicks has passed
    if (timer.get_elapsed() < cfg::flick_delay) return;

    // Calculate the vector for the flick
    float move_x = enemy_head[0] * cfg::flick_distance;
    float move_y = enemy_head[1] * cfg::flick_distance;

    // Instantly flick to the target
    mouse.move(move_x, move_y, 1); // Move instantly (smoothing = 1)

    // Simulate a click
    mouse.click();

    // If silent aim is enabled, flick back to the original position
    if (cfg::silent_aim) {
        mouse.move(-move_x, -move_y, 1);
    }

    // Update the flick timer
    timer.update();
}

// Render menu logic
void menu_thread() {
    utilities::set_thread_priority(THREAD_PRIORITY_BELOW_NORMAL);

    render_menu::create_window(L"Audacity");
    render_menu::create_device();
    render_menu::create_imgui();

    while (render_menu::is_running) {
        render_menu::begin_render();
        render_menu::render();
        render_menu::end_render();

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    render_menu::destroy_imgui();
    render_menu::destroy_device();
    render_menu::destroy_window();
}

// Button state callback
void handleButtonState(int state, control_mouse& mouse) {
    mouse.processButtonState(state);
}

// Main logic
int main() {
    utilities::set_process_priority(REALTIME_PRIORITY_CLASS);
    utilities::set_timer_resolution();

    // Initialize control_mouse instance
    control_mouse mouse("\\\\.\\COM3", 115200);

    // Start button polling with callback
    mouse.registerButtonCallback([&mouse](int state) { handleButtonState(state, mouse); });

    enemy_scanner enemy;  // Initialize enemy_scanner
    stopwatch timer;      // Initialize stopwatch

    std::thread t_recoil(recoil_control);
    std::thread t_menu(menu_thread);
    t_recoil.detach();
    t_menu.detach();

    std::vector<int> enemy_head{};
    while (mouse.is_connected() && render_menu::is_running) {

        if (cfg::colorbot_enabled || cfg::magnet_enabled || cfg::flickbot_enabled || cfg::triggerbot_enabled) {
            enemy.update(); // Update the screen capture only if any feature is enabled
        }

        if (utilities::is_pressed(cfg::aimbot_key) || utilities::is_pressed(cfg::magnet_key) || utilities::is_pressed(cfg::flick_key) || utilities::is_pressed(cfg::triggerbot_key)) {
            enemy.update();
        }

        if (cfg::flickbot_enabled && utilities::is_pressed(cfg::flick_key)) {
            // Flickbot logic
            std::vector<int> flick_target = enemy.find_flick_target(cfg::flick_fov_x, cfg::flick_fov_y);
            if (flick_target[0] != 0 || flick_target[1] != 0) {
                mouse.move(flick_target[0] * cfg::flick_distance, flick_target[1] * cfg::flick_distance, cfg::flick_smooth);
                mouse.click();
                std::this_thread::sleep_for(std::chrono::milliseconds(cfg::flick_delay));
            }
        }

        if (cfg::triggerbot_enabled && utilities::is_pressed(cfg::triggerbot_key)) {
            handle_triggerbot(mouse, enemy, timer);
        }

        if (cfg::magnet_enabled && utilities::is_pressed(cfg::magnet_key)) {
            std::vector<int> enemy_head = enemy.find_closest_enemy_head(cfg::magnet_fov);
            if (enemy_head[0] != 0 || enemy_head[1] != 0) {
                mouse.move(enemy_head[0], enemy_head[1], cfg::magnet_smooth);
            }

            if (enemy.is_enemy_in_crosshair() && timer.get_elapsed() > cfg::magnet_delay_between_shots) {
                mouse.click();
                timer.update();
            }
        }

        else if (cfg::colorbot_enabled && utilities::is_pressed(cfg::aimbot_key)) {
            std::vector<int> enemy_head = enemy.find_closest_enemy_head(cfg::aimbot_fov + static_cast<int>(cfg::recoil_offset));
            if (enemy_head[0] != 0 || enemy_head[1] != 0) {
                mouse.move(enemy_head[0], enemy_head[1], cfg::aimbot_smooth);
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1)); // Avoid CPU overutilization
    }

    return EXIT_FAILURE;
}
