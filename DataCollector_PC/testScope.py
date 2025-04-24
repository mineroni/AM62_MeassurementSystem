import socket

# Scope connection details
OSCILLOSCOPE_IP = "10.155.74.212"
PORT = 3000 

"""
Response:
Connected to 10.155.74.212:3000
Received data: OWON,XDS3204AE,1942097,V1.8.0->
"""

try: 
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        # Set higher timeout to prevent connection loss
        s.settimeout(5)
        # Connect to the oscilloscope
        s.connect((OSCILLOSCOPE_IP, PORT))
        print(f"Connected to {OSCILLOSCOPE_IP}:{PORT}")
        # Send commad to the oscilloscope to identify it
        s.sendall(b"*IDN?\n")
        # Receive the response from the oscilloscope
        data = s.recv(100).decode().strip()
        # Print the response
        print(f"Received data: {data}")

# Handle timeout exception
except socket.timeout:
    print("Connection timed out.")
# Handle socket error exception
except socket.error as e:
    print(f"Socket error: {e}")
# Handle other exceptions
except Exception as e:
    print(f"An error occurred: {e}")