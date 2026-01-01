# path_planner.py

from config import MAP_WIDTH, MAP_HEIGHT, STATIC_OBSTACLES

# A* 用
import heapq

def heuristic(a, b):
    return abs(a[0] - b[0]) + abs(a[1] - b[1])

# 產生地圖障礙（包含其他車）
def build_obstacle_map(other_cars):
    obstacles = set(STATIC_OBSTACLES)
    for c in other_cars:
        obstacles.add((c['x'], c['y']))
    return obstacles

# A* 搜尋
def a_star(start, goal, obstacles):
    if start == goal:
        return [start]

    open_list = []
    heapq.heappush(open_list, (0, start))
    came_from = {}
    g = {start: 0}

    while open_list:
        _, current = heapq.heappop(open_list)

        if current == goal:
            # 回溯路徑
            path = []
            while current in came_from:
                path.append(current)
                current = came_from[current]
            path.reverse()
            return path

        cx, cy = current
        for nx, ny in [(cx+1,cy), (cx-1,cy), (cx,cy+1), (cx,cy-1)]:
            if not (0 <= nx < MAP_WIDTH and 0 <= ny < MAP_HEIGHT):
                continue
            if (nx, ny) in obstacles:
                continue

            ng = g[current] + 1
            if (nx, ny) not in g or ng < g[(nx, ny)]:
                g[(nx, ny)] = ng
                f = ng + heuristic((nx, ny), goal)
                heapq.heappush(open_list, (f, (nx, ny)))
                came_from[(nx, ny)] = current

    return None  # 找不到路

# 判定是否應讓路
def should_yield(my_id, my_pos, other_cars):
    mx, my = my_pos

    for c in other_cars:
        if c["id"] < my_id:  # ID 小 → 我必須讓路
            if abs(mx - c["x"]) <= 1 and abs(my - c["y"]) <= 1:
                return True
    return False
