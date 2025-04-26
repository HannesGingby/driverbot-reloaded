import sensor, image, time, sys
import ubinascii

# Initialize camera
sensor.reset()
sensor.set_pixformat(sensor.RGB565)
sensor.set_framesize(sensor.QVGA)  # 320x240
sensor.skip_frames(time = 2000)    # Give time for auto adjustments

# Better quality for clearer images
quality = 80

while True:
    # Capture image
    img = sensor.snapshot()

    # Compress image to JPEG format
    buf = img.compress(quality=quality)

    # Encode the binary data as base64 to ensure safe transmission
    b64_data = ubinascii.b2a_base64(buf)

    # Get the size of the base64 encoded data
    size = len(b64_data)

    # Send a header with the size
    print("<<<IMG_B64:%d>>>" % size)

    # Write the base64 data
    sys.stdout.write(b64_data)

    # Send a footer
    print("<<<END>>>")

    # Wait a bit before next image
    time.sleep(2)
