from flask import Flask, request
import requests
import time
from database import update_vehicle, get_other_vehicles, get_vehicle_task, vehicles, tasks
from path_planner import a_star, build_obstacle_map

app = Flask(__name__)

# ===== [設定區] 請填入小車真正顯示的 IP =====
CAR_CONFIG = {
    "car1": {"ip": "172.20.10.8", "vid": 1}
}

def init_database():
    # 初始化座標為 (0,0)，目標設為 (3,3)
    vehicles[1] = {"id": "car1", "x": 0, "y": 0, "battery": 90, "status": "idle"}
    tasks[1] = {"id": 1, "vehicle_id": 1, "target_x": 3, "target_y": 3, "done": 0}

@app.route("/update", methods=["GET"])
def update_position():
    try:
        car_id = request.args.get("id", "car1")
        x = float(request.args.get("x", 0))
        y = float(request.args.get("y", 0))

        vid = CAR_CONFIG.get(car_id, {}).get("vid", 1)
        # 修正 battery 缺失報錯問題
        update_vehicle(vid, x, y, battery=90, status="idle")

        task = get_vehicle_task(vid)
        if task:
            # 檢查是否到達終點
            dist_to_goal = ((x - task["target_x"])**2 + (y - task["target_y"])**2)**0.5
            if dist_to_goal < 0.1:
                print(f"--- {car_id} 任務完成！已抵達目標 ({task['target_x']}, {task['target_y']}) ---")
                return "Arrived", 200

            others = get_other_vehicles(vid)
            obstacles = build_obstacle_map(others)
            path = a_star((int(x), int(y)), (task["target_x"], task["target_y"]), obstacles)

            if path and len(path) >= 2:
                next_x, next_y = path[1]
                car_ip = CAR_CONFIG[car_id]["ip"]
                # 推送指令給 ESP32
                try:
                    requests.get(f"http://{car_ip}/move", params={"x": next_x, "y": next_y}, timeout=1)
                    print(f">>> 指令發送: {car_id} 前往 ({next_x}, {next_y})")
                except Exception as e:
                    print(f"!!! 無法連線至小車: {e}")

        return f"OK at ({x},{y})", 200
    except Exception as e:
        print(f"伺服器錯誤: {e}")
        return str(e), 500

if __name__ == "__main__":
    init_database()
    app.run(debug=False, host="0.0.0.0", port=5000)