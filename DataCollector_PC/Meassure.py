import socket
import time
import re

# Scope connection details
OSCILLOSCOPE_IP = "10.155.74.212"
PORT = 3000 

# Setting meassure
sample_count = 1000
save_location = "data.csv"

def waitForTrigger(s, meassuirementCommand):
    # Starting a single sweep
    s.sendall(b":RUNning RUN\n")
    s.sendall(b":TRIGger:SINGle:SWEEp SINGle\n")

    # Send command to trigger the scope
    sendMeassureCommandSerial(meassuirementCommand)

    # Wait until the scope has finished the sweep
    start_time = time.time()
    while time.time() - start_time < 10:
        s.sendall(b":TRIGger:STATus?\n")
        data_str = s.recv(100).decode().strip()
        if data_str.startswith("STOP"):
            return True
    return False

def sendMeassureCommandSerial(command):
    return None

def setSingleTriggerForFFR(s):
    s.sendall(b":TRIGger:TYPE SINGle\n")
    s.sendall(b":TRIGger:SINGle:MODE EDGE\n")
    s.sendall(b":TRIGger:SINGle:EDGE:SOURce CH1\n")
    s.sendall(b":TRIGger:SINGle:EDGE:COUPling DC\n")
    s.sendall(b":TRIGger:SINGle:EDGE:SLOPe RISE\n")
    s.sendall(b":TRIGger:SINGle:EDGE:LEVel 1v\n")

def getFFR_time(s):
    s.sendall(b":MEASUrement:FRR? CH1,CH2\n")  # Send command as bytes
    data_str = s.recv(100).decode().strip()

    match = re.search(r"([\d.]+)ms", data_str)  # Find a number followed by "ms"
    if match:
        time_value = float(match.group(1))  # Convert extracted value to float
        return time_value

# Function to run the meassurement
def run_meassurement(sampleC, sLocation, initFunction = setSingleTriggerForFFR, meassurementFunction = getFFR_time, meassurementCommand = 0x01):
    i = 0
    while i < sampleC:
        try: 
            with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s, open(sLocation, "a") as file:
                s.settimeout(5)
                s.connect((OSCILLOSCOPE_IP, PORT))
                print(f"Connected to {OSCILLOSCOPE_IP}:{PORT}")

                print("Initializing meassurement...")
                initFunction(s)

                print(f"Starting meassurement from {i+1} to {sampleC}...")
                while i < sampleC:
                    while waitForTrigger(s, meassurementCommand) == False:
                        print("Coulden't get trigger. Retrying...")
                    ellapsedTime = meassurementFunction(s)
                    if ellapsedTime == None:
                        print(f"No meassurement result found. Rertrying to get data for the {i+1}. meassurement.")
                        continue
                    print(f"{i+1};{time.time()};{time.strftime("%Y-%m-%d %H:%M:%S")};{ellapsedTime}\n")
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
    
# Run the meassurement
run_meassurement(sample_count, save_location, setSingleTriggerForFFR, getFFR_time)