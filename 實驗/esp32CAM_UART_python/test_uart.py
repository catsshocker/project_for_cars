import serial
import time
import cv2
import numpy as np

# 設定你的 COM 埠與波特率
ser = serial.Serial('COM4', 961200, timeout=1)

ser.dtr = False
ser.rts = False

time.sleep(2)  # 等待序列埠初始化

print("開始接收資料...")

# ser.write(b'cap\n')

def capture_image():
    """發送拍照指令並接收 JPEG"""
    ser.reset_input_buffer()
    ser.write(b'cap\n')
    
    img_data = b""

    # 等待 <START>
    while True:
        line = ser.readline()
        if b"<START>" in line:
            print(">> 接收到 <START>")
            break
        elif line.strip():
            print(line.decode(errors='ignore'))  # 顯示 ESP32 的其他訊息

    # 讀取資料直到 <END>
    while True:
        chunk = ser.read(512)
        if not chunk:
            continue
        img_data += chunk
        if b"<END>" in img_data:
            img_data = img_data.split(b"<END>")[0]
            print(">> 接收到 <END>")
            break

    print(f"接收到影像資料，共 {len(img_data)} bytes")
    return img_data


try:
    while True:
        input("按 Enter 拍照...")
        img_bytes = capture_image()
        np_arr = np.frombuffer(img_bytes, np.uint8)
        img = cv2.imdecode(np_arr, cv2.IMREAD_COLOR)

        if img is not None:
            cv2.imshow("ESP32-CAM", img)
            cv2.waitKey(1)  # 保持視窗刷新
        else:
            print("⚠️ 解碼失敗")



    
except KeyboardInterrupt:
    ser.close() 
    cv2.destroyAllWindows()

