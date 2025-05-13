@echo off
REM This script is used to deploy the PinSetter application on the AM62 platform.

REM IP address of the target device
set ipAddr=10.155.74.12

REM Copy the dependencies and source files to the target device
scp Cargo.toml root@%ipAddr%:pinSetter_Rust
scp mainSerial.rs root@%ipAddr%:pinSetter_Rust/src/main.rs

REM Modify the script and set permissions and run it on the target device
ssh root@%ipAddr% -t "cd pinSetter_Rust && ~/.cargo/bin/cargo build --release && chmod +x target/release/pinSetter_Rust && ./target/release/pinSetter_Rust"