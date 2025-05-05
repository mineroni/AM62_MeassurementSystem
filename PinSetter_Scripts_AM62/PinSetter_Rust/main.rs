use gpiod::{Chip, Options, EdgeDetect};
use serialport::SerialPort;
use std::io::Read;
use std::time::{Duration, Instant};
use std::thread::sleep;

fn main() -> std::io::Result<()> {
    let port_name = "/dev/verdin-uart1";
    let baud_rate = 9600;

    // Open serial port
    let mut serial = serialport::new(port_name, baud_rate)
        .timeout(Duration::from_millis(10)) // non-blocking
        .open()
        .expect("Failed to open serial port");

    // Open GPIO chip
    let chip = Chip::new("/dev/gpiochip1")?;

    // Configure pin 1 as input with falling edge detection
    let input_opts = Options::input([1])
        .consumer("gpio_pin1")
        .edge(EdgeDetect::Falling);
    let mut inputs = chip.request_lines(input_opts)?;

    // Configure pin 2 as output, initially low
    let output_opts = Options::output([2])
        .values([false])
        .consumer("gpio_pin2");
    let outputs = chip.request_lines(output_opts)?;

    let mut serial_buf = [0u8; 1];
    println!("Monitoring serial and GPIO pin 1 for trigger...");

    loop {
        // ==== Check Serial ====
        if let Ok(n) = serial.read(&mut serial_buf) {
            if n > 0 && serial_buf[0] == 0x01 {
                println!("Serial 0x01 received → trigger!");
                trigger_pulse(&outputs)?;
                continue; // prevent double trigger if edge is also pending
            }
        }

        // ==== Check GPIO Edge ====
        if let Ok(event) = inputs.read_event() {
            if event.line == 0 {
                println!("Falling edge detected on GPIO pin 1 → trigger!");
                trigger_pulse(&outputs)?;
            }
        }

        // Optional: small sleep to avoid busy-wait loop
        sleep(Duration::from_millis(1));
    }
}

fn trigger_pulse(outputs: &gpiod::Lines) -> std::io::Result<()> {
    outputs.set_values([true])?;
    sleep(Duration::from_millis(10));
    outputs.set_values([false])?;
    Ok(())
}
