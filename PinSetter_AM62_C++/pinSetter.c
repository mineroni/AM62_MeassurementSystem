#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <time.h>
#include "gpiod.h"

// GPIO chip to define the pins on
#define CHIP_NAME "/dev/gpiochip1"
// GPIO input pin
#define IN_PIN      (1)
// GPIO output pin
#define OUT_PIN     (2)

// Flags for state
volatile int running = 1;

// Output pin global variable
struct gpiod_line* out_line;

// Sleep function for milliseconds
void sleep_ms(int milliseconds)
{
    struct timespec ts;
    ts.tv_sec = milliseconds / 1000;
    ts.tv_nsec = (milliseconds % 1000) * 1000000;
    nanosleep(&ts, NULL);
}

void setPin(const char* cause)
{
    gpiod_line_request_set_value(out_line, 0); // Set the pin to low
    printf("%s\n", cause);
    sleep_ms(10);                      // Wait for 10ms
    gpiod_line_request_set_value(out_line, 1); // Set the pin to high
}

// Ctrl+C signal handler
void handleSigint(int signum)
{
    (void)signum;
    running = 0;
}

int main()
{
    signal(SIGINT, handleSigint); // Handle Ctrl+C

    struct gpiod_chip* chip = gpiod_chip_open(CHIP_NAME);
    if (!chip)
    {
        perror("Failed to open GPIO chip");
        return 1;
    }

    struct gpiod_line* in_line = gpiod_chip_get_line(chip, IN_PIN);
    out_line = gpiod_chip_get_line(chip, OUT_PIN);
    if (!in_line || !out_line)
    {
        fprintf(stderr, "Failed to get GPIO line\n");
        gpiod_chip_close(chip);
        return 1;
    }

    if (gpiod_line_request_falling_edge_events(in_line, "gpio_interrupt_falling") < 0)
    {
        perror("Failed to request input event");
        gpiod_chip_close(chip);
        return 1;
    }

    if (gpiod_line_request_output(out_line, "gpio_output_line", 1) < 0)
    {
        perror("Failed to request output line");
        gpiod_chip_close(chip);
        return 1;
    }

    printf("Waiting for falling edge on GPIO %d...\n", IN_PIN);

    while (running)
    {
        struct gpiod_line_event event;
        int ret = gpiod_line_event_wait(in_line, NULL); // blocking wait
        if (ret < 0)
        {
            perror("Error while waiting for event");
            break;
        }
        else if (ret == 0)
        {
            continue; // Timeout occurred
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
