#include "gpiod.h"
#include <unistd.h>
#include <stdio.h>
#include <signal.h>

#include <termios.h>
#include <fcntl.h>

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

// Serial port to read from
#define SERIAL_PORT "/dev/verdin-uart1"
// GPIO chip to define the pins on
#define CHIP_NAME "/dev/gpiochip1"
// GPIO output pin
#define OUT_PIN     (2)

// Ctrl+C signal handler
volatile int running = 1;
void handleSigint(int signum)
{
    (void)signum;
    running = 0;
}

// Function to initialize a GPIO pin
struct gpiod_line_request* initPin(char* chipName, unsigned int pinNumber, enum gpiod_line_direction direction, enum gpiod_line_edge edgePolarity)
{
    struct gpiod_chip* chip = gpiod_chip_open(chipName);
    if (!chip)
    {
        perror("Failed to open GPIO chip");
        return NULL;
    }

    struct gpiod_line_settings* settings = gpiod_line_settings_new();
    if (!settings)
    {
        perror("Failed to create line settings");
        gpiod_chip_close(chip);
        return NULL;
    }

    gpiod_line_settings_set_direction(settings, direction);

    if (direction == GPIOD_LINE_DIRECTION_OUTPUT)
    {
        gpiod_line_settings_set_output_value(settings, 0); // Set initial value to low
    }
    else if (direction == GPIOD_LINE_DIRECTION_INPUT && edgePolarity != GPIOD_LINE_EDGE_NONE)
    {
        gpiod_line_settings_set_debounce_period_us(settings, 0); // Set 0 debounce period for immediate detection
        gpiod_line_settings_set_edge_detection(settings, edgePolarity); // Set up edge detection for input
    }
    
    struct gpiod_line_config* config = gpiod_line_config_new();
    if (!config)
    {
        perror("Failed to create line config");
        gpiod_line_settings_free(settings);
        gpiod_chip_close(chip);
        return NULL;
    }
  
    if (gpiod_line_config_add_line_settings(config, &pinNumber, 1, settings) < 0) 
    {
        perror("Failed to add line settings");
        gpiod_line_settings_free(settings);
        gpiod_line_config_free(config);
        gpiod_chip_close(chip);
        return NULL;
    }

    struct gpiod_line_request* request = gpiod_chip_request_lines(chip, NULL, config);

    gpiod_line_settings_free(settings);
    gpiod_line_config_free(config);
    gpiod_chip_close(chip);

    if (!request) 
    {
        perror("Failed to request GPIO line");
        return NULL;
    }

    return request;
}

int setup_serial(const char *device)
{
    int fd = open(device, O_RDONLY | O_NOCTTY);
    if (fd < 0) 
    {
        perror("Serial open failed");
        return -1;
    }
    struct termios tty;
    memset(&tty, 0, sizeof tty);
    if (tcgetattr(fd, &tty) != 0)
    {
        perror("tcgetattr failed");
        close(fd);
        return -1;
    }

    cfsetospeed(&tty, B9600);
    cfsetispeed(&tty, B9600);

    tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;
    tty.c_iflag = 0;
    tty.c_oflag = 0;
    tty.c_lflag = 0; // raw mode

    tcflush(fd, TCIFLUSH);
    if (tcsetattr(fd, TCSANOW, &tty) != 0) 
    {
        perror("tcsetattr failed");
        close(fd);
        return -1;
    }
    return fd;
}

int main() 
{
    signal(SIGINT, handleSigint); // Handle Ctrl+C

    struct gpiod_line_request* outputPin = initPin(CHIP_NAME, OUT_PIN, GPIOD_LINE_DIRECTION_OUTPUT, GPIOD_LINE_EDGE_NONE);
    
    int serial_fd = setup_serial(SERIAL_PORT);
    if (serial_fd < 0) 
    {
        perror("Failed to open serial port");
        gpiod_line_request_release(outputPin);
        return 1;
    }

    printf("Waiting for message 0x01 on %s.\n", SERIAL_PORT);

    while (running) 
    {
        uint8_t byte;
        int n = read(serial_fd, &byte, 1);
        if (n > 0 && byte == 0x01)
        {
            gpiod_line_request_set_value(outputPin, OUT_PIN, 1);
            usleep(10000); // Keep the output high for 10ms
            gpiod_line_request_set_value(outputPin, OUT_PIN, 0);
            printf("Message 0x01 detected on serial port %s.\n", SERIAL_PORT);
        }
    }

    gpiod_line_request_release(outputPin);
    close(serial_fd);
    return 0;
}