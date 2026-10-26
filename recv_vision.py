import socket
import numpy as np
import cv2
import threading
from queue import Queue

img_queue = Queue(maxsize=1)

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
        print(f"解析图片结果:img is {img},收到字节长度={len(buf)}")
        if img is not None:
            if not img_queue.empty():
                img_queue.get()
            img_queue.put(img)
    conn.close()
    s.close()

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
    conn.close()
    s.close()

if __name__ == "__main__":
    # 图像接收放子线程，只把图片丢队列
    t1 = threading.Thread(target=recv_img, daemon=True)
    t1.start()
    # 状态接收子线程
    t2 = threading.Thread(target=recv_state, daemon=True)
    t2.start()

    # ===== imshow 和 waitKey 放在主线程！！=====
    cv2.namedWindow("Godot View", cv2.WINDOW_NORMAL)
    while True:
        if not img_queue.empty():
            frame = img_queue.get()
            cv2.imshow("Godot View", frame)
        key = cv2.waitKey(1)
        if key == 27: # ESC退出
            break
    cv2.destroyAllWindows()
