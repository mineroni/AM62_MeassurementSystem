@echo off
REM This script is used to deploy the PinSetter application on the AM62 platform.

REM IP address of the target device
set ipAddr=10.155.74.12

REM Copy the pinSetter.py script to the target device
scp pinSetter_Serial.py root@%ipAddr%:pythonTest/

REM Modify the script and set permissions and run it on the target device
ssh root@%ipAddr% -t "cd pythonTest && chmod +x pinSetter_Serial.py && python3 pinSetter_Serial.py"
pause
