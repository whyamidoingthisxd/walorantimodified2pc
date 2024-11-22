#ifndef MOUSEHANDLER_HPP
#define MOUSEHANDLER_HPP

#include <cstdint>
#include "../SerialHandler.hpp"

// Define the buttons with bit masks and names
struct Button {
    uint8_t bitmask;
    const char* name;
};

class MouseHandler {
public:
    MouseHandler(SerialHandler& serialHandler);
    ~MouseHandler();

    // Process the button state
    void processButtonState(int state);

private:
    SerialHandler& serialHandler_;
    int last_button_state;

    void mouseDown(uint8_t btn);
    void mouseUp(uint8_t btn);
};

#endif // MOUSEHANDLER_HPP
