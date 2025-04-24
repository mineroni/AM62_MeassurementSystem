#include "gpiod.h"
#include <csignal>
#include <iostream>
#include <chrono>
#include <thread>

// Serial input port for the meassurement
//SerialPort serialPort = new SerialPort("/dev/verdin-uart1", 9600);

// GPIO chip to define the pins on
#define CHIP_NAME "gpiochip1"
// GPIO input pin
#define IN_PIN      (1)
// GPIO output pin
#define OUT_PIN     (2)
// Flag to indicate if the pin is set to high
bool isHigh = false;
// Flag to indicate exit interrupt
bool running = true;

// Output pin global variable to be modificable anywhere
gpiod_line* out_line;

void setPin(char* cause)
{
    gpiod_line_set_value(out_line, 0); // Set the pin to low
    std::cout << cause << "\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(10)); // Wait for 10ms
    gpiod_line_set_value(out_line, 1); // Set the pin to high
}

// Ctrl+C signal handler
void handleSigint(int)
{
    running = false;
}

int main()
{
    // Setting exit handler
    std::signal(SIGINT, handleSigint); // Allow Ctrl+C to exit

    // Opening the GPIO chip
    gpiod_chip* chip = gpiod_chip_open_by_name(CHIP_NAME);
    if (!chip)
    {
        std::cerr << "Failed to open GPIO chip\n";
        return 1;
    }

    // Getting the GPIO input line
    gpiod_line* in_line = gpiod_chip_get_line(chip, IN_PIN);
    // Getting the GPIO output line
    out_line = gpiod_chip_get_line(chip, OUT_PIN);
    if (!in_line || !out_line) 
    {
        std::cerr << "Failed to get GPIO line\n";
        gpiod_chip_close(chip);
        return 1;
    }

    // Request the line as input with edge detection
    if (gpiod_line_request_falling_edge_events(in_line, "gpio_interrupt_falling") < 0)
    {
        std::cerr << "Failed to request input event\n";
        gpiod_chip_close(chip);
        return 1;
    }

    // Request the line as output
    if (gpiod_line_request_output(out_line, "gpio_output_line", 1) < 0)
    {
        std::cerr << "Failed to request output line\n";
        gpiod_chip_close(chip);
        return 1;
    }

    std::cout << "Waiting for falling edge on GPIO " << IN_PIN << "...\n";

    while (running) 
    {
        gpiod_line_event event;
        int ret = gpiod_line_event_wait(in_line, nullptr); // blocking wait
        if (ret < 0) 
        {
            std::cerr << "Error while waiting for event\n";
            break;
        } 
        else if (ret == 0)
        {
            continue; // timeout occurred, not expected here
        }

        if (gpiod_line_event_read(in_line, &event) == 0) 
        {
            setPin("Falling edge detected");
        }
    }

    gpiod_line_release(in_line);
    gpiod_line_release(out_line);
    gpiod_chip_close(chip);

    return 0;
}
