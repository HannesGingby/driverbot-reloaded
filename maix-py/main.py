import sensor, time, lcd, image, _thread, utime
from fpioa_manager import fm
from Maix import GPIO
from machine import UART
import KPU as kpu

# setup
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

# shared state between threads
state_lock = _thread.allocate_lock()
shared_pred, shared_conf = None, 0
model_result = None
is_model_running = False
last_model_run = utime.ticks_ms()
last_ms = utime.ticks_ms()

# exploration state
mission_command = ""
map_size_x = 10
map_size_y = 5
start_tile_x = 0
start_tile_y = 0

tile_x = 0
tile_y = 0
prev_tile_x = 0
prev_tile_y = 0

snapped_headings = [0, 90, 180, 270]
heading_idx = 0

visited = set()   # map
stack = []

# exploration control
exploration_active = False
exploration_complete = False
waiting_for_tile_change = False
tile_change_timeout = 0

road_tile_buffer = []
inference_queue = []
model_interval = 1000   # ms

# line color threshold
th = (0, 100, 25, 127, -27, 127)

# road type classifier model
task = kpu.load(0x500000)
kpu.set_outputs(task, 0, 1, 1, 7)
labels = ["forward", "intersection", "left_right_t", "left_turn",
          "right_turn", "straight_left", "straight_right"]

# sensor and LCD setup
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


def neighbors(x, y):
    # yield (nx,ny,heading) for 4-connected neighbors
    yield x, y - 1, 0
    yield x + 1,y, 90
    yield x, y + 1, 180
    yield x - 1, y, 270


def start_exploration():
    """
    initialize exploration from current position
    """
    global exploration_active, exploration_complete, tile_x, tile_y, visited, stack

    print("Starting exploration from ({}, {})".format(tile_x, tile_y))
    exploration_active = True
    exploration_complete = False
    visited.clear()
    stack.clear()

    # mark starting position as visited
    visited.add((tile_x, tile_y))


def explore_step():
    """
    One step of depth-first-search:
      - look at neighbors of current tile
      - pick an unvisited one, turn there, move, mark visited
      - if none remain, backtrack (pop stack)
    """
    global tile_x, tile_y, waiting_for_tile_change, tile_change_timeout

    if waiting_for_tile_change:
        # check if we've moved to a new tile or timed out
        if (tile_x, tile_y) != (prev_tile_x, prev_tile_y):
            print("Arrived at new tile: ({}, {})".format(tile_x, tile_y))
            waiting_for_tile_change = False
            motors_stop()
            return False
        elif utime.ticks_diff(utime.ticks_ms(), tile_change_timeout) > 10000:  # 10 second timeout
            print("Tile change timeout - stopping movement")
            waiting_for_tile_change = False
            motors_stop()
            return False
        else:
            return False  # still waiting for movement to complete

    print("Current position: ({}, {}), Visited: {}".format(tile_x, tile_y, len(visited)))

    # find an unvisited neighbor
    unvisited_neighbors = []
    for nx, ny, hd in neighbors(tile_x, tile_y):
        if 0 <= nx < map_size_x and 0 <= ny < map_size_y and (nx, ny) not in visited:
            unvisited_neighbors.append((nx, ny, hd))

    if unvisited_neighbors:
        # choose first unvisited neighbor
        nx, ny, target_heading = unvisited_neighbors[0]
        print("Moving to unvisited neighbor: ({}, {})".format(nx, ny))

        # push current position to stack for backtracking
        stack.append((tile_x, tile_y))

        # turn to face that neighbor
        turn_to(target_heading)
        utime.sleep_ms(500)  # give time for turn to complete

        # start moving forward
        move_one_tile()

        # mark new position as visited (we'll update tile_x, tile_y when position updates)
        visited.add((nx, ny))

        return False   # still exploring

    # no unvisited neighbors → backtrack
    if stack:
        px, py = stack.pop()
        print("Backtracking to: ({}, {})".format(px, py))

        # compute heading to go back
        dx, dy = px - tile_x, py - tile_y
        if dx == 1 and dy == 0:     back_hd = 90   # east
        elif dx == -1 and dy == 0:  back_hd = 270  # west
        elif dx == 0 and dy == 1:   back_hd = 180  # south
        elif dx == 0 and dy == -1:  back_hd = 0    # north
        else:
            print("Invalid backtrack direction: dx={}, dy={}".format(dx, dy))
            return True  # end exploration on error

        turn_to(back_hd)
        utime.sleep_ms(500)  # give time for turn to complete

        # start moving back
        move_one_tile()

        return False

    # stack empty → fully explored
    print("Exploration complete! Visited {} tiles".format(len(visited)))
    return True


def move_one_tile(direction="forward"):
    """
    Start moving one tile in specified direction
    """
    global prev_tile_x, prev_tile_y, waiting_for_tile_change, tile_change_timeout

    print("Starting move from ({}, {})".format(tile_x, tile_y))
    prev_tile_x, prev_tile_y = tile_x, tile_y
    waiting_for_tile_change = True
    tile_change_timeout = utime.ticks_ms()

    # start movement
    '''
    if direction == "forward":
        motors_forward()
    elif direction == "reverse":
        motors_reverse()
    else:
        print("move_one_tile: invalid direction:", direction)
        waiting_for_tile_change = False
    '''

def process_uart():
    global tile_x, tile_y, uart_buffer, uart_busy, last_send_time, mission_command, start_tile_x, start_tile_y, map_size_x, map_size_y

    # don't read immediately after sending to avoid reading our own transmission
    if uart_busy and utime.ticks_diff(utime.ticks_ms(), last_send_time) < 50:
        return

    uart_busy = False

    if uart.any():
        try:
            # read available data and add to buffer
            new_data = uart.read()
            if new_data:
                # handle both string and bytes
                if isinstance(new_data, bytes):
                    uart_buffer += new_data.decode('utf-8')
                else:
                    uart_buffer += str(new_data)
            else:
                return None
        except Exception as e:
            print("process_uart: UART Read Error:", str(e))
            return None

        # process complete lines (commands ending with \n)
        while '\n' in uart_buffer:
            # extract one complete line
            line_end = uart_buffer.find('\n')
            buf = uart_buffer[:line_end].strip()
            uart_buffer = uart_buffer[line_end + 1:]

            # skip empty lines
            if not buf:
                continue

            print("Processing command:", repr(buf))

            # process the complete command
            if buf.startswith("p,"):
                try:
                    # clean up the command - remove any extra characters
                    clean_buf = buf.strip()

                    # split by comma
                    parts = clean_buf.split(',')

                    # ensure we have at least 3 parts: p, x, y
                    if len(parts) >= 3:
                        # validate that parts[1] and parts[2] are not empty and are numeric
                        x_str = parts[1].strip()
                        y_str = parts[2].strip()

                        if x_str and y_str and x_str.isdigit() and y_str.isdigit():
                            # update globals
                            tile_x = int(x_str)
                            tile_y = int(y_str)

                            print("NEW TILE: {}, {}".format(tile_x, tile_y))
                        else:
                            print("Invalid position values: x='{}', y='{}'".format(x_str, y_str))
                    else:
                        print("process_uart: Insufficient position parts: {}".format(len(parts)))
                except (ValueError, IndexError) as e:
                    print("process_uart: Error parsing position data: {} - '{}'".format(str(e), buf))
            elif buf.startswith("c"):
                try:
                    # clean up the command - remove any extra characters
                    clean_buf = buf.strip()

                    # split by comma
                    parts = clean_buf.split(',')

                    # ensure we have at least 6 parts: c, command, start_pos_x, start_pos_y, map_size_x, map_size_y
                    if len(parts) >= 6:
                        command_str = parts[1].strip()
                        start_pos_x_str = parts[2].strip()
                        start_pos_y_str = parts[3].strip()
                        map_size_x_str = parts[4].strip()
                        map_size_y_str = parts[5].strip()

                        if command_str and start_pos_x_str and start_pos_y_str and map_size_x_str and map_size_y_str and start_pos_x_str.isdigit() and start_pos_y_str.isdigit() and map_size_x_str.isdigit() and map_size_y_str.isdigit():
                            # update globals
                            mission_command = command_str
                            start_tile_x = int(start_pos_x_str)
                            start_tile_y = int(start_pos_y_str)
                            map_size_x = int(map_size_x_str)
                            map_size_y = int(map_size_y_str)

                            # set current position to start position
                            tile_x = start_tile_x
                            tile_y = start_tile_y

                            print("Got mission start: {}, {}, {}, {}, {}".format(mission_command, start_tile_x, start_tile_y, map_size_x, map_size_y))

                            if command_str.lower() == "start":
                                start_exploration()
                        else:
                            print("process_uart: Invalid task command values")
                    else:
                        print("process_uart: Insufficient task command parts: {}".format(len(parts)))
                except (ValueError, IndexError) as e:
                    print("process_uart: Error parsing task command data: {} - '{}'".format(str(e), buf))
            #elif buf == "r":
            #    led_r.value(1)
            #    print("Red LED on")
            #elif buf == "g":
            #    led_g.value(1)
            #    print("Green LED on")
            #elif buf == "b":
            #    led_b.value(1)
            #    print("Blue LED on")
            else:
                print("process_uart: Unknown command: '{}'".format(buf))


def p_controller(e, kp, p0):
    return kp * e + p0


def turn_to(target_heading):
    global heading_idx
    try:
        target_idx = snapped_headings.index(target_heading)
    except ValueError:
        print("turn_to: Invalid heading:", target_heading)
        return
    heading_idx = target_idx
    motors_send_turn(target_heading)


def motors_send_turn(heading):
    global uart_busy, last_send_time

    # format as U,<heading>\n
    cmd = "U,{}\n".format(heading)
    try:
        if uart.any():
            uart.read()

        uart.write(cmd)
        uart_busy = True
        last_send_time = utime.ticks_ms()
        # print("Turning to", heading)
    except Exception as e:
        print("motors_send_turn: UART write error:", str(e))


def motors_steer(speed_1, direction_1, speed_2, direction_2):
    global uart_busy, last_send_time

    # ensure speeds are within valid range
    speed_1 = max(0, min(255, int(speed_1)))
    speed_2 = max(0, min(255, int(speed_2)))

    # ensure directions are 0 or 1
    direction_1 = 1 if direction_1 else 0
    direction_2 = 1 if direction_2 else 0

    # format command as t,<speed1>,<direction1>,<speed2>,<direction2>\n
    cmd = "t,{},{},{},{}\n".format(speed_1, direction_1, speed_2, direction_2)
    try:
        # clear any pending received data before sending
        if uart.any():
            uart.read()   # flush receive buffer

        uart.write(cmd)
        uart_busy = True
        last_send_time = utime.ticks_ms()
        #print("Sending command:", cmd.strip())
    except Exception as e:
        print("motors_steer: UART write error:", str(e))


def motors_stop():
    global uart_busy, last_send_time
    try:
        if uart.any():
            uart.read()   # flush receive buffer

        uart.write("x\n")
        uart_busy = True
        last_send_time = utime.ticks_ms()
        #print("Stop")
    except Exception as e:
        print("motors_stop: UART write error:", str(e))


def motors_forward():
    global uart_busy, last_send_time
    try:
        if uart.any():
            uart.read()   # flush receive buffer

        uart.write("w\n")
        uart_busy = True
        last_send_time = utime.ticks_ms()
        #print("Forward")
    except Exception as e:
        print("motors_forward: UART write error:", str(e))


def motors_reverse():
    global uart_busy, last_send_time
    try:
        if uart.any():
            uart.read()   # flush receive buffer

        uart.write("s\n")
        uart_busy = True
        last_send_time = utime.ticks_ms()
        #print("Reverse")
    except Exception as e:
        print("motors_reverse: UART write error:", str(e))


def send_road_tile(road_tile):
    global uart_busy, last_send_time
    try:
        if uart.any():
            uart.read()   # flush receive buffer

        cmd = "r,{}\n".format(road_tile)
        uart.write(cmd)
        uart_busy = True
        last_send_time = utime.ticks_ms()
        print("Sent road tile:", road_tile)
    except Exception as e:
        print("send_road_tile: UART write error:", str(e))


def adjust_after_line(line, img, forward=False):
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
            if abs(k) <= dz and forward:
                motors_forward()
                #print("Forward")
            if k > dz:
                motors_steer(150, 1, speed, 1)
                #print("Left")
            else:
                motors_steer(speed, 1, 150, 1)
                #print("Right")
        else:
            if dist > rz:
                motors_steer(200, 1, 200, 0)
                print("Repositiong left")
            else:
                motors_steer(200, 0, 200, 1)
                print("Repositiong right")
    else:
        #print("No line found")
        motors_stop()


def model_callback(result):
    global model_result, is_model_running, shared_pred, shared_conf

    # store the result
    state_lock.acquire()
    try:
        model_result = result
        is_model_running = False

        # process result
        if result:
            try:
                plist = result[:]
                idx = plist.index(max(plist))
                prediction = labels[idx]
                confidence = max(plist)
                #print("Prediction:", prediction, "confidence:", confidence)
                shared_pred = prediction
                shared_conf = confidence
            except Exception as e:
                print("model_callback: Result processing error:", e)
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
                # process image for model
                state_lock.acquire()
                try:
                    is_model_running = True
                finally:
                    state_lock.release()

                last_model_run = now
                m = img.binary([th]).resize(160, 120).to_rgb565()
                m.pix_to_ai()

                # use the non-blocking forward_async method
                success = kpu.forward_async(task, m, model_callback)
                if not success:
                    print("model_thread: Failed to start model")
                    is_model_running = False
            except Exception as e:
                print("model_thread: Model start error:", e)
                is_model_running = False
        else:
            utime.sleep_ms(10)


def get_road_tile(img):
    global last_ms, shared_pred, shared_conf, is_model_running

    # run road classifier on interval
    now = utime.ticks_ms()
    # print(is_model_running)
    if utime.ticks_diff(now, last_ms) >= model_interval:
        state_lock.acquire()
        try:
            inference_queue.append(img)
        finally:
            state_lock.release()
        last_ms = now

    # model results debug
    if shared_pred:
        print(shared_pred, shared_conf)

    # add model result to road buffer
    if shared_pred is not None:
        road_tile_buffer.append(shared_pred)
        if len(road_tile_buffer) > 5:
            road_tile_buffer.pop(0)

        # clear the shared prediction so we don't process it again
        shared_pred = None

    # get road tile - only process when we have exactly 5 items
    if len(road_tile_buffer) == 5:
        count_dict = {}
        for item in road_tile_buffer:
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

        # clear the buffer after processing to avoid reprocessing
        road_tile_buffer.clear()

        return most_common_pred

    return None


def main():
    global shared_pred, shared_conf, exploration_active, exploration_complete
    while True:
        process_uart()

        img = sensor.snapshot()

        if exploration_active and not exploration_complete:
            # follow line
            line = img.get_regression([th], area_threshold=100)
            adjust_after_line(line, img)

            exploration_complete = explore_step()
            if exploration_complete:
                exploration_active = False

        get_road_tile(img)

        lcd.display(img)
        utime.sleep_ms(10)


if __name__ == "__main__":
    try:
        _thread.start_new_thread(model_thread, ())   # start model thread
        main()
    finally:
        kpu.deinit(task)
