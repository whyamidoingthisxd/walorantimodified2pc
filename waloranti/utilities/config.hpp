#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <Windows.h>
#include <cmath>

namespace cfg
{
	inline int aimbot_key = VK_LBUTTON;  // Default value
	inline int magnet_key{ VK_MBUTTON };
	inline int flick_key{ VK_SPACE }; // Default to Mouse Button 4
	inline int triggerbot_key = VK_XBUTTON1; // Default trigger key
	inline int head_offset_x = 1;

	inline int head_offset_y = -2;

	inline int magnet_fov{ 27 };
	inline int magnet_smooth{ 3 };
	inline int magnet_delay_between_shots{ 200 };

	inline int aimbot_fov{ 27 };
	inline int aimbot_smooth{ 3 };
	inline int recoil_length{ 25 };
	inline double recoil_offset{ 0 };

	inline bool use_perlin_noise = true;  // Toggle for enabling/disabling Perlin noise
	inline float perlin_frequency = 0.5f; // Frequency of Perlin noise
	inline float perlin_amplitude = 5.0f;  // Amplitude of Perlin noise

	inline bool silent_aim = false; // If true, aim snaps back to the original position
	inline float sens = 0.35f;         // Default sensitivity value
	inline float flick_distance = 1.07437623 * pow(sens, -0.9936827126); // Distance calculation formula
	inline int flick_fov_x = 100;     // Horizontal FOV for flickbot
	inline int flick_fov_y = 50;      // Vertical FOV for flickbot
	inline int flick_smooth = 5;      // Smoothing for flickbot movements
	inline int flick_delay = 250;     // Delay in milliseconds between flicks
	inline void update_flick_distance() {
		flick_distance = 1.07437623 * pow(sens, -0.9936827126);
	}

	inline int menorRGB[3] = { 105, 50, 105 };  // Min RGB for purple
	inline int maiorRGB[3] = { 255, 150, 255 }; // Max RGB for purple

	inline int menorHSV[3] = { 260, 20, 40 };   // Min HSV for purple (Hue, Saturation, Value)
	inline int maiorHSV[3] = { 300, 80, 100 };  // Max HSV for purple (Hue, Saturation, Value)

	inline int triggerbot_tolerance = 70; // Tolerance for color change
	inline int triggerbot_delay = 80; // Delay between triggers in ms

	inline bool colorbot_enabled = true;
	inline bool magnet_enabled = true;
	inline bool flickbot_enabled = false;
	inline bool triggerbot_enabled = false; // Enable or disable triggerbot


}

inline int RED_RANGE[] = { 200, 255 };
inline int GREEN_RANGE[] = { 0, 200 };
inline int BLUE_RANGE[] = { 200, 255 };

//inline int RED_RANGE[] = { 200, 255 };
//inline int GREEN_RANGE[] = { 200, 255 };
//inline int BLUE_RANGE[] = { 0, 100 };


inline float RED_DOMINANCE_THRES[] = { 0.8f, 1.0f };
inline float GREEN_DOMINANCE_THRES[] = { 0.0f, 0.80f };
inline float BLUE_DOMINANCE_THRES[] = { 0.5f, 1.0f };

//inline float RED_DOMINANCE_THRES[] = { 0.8f, 1.0f };
//inline float GREEN_DOMINANCE_THRES[] = { 0.8f, 1.0f };
//inline float BLUE_DOMINANCE_THRES[] = { 0.0f, 0.2f };




#endif // !CONFIG_HPP
