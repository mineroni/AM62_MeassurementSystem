import serial
import gpiod
import time

SERIAL_PORT = "/dev/verdin-uart1"
CHIP_NAME = "/dev/gpiochip1"
OUT_PIN = 2

# Open serial port to receive messages on
serial_port = serial.Serial(SERIAL_PORT, 9600, timeout=0)

# Open gpio chip
chip = gpiod.chip(CHIP_NAME)

# Configure output line
out_line = chip.get_line(OUT_PIN)
out_config = gpiod.line_request()
out_config.consumer = "pinSetter"
out_config.request_type = gpiod.line_request.DIRECTION_OUTPUT
out_config.default_vals = [0]
out_line.request(out_config)

print(f"Waiting for serial message 0x01 on \"{SERIAL_PORT}\"...")

# Main loop: reset output pin if it was set high
try:
    while True:
        # Check for incoming serial data
        byte_read = serial_port.read(1)
        if byte_read == b'\x01':
            out_line.set_value(1)
            time.sleep(0.01)
            out_line.set_value(0)
            print(f"Serial message 0x01 detected on port \"{SERIAL_PORT}\"")
except KeyboardInterrupt:
    print("Exiting program...")

# Cleanup
out_line.release()
serial_port.close()