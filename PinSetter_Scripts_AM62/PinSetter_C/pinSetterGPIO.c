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

int main()
{
    signal(SIGINT, handleSigint); // Handle Ctrl+C

    struct gpiod_edge_event_buffer* events = gpiod_edge_event_buffer_new(1);
    if (!events) 
    {
        perror("Failed to create edge event buffer");
        return 1;
    }

    struct gpiod_line_request* inputPin = initPin(CHIP_NAME, IN_PIN, GPIOD_LINE_DIRECTION_INPUT, GPIOD_LINE_EDGE_FALLING);
    struct gpiod_line_request* outputPin = initPin(CHIP_NAME, OUT_PIN, GPIOD_LINE_DIRECTION_OUTPUT, GPIOD_LINE_EDGE_NONE);

    if (!inputPin || !outputPin)
    {
        perror("Failed to initialize GPIO pins\n");
        gpiod_edge_event_buffer_free(events);
        gpiod_line_request_release(inputPin);
        gpiod_line_request_release(outputPin);
        return 1;
    }    
    
    printf("Waiting for falling edge on GPIO pin %d.\n", IN_PIN);

    while (running)
    {
        int ret = gpiod_line_request_wait_edge_events(inputPin, -1);
        if (ret < 0) 
        {
            perror("Error waiting for event");
            break;
        }
        else if (ret == 0)
        {
            continue;
        }

        if (gpiod_line_request_read_edge_events(inputPin, events, 1) != 1) 
        {
            perror("Error reading edge events");
            break;
        }
        
        struct gpiod_edge_event* event = gpiod_edge_event_buffer_get_event(events, 0);
        if(gpiod_edge_event_get_line_offset(event) == IN_PIN && gpiod_edge_event_get_event_type(event) == GPIOD_EDGE_EVENT_FALLING_EDGE) 
        {
            gpiod_line_request_set_value(outputPin, OUT_PIN, 1);
            usleep(10000); // Keep the output high for 10ms
            gpiod_line_request_set_value(outputPin, OUT_PIN, 0);
            printf("Falling edge detected on pin %d\n", IN_PIN);
        }
    }

    gpiod_edge_event_buffer_free(events);
    gpiod_line_request_release(inputPin);
    gpiod_line_request_release(outputPin);
    return 0;
}
