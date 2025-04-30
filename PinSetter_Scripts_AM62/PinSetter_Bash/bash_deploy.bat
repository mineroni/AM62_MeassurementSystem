@echo off

set ipAddr=10.155.74.12

REM This script is used to deploy the PinSetter application on the AM62 platform.

REM Copy the pinSetter.sh script to the target device
scp pinSetter.sh root@%ipAddr%:.

ssh root@%ipAddr% -t "sed -i 's/\r$//' pinSetter.sh; chmod +x pinSetter.sh; ./pinSetter.sh"

