import socket
import cv2
import numpy as np

def recv_exact(conn, n):
    data = b""
    while len(data) < n:
        chunk = conn.recv(n - len(data))
        if not chunk:
            return None
        data += chunk
    return data

s = socket.socket()
s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
s.bind(("127.0.0.1", 9000))
s.listen(1)
print("等待连接...")
conn, addr = s.accept()
print("已连接:", addr)

while True:
    size_data = recv_exact(conn, 4)
    if size_data is None:
        break
    size = int.from_bytes(size_data, "little")
    buf = recv_exact(conn, size)
    if buf is None:
        break
    
    arr = np.frombuffer(buf, np.uint8)
    img = cv2.imdecode(arr, cv2.IMREAD_COLOR)
    if img is not None:
        cv2.imshow("Godot View", img)
        print(f"收到一帧: {size} bytes")
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

conn.close()
s.close()
cv2.destroyAllWindows()
