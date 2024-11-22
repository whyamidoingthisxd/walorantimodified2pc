#ifndef CONTROL_MOUSE_HPP
#define CONTROL_MOUSE_HPP

#include <Windows.h>
#include <cstdint>
#include <functional>
#include <mutex>
#include <string>
#include <thread>

// Button struct for button mappings
struct Button {
    uint8_t bitmask;
    const char* name;
};

class control_mouse {
private:
    // Private variables
    HANDLE m_handler{ nullptr };                  // Serial handler
    COMSTAT m_status{};                           // Serial status
    DWORD m_errors{ 0 };                          // Serial errors
    bool m_connected{ false };                    // Connection status
    std::atomic<bool> polling{ false };           // Polling state
    std::thread polling_thread;                   // Polling thread
    std::mutex callback_mutex;                    // Mutex for callbacks
    std::function<void(int)> button_callback;     // Button callback

    int last_button_state{ 0 };                   // Tracks button state
    static const Button buttons[];               // Button mappings (defined in .cpp)

    // Private methods
    bool initSerial(const char* com_port, DWORD baud_rate);
    void pollButtons();
    void processIncomingData(const std::string& data);
    void add_overflow(double& Input, double& Overflow);
    void mouseDown(uint8_t btn);
    void mouseUp(uint8_t btn);

public:
    // Constructor and Destructor
    control_mouse(const char* com_port, DWORD baud_rate);
    ~control_mouse();

    // Public methods
    bool is_connected();
    bool write_port(const char* buffer);
    bool send_coordinates(int x, int y);
    void move(double x, double y, double smoothing);
    bool click();
    void processButtonState(int state);
    void registerButtonCallback(std::function<void(int)> callback);
    void startPolling();
    void stopPolling();
};

#endif // CONTROL_MOUSE_HPP
