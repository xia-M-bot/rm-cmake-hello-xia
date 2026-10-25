import socket
import threading
import cv2
import numpy as np

def recv_img():
    s = socket.socket()
    s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    s.bind(("127.0.0.1", 9000))
    s.listen(1)
    print("【图像】端口9000等待连接")
    conn, addr = s.accept()
    print("【图像】已连接", addr)
    while True:
        size_data = conn.recv(4)
        if not size_data:
            break
        size = int.from_bytes(size_data, byteorder="little")
        buf = b""
        while len(buf) < size:
            buf += conn.recv(size - len(buf))
        arr = np.frombuffer(buf, np.uint8)
        img = cv2.imdecode(arr, 1)
        cv2.imshow("Godot View", img)
        cv2.waitKey(1)

def recv_state():
    s = socket.socket()
    s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    s.bind(("127.0.0.1", 9001))
    s.listen(1)
    print("【状态】端口9001等待连接")
    conn, addr = s.accept()
    print("【状态】已连接", addr)
    while True:
        data = conn.recv(1024)
        if not data:
            break
        print("收到状态：", data.decode("utf-8"))

if __name__ == "__main__":
    t1 = threading.Thread(target=recv_img, daemon=True)
    t2 = threading.Thread(target=recv_state, daemon=True)
    t1.start()
    t2.start()
    t1.join()
    t2.join()
