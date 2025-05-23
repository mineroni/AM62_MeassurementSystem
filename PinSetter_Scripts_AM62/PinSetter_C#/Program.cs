using System;
using System.IO.Ports;
using System.Device.Gpio;
using System.Device.Gpio.Drivers;

class Program
{
    // Serial input port for the meassurement
    private static SerialPort serialPort = new SerialPort("/dev/verdin-uart1", 9600);

    // GPIO input port for the meassurement
    private static GpioController gpioController = new GpioController(PinNumberingScheme.Logical, new LibGpiodDriver(1));
    // GPIO input pin
    private static int gpioPinInput = 1;
    // GPIO output pin
    private static int gpioPinOutput = 2;
    // Flag to indicate if the pin is set to high
    private static bool isHigh = false;
    static void Main()
    {
        // Setting up serial communication
        serialPort.DataReceived += DataReceived;
        serialPort.Open();

        // Setting up the GPIO controller
        gpioController.OpenPin(gpioPinOutput, PinMode.Output);
        gpioController.Write(gpioPinOutput, PinValue.Low);
	    gpioController.OpenPin(gpioPinInput, PinMode.Input);
        gpioController.RegisterCallbackForPinValueChangedEvent(gpioPinInput, PinEventTypes.Falling, PinChanged);
        Console.WriteLine("Waiting for triggers...");

        // Main loop
        // Waiting for triggers. If the pin is high, reset it to low
        while (true)
        {
            if  (isHigh)
            {
                Thread.Sleep(10);
                gpioController.Write(gpioPinOutput, PinValue.Low);
                Console.WriteLine("Pin high level detected, resetting it to low!");
                isHigh = false;
            }
            Thread.Sleep(10);
        }
    }
    private static void DataReceived(object sender, SerialDataReceivedEventArgs e)
    {
        // Check if the received byte is 0x01
        int readedByte = serialPort.ReadByte();
        if(readedByte != 0x01)
            return;
        // Set the pin to high as fast as possible
        setPinHigh();
    }

    private static void PinChanged(object sender, PinValueChangedEventArgs e)
    {
        // Set the pin to high as fast as possible
        setPinHigh();
    }
    private static void setPinHigh()
    {
        // Setting the pin high
        gpioController.Write(gpioPinOutput, PinValue.High);
        // Indicating that the pin is high for the main loop
        isHigh = true;
    }
}
