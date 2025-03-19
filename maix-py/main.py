import sensor, image, time, lcd
from fpioa_manager import fm
from Maix import GPIO
import utime
from machine import UART

# Define LED and UART pins
io_led_blue = 12
io_led_green = 13
io_led_red = 14
TX_PIN = 15
RX_PIN = 17

# Register GPIO pins for LEDs
fm.register(io_led_red, fm.fpioa.GPIO0)
fm.register(io_led_green, fm.fpioa.GPIO1)
fm.register(io_led_blue, fm.fpioa.GPIO2)

led_r = GPIO(GPIO.GPIO0, GPIO.OUT)
led_g = GPIO(GPIO.GPIO1, GPIO.OUT)
led_b = GPIO(GPIO.GPIO2, GPIO.OUT)

# Register and configure UART
fm.register(TX_PIN, fm.fpioa.UART1_TX, force=True)
fm.register(RX_PIN, fm.fpioa.UART1_RX, force=True)

baud = 115200
uart = UART(UART.UART1, baud, 8, None, 1, timeout=1000, read_buf_len=4096)

# Function to check UART input
def check_input():
    global uart
    if uart.any():
        try:
            buf = uart.read()
            if buf:  # Ensure data is not None
                buf = buf.decode('utf-8').strip()
            else:
                return None
        except Exception as e:
            print("UART Read Error:", e)
            return None

        if buf == "f":
            print("Received 'f', starting line-following")
            return "f"
        elif buf == "s":
            print("Received 's', stopping line-following")
            return "s"
        elif buf == "red":
            print("Switching to red LED")
            return led_r
        elif buf == "green":
            print("Switching to green LED")
            return led_g
        elif buf == "blue":
            print("Switching to blue LED")
            return led_b
        else:
            print("Invalid input:", buf)
    return None

def p_controller(sp, pv, kp, p0):
    e = sp - pv
    output = kp * e + p0

    return output

def follow_blob(blobs, img, min_blob_pixel_size=2000):
    if blobs:
        b = max(blobs, key=lambda x: x.pixels())  # Find the largest blob
        if b.pixels() > min_blob_pixel_size:
            cy_value = b.cy()
            center = img.height() / 2

            correction = p_controller(center, cx_value, 0.5, 90)
            servo_angle = max(0, min(180, int(correction)))

            uart.write(str(servo_angle) + "\n")
            uart.write("w\n")
        else:
            uart.write("x\n")

def draw_blob_rectangle(blobs, img):
    if blobs:
        b = max(blobs, key=lambda x: x.pixels())  # Largest blob
        if b.pixels() > 2000:
            print(b)
            e = img.height() / 2 - b.cy()
            print(e)
            img.draw_rectangle(b[0:4])

# Initialize LCD and Camera
lcd.init(freq=15000000)
lcd.rotation(2)
sensor.reset()
sensor.set_pixformat(sensor.RGB565)
sensor.set_framesize(sensor.QVGA)
sensor.skip_frames(time=2000)

clock = time.clock()

led_state = 0
last_toggle_time = utime.ticks_ms()
led_pin = led_r  # Default LED

should_follow_blob = True

while True:
    clock.tick()

    img = sensor.snapshot()
    img = img.rotation_corr(z_rotation=90)

    th = (0, 100, 12, 49, -8, 127)
    blobs = img.find_blobs([th])

    lcd.display(img)

    # command = check_input()
    # if command == "f":
    #     should_follow_blob = True
    # elif command == "s":
    #     should_follow_blob = False
    # elif command is not None:
    #     led_pin = command  # Update LED selection

    if should_follow_blob:
        follow_blob(blobs, img)
        draw_blob_rectangle(blobs, img)

    current_time = utime.ticks_ms()
    if utime.ticks_diff(current_time, last_toggle_time) > 500:
        led_state = not led_state
        led_pin.value(led_state)
        last_toggle_time = current_time
