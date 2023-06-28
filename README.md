# Audio Processing and Wi-Fi Alert Transmission using ESP32 and FreeRTOS with Microphone Interface

This project aims to develop a safety-enhancing system that integrates embedded systems, audio processing, machine learning, and Wi-Fi communications. The system utilizes ESP-32 and FreeRTOS to enable real-time audio analysis and transmission of classified audio signals as alerts or notifications over a Wi-Fi network.

## Features

- Audio processing module for real-time analysis of audio signals
- Machine learning module using Support Vector Machines (SVM) to classify audio signals
- Wi-Fi communication module for transmitting classified audio signals as alerts or notifications
- Microphone interface for capturing audio input

## Block Diagram
![image](https://github.com/muneebelahimalik/Embedded_Hardware_Software/assets/59524535/8201eef9-07e4-42fd-9b8e-1f583022a91b)


## Hardware
-  ESP 32 Development Board
-  INMP441 I2S Mic

## Software 
-  FreeRTOS operating system
-  Fast Fourier Transform (FFT) algorithm for audio frequency analysis

## Prerequisites
- Embedded C 
- FreeRTOS Functionalities

## Results
The system was able to successfully detect fire alarms and doorbells from audio input and transmit the appropriate alerts to the server over WiFi. The use of deferred interrupt processing helped in resource management by allowing the system to prioritize the processing of other tasks. This helped to improve the overall performance of the system.


## Installation

1. Clone the repository:
   https://github.com/muneebelahimalik/Embedded_Hardware_Software.git


2. Open the Arduino IDE and navigate to **Sketch > Include Library > Add .ZIP Library**.
3. Select the necessary libraries from the `lib/` directory of the cloned repository.
4. Connect the ESP32 development board to the I2C microphone interface.
5. Upload the source code files (`src/`) to your ESP32 using the Arduino IDE.

## Usage

1. Ensure that the ESP32 is properly connected to the I2C microphone and the Wi-Fi network.
2. Open the Arduino IDE and open the project files (`src/`).
3. Configure the necessary settings (e.g., Wi-Fi credentials) in the source code.
4. Upload the code to the ESP32 board.
5. Monitor the serial output for debugging and verification.

## Examples

The `examples/` directory contains example usage files to help you get started quickly. You can refer to these examples to understand how to integrate the system into your own projects.

## Contributing

Contributions are welcome! If you find any issues or have suggestions for improvements, please open an issue or submit a pull request.
