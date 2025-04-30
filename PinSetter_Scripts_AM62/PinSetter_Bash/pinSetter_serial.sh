#!/bin/bash

# This script is used for reacting to serial messages
# to do meassurements on the AM62x platform.

SERIAL_PORT="/dev/verdin-uart1"
OUTPUT_PIN=2

# Kill all tasks using the serial device
pkill -f "$SERIAL_PORT"

# Open the serial device in binary mode, and read byte-by-byte
stty -F "$SERIAL_PORT" raw -echo

# Setting the output pin to its default value
gpioset --consumer "pinSetter" --chip "/dev/gpiochip1" "$OUTPUT_PIN=0" &
# Kill the task to be able to set the pin again
pkill -f gpioset
# Clear the screen to start the meassurements
clear
echo "Waiting for message 0x01 on $SERIAL_PORT."

while true
do
    byte=$(dd if="$SERIAL_PORT" bs=1 count=1 2>/dev/null)
    if [ "$byte" = "$(printf '\x01')" ]
    then
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
        echo "Message 0x01 detected on serial port $SERIAL_PORT."
    fi
done
