import socket, serial, time, re, os

# Scope connection details
OSCILLOSCOPE_IP = "10.155.74.212"
PORT = 3000 

# ESP connection details
SERIAL_PORT = "COM12"
SERIAL_BAUDRATE = 115200
ser = None

# Setting up meassure
sample_count = 1000000
save_location = "dataGpio.csv"
save_location2 = "dataSerial.csv"
meassure_pin = 4

def waitForTrigger(s, meassuirementCommand):
    while True:
        # Starting a single sweep
        s.sendall(b":TRIGger:SINGle:SWEEp SINGle\n")
        s.sendall(b":RUNning RUN\n")

        # Check if the scope is running
        s.sendall(b":RUNning?\n")
        run_status = s.recv(100).decode().strip()

        # Check if Single trigger is set
        s.sendall(b":TRIGger:STATus?\n")
        trigger_status = s.recv(100).decode().strip()

        # Break if scope is running
        if run_status.startswith("RUN") and trigger_status.startswith("READy"):
            break

    # Send command to trigger the scope and read the response data
    if not sendMeassureCommandSerial(meassuirementCommand):
        return False

    # Wait until the scope has finished the sweep
    start_time = time.time()
    while time.time() - start_time < 10:
        s.sendall(b":TRIGger:STATus?\n")
        data_str = s.recv(100).decode().strip()
        if data_str.startswith("STOP"):
            return True
    return False

def sendMeassureCommandSerial(command):
    # Read all data which we dont need
    data = ser.read_all()

    # Send command to the ESP
    data = [0x55, command]
    if command == 0x01:     # Set pin mode
        data.append(6)                      # Length
        data.append(meassure_pin)           # Pin number
        data.append(0x03)                   # Mode: Output
        
        data.append(0x00)                   # Create reserved space for the checksum
        data[-1] = calculateChecksum(data)  # CHECKSUM
        ser.write(bytes(data))              # Convert list to byte array and send

        # Wait and read the response
        time.sleep(0.250)
        resp = ser.read_all()

        # If the command failed return failed
        if len(resp) < 4 or resp[0] != 0x55 or calculateChecksum(resp) != resp[-1] or resp[1] != 0x00:
            return False
        
        # Resetting the pin to default state
        data = [0x55, 0x02, 6, meassure_pin, 0x01]

    elif command == 0x02:   # Set pin low
        data.append(6)                      # Length
        data.append(meassure_pin)           # Pin number
        data.append(0)                      # Level: Low

        data.append(0x00)                   # Create reserved space for the checksum
        data[-1] = calculateChecksum(data)  # CHECKSUM
        ser.write(bytes(data))              # Convert list to byte array and send

        # Wait and read the response
        time.sleep(0.250)
        resp = ser.read_all()

        # If the command failed return failed
        if len(resp) < 4 or resp[0] != 0x55 or calculateChecksum(resp) != resp[-1] or resp[1] != 0x00:
            return False
        
        # Resetting the pin to default state
        data = [0x55, 0x02, 6, meassure_pin, 0x01]

    elif command == 0x03:   # Send serial 0x01 
        data.append(5)                      # Length
        data.append(0x01)                   # Send command
    else:                   # Unknown command
        return False
    
    data.append(0x00)                       # Create reserved space for the checksum
    data[-1] = calculateChecksum(data)      # CHECKSUM
    ser.write(bytes(data))                  # Convert list to byte array and send

    # Wait and read the response
    time.sleep(0.250)
    resp = ser.read_all()
    
    # If the command failed we don't want to find the response on the scope (Message could fail due: incorrect length, incorrect start byte, incorrect checksum or failed command)
    return len(resp) >= 4 and resp[0] == 0x55 and calculateChecksum(resp) == resp[-1] and resp[1] == 0x00

def calculateChecksum(data):
    crc = 0
    for i in range(0, len(data)-1):
        crc += data[i]
    return crc & 0xFF

def setSingleTriggerForFFR(s):
    s.sendall(b":TRIGger:TYPE SINGle\n")
    s.sendall(b":TRIGger:SINGle:MODE EDGE\n")
    s.sendall(b":TRIGger:SINGle:EDGE:SOURce CH1\n")
    s.sendall(b":TRIGger:SINGle:EDGE:COUPling DC\n")
    s.sendall(b":TRIGger:SINGle:EDGE:SLOPe FALL\n")
    s.sendall(b":TRIGger:SINGle:EDGE:LEVel 1v\n")

def getFFR_time(s):
    s.sendall(b":MEASUrement:FFR? CH1,CH2\n")  # Send command as bytes
    data_str = s.recv(100).decode().strip()

    match = re.search(r"([\d.]+)ms", data_str)  # Find a number followed by "ms"
    if match:
        time_value = float(match.group(1))  # Convert extracted value to float
        return time_value

# Function to run the meassurement
def run_meassurement(sampleCount, saveLocation, initFunction = setSingleTriggerForFFR, meassurementFunction = getFFR_time, meassurementCommand = 0x01):
    # Delete file if it already exists
    if os.path.exists(saveLocation):
        os.remove(saveLocation)
    
    # Run the meassurement and collect meassured values
    i = 0
    while i < sampleCount:
        try: 
            with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s, open(saveLocation, "a") as file:
                s.settimeout(5)
                s.connect((OSCILLOSCOPE_IP, PORT))
                print(f"Connected to {OSCILLOSCOPE_IP}:{PORT}")

                print("Initializing meassurement...")
                initFunction(s)

                print(f"Starting meassurement from {i+1} to {sampleCount}...")
                while i < sampleCount:
                    while waitForTrigger(s, meassurementCommand) == False:
                        print("Coulden't get trigger. Retrying...")
                    ellapsedTime = meassurementFunction(s)
                    if ellapsedTime == None:
                        print(f"No meassurement result found. Rertrying to get data for the {i+1}. meassurement.")
                        continue
                    print(f"{i+1};{time.time()};{time.strftime("%Y-%m-%d %H:%M:%S")};{ellapsedTime}")
                    file.write(f"{i+1};{time.time()};{time.strftime("%Y-%m-%d %H:%M:%S")};{ellapsedTime}\n")
                    file.flush()
                    i += 1
        # Handle timeout exception
        except socket.timeout:
            print("Connection timed out.")
        # Handle socket error exception
        except socket.error as e:
            print(f"Socket error: {e}")
        # Handle other exceptions
        except Exception as e:
            print(f"An error occurred: {e}")
    
    ser.close()
    
# Open serial port
ser = serial.Serial(port=SERIAL_PORT, baudrate=SERIAL_BAUDRATE, timeout=1)
time.sleep(1)
# Init output pin as output
if not sendMeassureCommandSerial(0x01):
    print("Failed to init pin as output.")
    ser.close()
    exit()

# Run meassurements
run_meassurement(sample_count, save_location, setSingleTriggerForFFR, getFFR_time, 0x02)
#run_meassurement(sample_count, save_location2, setSingleTriggerForFFR, getFFR_time, 0x03)