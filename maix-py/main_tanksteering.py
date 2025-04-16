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

should_follow_line = True
#th = (0, 100, 12, 49, -8, 127)
th = (0, 100, 25, 127, -27, 127)


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


def steer(speed, direction):
    # Format command as: t,<speed>,<direction>\n
    cmd = "t,{0},{1}\n".format(speed, direction)
    uart.write(cmd)


def stop_motors():
    uart.write("x\n")


def stop_steering():
    uart.write("z\n")


def follow_line(line, img):
    if line:
        # Draw the detected line
        img.draw_line(line.x1(), line.y1(), line.x2(), line.y2(), image.rgb_to_lab((0, 255, 0)))

        # Calculate normalized theta (0 is vertical, positive is right, negative is left)
        #theta = line.theta()
        #if theta > 90:
        #    theta = 270 - theta
        #else:
        #    theta = 90 - theta

        x_center = img.height() / 2  # Camera flipped
        line_center_x = (line.y1() + line.y2()) / 2
        distance_to_center = x_center - line_center_x

        # Calculate tilt
        try:
            k = (line.y2() - line.y1()) / (line.x2() - line.x1())
        except ZeroDivisionError:
            k = 0

        if -80 <= distance_to_center <= 80:
            if -0.08 <= k <= 0.08:
                stop_motors()
                uart.write("w\n")  # Drive forward
                print("Driving forward")
            elif k > 0.08:
                steer(412, -1)  # Left turn
                print("Turn left")
            elif k < -0.08:
                steer(412, 1)  # Right turn
                print("Turn right")
        elif distance_to_center > 80:
            steer(412, 1)  # Right turn
            print("Repositioning right")
        elif distance_to_center < -80:
            steer(412, -1)  # Left turn
            print("Repositioning left")
    else:
        # No line detected, stop steering
        stop_motors()
        print("No line detected, stopping mottors")


while True:
    clock.tick()
    img = sensor.snapshot()

    line = img.get_regression([th], area_threshold = 100)
    follow_line(line, img)

    # command = check_input()
    # if command == "f":
    #     should_follow_line = True
    # elif command == "s":
    #     should_follow_line = False
    # elif command is not None:
    #     led_pin = command  # Update LED selection

    #if should_follow_line:
    #    line = img.get_regression([th], area_threshold = 100)
    #    follow_line(line, img, 5)

    current_time = utime.ticks_ms()
    if utime.ticks_diff(current_time, last_toggle_time) > 500:
        led_state = not led_state
        led_pin.value(led_state)
        last_toggle_time = current_time

    lcd.display(img)
