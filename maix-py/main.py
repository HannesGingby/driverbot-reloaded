import sensor, time, lcd, image, _thread, utime
from fpioa_manager import fm
from Maix import GPIO
from machine import UART
import KPU as kpu

# Setup
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
uart = UART(UART.UART1, 115200, 8, None, 1, read_buf_len=4096)

# Shared state between threads
state_lock = _thread.allocate_lock()
shared_pred, shared_conf = None, 0
model_result = None
is_model_running = False
last_model_run = utime.ticks_ms()
last_ms = utime.ticks_ms()

# Logic, algorithm
tile_x = 0
tile_y = 0

map_x_size = 10
map_y_size = 5
start_tile_x = 0
start_tile_y = 0

discovered = []   # Map

should_follow = True
road_buffer = []
inference_queue = []
model_interval = 30000  # ms

# Line threshold
th = (0, 100, 25, 127, -27, 127)

# Road type classifier model
task = kpu.load(0x500000)
kpu.set_outputs(task, 0, 1, 1, 7)
labels = ['forward', 'intersection', 'left_right_t', 'left_turn',
          'right_turn', 'straight_left', 'straight_right']

# Sensor and LCD setup
lcd.init(freq=15000000)
lcd.rotation(2)
sensor.reset()
sensor.set_pixformat(sensor.RGB565)
sensor.set_framesize(sensor.QVGA)
sensor.skip_frames(time=2000)
clock = time.clock()

uart_buffer = ""
uart_busy = False
last_send_time = 0

'''
def process_uart():
    global should_follow, tile_x, tile_y, uart_buffer

    if uart.any():
        try:
            # Read available data and add to buffer
            new_data = uart.read()
            if new_data:
                uart_buffer += new_data.decode('utf-8')
            else:
                return None
        except Exception as e:
            print("UART Read Error:", e)
            return None

        # Process complete lines (commands ending with \n)
        while '\n' in uart_buffer:
            # Extract one complete line
            line_end = uart_buffer.find('\n')
            buf = uart_buffer[:line_end].strip()
            uart_buffer = uart_buffer[line_end + 1:]  # Remove processed line

            # Skip empty lines
            if not buf:
                continue

            print("Processing command:", repr(buf))

            # Process the complete command
            if buf == "f":
                should_follow = True
            elif buf == "r":
                led_r.value(1)
            elif buf == "g":
                led_g.value(1)
            elif buf == "b":
                led_b.value(1)
            elif buf.startswith("p,"):
                try:
                    # Handle multiple smooshed messages
                    if buf.count("p,") > 1:
                        buf = "p," + buf.split("p,")[-1]

                    parts = buf.strip().split(',')

                    if len(parts) >= 3 and parts[1].strip() and parts[2].strip():
                        tile_x = int(parts[1].strip())
                        tile_y = int(parts[2].strip())

                        print("NEW TILE: " + str(tile_x) + ", " + str(tile_y))


                        #should_follow = False
                        #motors_stop()
                        #utime.sleep_ms(2000)
                        #should_follow = True

                    else:
                        print("Invalid position format:", repr(buf))

                except (ValueError, IndexError) as e:
                    print("Error parsing position data:", e, repr(buf))
            else:
                print("Invalid input:", buf)

'''


def process_uart():
    global should_follow, tile_x, tile_y, uart_buffer, uart_busy, last_send_time

    # Don't read immediately after sending to avoid reading our own transmission
    if uart_busy and utime.ticks_diff(utime.ticks_ms(), last_send_time) < 50:
        return

    uart_busy = False

    if uart.any():
        try:
            # Read available data and add to buffer
            new_data = uart.read()
            if new_data:
                # Handle both string and bytes
                if isinstance(new_data, bytes):
                    uart_buffer += new_data.decode('utf-8')
                else:
                    uart_buffer += str(new_data)
            else:
                return None
        except Exception as e:
            print("UART Read Error:", str(e))
            return None

        # Process complete lines (commands ending with \n)
        while '\n' in uart_buffer:
            # Extract one complete line
            line_end = uart_buffer.find('\n')
            buf = uart_buffer[:line_end].strip()
            uart_buffer = uart_buffer[line_end + 1:]  # Remove processed line

            # Skip empty lines
            if not buf:
                continue

            # FILTER OUT OWN TRANSMITTED COMMANDS AND CORRUPTED DATA
            # Skip commands that look like motor commands (contain multiple numbers)
            #if ',' in buf and any(c.isdigit() for c in buf):
            #    # Check if this looks like a motor command (numbers with commas)
            #    parts = buf.split(',')
            #    if len(parts) >= 3 and any(part.replace('.', '').isdigit() for part in parts):
            #        print("Ignoring suspected motor echo:", repr(buf))
            #        continue

            # Process the complete command
            print("Processing command:", repr(buf))

            if buf == "f":
                should_follow = True
                print("Following enabled")
            #elif buf == "r":
            #    led_r.value(1)
            #    print("Red LED on")
            #elif buf == "g":
            #    led_g.value(1)
            #    print("Green LED on")
            #elif buf == "b":
            #    led_b.value(1)
            #    print("Blue LED on")
            elif buf.startswith("p,"):
                try:
                    # Clean up the command - remove any extra characters
                    clean_buf = buf.strip()

                    # Split by comma
                    parts = clean_buf.split(',')

                    # Ensure we have at least 3 parts: p, x, y
                    if len(parts) >= 3:
                        # Validate that parts[1] and parts[2] are not empty and are numeric
                        x_str = parts[1].strip()
                        y_str = parts[2].strip()

                        if x_str and y_str and x_str.isdigit() and y_str.isdigit():
                            tile_x = int(x_str)
                            tile_y = int(y_str)
                            print("NEW TILE: {}, {}".format(tile_x, tile_y))

                            should_follow = False
                            motors_stop()
                            utime.sleep_ms(2000)
                            should_follow = True
                        else:
                            print("Invalid position values: x='{}', y='{}'".format(x_str, y_str))
                    else:
                        print("Insufficient position parts: {}".format(len(parts)))

                except (ValueError, IndexError) as e:
                    print("Error parsing position data: {} - '{}'".format(str(e), buf))
            else:
                print("Unknown command: '{}'".format(buf))



'''
elif buf.startswith("p,"):
    try:
        # Handle multiple smooshed messages
        if buf.count("p,") > 1:
            buf = "p," + buf.split("p,")[-1]

        parts = buf.strip().split(',')

        if len(parts) >= 3 and parts[1].strip() and parts[2].strip():
            tile_x = int(parts[1].strip())
            tile_y = int(parts[2].strip())

            print("NEW TILE: " + str(tile_x) + ", " + str(tile_y))


            #should_follow = False
            #motors_stop()
            #utime.sleep_ms(2000)
            #should_follow = True

        else:
            print("Invalid position format:", repr(buf))

    except (ValueError, IndexError) as e:
        print("Error parsing position data:", e, repr(buf))
'''

def p_controller(e, kp, p0):
    return kp * e + p0

'''
def motors_steer(speed_1, direction_1, speed_2, direction_2):
    # Format command as t,<speed1>,<direction1>,<speed2>,<direction2>\n
    cmd = "t,{0},{1},{2},{3}\n".format(speed_1, direction_1, speed_2, direction_2)
    print("Sending command:", cmd)
    uart.write(cmd)


def motors_stop():
    uart.write("x\n")


def motors_forward():
    uart.write("w\n")


def motors_reverse():
    uart.write("s\n")


def send_road_tile(road_tile):
    uart.write("r,{0}\n".format(road_tile))
'''


def motors_steer(speed_1, direction_1, speed_2, direction_2):
    global uart_busy, last_send_time

    # Ensure speeds are within valid range
    speed_1 = max(0, min(255, int(speed_1)))
    speed_2 = max(0, min(255, int(speed_2)))

    # Ensure directions are 0 or 1
    direction_1 = 1 if direction_1 else 0
    direction_2 = 1 if direction_2 else 0

    # Format command as t,<speed1>,<direction1>,<speed2>,<direction2>\n
    cmd = "t,{},{},{},{}\n".format(speed_1, direction_1, speed_2, direction_2)
    try:
        # Clear any pending received data before sending
        if uart.any():
            uart.read()  # Flush receive buffer

        uart.write(cmd)
        uart_busy = True
        last_send_time = utime.ticks_ms()
        #print("Sending command:", cmd.strip())
    except Exception as e:
        print("UART write error:", str(e))

def motors_stop():
    global uart_busy, last_send_time
    try:
        if uart.any():
            uart.read()  # Flush receive buffer

        uart.write("x\n")
        uart_busy = True
        last_send_time = utime.ticks_ms()
        print("Stop")
    except Exception as e:
        print("UART write error:", str(e))

def motors_forward():
    global uart_busy, last_send_time
    try:
        if uart.any():
            uart.read()  # Flush receive buffer

        uart.write("w\n")
        uart_busy = True
        last_send_time = utime.ticks_ms()
        print("Forward")
    except Exception as e:
        print("UART write error:", str(e))

def motors_reverse():
    global uart_busy, last_send_time
    try:
        if uart.any():
            uart.read()  # Flush receive buffer

        uart.write("s\n")
        uart_busy = True
        last_send_time = utime.ticks_ms()
        print("Reverse")
    except Exception as e:
        print("UART write error:", str(e))

def send_road_tile(road_tile):
    global uart_busy, last_send_time
    try:
        if uart.any():
            uart.read()  # Flush receive buffer

        cmd = "r,{}\n".format(road_tile)
        uart.write(cmd)
        uart_busy = True
        last_send_time = utime.ticks_ms()
        print("Sent road tile:", road_tile)
    except Exception as e:
        print("UART write error:", str(e))



def adjust_after_line(line, img):
    if line:
        img.draw_line(line.x1(), line.y1(), line.x2(), line.y2(), image.rgb_to_lab((0,255,0)))
        xC = img.height() / 2
        lC = (line.y1() + line.y2()) / 2
        dist = xC - lC
        try:
            k = (line.y2() - line.y1()) / (line.x2() - line.x1())
        except ZeroDivisionError:
            k = 0
        speed = p_controller(abs(k) * 100, 2, 255)
        dz, rz = 0.04, 80
        if abs(dist) <= rz:
            if abs(k) <= dz:
                motors_forward()
                print("Forward")
            elif k > dz:
                motors_steer(150, 1, speed, 1)
                print("Left")
            else:
                motors_steer(speed, 1, 150, 1)
                print("Right")
        else:
            if dist > rz:
                motors_steer(200, 1, 200, 0)
                print("Repositiong left")
            else:
                motors_steer(200, 0, 200, 1)
                print("Repositiong right")
    else:
        print("No line found")
        motors_stop()


def model_callback(result):
    global model_result, is_model_running, shared_pred, shared_conf

    # Store the result
    state_lock.acquire()
    try:
        model_result = result
        is_model_running = False

        # Process result
        if result:
            try:
                plist = result[:]
                idx = plist.index(max(plist))
                prediction = labels[idx]
                confidence = max(plist)
                print("Prediction:", prediction, "confidence:", confidence)
                shared_pred = prediction
                shared_conf = confidence
            except Exception as e:
                print("Result processing error:", e)
    finally:
        state_lock.release()


def model_thread():
    global shared_pred, shared_conf, last_model_run, is_model_running
    while True:
        now = utime.ticks_ms()
        state_lock.acquire()
        if inference_queue:
            img = inference_queue.pop(0)
            state_lock.release()
        else:
            state_lock.release()
            img = None
        if img:
            try:
                # Process image for model
                state_lock.acquire()
                try:
                    is_model_running = True
                finally:
                    state_lock.release()

                last_model_run = now
                m = img.binary([th]).resize(128, 128).to_rgb565()
                m.pix_to_ai()

                # Use the non-blocking forward_async method
                success = kpu.forward_async(task, m, model_callback)
                if not success:
                    print("Failed to start model")
                    is_model_running = False
            except Exception as e:
                print("Model start error:", e)
                is_model_running = False
        else:
            utime.sleep_ms(10)

def get_road_tile(img):
    global last_ms, shared_pred, shared_conf, is_model_running

    # Run road classifier on interval
    now = utime.ticks_ms()
    # print(is_model_running)
    if utime.ticks_diff(now, last_ms) >= model_interval:
        state_lock.acquire()
        try:
            inference_queue.append(img)
        finally:
            state_lock.release()
        last_ms = now

    # Model results debug
    if shared_pred:
        print(shared_pred, shared_conf)

    # Add model result to road buffer
    if shared_pred is not None:
        road_buffer.append(shared_pred)
        if len(road_buffer) > 5:
            road_buffer.pop(0)

        # Clear the shared prediction so we don't process it again
        shared_pred = None

    # Get road tile - only process when we have exactly 5 items
    if len(road_buffer) == 5:
        count_dict = {}
        for item in road_buffer:
            if item in count_dict:
                count_dict[item] += 1
            else:
                count_dict[item] = 1

        most_common_pred = None
        highest_count = 0
        for item, count in count_dict.items():
            if count > highest_count:
                highest_count = count
                most_common_pred = item

        print("Road tile:", most_common_pred)
        print("Sending tile")
        send_road_tile(most_common_pred)

        # Clear the buffer after processing to avoid reprocessing
        road_buffer.clear()

        return most_common_pred

    return None


'''
def explore_unknown(img):
    if len(discovered) <= map_x_size * map_y_size:
        road_tile = get_road_tile(img)

        if tile and tile not in discovered:
            discovered.append(tile)
        return False
    else:
        print("Explored map fully")
        return True
'''

def main():
    global should_follow, shared_pred, shared_conf
    #counter = 0
    while True:
        elapsed = clock.tick()
        img = sensor.snapshot()

        process_uart()

        #counter += 1
        #print("counter:", counter)

        # Follow line
        if should_follow:
            line = img.get_regression([th], area_threshold=100)
            adjust_after_line(line, img)
        else:
            motors_stop()

        get_road_tile(img)

        # Task
        '''
        done = explore_unknown(img)
        if done:
            break
        '''

        lcd.display(img)
        utime.sleep_ms(10)


if __name__ == "__main__":
    try:
        _thread.start_new_thread(model_thread, ())   # Start model thread
        main()
    finally:
        kpu.deinit(task)





'''


import sensor, time, lcd, image, _thread, utime
from fpioa_manager import fm
from Maix import GPIO
from machine import UART
import KPU as kpu

# Setup
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

# Shared state between threads
state_lock = _thread.allocate_lock()
shared_pred, shared_conf = None, 0

# Logic, algorihtm
x = 0
y = 0
heading = 0
tile_x = 0
tile_y = 0

map_x_size = 10
map_y_size = 5
start_tile_x = 0
start_tile_y = 0

discovered = []   # Map

should_follow = True
road_buffer = []
inference_queue = []
model_interval = 500  # ms

# Line threshold
th = (0, 100, 25, 127, -27, 127)

# Road type classifier model
task = kpu.load(0x500000)
kpu.set_outputs(task, 0, 1, 1, 7)
labels = ['forward', 'intersection', 'left_right_t', 'left_turn',
          'right_turn', 'straight_left', 'straight_right']

# Sensor and LCD setup
lcd.init(freq=15000000);
lcd.rotation(2)
sensor.reset()
sensor.set_pixformat(sensor.RGB565)
sensor.set_framesize(sensor.QVGA)
sensor.skip_frames(time=2000)
clock = time.clock()


def process_uart():
    global should_follow, x, y, heading, tileX, tileY
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
            should_follow = True
        elif buf == "s":
            should_follow = False
        elif buf == "red":
            led_r.value(1)
        elif buf == "green":
            led_g.value(1)
        elif buf == "blue":
            led_b.value(1)
        elif buf[0] == "p":
            try:
                parts = buf.split(',')
                if len(parts) >= 4:
                    x = float(parts[1])
                    y = float(parts[2])
                    heading = float(parts[3])

                    # Optional
                    tile_x = int(parts[4]) if len(parts) > 4 else None
                    tile_y = int(parts[5]) if len(parts) > 5 else None
                else:
                    print("Invalid position format:", buf)
            except Exception as e:
                print("Error parsing position data:", e, buf)
        else:
            print("Invalid input:", buf)


def p_controller(e, kp, p0):
    return kp * e + p0


def steer(speed_1, direction_1, speed_2, direction_2):
    # Format command as t,<speed>,<direction>\n
    cmd = "t,{0},{1},{2},{3}\n".format(speed_1, direction_1, speed_2, direction_2)
    uart.write(cmd)


def send_road_tile(road_tile):
    uart.write("r{0}\n".format(road_tile))


def stop_motors():
    uart.write("x\n")


def reverse():
    uart.write("s\n")


def adjust_after_line(line, img):
    if line:
        img.draw_line(line.x1(), line.y1(), line.x2(), line.y2(), image.rgb_to_lab((0,255,0)))
        xC = img.height() / 2
        lC = (line.y1() + line.y2()) / 2
        dist = xC - lC
        try:
            k = (line.y2() - line.y1()) / (line.x2() - line.x1())
        except ZeroDivisionError:
            k = 0
        speed = p_controller(abs(k) * 100, 2, 200)
        dz, rz = 0.04, 80
        if abs(dist) <= rz:
            if abs(k) <= dz:
                uart.write("w\n")
            elif k > dz:
                steer(speed, 1, 140, 1)
            else:
                steer(140, 1, speed, 1)
        else:
            if dist > rz:
                steer(200, -1, 200, 1)
            else:
                steer(200, 1, 200, -1)
    else:
        print("No line found")
        reverse()


def model_callback(result):
    global model_result, is_model_running, shared_pred, shared_conf

    # Store the result
    state_lock.acquire()
    try:
        model_result = result
        is_model_running = False

        # Process result
        if result:
            try:
                plist = result[:]
                idx = plist.index(max(plist))
                prediction = labels[idx]
                confidence = max(plist)
                print("Prediction:", prediction, "confidence:", confidence)
                shared_pred = prediction
                shared_conf = confidence
            except Exception as e:
                print("Result processing error:", e)
    finally:
        state_lock.release()


# Model thread
def model_thread():
    global shared_pred, shared_conf, last_model_run
    while True:
        now = utime.ticks_ms()
        state_lock.acquire()
        if inference_queue:
            img = inference_queue.pop(0)
            state_lock.release()
        else:
            state_lock.release()
            img = None
        if img:
            try:
                # Process image for model
                state_lock.acquire()
                try:
                    is_model_running = True
                finally:
                    state_lock.release()

                last_model_run = now
                m = img.binary([th]).resize(128, 128).to_rgb565()
                m.pix_to_ai()

                # Use the non-blocking forward_async method
                success = kpu.forward_async(task, m, model_callback)
                if not success:
                    print("Failed to start model")
                    is_model_running = False
            except Exception as e:
                print("Model start error:", e)
                is_model_running = False
        else:
            utime.sleep_ms(10)

def get_tile(img):
    # Run road classifier on interval
    now = utime.ticks_ms()
    if utime.ticks_diff(now, last_ms) >= model_interval:
        snapshot = img.copy()
        state_lock.acquire()
        try:
            inference_queue.append(snapshot)
        finally:
            state_lock.release()
        last_ms = now

    # Model results debug
    if shared_pred:
        print(shared_pred, shared_conf)

    # Add model result to road buffer
    if shared_pred is not None:
        road_buffer.append(shared_pred)
        if len(road_buffer) > 5:
            road_buffer.pop(0)

    # Get road tile
    if len(road_buffer) == 5:
        count_dict = {}
        for item in road_buffer:
            if item in count_dict:
                count_dict[item] += 1
            else:
                count_dict[item] = 1

        most_common_pred = None
        highest_count = 0
        for item, count in count_dict.items():
            if count > highest_count:
                highest_count = count
                most_common_pred = item

        print("Road tile:", most_common_pred)
        return most_common_pred
        send_road_tile(most_common_pred)   # Sync with web


def explore_unknown(img):
    if discovered.length() <= map_x_size * map_y_size:
        get_tile(img)


        return False
    else:
        print("Explored map fully")
        return True


# Main thread
def main():
    global should_follow, shared_pred, shared_conf
    #counter = 0
    last_ms = utime.ticks_ms()
    while True:
        elapsed = clock.tick()
        img = sensor.snapshot()

        process_uart()

        #counter += 1
        #print("counter:", counter)


        # Follow line
        if should_follow:
            line = img.get_regression([th], area_threshold=100)
            adjust_after_line(line, img)
        else:
            stop_motors()

        get_tile(img)





        lcd.display(img)
        utime.sleep_ms(10)


if __name__ == "__main__":
    try:
        _thread.start_new_thread(model_thread, ())   # Start model thread
        main()
    finally:
        kpu.deinit(task)


'''

