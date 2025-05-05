use gpiod::{Chip, Options};
use std::time::Duration;
use std::io::Read;
use std::thread::sleep;

fn main() -> std::io::Result<()> {
    // Open serial port (adjust as needed)
    let port_name = "/dev/verdin-uart1";
    let baud_rate = 9_600;

    let mut serial = serialport::new(port_name, baud_rate)
        .timeout(Duration::from_millis(100))
        .open()
        .expect("Failed to open serial port");

    // Open the GPIO chip
    let chip = Chip::new("/dev/gpiochip1")?;

    // Configure pin 2 as output and set to low
    let opts = Options::output([2])
        .values([false])
        .consumer("gpio_trigger");
    let outputs = chip.request_lines(opts)?;

    let mut buf = [0u8; 1];
    println!("Waiting for message 0x01 on {}.", port_name);
    loop {
        // Read a byte from the serial port
        if let Ok(n) = serial.read(&mut buf) {
            if n > 0 && buf[0] == 0x01 {
                // Set GPIO pin 2 high
                outputs.set_values([true])?;

                // Keep high for 10ms
                sleep(Duration::from_millis(10));

                // Set GPIO pin 2 low
                outputs.set_values([false])?;
                println!("Message 0x01 detected on serial port {}.", port_name);
            }
        }
    }
}
