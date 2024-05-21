import serial # type: ignore
import time
from datetime import datetime
import os

# Replace with the correct serial port for your ESP32-S3
SERIAL_PORT = 'COM3'  # Use the correct port for your system, e.g., 'COM3' on Windows or '/dev/ttyUSB0' on Linux
BAUD_RATE = 115200
FILE_NAME = 'datalog.csv'

def download_file_from_esp32(delete_after_download=False):
    # Generate a timestamped output filename
    timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
    output_file = f"downloaded_{FILE_NAME.split('.')[0]}_{timestamp}.csv"

    # Ensure the "downloads" subfolder exists
    output_folder = "downloads"
    if not os.path.exists(output_folder):
        os.makedirs(output_folder)

    output_path = os.path.join(output_folder, output_file)

    ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
    time.sleep(2)  # Wait for the serial connection to establish

    ser.write(f"DOWNLOAD {FILE_NAME}\n".encode())
    time.sleep(1)  # Give ESP32 time to process the command

    with open(output_path, 'wb') as file:
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
    print(f"File {FILE_NAME} downloaded successfully as {output_path}")

    if delete_after_download:
        delete_file_on_esp32()

def delete_file_on_esp32():
    ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
    time.sleep(2)  # Wait for the serial connection to establish

    ser.write(f"DELETE {FILE_NAME}\n".encode())
    time.sleep(1)  # Give ESP32 time to process the command

    while True:
        line = ser.readline().strip()
        if line:
            print(line.decode())
        if line == b"DELETE_SUCCESS":
            print(f"File {FILE_NAME} deleted successfully on ESP32.")
            break
        elif line == b"DELETE_FAILED":
            print(f"Failed to delete file {FILE_NAME} on ESP32.")
            break

    ser.close()

if __name__ == "__main__":
    delete_option = input("Do you want to delete the datalog file after downloading? (yes/no): ").strip().lower()
    delete_after_download = delete_option == 'yes'
    download_file_from_esp32(delete_after_download)
