import serial
import cv2
import numpy as np

ser = serial.Serial('COM4', 115200, timeout=5)

ser.dtr = False
ser.rts = False

def capture_image():
    ser.reset_input_buffer()
    ser.write(b'cap\n')

    # 等待 <START>
    while True:
        line = ser.readline().decode(errors='ignore').strip()
        if line == "<START>":
            break

    # 接收 JPEG
    img_data = b""
    while True:
        if ser.in_waiting:
            chunk = ser.read(ser.in_waiting)
            img_data += chunk
            end_index = img_data.find(b"<END>")
            if end_index != -1:
                img_data = img_data[:end_index]
                break

    # 移除可能的換行
    img_data = img_data.lstrip(b'\r\n')
    return img_data

while True:
    print("拍照中...")
    img_bytes = capture_image()
    print(f"收到 {len(img_bytes)} bytes")

    np_arr = np.frombuffer(img_bytes, np.uint8)
    img = cv2.imdecode(np_arr, cv2.IMREAD_COLOR)

    if img is not None:
        cv2.imshow("ESP32-CAM", img)
        cv2.waitKey(0)     
    else:
        print("⚠️ 解碼失敗")

    if cv2.waitKey(10) & 0xFF == ord('q'):
        break

ser.close()
cv2.destroyAllWindows()
