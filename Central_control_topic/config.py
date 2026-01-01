# config.py

MYSQL_CONFIG = {
    "host": "localhost",
    "user": "root",
    "password": "12345678",
    "database": "logistics"
}

# 你的倉庫地圖大小
MAP_WIDTH = 20
MAP_HEIGHT = 20

# 固定障礙（如果沒有可以留空）
STATIC_OBSTACLES = [
    (5, 5), (5, 6), (5, 7)
]
