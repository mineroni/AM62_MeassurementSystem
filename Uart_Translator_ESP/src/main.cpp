#include <Arduino.h>

// Size of command buffer
#define BUFF_LEN (64)

// Serial2 (output serial) pins
#define TX2_PIN 17   // Serial2 TX
#define RX2_PIN 16   // Serial2 RX

// Pin to show if command processing in progress
#define LED_PIN 2

// Free GPIO pins to use by the commands
uint8_t freeGPIOPins[] = {4, 5, 12, 13, 14, 15, 18, 19, 21, 22, 23, 25, 26, 27, 32, 33, 34};

// Command structure:

// 0. Start byte: 0x55
// 1. Command type: 1 byte
// 2. Message length: 1 byte (4-64)
// 3-62. Data: 0-60 bytes
// dataLen+3. Checksum: 1 byte

// Response structure:
// 0. Start byte: 0x55
// 1. Response code: 0x00 -> Success, 0x01 -> Checksum Error, 0x02 -> Invalid Command, 0x03 -> Invalid Pin, 0x04 -> Invalid Command Length, 0x05 -> Invalid Pin Mode, 0x06 -> Invalid Level
// 2. Response message length: 0x04
// 3. Checksum: 1 byte

// Command types:
#define COMMAND_GPIO_MODE (0x01)
#define COMMAND_GPIO_WRITE (0x02)
#define COMMAND_SERIAL_WRITE (0x03)

// Response codes:
#define RESPONSE_SUCCESS (0x00)
#define RESPONSE_CHECKSUM_ERROR (0x01)
#define RESPONSE_INVALID_COMMAND (0x02)
#define RESPONSE_INVALID_PIN (0x03)
#define RESPONSE_INVALID_COMMAND_LENGTH (0x04)
#define RESPONSE_INVALID_PIN_MODE (0x05)
#define RESPONSE_INVALID_LEVEL (0x06)


// Command buffer
byte buffer[BUFF_LEN];
// Command buffer length
int cmdLen = 0;

/// @brief Check if pin is free to use by commands
/// @param pin number of the pin to check
/// @return true if pin is free, false otherwise
bool pinFree(byte pin)
{
    for(int i = 0; i < sizeof(freeGPIOPins); ++i)
        if(freeGPIOPins[i] == pin)
            return true;
    return false;
}

/// @brief Function to calculate checksum for the serial commands
/// @param data Command buffer to calculate the checksum on
/// @return Checksum value
byte createChecksum(byte data[]) 
{
    byte checksum = 0;
    for (int i = 0; i < data[2]-1; ++i)
    {
        checksum += data[i];
    }
    return checksum;
}

/// @brief Function to send response back to the PC
/// @param resp Response code to answer
void sendResponse(byte resp)
{
    byte respBuff[] = {0x55, resp, 0x04, 0x00};
    respBuff[3] = createChecksum(respBuff);
    Serial.write(respBuff, 4);
}

/// @brief Function to precess incoming commands
void processCommand();

/// @brief Function to setup the ESP32 before starting the main loop
void setup() 
{
    // Initializing Serial lines
    Serial.begin(115200);
    Serial.setTimeout(50);
    Serial2.begin(9600, SERIAL_8N1, RX2_PIN, TX2_PIN);

    // Initializing command processing pin
    pinMode(LED_PIN, OUTPUT);

    // Initalize all unused GPIO pins as INPUT
    for(int i = 0; i < sizeof(freeGPIOPins)/sizeof(freeGPIOPins[0]); ++i)
        pinMode(freeGPIOPins[i], INPUT);
}

/// @brief Main loop of the program
void loop() 
{
    // Read avaible commands to the buffer
    cmdLen += Serial.readBytes(buffer + cmdLen, BUFF_LEN - cmdLen);
    
    // Find and process commands in the serial buffer
    while (cmdLen > 2)
    {
        int i = 0;
        // Find the start of the command
        while (i < cmdLen && buffer[i] != 0x55) { ++i; }
        // Remove invalid data before the command
        if (i > 0 && i < cmdLen)
        {
            cmdLen -= i;
            memmove(buffer, buffer + i, cmdLen);
        }
        // Check if the command is complete
        if (buffer[2] <= cmdLen)
        {
            // Check the checksum
            if (buffer[buffer[2]-1] == createChecksum(buffer))
            {
                // Process the command
                digitalWrite(LED_PIN, HIGH);
                processCommand();
                digitalWrite(LED_PIN, LOW);
            }
            else // Invalid checksum
            {
                sendResponse(RESPONSE_CHECKSUM_ERROR);
            }
            
            // Remove the command from the buffer
            cmdLen -= buffer[2];
            memmove(buffer, buffer + buffer[2], cmdLen);
        }
    }
}

/// @brief Function to precess incoming commands
void processCommand() 
{
    switch (buffer[1]) // Command Type
    {
        case COMMAND_GPIO_MODE:
            if(buffer[2] != 6) // Invalid command length
                sendResponse(RESPONSE_INVALID_COMMAND_LENGTH);
            else if (!pinFree(buffer[3])) // Invalid pin
                sendResponse(RESPONSE_INVALID_PIN);
            else if(buffer[4] > 0x03) // Invalid mode
                sendResponse(RESPONSE_INVALID_COMMAND);
            else // Command is valid, process it
            {
                sendResponse(RESPONSE_SUCCESS);
                pinMode(buffer[3], buffer[4]);
            }
            break;

        case COMMAND_GPIO_WRITE:
            if(buffer[2] != 6) // Invalid command length
                sendResponse(RESPONSE_INVALID_COMMAND_LENGTH);
            else if (!pinFree(buffer[3])) // Invalid pin
                sendResponse(RESPONSE_INVALID_PIN);
            else if(buffer[4] > 1) // Invalid level
                sendResponse(RESPONSE_INVALID_LEVEL);
            else // Command is valid, process it
            {
                sendResponse(RESPONSE_SUCCESS);
                digitalWrite(buffer[3], buffer[4]);
            }
            break;
        
        case COMMAND_SERIAL_WRITE:
            if(buffer[2] < 5) // Invalid command length (at least 1 byte to send on Serial2)
                sendResponse(RESPONSE_INVALID_COMMAND_LENGTH);
            else // Command is valid, process it
            {
                Serial2.write(buffer + 3, buffer[2] - 4);
                sendResponse(RESPONSE_SUCCESS);
            }
            break;
        
        default: // Invalid command
            sendResponse(RESPONSE_INVALID_COMMAND);
            break;
    }
}