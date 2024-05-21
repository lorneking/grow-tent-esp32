import serial # type: ignore
import time
import os
from datetime import datetime

SERIAL_PORT = 'COM3'  # Update with your actual serial port
BAUD_RATE = 115200
TIMEOUT = 10  # Increased timeout in seconds

def download_file_from_esp32(delete_after_download=False):
    try:
        with serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1) as ser:
            ser.reset_input_buffer()
            ser.reset_output_buffer()
            time.sleep(2)  # Allow some time for the serial port to settle

            print("Sending DOWNLOAD command...")
            ser.write(b"DOWNLOAD datalog.csv\n")
            start_signal = wait_for_signal(ser, "START")

            if not start_signal:
                print("Timeout waiting for START signal")
                return

            file_name = generate_file_name()
            print(f"Writing data to {file_name}")
            
            with open(file_name, 'wb') as file:
                while True:
                    line = ser.readline()
                    if b"END" in line:
                        break
                    file.write(line)

            print(f"File datalog.csv downloaded successfully as {file_name}")

            if delete_after_download:
                time.sleep(2)  # Ensure the port is ready for the next command
                print("Sending DELETE command...")
                ser.write(b"DELETE datalog.csv\n")
                delete_signal = wait_for_signal(ser, "DELETE_SUCCESS")
                
                if not delete_signal:
                    print("Failed to delete file datalog.csv on ESP32.")
                else:
                    print("File datalog.csv deleted successfully on ESP32.")
    except Exception as e:
        print(f"Error: {e}")

def delete_file_on_esp32():
    try:
        with serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1) as ser:
            ser.reset_input_buffer()
            ser.reset_output_buffer()
            time.sleep(2)  # Allow some time for the serial port to settle

            print("Sending DELETE command...")
            ser.write(b"DELETE datalog.csv\n")
            delete_signal = wait_for_signal(ser, "DELETE_SUCCESS")
            
            if not delete_signal:
                print("Failed to delete file datalog.csv on ESP32.")
            else:
                print("File datalog.csv deleted successfully on ESP32.")
    except Exception as e:
        print(f"Error: {e}")

def wait_for_signal(ser, signal, timeout=TIMEOUT):
    start_time = time.time()
    while time.time() - start_time < timeout:
        if ser.in_waiting:
            line = ser.readline().decode().strip()
            print(f"Received line: {line}")  # Debugging output
            if signal in line:
                return True
    return False

def generate_file_name():
    timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
    folder_path = os.path.join(os.getcwd(), "downloads")
    if not os.path.exists(folder_path):
        os.makedirs(folder_path)
    return os.path.join(folder_path, f"downloaded_datalog_{timestamp}.csv")

def main():
    while True:
        action = input("What would you like to do? (download/delete/delete_after_download): ").strip().lower()
        if action == "download":
            download_file_from_esp32()
            break
        elif action == "delete":
            delete_file_on_esp32()
            break
        elif action == "delete_after_download":
            download_file_from_esp32(delete_after_download=True)
            break

if __name__ == "__main__":
    main()
