#include "control_mouse.hpp"
#include <iostream>
#include <cmath>
#include <chrono>
#include <thread>
#include <sstream>

// Define button mappings
const Button control_mouse::buttons[] = {
    {0x01, "LEFT"},
    {0x02, "RIGHT"},
    {0x04, "MIDDLE"},
    {0x08, "MOUSE_X4"},
    {0x10, "MOUSE_X5"}
};

// Constructor
control_mouse::control_mouse(const char* com_port, DWORD baud_rate) {
    if (initSerial(com_port, baud_rate)) {
        m_connected = true;
        startPolling();
        std::cout << "[INFO] Serial port initialized successfully." << std::endl;
    }
    else {
        std::cerr << "[ERROR] Failed to initialize serial port." << std::endl;
    }
}

// Destructor
control_mouse::~control_mouse() {
    stopPolling();
    if (m_handler != INVALID_HANDLE_VALUE) {
        CloseHandle(m_handler);
        std::cout << "[INFO] Serial port closed." << std::endl;
    }
}

// Initializes the serial port
bool control_mouse::initSerial(const char* com_port, DWORD baud_rate) {
    m_handler = CreateFileA(
        com_port,
        GENERIC_READ | GENERIC_WRITE,
        0,
        NULL,
        OPEN_EXISTING,
        0,
        NULL
    );

    if (m_handler == INVALID_HANDLE_VALUE) {
        std::cerr << "[ERROR] Error opening serial port: " << com_port << std::endl;
        return false;
    }

    DCB serialParameters = { 0 };
    serialParameters.DCBlength = sizeof(serialParameters);

    if (!GetCommState(m_handler, &serialParameters)) {
        std::cerr << "[ERROR] Error getting current serial parameters." << std::endl;
        CloseHandle(m_handler);
        m_handler = INVALID_HANDLE_VALUE;
        return false;
    }

    serialParameters.BaudRate = baud_rate;
    serialParameters.ByteSize = 8;
    serialParameters.StopBits = ONESTOPBIT;
    serialParameters.Parity = NOPARITY;

    if (!SetCommState(m_handler, &serialParameters)) {
        std::cerr << "[ERROR] Error setting serial port state." << std::endl;
        CloseHandle(m_handler);
        m_handler = INVALID_HANDLE_VALUE;
        return false;
    }

    COMMTIMEOUTS timeouts = { 0 };
    timeouts.ReadIntervalTimeout = 1;
    timeouts.ReadTotalTimeoutConstant = 1;
    timeouts.ReadTotalTimeoutMultiplier = 1;
    timeouts.WriteTotalTimeoutConstant = 1;
    timeouts.WriteTotalTimeoutMultiplier = 1;

    if (!SetCommTimeouts(m_handler, &timeouts)) {
        std::cerr << "[ERROR] Error setting timeouts." << std::endl;
        CloseHandle(m_handler);
        m_handler = INVALID_HANDLE_VALUE;
        return false;
    }

    return true;
}

// Writes data to the port
bool control_mouse::write_port(const char* buffer) {
    DWORD bytesWritten;
    if (!WriteFile(m_handler, buffer, static_cast<DWORD>(strlen(buffer)), &bytesWritten, NULL)) {
        std::cerr << "[ERROR] Failed to write to serial port." << std::endl;
        return false;
    }
    return true;
}

//// Send coordinates to the Arduino
//bool control_mouse::send_coordinates(int x, int y) {
//    if (!m_connected)
//        return false;
//
//    char data[255] = "";
//
//    char bufferX[100];
//    sprintf_s(bufferX, "%d", x);
//
//    char bufferY[100];
//    sprintf_s(bufferY, "%d", y);
//
//    strcat_s(data, bufferX);
//    strcat_s(data, ",");
//    strcat_s(data, bufferY);
//
//    return this->write_port(data);
//}

// Send coordinates to the Arduino using the km.move protocol
bool control_mouse::send_coordinates(int x, int y) {
    if (!m_connected)
        return false;

    // Format the command as "km.move(x, y)\r\n"
    std::string command = "km.move(" + std::to_string(x) + "," + std::to_string(y) + ")\r\n";

    // Write the command to the serial port
    return write_port(command.c_str());
}

// Adds overflow for smooth mouse movement
void control_mouse::add_overflow(double& Input, double& Overflow) {
    Overflow = std::modf(Input, &Input) + Overflow;

    if (Overflow > 1.0) {
        double Integral = 0.0;
        Overflow = std::modf(Overflow, &Integral);
        Input += Integral;
    }
}

// Moves the mouse
void control_mouse::move(double x, double y, double smoothing) {
    double x_{ 0.0 }, y_{ 0.0 }, overflow_x{ 0.0 }, overflow_y{ 0.0 };

    double u_x{ x / smoothing };
    double u_y{ y / smoothing };

    for (int i{ 1 }; i <= smoothing; ++i) {
        double xI{ i * u_x };
        double yI{ i * u_y };

        add_overflow(xI, overflow_x);
        add_overflow(yI, overflow_y);

        int final_x{ static_cast<int>(xI - x_) };
        int final_y{ static_cast<int>(yI - y_) };

        if (final_x != 0 || final_y != 0) {
            this->send_coordinates(final_x, final_y);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        x_ = xI; y_ = yI;
    }
}

// Simulates a mouse click
bool control_mouse::click() {
    if (!m_connected)
        return false;

    return this->send_coordinates(0, 0);
}

// Processes button states
void control_mouse::processButtonState(int state) {
    int pressed = state & ~last_button_state;
    int released = last_button_state & ~state;

    for (const auto& btn : buttons) {
        if (pressed & btn.bitmask) {
            mouseDown(btn.bitmask);
            std::cout << btn.name << " button pressed." << std::endl;
        }
        if (released & btn.bitmask) {
            mouseUp(btn.bitmask);
            std::cout << btn.name << " button released." << std::endl;
        }
    }

    last_button_state = state;
}

// Handles mouse button press
void control_mouse::mouseDown(uint8_t btn) {
    switch (btn) {
    case 0x01: mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0); break;
    case 0x02: mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, 0); break;
    case 0x04: mouse_event(MOUSEEVENTF_MIDDLEDOWN, 0, 0, 0, 0); break;
    case 0x08: mouse_event(MOUSEEVENTF_XDOWN, 0, 0, XBUTTON1, 0); break;
    case 0x10: mouse_event(MOUSEEVENTF_XDOWN, 0, 0, XBUTTON2, 0); break;
    }
}

// Handles mouse button release
void control_mouse::mouseUp(uint8_t btn) {
    switch (btn) {
    case 0x01: mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0); break;
    case 0x02: mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, 0); break;
    case 0x04: mouse_event(MOUSEEVENTF_MIDDLEUP, 0, 0, 0, 0); break;
    case 0x08: mouse_event(MOUSEEVENTF_XUP, 0, 0, XBUTTON1, 0); break;
    case 0x10: mouse_event(MOUSEEVENTF_XUP, 0, 0, XBUTTON2, 0); break;
    }
}

// Polls button states
void control_mouse::pollButtons() {
    const DWORD bytes_to_read = 1;
    BYTE buffer[1];
    DWORD bytes_read;
    std::string incomingData;

    while (polling) {
        if (ReadFile(m_handler, buffer, bytes_to_read, &bytes_read, NULL)) {
            if (bytes_read > 0) {
                char receivedChar = static_cast<char>(buffer[0]);
                if (receivedChar == '\n') {
                    processIncomingData(incomingData);
                    incomingData.clear();
                }
                else {
                    incomingData += receivedChar;
                }
            }
        }
    }
}

// Processes incoming data
void control_mouse::processIncomingData(const std::string& data) {
    std::lock_guard<std::mutex> lock(callback_mutex);

    if (data.size() >= 4 && data.substr(0, 4) == "BTX:") {
        try {
            int value = std::stoi(data.substr(4));
            if (button_callback) {
                button_callback(value);
            }
        }
        catch (...) {
            std::cerr << "[ERROR] Malformed data: " << data << std::endl;
        }
    }
}

// Check connection status
bool control_mouse::is_connected() {
    if (!ClearCommError(m_handler, &m_errors, &m_status)) {
        m_connected = false;
    }
    return m_connected;
}

// Registers button callback
void control_mouse::registerButtonCallback(std::function<void(int)> callback) {
    std::lock_guard<std::mutex> lock(callback_mutex);
    button_callback = callback;
}

// Starts polling
void control_mouse::startPolling() {
    polling = true;
    polling_thread = std::thread(&control_mouse::pollButtons, this);
}

// Stops polling
void control_mouse::stopPolling() {
    polling = false;
    if (polling_thread.joinable()) {
        polling_thread.join();
    }
}
