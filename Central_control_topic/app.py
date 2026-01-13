from flask import Flask, request
from database import update_vehicle, get_other_vehicles, get_vehicle_task, vehicles, tasks
from path_planner import a_star, build_obstacle_map, should_yield

app = Flask(__name__)

@app.route("/update", methods=["GET"])
def update_position():
    # 取得 GET 參數
    car_id = request.args.get("id", "car1")
    x = int(request.args.get("x", 0))
    y = int(request.args.get("y", 0))

    # 轉成車輛編號
    try:
        vid = int(car_id.replace("car", ""))
    except:
        vid = 1

    # 更新車輛位置
    update_vehicle(vid, x, y, battery=90, status="idle")

    # 取得其他車輛資訊
    others = get_other_vehicles(vid)

    # 判斷是否要讓路
    if should_yield(vid, (x, y), others):
        next_x, next_y = x, y
    else:
        # 查詢任務
        task = get_vehicle_task(vid)
        if task is None:
            next_x, next_y = x, y
        else:
            goal = (task["target_x"], task["target_y"])
            obstacles = build_obstacle_map(others)
            path = a_star((x, y), goal, obstacles)
            if path is None or len(path) < 2:
                next_x, next_y = x, y
            else:
                next_x, next_y = path[1]

    # 回傳格式：CSV（確保一行、無多餘換行）
    response_text = f"{next_x},{next_y},idle"
    return response_text, 200, {"Content-Type": "text/plain; charset=utf-8"}

def init_database():
    vehicles[1] = {"id": "car1", "x": 0, "y": 0, "battery": 90, "status": "idle"}
    tasks[1] = {"vehicle_id": 1, "target_x": 3, "target_y": 3, "done": 0}

if __name__ == "__main__":
    init_database()
    app.run(debug=True, host="0.0.0.0", port=5000)
