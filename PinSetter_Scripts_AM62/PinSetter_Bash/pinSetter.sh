#!/bin/bash

# This script is used for reacting to pin level changes
# to do meassurements on the AM62x platform.

# Pin settings
INPUT_PIN=1
OUTPUT_PIN=2

# Kill all task setting gpio pins
pkill -f gpioset

# Setting the output pin to its default value
gpioset --consumer "pinSetter" --chip "/dev/gpiochip1" "$OUTPUT_PIN=0" &
# Kill the task to be able to set the pin again
pkill -f gpioset
# Clear the screen to start the meassurements
clear
echo "Waiting for falling edge on GPIO pin $INPUT_PIN."

while true
do
    # Waiting for a falling edge on the input pin
    gpiomon --chip "/dev/gpiochip1" --consumer "pinSetter" --edge falling --debounce-period 0 --num-events 1 "$INPUT_PIN"

    # Setting the output pin to high
    gpioset --consumer "pinSetter" --chip "/dev/gpiochip1" "$OUTPUT_PIN=1" &
    # Wait for a little time to detect the rising edge
    sleep 0.01
    # Kill the task to reset pin to 0
    pkill -f gpioset

    # Setting the output pin back to low
    gpioset --consumer "pinSetter" --chip "/dev/gpiochip1" "$OUTPUT_PIN=0" &
    # Kill the task to be able to set the pin again
    pkill -f gpioset

    # Printing out the result to the console
    echo "Falling edge detected on GPIO pin $INPUT_PIN."
done