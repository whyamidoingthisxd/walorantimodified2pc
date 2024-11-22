#include "MouseHandler.hpp"
#include <windows.h>
#include <iostream>

// Define buttons with bit masks
Button buttons[] = {
    {0x01, "LEFT"},
    {0x02, "RIGHT"},
    {0x04, "MIDDLE"},
    {0x08, "MOUSE_X4"},
    {0x10, "MOUSE_X5"}
};

// Constructor: Initialize with 



MouseHandler::MouseHandler(SerialHandler& serialHandler)
    : serialHandler_(serialHandler), last_button_state(0) {}

MouseHandler::~MouseHandler() {}

void MouseHandler::processButtonState(int current_state) {
    int pressed = current_state & ~last_button_state;  // Detect new button presses
    int released = last_button_state & ~current_state; // Detect button releases

    for (const auto& btn : buttons) {
        if (pressed & btn.bitmask) {
            // Send command for button press to the 2nd PC
            serialHandler_.sendCommand("km.click(" + std::to_string(btn.bitmask) + ")\r\n");
            std::cout << btn.name << " button pressed and sent.\n";
        }
        if (released & btn.bitmask) {
            // Send command for button release to the 2nd PC
            serialHandler_.sendCommand("km.release(" + std::to_string(btn.bitmask) + ")\r\n");
            std::cout << btn.name << " button released and sent.\n";
        }
    }

    last_button_state = current_state; // Update the button state
}

void MouseHandler::mouseDown(uint8_t btn) {
    switch (btn) {
    case 0x01:
        mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
        break;
    case 0x02:
        mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, 0);
        break;
    case 0x04:
        mouse_event(MOUSEEVENTF_MIDDLEDOWN, 0, 0, 0, 0);
        break;
    case 0x08:
        mouse_event(MOUSEEVENTF_XDOWN, 0, 0, XBUTTON1, 0);
        break;
    case 0x10:
        mouse_event(MOUSEEVENTF_XDOWN, 0, 0, XBUTTON2, 0);
        break;
    }
}

void MouseHandler::mouseUp(uint8_t btn) {
    switch (btn) {
    case 0x01:
        mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
        break;
    case 0x02:
        mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, 0);
        break;
    case 0x04:
        mouse_event(MOUSEEVENTF_MIDDLEUP, 0, 0, 0, 0);
        break;
    case 0x08:
        mouse_event(MOUSEEVENTF_XUP, 0, 0, XBUTTON1, 0);
        break;
    case 0x10:
        mouse_event(MOUSEEVENTF_XUP, 0, 0, XBUTTON2, 0);
        break;
    }
}
