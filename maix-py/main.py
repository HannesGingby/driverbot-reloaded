import sensor, time, lcd, image, _thread
from fpioa_manager import fm
from Maix import GPIO
import utime
from machine import UART
import KPU as kpu
from time import sleep

# --- Pin and Peripheral Setup ---
io_led_red, io_led_green, io_led_blue = 14, 13, 12
TX_PIN, RX_PIN = 15, 17
fm.register(io_led_red, fm.fpioa.GPIO0)
fm.register(io_led_green, fm.fpioa.GPIO1)
fm.register(io_led_blue, fm.fpioa.GPIO2)
led_r = GPIO(GPIO.GPIO0, GPIO.OUT)
led_g = GPIO(GPIO.GPIO1, GPIO.OUT)
led_b = GPIO(GPIO.GPIO2, GPIO.OUT)
fm.register(TX_PIN, fm.fpioa.UART1_TX, force=True)
fm.register(RX_PIN, fm.fpioa.UART1_RX, force=True)
uart = UART(UART.UART1, 115200, 8, None, 1, timeout=1000, read_buf_len=4096)

# --- Shared State & Locks ---
lock = _thread.allocate_lock()
shared_pred, shared_conf = None, 0
should_follow = True
road_buffer = []
counter = 0
last_model_ms = utime.ticks_ms()
model_interval = 2000  # ms

# --- Line Detection Threshold ---
th = (0, 100, 25, 127, -27, 127)

# --- KPU Model ---
task = kpu.load(0x500000)
kpu.set_outputs(task, 0, 1, 1, 7)
labels = ['forward', 'intersection', 'left_right_t', 'left_turn', 'right_turn', 'straight_left', 'straight_right']

# --- Sensor & LCD ---
lcd.init(freq=15000000); lcd.rotation(2)
sensor.reset(); sensor.set_pixformat(sensor.RGB565); sensor.set_framesize(sensor.QVGA)
sensor.skip_frames(time=2000)
clock = time.clock()

def check_input():
    if uart.any():
        try:
            buf = uart.read()
            if buf:
                buf = buf.decode('utf-8').strip()
            else:
                return None
        except Exception as e:
            print("UART Read Error:", e)
            return None

        if buf == "f":
            return "f"
        elif buf == "s":
            return "s"
        elif buf == "red":
            return led_r
        elif buf == "green":
            return led_g
        elif buf == "blue":
            return led_b
        else:
            print("Invalid input:", buf)
    return None

def p_controller(e, kp, p0):
    return kp * e + p0

def steer(speed_1, direction_1, speed_2, direction_2):
    # Format command as t,<speed>,<direction>\n
    cmd = "t,{0},{1},{2},{3}\n".format(speed_1, direction_1, speed_2, direction_2)
    uart.write(cmd)

def stop_motors():
    uart.write("x\n")

def reverse():
    uart.write("s\n")

def follow_line(line, img):
    if line:
        img.draw_line(line.x1(), line.y1(), line.x2(), line.y2(), image.rgb_to_lab((0,255,0)))
        xC = img.height()/2; lC = (line.y1()+line.y2())/2; dist = xC - lC
        try: k=(line.y2()-line.y1())/(line.x2()-line.x1())
        except: k=0
        speed = p_controller(abs(k)*100,2,400)
        dz=0.04; rz=80
        if abs(dist)<=rz:
            if abs(k)<=dz: uart.write("w\n")
            elif k>dz: steer(speed,1,100,1)
            else: steer(100,1,speed,1)
        else:
            steer(200,-1,200,1) if dist>rz else steer(200,1,200,-1)
    else: reverse()

def classify_task(img):
    """Runs in thread; non-blocking classification."""
    try:
        print("RUNNING MODEL")
        sleep(0.01)
        # Preprocess pipeline
        m = img.copy().binary([th]).resize(128,128).to_rgb565()
        m.pix_to_ai()
        sleep(0.01)
        # Forward
        fmap = kpu.forward(task, m)
        sleep(0.01)
        # Extract list via slice
        plist = fmap[:]
        idx = plist.index(max(plist)); lbl = labels[idx]; conf = max(plist)
        with lock:
            global shared_pred, shared_conf, road_buffer
            shared_pred, shared_conf = lbl, conf
            road_buffer.append(lbl)
            if len(road_buffer) >= 5:
                tile = max(set(road_buffer), key=road_buffer.count)
                print("Road tile:", tile)
                road_buffer.clear()
    except Exception as e:
        print("Classification error:", e)
    finally:
        sleep(0.01)

def main():
    global counter, last_model_ms, should_follow
    while True:
        clock.tick()
        print("counter", str(counter))
        img = sensor.snapshot()
        now = utime.ticks_ms()
        if utime.ticks_diff(now, last_model_ms) >= model_interval:
            _thread.start_new_thread(classify_task, (img,))
            last_model_ms = now
            counter += 1
        if should_follow:
            line = img.get_regression([th], area_threshold=100)
            follow_line(line, img)
        with lock:
            pred, conf = shared_pred, shared_conf
        if pred:
            print(pred)
        cmd = check_input()
        if cmd == 'f': should_follow = True
        elif cmd == 's': should_follow = False; stop_motors()
        lcd.display(img)
        sleep(0.2)

if __name__ == '__main__':
    main()
    kpu.deinit(task)








'''

import sensor, time, lcd, image, _thread
from fpioa_manager import fm
from Maix import GPIO
import utime
from machine import UART
import KPU as kpu
from time import sleep

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

# Initialize shared resources and locks
lock = _thread.allocate_lock()

# Shared variables
shared_prediction = None
shared_confidence = 0
shared_img = None
road_types_tile = []
should_follow_line = True

# Thresholds for line detection
th = (0, 100, 25, 127, -27, 127)

# Load the model
task = kpu.load(0x500000)  # model address, road_classifier.kmodel
kpu.set_outputs(task, 0, 1, 1, 7)
labels = ['forward', 'intersection', 'left_right_t', 'left_turn', 'right_turn', 'straight_left', 'straight_right']

# Timing control
model_interval = 2000  # ms
last_model_run_time = utime.ticks_ms()

counter = 0

# Initialize LCD and Camera
lcd.init(freq=15000000)
lcd.rotation(2)
sensor.reset()
sensor.set_pixformat(sensor.RGB565)
sensor.set_framesize(sensor.QVGA)
sensor.skip_frames(time=2000)

clock = time.clock()

def check_input():
    if uart.any():
        try:
            buf = uart.read()
            if buf:
                buf = buf.decode('utf-8').strip()
            else:
                return None
        except Exception as e:
            print("UART Read Error:", e)
            return None

        if buf == "f":
            return "f"
        elif buf == "s":
            return "s"
        elif buf == "red":
            return led_r
        elif buf == "green":
            return led_g
        elif buf == "blue":
            return led_b
        else:
            print("Invalid input:", buf)
    return None

def p_controller(e, kp, p0):
    return kp * e + p0

def steer(speed_1, direction_1, speed_2, direction_2):
    # Format command as t,<speed>,<direction>\n
    cmd = "t,{0},{1},{2},{3}\n".format(speed_1, direction_1, speed_2, direction_2)
    uart.write(cmd)

def stop_motors():
    uart.write("x\n")

def reverse():
    uart.write("s\n")

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

        speed = p_controller(abs(k) * 100, 2, 400)

        if -repositioning_deadzone <= distance_to_center <= repositioning_deadzone:
            if -deadzone_k_micro_adjust <= k <= deadzone_k_micro_adjust:
                uart.write("w\n")  # Drive forward
                #print("Driving forward")
            elif k > deadzone_k_micro_adjust:
                steer(speed, 1, 100, 1)  # Left turn
                #print("Turning left")
            elif k < -deadzone_k_micro_adjust:
                steer(100, 1, speed, 1)  # Right turn
                #print("Turning right")
        elif distance_to_center > repositioning_deadzone:
            steer(200, -1, 200, 1)  # Right turn
            #print("Repositioning right")
        elif distance_to_center < -repositioning_deadzone:
            steer(200, 1, 200, -1)  # Left turn
            #print("Repositioning left")
    else:
        #print("Reverse")
        reverse()

def classify_road(img):
    try:
        # Create a copy to avoid modifying the original
        img_copy = img.copy()
        red_mask = img_copy.binary([th])
        red_mask = red_mask.resize(128, 128).to_rgb565()
        red_mask.pix_to_ai()

        print("Model running...")
        fmap = kpu.forward(task, red_mask)
        print("Model finished")

        plist = fmap[:]
        max_idx = plist.index(max(plist))
        prediction = labels[max_idx]
        print("Road type:", prediction)

        return prediction, max(plist)
    except Exception as e:
        print("Model error:", e)
        return "unknown", 0

def identify_road_tile(labels_list):
    if not labels_list:
        return "unknown"
    return max(set(labels_list), key=labels_list.count)

def model_thread():
    global shared_prediction, shared_confidence, road_types_tile, last_model_run_time, shared_img, counter

    while True:
        with lock:
            counter += 2

        try:
            current_time = utime.ticks_ms()

            # Only run the model at the specified interval
            if utime.ticks_diff(current_time, last_model_run_time) >= model_interval:
                with lock:
                    img = sensor.snapshot()

                    prediction, confidence = classify_road(img)

                    shared_prediction = prediction
                    shared_confidence = confidence
                    road_types_tile.append(prediction)

                    if len(road_types_tile) >= 5:
                        tile = identify_road_tile(road_types_tile)
                        print("Road tile:", tile)
                        road_types_tile = []

                last_model_run_time = current_time

            # Don't hog CPU
            sleep(0.2)

        except Exception as e:
            print("Model thread error:", e)
            sleep(1)  # Wait a bit before retrying

def main_thread():
    global should_follow_line

    while True:
        with lock:
            print("counter: {}".format(counter))

        try:
            clock.tick()

            img = sensor.snapshot()

            current_prediction = None
            current_confidence = 0

            with lock:
                current_prediction = shared_prediction
                current_confidence = shared_confidence

            # Process line following if enabled
            if should_follow_line:
                line = img.get_regression([th], area_threshold=100)
                follow_line(line, img)

            # Add prediction text to the image
            if current_prediction:
                img.draw_string(10, 10, "{} {:.2f}".format(current_prediction, current_confidence))

            # Check for user input
            command = check_input()
            if command == "f":
                should_follow_line = True
                print("Line following enabled")
            elif command == "s":
                should_follow_line = False
                stop_motors()
                print("Line following disabled")

            # Display the image
            lcd.display(img)

            # Don't hog CPU
            sleep(0.2)

        except Exception as e:
            print("Main thread error:", e)
            sleep(1)  # Wait a bit before retrying

# Start the model thread
_thread.start_new_thread(model_thread, ())

# Start the main thread (this will block)
try:
    main_thread()
except Exception as e:
    print("Fatal error:", e)
    stop_motors()  # Safety: stop motors on crash
    kpu.deinit(task)  # Clean up KPU resources

# This line only runs if main_thread exits
kpu.deinit(task)


'''







'''

import sensor, time, lcd, image, _thread
from fpioa_manager import fm
from Maix import GPIO
import utime
from machine import UART
import KPU as kpu
from time import sleep

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

task = kpu.load(0x500000) # model adress, road_classifier.kmodel
kpu.set_outputs(task, 0, 1, 1, 7)
labels = ['forward', 'intersection', 'left_right_t', 'left_turn', 'right_turn', 'straight_left', 'straight_right']

model_interval = 2000  # ms
last_model_run_time = utime.ticks_ms()
# Multithreading setup
lock = _thread.allocate_lock()
shared_prediction = None
shared_confidence = 0

road_types_tile = []
current_road_tile = ""

# Initialize LCD and Camera
lcd.init(freq=15000000)
lcd.rotation(2)
sensor.reset()
sensor.set_pixformat(sensor.RGB565)
sensor.set_framesize(sensor.QVGA)
sensor.skip_frames(time=2000)

clock = time.clock()

led_state = 0
last_led_time = utime.ticks_ms()
led_pin = led_r  # Default LED

should_follow_line = True
#th = (0, 100, 12, 49, -8, 127)
th = (0, 100, 25, 127, -27, 127)


# Function to check UART input
def check_input():
    if uart.any():
        try:
            buf = uart.read()
            if buf:
                buf = buf.decode('utf-8').strip()
            else:
                return None
        except Exception as e:
            print("UART Read Error:", e)
            return None

        if buf == "f":
            # print("Received 'f', starting line-following")
            return "f"
        elif buf == "s":
            # print("Received 's', stopping line-following")
            return "s"
        elif buf == "red":
            # print("Switching to red LED")
            return led_r
        elif buf == "green":
            # print("Switching to green LED")
            return led_g
        elif buf == "blue":
            # print("Switching to blue LED")
            return led_b
        else:
            print("Invalid input:", buf)
    return None


def p_controller(e, kp, p0):
    return kp * e + p0


def steer(speed_1, direction_1, speed_2, direction_2):
    # Format command as t,<speed>,<direction>\n
    cmd = "t,{0},{1},{2},{3}\n".format(speed_1, direction_1, speed_2, direction_2)
    uart.write(cmd)


def stop_motors():
    uart.write("x\n")


def reverse():
    uart.write("s\n")


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

        speed = p_controller(abs(k) * 100, 2, 400)
        # print("Speed:", speed)

        if -repositioning_deadzone <= distance_to_center <= repositioning_deadzone:
            if -deadzone_k_micro_adjust <= k <= deadzone_k_micro_adjust:
                uart.write("w\n")  # Drive forward
                print("Driving forward")
            elif k > deadzone_k_micro_adjust:
                steer(speed, 1, 100, 1)  # Left turn
                print("Turning left")
            elif k < -deadzone_k_micro_adjust:
                steer(100, 1, speed, 1)  # Right turn
                print("Turning right")
        elif distance_to_center > repositioning_deadzone:
            steer(200, -1, 200, 1)  # Right turn
            print("Repositioning right")
        elif distance_to_center < -repositioning_deadzone:
            steer(200, 1, 200, -1)  # Left turn
            print("Repositioning left")
    else:
        print("Reverse")
        reverse()


def classify_road(img) -> str:
    # red_mask -> resize -> to_rgb565 -> pix_to_ai
    img_copy = img.copy()
    red_mask = img_copy.binary([th])
    red_mask = red_mask.resize(128,128).to_rgb565()
    red_mask.pix_to_ai()

    print("Model running...")
    fmap = kpu.forward(task, red_mask)
    print("Model finished")

    plist = fmap[:]
    max_idx = plist.index(max(plist))
    prediction = labels[max_idx]
    print("Road type", prediction)

    return prediction, max(plist)


def identify_road_tile(labels: list) -> str:
    return max(set(labels), key=labels.count)


def explore_world():
    pass


def model_thread():
    global shared_prediction, shared_confidence, road_types_tile, last_model_run_time
    while True:
        current_time = utime.ticks_ms()
        if utime.ticks_diff(current_time, last_model_run_time) >= model_interval:
            img = sensor.snapshot()  # take snapshot separately for this thread
            prediction, confidence = classify_road(img)

            with lock:
                shared_prediction = prediction
                shared_confidence = confidence
                road_types_tile.append(prediction)

            if len(road_types_tile) >= 5:
                with lock:
                    tile = identify_road_tile(road_types_tile)
                    print("Road tile:", tile)
                    road_types_tile = []

            last_model_run_time = current_time

        sleep(0.1)


_thread.start_new_thread(model_thread, ())


def main_thread():
    while True:
        clock.tick()
        current_time = utime.ticks_ms()

        img = sensor.snapshot()

        print(current_time)

        if should_follow_line:
            line = img.get_regression([th], area_threshold = 100)
            follow_line(line, img)

        #if utime.ticks_diff(current_time, last_model_run_time) >= model_interval:
        #    prediction, confidence = classify_road(img)
        #    road_types_tile.append(prediction)
#
 #           img.draw_string(10, 10, "{} {:.2f}".format(prediction, confidence))
#
 #           if len(road_types_tile) >= 5:
  #              current_road_tile = identify_road_tile(road_types_tile)
   #             print("Road tile:", current_road_tile)
    #            road_types_tile = []
#
#            last_model_run_time = current_time
#
 #       with lock:
 #           if shared_prediction is not None:
  #              img.draw_string(10, 10, "{} {:.2f}".format(shared_prediction, shared_confidence))
   #             print(shared_prediction, shared_confidence)


        # command = check_input()
        # if command == "f":
        #     should_follow_line = True
        # elif command == "s":
        #     should_follow_line = False
        # elif command is not None:
        #     led_pin = command  # Update LED selection


        # LED blink
        #if utime.ticks_diff(current_time, last_led_time) > 500:
         #   led_state = not led_state
          #  led_pin.value(led_state)
           # last_led_time = current_time

        lcd.display(img)
        sleep(0.1)

main_thread()

#KPU.deinit(task)

'''
