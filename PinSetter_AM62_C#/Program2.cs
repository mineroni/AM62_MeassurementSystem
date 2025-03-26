using System.Device.Gpio;
using System.Device.Gpio.Drivers;

class Program
{
    static void Main()
    {
        int gpioLine = 1;
        int gpioLine2 = 2;
	    int gpioChip = 1;
        LibGpiodDriver gpiodDriver = new LibGpiodDriver(gpioChip);
        GpioController gpioController = new GpioController(PinNumberingScheme.Logical, gpiodDriver);

        gpioController.OpenPin(gpioLine, PinMode.Output);
	    gpioController.OpenPin(gpioLine2, PinMode.Output);

        while (true)
        {
            gpioController.Write(gpioLine, PinValue.Low);
	        Thread.Sleep(10);
	        gpioController.Write(gpioLine2, PinValue.Low);
            Thread.Sleep(250);
            gpioController.Write(gpioLine, PinValue.High);
	        Thread.Sleep(10);
	        gpioController.Write(gpioLine2, PinValue.High);
            Thread.Sleep(250);
        }
    }
}
