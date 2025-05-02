use gpiod::{Chip, EdgeKind, LineRequestFlags, Request};
use serialport::SerialPort;
use std::io::Read;
use std::sync::{
    atomic::{AtomicBool, Ordering},
    Arc,
};
use std::thread;
use std::time::Duration;

fn main() {
    // Set up GPIO chip (e.g. /dev/gpiochip1, index may vary on your board)
    let mut chip = Chip::new("/dev/gpiochip1").expect("Failed to open gpiochip");

    // Define GPIO lines
    let input_line = 1;
    let output_line_offset = 2;

    // Request input line with edge detection
    let input_line = chip
        .get_line(input_line_offset)
        .expect("Failed to get input line");
    let input_request = input_line
        .request(
            "gpio_input",
            LineRequestFlags::INPUT | LineRequestFlags::EDGE_FALLING,
            0,
        )
        .expect("Failed to request input line");

    // Request output line
    let output_line = chip
        .get_line(output_line_offset)
        .expect("Failed to get output line");
    let output_request = output_line
        .request("gpio_output", LineRequestFlags::OUTPUT, 0)
        .expect("Failed to request output line");

    let is_high = Arc::new(AtomicBool::new(false));
    let is_high_gpio = Arc::clone(&is_high);

    // Thread for GPIO edge detection
    let output_req_clone = output_request.clone();
    thread::spawn(move || {
        loop {
            if let Ok(event) = input_request.read_edge_event() {
                println!(
                    "GPIO event: {:?} at {:?}",
                    event.edge_kind(),
                    event.timestamp()
                );
                set_pin_high(&output_req_clone, &is_high_gpio);
            }
        }
    });

    // Thread for serial communication
    let is_high_serial = Arc::clone(&is_high);
    let output_req_clone2 = output_request.clone();
    thread::spawn(move || {
        let mut port = serialport::new("/dev/verdin-uart1", 9600)
            .timeout(Duration::from_millis(10))
            .open()
            .expect("Failed to open serial port");

        let mut buffer = [0u8; 1];

        loop {
            if port.read_exact(&mut buffer).is_ok() {
                if buffer[0] == 0x01 {
                    println!("Serial trigger received!");
                    set_pin_high(&output_req_clone2, &is_high_serial);
                }
            }
        }
    });

    // Main loop
    println!("Waiting for input...");
    loop {
        if is_high.load(Ordering::Relaxed) {
            thread::sleep(Duration::from_millis(10));
            output_request
                .set_value(0)
                .expect("Failed to reset output pin");
            println!("Pin high level detected, resetting it to low!");
            is_high.store(false, Ordering::Relaxed);
        }
        thread::sleep(Duration::from_millis(10));
    }
}

fn set_pin_high(req: &Request, flag: &AtomicBool) {
    req.set_value(1).expect("Failed to set pin high");
    flag.store(true, Ordering::Relaxed);
}