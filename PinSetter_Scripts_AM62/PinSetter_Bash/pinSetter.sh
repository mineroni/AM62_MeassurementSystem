#!/bin/bash

# This script is used for reacting to pin level changes
# to do meassurements on the AM62x platform.

# Pin settings
input_pin=1
output_pin=2

# Setting the output pin to its default value
gpioset -hold-period 10ms -consumer "pinSetter" -chip "/dev/gpiochip1" $output_pin=0

while true; 
do
    # Waiting for a falling edge on the input pin
    gpiomon -chip "/dev/gpiochip1" -consumer "pinSetter" -edge falling -debounce-period 0 1
    # Setting the output pin to high
    gpioset -hold-period 10ms -consumer "pinSetter" -chip "/dev/gpiochip1" $output_pin=1
    # Wait for a little time to detect the rising edge
    sleep 0.1
    # Setting the output pin back to low
    gpioset -hold-period 10ms -consumer "pinSetter" -chip "/dev/gpiochip1" $output_pin=0
    # Printing out the result to the console
    echo "Falling edge detected on pin $input_pin"
done