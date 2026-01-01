# app.py

from flask import Flask, request, jsonify
from database import update_vehicle, get_other_vehicles, get_vehicle_task
from path_planner import a_star, build_obstacle_map, should_yield

app = Flask(__name__)

# ----------------------------------------
# ESP32 上傳目前位置
# ----------------------------------------
@app.route("/update", methods=["POST"])
def update_position():
    data = request.get_json()
    print(f"收到車輛資料: {data}")
    vid = int(data["id"].replace("car", ""))

    x = data["x"]
    y = data["y"]

    update_vehicle(vid, x, y, data["battery"], data["status"])

    # 查詢其他車輛
    others = get_other_vehicles(vid)

    # 是否要讓路？
    if should_yield(vid, (x, y), others):
        print(f"車 {vid} 讓路（附近有小 ID 車）")
        return jsonify({"next_x": x, "next_y": y})

    # 查詢任務
    task = get_vehicle_task(vid)

    if task is None:
        return jsonify({"next_x": x, "next_y": y})

    goal = (task["target_x"], task["target_y"])

    # 路徑規劃
    obstacles = build_obstacle_map(others)
    path = a_star((x, y), goal, obstacles)

    if path is None or len(path) < 2:
        return jsonify({"next_x": x, "next_y": y})

    next_pos = path[1]
    return jsonify({"next_x": next_pos[0], "next_y": next_pos[1]})

@app.route("/")
def home():
    return "Logistics Control Center Running"

def init_database():
    from database import vehicles, tasks
    vehicles[1] = {
        "id": 1,
        "x": 0,
        "y": 0,
        "battery": 90,
        "status": "idle"
    }

    # 新增任務
    tasks[1] = {
        "vehicle_id": 1,
        "target_x": 3,
        "target_y": 3,
        "done": 0
    }


if __name__ == "__main__":
    init_database()
    app.run(debug=True, host="0.0.0.0")
