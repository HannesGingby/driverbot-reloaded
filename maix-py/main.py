import sensor, time, lcd, image
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
            cx_value = b.cx()
            center = img.width() / 2

            correction = p_controller(center, cx_value, 0.5, 90)
            servo_angle = max(0, min(180, int(correction)))

            print(servo_angle)
            uart.write(str(servo_angle) + "\n")
            uart.write("w\n")
        else:
            uart.write("x\n")

prev_servo_angle = 0

def follow_line(line, img, max_allowed_servo_change):
    global prev_servo_angle

    if line:
        # Draw the detected line
        img.draw_line(line.x1(), line.y1(), line.x2(), line.y2(), image.rgb_to_lab((0, 255, 0)))

        # Calculate normalized theta (0 is vertical, positive is right, negative is left)
        theta = line.theta()
        if theta > 90:
            theta = 270 - theta
        else:
            theta = 90 - theta

        # Calculate line position relative to image center
        x_center = img.width() / 2
        line_center_x = (line.x1() + line.x2()) / 2
        distance_to_center = x_center - line_center_x

        # Calculate desired servo output
        servo_output = max(-90, min(90, int(distance_to_center * 0.5)))  # Scaled down for smoother response

        # Smooth servo angle changes
        degree_change = abs(servo_output - prev_servo_angle)

        # Apply smooth servo angle transition
        if degree_change > max_allowed_servo_change:
            # Gradually move towards the target angle
            if servo_output > prev_servo_angle:
                servo_angle = prev_servo_angle + max_allowed_servo_change
            else:
                servo_angle = prev_servo_angle - max_allowed_servo_change
        else:
            # Use a weighted average for smoother transition
            servo_angle = int(0.7 * prev_servo_angle + 0.3 * servo_output)

        # Update previous servo angle
        prev_servo_angle = servo_angle

        # Send servo command
        print(servo_angle)
        uart.write(str(servo_angle) + "\n")
        uart.write("w\n")

    else:
        # Reset to center when no line is detected
        servo_angle = 0
        prev_servo_angle = 0

        print(servo_angle)
        uart.write(str(servo_angle) + "\n")
        uart.write("x\n")


def draw_blob_rectangle(blobs, img):
    if blobs:
        b = max(blobs, key=lambda x: x.pixels())  # Largest blob
        if b.pixels() > 2000:
            print(b)
            e = img.width() / 2 - b.cx()
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

    th = (0, 100, 12, 49, -8, 127)
    blobs = img.find_blobs([th])

    line = img.get_regression([th], area_threshold = 100)
    follow_line(line, img, 5)

    lcd.display(img)

    # command = check_input()
    # if command == "f":
    #     should_follow_blob = True
    # elif command == "s":
    #     should_follow_blob = False
    # elif command is not None:
    #     led_pin = command  # Update LED selection

    #if should_follow_blob:
    #    follow_blob(blobs, img)
    #    draw_blob_rectangle(blobs, img)

    current_time = utime.ticks_ms()
    if utime.ticks_diff(current_time, last_toggle_time) > 500:
        led_state = not led_state
        led_pin.value(led_state)
        last_toggle_time = current_time
