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
    // Flag to indicate if the pin is set to low
    private static bool isHigh = false;
    static void Main()
    {
        // Setting up serial communication
        serialPort.DataReceived += DataReceived;
        serialPort.Open();

        // Setting up the GPIO controller
        gpioController.OpenPin(gpioPinOutput, PinMode.Output);
        gpioController.Write(gpioPinOutput, PinValue.High);
	    gpioController.OpenPin(gpioPinInput, PinMode.Input);
        gpioController.RegisterCallbackForPinValueChangedEvent(gpioPinInput, PinEventTypes.Rising, PinChanged);

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
        // Set the pin to low as fast as possible
        setPinLow();
    }

    private static void PinChanged(object sender, PinValueChangedEventArgs e)
    {
        // Set the pin to low as fast as possible
        setPinLow();
    }
    private static void setPinLow()
    {
        // Setting the pin low
        gpioController.Write(gpioPinOutput, PinValue.High);
        // Indicating that the pin is low for the main loop
        isHigh = true;
    }
}
