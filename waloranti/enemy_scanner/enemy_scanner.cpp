#define NOMINMAX // Prevent Windows macros for min and max

#include "enemy_scanner.hpp"
#include "../utilities/config.hpp"
#include <initializer_list>
#include <algorithm> // For std::max and std::min
#include <cmath>     // For abs() and other math functions


bool enemy_scanner::is_enemy_outline(const std::vector<int>& pixel) const {
    // Variables for HSV conversion
    float hue = 0.0f;
    float saturation = 0.0f;
    float value = 0.0f;

    // RGB to HSV conversion
    float red = pixel[0] / 255.0f;
    float green = pixel[1] / 255.0f;
    float blue = pixel[2] / 255.0f;

    // Use std::max and std::min for RGB range calculation
    float max_value = std::max(red, std::max(green, blue));
    float min_value = std::min(red, std::min(green, blue));
    float delta = max_value - min_value;

    // Calculate Saturation
    if (max_value != 0) {
        saturation = delta / max_value;
    }

    // Calculate Hue
    if (delta != 0) {
        if (max_value == red) {
            hue = (green - blue) / delta + (green < blue ? 6 : 0);
        }
        else if (max_value == green) {
            hue = (blue - red) / delta + 2;
        }
        else {
            hue = (red - green) / delta + 4;
        }
        hue *= 60; // Convert to degrees
    }

    // Value is the maximum RGB component
    value = max_value;

    // Convert HSV values to integers
    int hue_int = static_cast<int>(hue);
    int saturation_int = static_cast<int>(saturation * 100);
    int value_int = static_cast<int>(value * 100);

    // Purple detection logic
    if (pixel[0] >= cfg::menorRGB[0] && pixel[0] <= cfg::maiorRGB[0] &&
        pixel[1] >= cfg::menorRGB[1] && pixel[1] <= cfg::maiorRGB[1] &&
        pixel[2] >= cfg::menorRGB[2] && pixel[2] <= cfg::maiorRGB[2] &&
        hue_int >= cfg::menorHSV[0] && hue_int <= cfg::maiorHSV[0] &&
        saturation_int >= cfg::menorHSV[1] && saturation_int <= cfg::maiorHSV[1] &&
        value_int >= cfg::menorHSV[2] && value_int <= cfg::maiorHSV[2]) {
        return true;
    }

    // Additional brightness filtering
    if (pixel[0] + pixel[1] - pixel[2] < 450) {
        return false; // Ignore less bright pixels
    }

    return false;
}





bool enemy_scanner::is_enemy_outline_old( const std::vector< int >& pixel ) const
{
    if (pixel[1] >= 170) {
        return false;
    }

    if (pixel[1] >= 120) {
        return abs(pixel[0] - pixel[2]) <= 8 &&
            pixel[0] - pixel[1] >= 50 &&
            pixel[2] - pixel[1] >= 50 &&
            pixel[0] >= 105 &&
            pixel[2] >= 105;
    }

    return abs(pixel[0] - pixel[2]) <= 13 &&
        pixel[0] - pixel[1] >= 60 &&
        pixel[2] - pixel[1] >= 60 &&
        pixel[0] >= 110 &&
        pixel[2] >= 100;
}

bool enemy_scanner::is_enemy_in_crosshair() const {
    const int center_x{ this->capture->get_width() / 2 };
    const int center_y{ this->capture->get_height() / 2 };

    bool found_left_side{ false };
    bool center_aligned{ false };
    bool found_right_side{ false };

    double avg_point{ 0.0 };
    int idx{ 0 };

    std::vector<vec2> vects;

    for (int x{ center_x - cfg::magnet_fov }; x < center_x + cfg::magnet_fov; ++x) {
        for (int y{ center_y - cfg::magnet_fov }; y < center_y + cfg::magnet_fov; ++y) {
            if (!is_enemy_outline(this->capture->get_rgb(x, y))) { // Use updated logic
                continue; // Skip non-target pixels
            }

            if (x < center_x) {
                found_left_side = true;
            }
            else if (x > center_x) {
                found_right_side = true;
            }

            vects.push_back(vec2(x - center_x, y - center_y));

            for (auto& p : vects) {
                ++idx;

                avg_point += sqrt(pow(p.x, 2.0) + pow(p.y, 2.0));

                if (!(avg_point / idx < cfg::magnet_fov) || idx < 4) {
                    continue;
                }

                center_aligned = true;

                break;
            }

            if (!found_right_side || !found_left_side || !center_aligned) {
                continue;
            }

            return true;
        }
    }

    return false;
}



std::vector<int> enemy_scanner::find_closest_enemy_head(int fov_type) const {
    const int center_x{ this->capture->get_width() / 2 };
    const int center_y{ this->capture->get_height() / 2 };

    // Initialize bounding box values
    int min_x = center_x + fov_type; // Start with max possible values
    int max_x = center_x - fov_type;
    int min_y = center_y + fov_type;
    int max_y = center_y - fov_type;

    // Loop through the FOV to find bounding box
    for (int y{ center_y - fov_type }; y < center_y + fov_type; ++y) {
        for (int x{ center_x - fov_type }; x < center_x + fov_type; ++x) {
            if (!this->is_enemy_outline(this->capture->get_rgb(x, y))) { // Use updated logic
                continue; // Skip non-target pixels
            }

            // Update bounding box values
            if (x < min_x) min_x = x;
            if (x > max_x) max_x = x;
            if (y < min_y) min_y = y;
            if (y > max_y) max_y = y;
        }
    }

    // Check if no bounding box was found
    if (min_x > max_x || min_y > max_y) {
        return { 0, 0 }; // No target found
    }

    // Calculate target position: aim slightly below the topmost part of the bounding box
    int target_x = (min_x + max_x) / 2 + cfg::head_offset_x; // Add horizontal offset
    int target_y = min_y + cfg::head_offset_y; // Slightly below the top of the box

    // Return target relative to the screen center
    return { target_x - center_x, target_y - center_y };
}



std::vector<int> enemy_scanner::find_flick_target(int fov_x, int fov_y) const {
    int center_x = this->capture->get_width() / 2; // Calculate screen center
    int center_y = this->capture->get_height() / 2;

    for (int y = center_y - fov_y; y < center_y + fov_y; ++y) {
        for (int x = center_x - fov_x; x < center_x + fov_x; ++x) {
            if (!this->is_enemy_outline_old(this->capture->get_rgb(x, y))) {
                continue; // Skip if the pixel doesn't match an enemy outline
            }

            return { x - center_x, y - center_y }; // Return relative position
        }
    }

    return { 0, 0 }; // Return (0, 0) if no target found
}

