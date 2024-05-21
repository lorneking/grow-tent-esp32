import serial
import time

# Replace with the correct serial port for your ESP32-S3
SERIAL_PORT = 'COM3'  # Use the correct port for your system, e.g., '/dev/ttyUSB0' on Linux
BAUD_RATE = 115200
FILE_NAME = 'datalog.csv'
OUTPUT_FILE = 'downloaded_datalog.csv'

def download_file_from_esp32():
    ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
    time.sleep(2)  # Wait for the serial connection to establish

    ser.write(f"DOWNLOAD {FILE_NAME}\n".encode())
    time.sleep(1)  # Give ESP32 time to process the command

    with open(OUTPUT_FILE, 'wb') as file:
        while True:
            line = ser.readline().strip()
            if line == b"START":
                break

        while True:
            line = ser.readline().strip()
            if line == b"END":
                break
            file.write(line + b'\n')

    ser.close()
    print(f"File {FILE_NAME} downloaded successfully as {OUTPUT_FILE}")

if __name__ == "__main__":
    download_file_from_esp32()
