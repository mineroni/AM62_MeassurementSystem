import time
import gpiod

line = 2

def set_line_state(line,state):
    gpioline.request(config, 1 if state else 0)

gpioline = gpiod.find_line(line)

if gpioline is None:
    print("Invalid line name.")
    exit(1)

config = gpiod.line_request()

config.consumer="GPIO sample"
config.request_type=gpiod.line_request.DIRECTION_OUTPUT

while True:
    set_line_state(line,True)
    time.sleep(1)
    set_line_state(line,False)
    time.sleep(1)
        