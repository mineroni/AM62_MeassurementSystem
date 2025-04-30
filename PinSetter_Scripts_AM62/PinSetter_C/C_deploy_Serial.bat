@echo off
REM This script is used to deploy the PinSetter application on the AM62 platform.

REM IP address of the target device
set ipAddr=10.155.74.12

REM Copy the pinSetter.sh script to the target device
scp pinSetterSerial.c root@%ipAddr%:cTest/

REM Modify the script and set permissions and run it on the target device
ssh root@%ipAddr% -t "cd cTest && rm pinSetterSerial; gcc -Wall -O3 -o pinSetterSerial pinSetterSerial.c -lgpiod && chmod +x pinSetterSerial && ./pinSetterSerial"