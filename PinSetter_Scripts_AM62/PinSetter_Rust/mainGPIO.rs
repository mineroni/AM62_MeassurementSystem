use gpiod::{Chip, Options, EdgeDetect};
use std::thread::sleep;
use std::time::Duration;

fn main() -> std::io::Result<()> {
    // Open the GPIO chip (gpiochip1)
    let chip = Chip::new("/dev/gpiochip1")?;

    // Configure pin 1 as input and pin 2 as output
    let opts = Options::input([1])
        .consumer("gpio_pin1")
        .edge(EdgeDetect::Falling); // Detect falling edge on pin 1

    let mut inputs = chip.request_lines(opts)?;

    let opts = Options::output([2])
        .values([false]) // Set initial state of pin 2 to low
        .consumer("gpio_pin2");

    let outputs = chip.request_lines(opts)?;

    println!("Waiting for falling edge on GPIO pin {}.", 1);
    loop {
        // Wait for a falling edge on pin 1
        let event = inputs.read_event()?;
        // Once pin 1 is pulled to 0 (falling edge detected), pull pin 2 high
        if event.line == 0 {
            // Set pin 2 high
            outputs.set_values([true])?;

            // Sleep for 10 milliseconds
            sleep(Duration::from_millis(10));

            // Set pin 2 low
            outputs.set_values([false])?;
            println!("Falling edge detected on pin {}", 1);
        }
    }
}
