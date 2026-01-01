# database.py - 用陣列代替資料庫

# ========== 全局資料結構 ==========
vehicles = {}  # {車ID: {"id": ID, "x": x, "y": y, "battery": 電池, "status": 狀態}}
tasks = {}     # {任務ID: {"vehicle_id": 車ID, "target_x": 目標X, "target_y": 目標Y, "done": 0/1}}

def update_vehicle(vehicle_id, x, y, battery, status):
    """更新車輛位置（若不存在則新增）"""
    if vehicle_id not in vehicles:
        vehicles[vehicle_id] = {}
    
    vehicles[vehicle_id].update({
        "id": vehicle_id,
        "x": x,
        "y": y,
        "battery": battery,
        "status": status
    })

def get_other_vehicles(vehicle_id):
    """取得除了指定車輛外的其他車輛"""
    return [v for vid, v in vehicles.items() if vid != vehicle_id]

def get_vehicle_task(vehicle_id):
    """取得某車的第一個未完成任務"""
    for task_id, task in tasks.items():
        if task.get("vehicle_id") == vehicle_id and task.get("done") == 0:
            return task
    return None
