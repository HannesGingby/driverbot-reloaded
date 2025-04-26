import serial
import re
import time
import os
import base64

SERIAL_PORT = 'COM5'
BAUDRATE    = 115200
TIMEOUT     = 10          # Increased timeout for larger images

type_to_receive = "left_right_t"

def main():
    ser = serial.Serial(SERIAL_PORT, BAUDRATE, timeout=TIMEOUT)
    print("Listening on %s…" % SERIAL_PORT)

    # Create images directory if it doesn't exist
    if not os.path.exists(type_to_receive):
        os.makedirs(type_to_receive)

    try:
        while True:
            # Wait for header
            while True:
                line = ser.readline().decode('ascii', errors='ignore').strip()
                if not line:
                    continue

                # Look for base64 header pattern
                m = re.match(r'<<<IMG_B64:(\d+)>>>', line)
                if m:
                    b64_size = int(m.group(1))
                    print(f"Receiving base64 image data of {b64_size} bytes...")

                    # Read the base64 data
                    b64_data = ser.read(b64_size)
                    if len(b64_data) < b64_size:
                        print(f"Warning: Only received {len(b64_data)} of {b64_size} bytes")

                    # Decode the base64 data to binary
                    try:
                        img_data = base64.b64decode(b64_data)
                        print(f"Decoded to {len(img_data)} bytes of image data")

                        # Write to file
                        fname = os.path.join(type_to_receive, f"snapshot_{int(time.time())}.jpg")
                        with open(fname, 'wb') as f:
                            f.write(img_data)
                        print(f"Saved to {fname}")

                    except Exception as e:
                        print(f"Error decoding image: {e}")

                    # Read the footer
                    footer = ser.readline()
                    print(f"Footer: {footer}")
                    break  # Break inner loop to wait for next image

                # Otherwise, print the line for debugging
                elif line:
                    print(f"Received: {line}")

    except KeyboardInterrupt:
        print("Interrupted by user")
    finally:
        ser.close()
        print("Serial connection closed")

if __name__ == '__main__':
    main()
