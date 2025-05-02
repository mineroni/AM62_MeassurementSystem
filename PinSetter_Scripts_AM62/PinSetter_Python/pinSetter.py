import serial
import lgpio
import time

# Handle falling edge interrupt on input pin
def pin_changed(gpio_chip, gpio_line, level, tick):
    if level == 0:  # Falling edge
        set_pin_high()

# Set pin to high, generate rising edge for the scope to trigger
def set_pin_high():
    global is_high
    gpio.tx(GPIO_PIN_OUTPUT, 1)
    is_high = True

# Open serial port to receive messages on
serial_port = serial.Serial('/dev/verdin-uart1', 9600, timeout=0)

# Open gpio chip
gpio = lgpio.gpiochip(1)

# GPIO pin numbers to use
GPIO_PIN_INPUT = 1
GPIO_PIN_OUTPUT = 2

# Set up GPIO pins
gpio.set_mode(GPIO_PIN_OUTPUT, lgpio.OUTPUT)
gpio.tx(GPIO_PIN_OUTPUT, 0)  # Set initial output to low
gpio.set_mode(GPIO_PIN_INPUT, lgpio.INPUT)
# Register interrupt callback
gpio.set_alert_func(GPIO_PIN_INPUT, pin_changed)

# Track if output pin was set to high
is_high = False

print("Waiting for input...")

# Main loop: reset output pin if it was set high
try:
    while True:
        # Check for incoming serial data
        if serial_port.in_waiting > 0:
            byte_read = serial_port.read(1)
            if byte_read == b'\x01':
                set_pin_high()
        # Check if the pin is high and reset it to low
        if is_high:
            time.sleep(0.01)
            gpio.tx(GPIO_PIN_OUTPUT, 0)
            print("Pin high level detected, resetting it to low!")
            is_high = False
except KeyboardInterrupt:
    print("Exiting program...")

# Cleanup
gpio.close()
serial_port.close()