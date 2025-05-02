import _thread
import utime
from time import sleep

counter = 0
lock = _thread.allocate_lock()


def core1_thread():
    global counter
    while True:
        sleep(2)

        with lock:
            counter += 2
        sleep(0.1)

_thread.start_new_thread(core1_thread, ())

def core0_thread():
    while True:
        with lock:
            print("counter: {}".format(counter))
        sleep(0.1)


core0_thread()
