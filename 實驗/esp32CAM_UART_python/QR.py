import serial
import time
import cv2
import numpy as np
import math

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
            btlong = int(ser.readline().decode(encoding='ascii',errors='ignore'))
            print(">> 接收到 <START>,長度:",btlong)
            break
        elif line.strip():
            print(line.decode(encoding='ascii',errors='ignore'))  # 顯示 ESP32 的其他訊息

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


def boxSize(arr):
    # global data
    box_roll = np.rollaxis(arr,1,0)   # 轉置矩陣，把 x 放在同一欄，y 放在同一欄
    xmax = int(np.amax(box_roll[0]))  # 取出 x 最大值
    xmin = int(np.amin(box_roll[0]))  # 取出 x 最小值
    ymax = int(np.amax(box_roll[1]))  # 取出 y 最大值
    ymin = int(np.amin(box_roll[1]))  # 取出 y 最小值
    return (xmin,ymin,xmax,ymax)

try:
    while True:
        input("按 Enter 拍照...")
        img_bytes = capture_image()
        np_arr = np.frombuffer(img_bytes, np.uint8)
        img = cv2.imdecode(np_arr, cv2.IMREAD_COLOR)

        qrcode = cv2.QRCodeDetector()
        data, bbox, rectified = qrcode.detectAndDecode(img)

        if img is not None:
            if bbox is not None:
                print(f"QRcode Data>>{data}")
            # print(rectified)
            # box = boxSize(bbox[0])
            # cv2.rectangle(img,(box[0],box[1]),(box[2],box[3]),(0,0,255),3)  # 畫矩形
                bbox = bbox[0].astype(int)

                (x1,y1),(x2,y2) = bbox[0],bbox[1]
                dx = x2 - x1
                dy = y2 - y1
                angle = math.degrees(math.atan2(dy,dx))
                print(f"位置:( {x1}, {y1} ),({x2}, {y2}), 角度: {angle:.2f} 度")

            # 繪製框線
                for i in range(len(bbox)):
                    cv2.line(img, tuple(bbox[i]), tuple(bbox[(i + 1) % len(bbox)]), (0, 255, 0), 2)
            else :
                print("⚠️ 未偵測到 QRcode")
                
            cv2.imshow("ESP32-CAM", img)
            cv2.waitKey(1)  # 保持視窗刷新
        else:
            print("⚠️ 解碼失敗")



    
except KeyboardInterrupt:
    ser.close() 
    cv2.destroyAllWindows()

