import socket

from concurrent.futures import ThreadPoolExecutor
import numpy as np
import QLearning_sample
import models

HOST = '127.0.0.1'  # Standard loopback interface address (localhost)
PORT = 1372  # Port to listen on (non-privileged ports are > 1023)
num_of_threads = 11


def init_models():
    models.load_model('abalone', './abalone_model')




def handler(arg):
    conn, addr = arg
    with conn:
        # print('Connected by', addr)
        while True:
            data = conn.recv(1024)
            if not data:
                break
            data = data.decode()
            print(data)
            data = data.split(" ")

            function = getattr(globals().get(data[0]), data[1])

            result = function(data[2:])

            #print(return_message)
            # print(result)
            conn.sendall(result)
        # print('Disconnected by', addr)


executor = ThreadPoolExecutor(num_of_threads)
init_models()

print("server starting...")
with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.bind((HOST, PORT))
    s.listen()
    while True:
        conn, addr = s.accept()
        # print("new conn!")
        arg = (conn, addr)
        future = executor.submit(handler, (arg))
        print(future.result(timeout=10))

        # threading.Thread(target=handler, args=(conn, addr)).start()
