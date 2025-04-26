import sensor, time, lcd, image
from fpioa_manager import fm
from Maix import GPIO
import utime
from machine import UART
import KPU as kpu

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

#task = kpu.load("/sd/road_classifier.kmodel")
task = kpu.load(0x500000)
kpu.set_outputs(task, 0, 1, 1, 7)
labels = ['forward', 'intersection', 'left_right_t', 'left_turn', 'right_turn', 'straight_left', 'straight_right']

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


def steer(speed_1, direction_1, speed_2, direction_2):
    # Format command as t,<speed>,<direction>\n
    cmd = "t,{0},{1},{2},{3}\n".format(speed_1, direction_1, speed_2, direction_2)
    uart.write(cmd)


def stop_motors():
    uart.write("x\n")


def reverse():
    uart.write("s\n")


def p_controller(e, kp, p0):
    output = kp * e + p0

    return output


def follow_line(line, img):
    if line:
        # Draw the detected line
        img.draw_line(line.x1(), line.y1(), line.x2(), line.y2(), image.rgb_to_lab((0, 255, 0)))

        x_center = img.height() / 2  # Camera flipped
        line_center_x = (line.y1() + line.y2()) / 2
        distance_to_center = x_center - line_center_x

        deadzone_k_micro_adjust = 0.04
        repositioning_deadzone = 80

        # Calculate tilt
        try:
            k = (line.y2() - line.y1()) / (line.x2() - line.x1())
        except ZeroDivisionError:
            k = 0

        speed = p_controller(abs(k), 200, 400)
        print("Speed:", speed)

        if -repositioning_deadzone <= distance_to_center <= repositioning_deadzone:
            if -deadzone_k_micro_adjust <= k <= deadzone_k_micro_adjust:
                stop_motors()
                uart.write("w\n")  # Drive forward
                print("Driving forward")
            elif k > deadzone_k_micro_adjust:
                steer(speed, 1, 100, 1)  # Left turn
                print("Turn left")
            elif k < -deadzone_k_micro_adjust:
                steer(100, 1, speed, 1)  # Right turn
                print("Turn right")
        elif distance_to_center > repositioning_deadzone:
            steer(400, -1, 400, 1)  # Right turn
            print("Repositioning right")
        elif distance_to_center < -repositioning_deadzone:
            steer(400, 1, 400, -1)  # Left turn
            print("Repositioning left")
    else:
        # No line detected
        # stop_motors()
        reverse()
        print("No line detected, stopping mottors")


while True:
    clock.tick()
    img = sensor.snapshot()

    line = img.get_regression([th], area_threshold = 100)

    if line:
        #img_resized = img.resize(128, 128)  # Resize to match model input
        #img_resized.pix_to_ai()             # Convert to KPU format
        #fmap = kpu.forward(task, img_resized)


        # img_hsv = img.to_rainbow()

        # Create red mask - isolate red colors
        # You may need to adjust these thresholds to match your OpenCV preprocessing
        #red_mask = img_hsv.binary([(0, 30, 30, 255, 30, 255), (330, 360, 30, 255, 30, 255)])
        red_mask = img.binary([(0, 100, 25, 127, -27, 127)])

        # Resize to model input size (128x128)
        red_mask = red_mask.resize(128, 128)

        # Convert to RGB for the model
        # Even though it's black and white, the model expects 3 channels
        input_img = red_mask.to_rgb565()

        # Convert to format for KPU
        input_img.pix_to_ai()

        # Run inference
        fmap = kpu.forward(task, input_img)

        # Get classification results
        plist = fmap[:]
        max_idx = plist.index(max(plist))
        prediction = labels[max_idx]
        print(prediction)

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

    if line:
        img.draw_string(10, 10, "{} {:.2f}".format(prediction, max(plist)))

    lcd.display(img)




    time.sleep(0.5)

KPU.deinit(task)
