@echo off
REM This script is used to deploy the PinSetter application on the AM62 platform.

REM IP address of the target device
set ipAddr=10.155.74.12

REM This script is used to deploy the PinSetter application on the AM62 platform.

REM Copy the pinSetter.sh script to the target device
scp pinSetter_serial.sh root@%ipAddr%:bashTest/

REM Modify the script and set permissions and run it on the target device
ssh root@%ipAddr% -t "cd bashTest && sed -i 's/\r$//' pinSetter_serial.sh && chmod +x pinSetter_serial.sh && ./pinSetter_serial.sh"

