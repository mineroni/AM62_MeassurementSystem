import gpiod
import time
import datetime

CHIP_NAME = "/dev/gpiochip1"
IN_PIN = 1
OUT_PIN = 2

chip = gpiod.chip(CHIP_NAME)

# Configure output line
out_line = chip.get_line(OUT_PIN)
out_config = gpiod.line_request()
out_config.consumer = "pinSetter"
out_config.request_type = gpiod.line_request.DIRECTION_OUTPUT
out_config.default_vals = [0]
out_line.request(out_config)

# Configure input line with falling edge detection
in_line = chip.get_line(IN_PIN)
in_config = gpiod.line_request()
in_config.consumer = "pinSetter"
in_config.request_type = gpiod.line_request.EVENT_FALLING_EDGE
in_line.request(in_config)

print(f"Waiting for falling edge on GPIO {IN_PIN}...")

try:
    while True:
        if in_line.event_wait(datetime.timedelta(days=1)):
            event = in_line.event_read()
            if event.event_type == gpiod.line_event.FALLING_EDGE:
                out_line.set_value(1)
                time.sleep(0.01)
                out_line.set_value(0)
                print(f"Falling edge detected on pin {IN_PIN}")
except KeyboardInterrupt:
    print("Exiting program...")

out_line.release()
in_line.release()
