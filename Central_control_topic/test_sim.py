# test_sim.py
import requests
import time

# 模擬從 (0,0) 移動到 (3,3)
positions = [(0,0), (1,0), (1,1), (2,1), (2,2), (3,2), (3,3)]

for x, y in positions:
    data = {
        "id": "car1",
        "x": x,
        "y": y,
        "battery": 90,
        "status": "moving" if (x, y) != (3, 3) else "idle"
    }
    
    response = requests.post("http://localhost:5000/update", json=data)
    result = response.json()
    
    print(f"📍 位置 ({x}, {y}) → 下一步 ({result['next_x']}, {result['next_y']})")
    time.sleep(1)