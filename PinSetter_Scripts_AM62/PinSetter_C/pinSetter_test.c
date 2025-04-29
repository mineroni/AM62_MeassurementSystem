#include "gpiod.h"
#include <unistd.h>
#include <stdio.h>
#include <signal.h>

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

/*void setPin(const char* cause)
{
    gpiod_line_request_set_value(out_line, 1, 0); // Set the pin to low
    printf("%s\n", cause);
    sleep(1);
    gpiod_line_request_set_value(out_line, 1, 1); // Set the pin to high
}*/

struct gpiod_line_request* initPin(char* chipName, unsigned int pinNumber, enum gpiod_line_direction direction)
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
    
    struct gpiod_line_config* config = gpiod_line_config_new();
    if (!config)
    {
        perror("Failed to create line config");
        gpiod_line_settings_free(settings);
        gpiod_chip_close(chip);
        return NULL;
    }

    unsigned int offset = pinNumber; // The offset of the GPIO line    
    if (gpiod_line_config_add_line_settings(config, &offset, 1, settings) < 0) 
    {
        perror("Failed to add line settings");
        gpiod_line_settings_free(settings);
        gpiod_line_config_free(config);
        gpiod_chip_close(chip);
        return NULL;
    }

    struct gpiod_line_request* request = gpiod_chip_request_lines(chip, NULL, config);
    if (!request) 
    {
        perror("Failed to request GPIO line");
        gpiod_line_config_free(config);
        gpiod_line_settings_free(settings);
        gpiod_chip_close(chip);
        return NULL;
    }

    gpiod_line_settings_free(settings);
    gpiod_line_config_free(config);
    gpiod_chip_close(chip);

    return request;
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

    struct gpiod_line_request* inputPin = initPin(CHIP_NAME, IN_PIN, GPIOD_LINE_DIRECTION_INPUT);
    struct gpiod_line_request* outputPin = initPin(CHIP_NAME, OUT_PIN, GPIOD_LINE_DIRECTION_OUTPUT);
    if (inputPin)
    {
        printf("Pin %d initialized successfully\n", IN_PIN);
    }
    if (outputPin)
    {
        printf("Pin %d initialized successfully\n", OUT_PIN);
    }
    

    if (!inputPin || !outputPin)
    {
        perror("Failed to initialize GPIO pins\n");
        return 1;
    }    

    while (running)
    {
        gpiod_line_request_set_value(outputPin, OUT_PIN, 0);
        printf("GPIO %d set to low\n", OUT_PIN);
        sleep(1);
        gpiod_line_request_set_value(outputPin, OUT_PIN, 1);
        printf("GPIO %d set to high\n", OUT_PIN);
        sleep(1);
    }

    gpiod_line_request_release(inputPin);
    gpiod_line_request_release(outputPin);
    return 0;
}
