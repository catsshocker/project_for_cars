import matplotlib.pyplot as plt
import numpy as np


class AccelerationProfiler:
    """
    加減速控制物件 - 用於規劃運動軌跡
    根據位置、速度和加速度限制計算每個時刻的速度輸出
    """
    
    def __init__(self, target_position, max_speed, acceleration):
        """
        初始化加減速控制器
        
        Args:
            target_position: 目標位置
            max_speed: 最大速度
            acceleration: 加速度大小
        """
        self.target_position = target_position
        self.max_speed = max_speed
        self.acceleration = acceleration
        
        # 狀態變數
        self.current_position = 0.0
        self.current_speed = 0.0
        self.time_elapsed = 0.0
    
    def update(self, dt):
        """
        執行一次更新，計算應執行的速度並更新位置
        
        Args:
            dt: 時間步長(秒)
            
        Returns:
            float: 當前應執行的速度
        """
        # 計算剩餘距離
        remaining_distance = self.target_position - self.current_position
        
        # 計算以當前速度減速到 0 所需的距離
        deceleration_distance = self.current_speed**2 / self.acceleration / 2
        
        # 判斷是否應該減速
        if deceleration_distance >= remaining_distance:
            # 開始減速
            self.current_speed = max(self.current_speed - self.acceleration * dt, 0)
        else:
            # 繼續加速到最大速度
            self.current_speed = min(self.current_speed + self.acceleration * dt, self.max_speed)
        
        # 更新位置
        self.current_position += self.current_speed * dt
        self.time_elapsed += dt
        
        return self.current_speed
    
    def get_state(self):
        """
        取得當前狀態
        
        Returns:
            dict: 包含位置、速度、時間的字典
        """
        return {
            'position': self.current_position,
            'speed': self.current_speed,
            'time': self.time_elapsed,
            'remaining_distance': self.target_position - self.current_position
        }
    
    def is_finished(self):
        """
        檢查是否已到達目標
        
        Returns:
            bool: True 如果位置 >= 目標位置
        """
        return self.current_position >= self.target_position
    
    def reset(self):
        """重置所有狀態"""
        self.current_position = 0.0
        self.current_speed = 0.0
        self.time_elapsed = 0.0


# Test the AccelerationProfiler
if __name__ == "__main__":
    # 建立加減速控制物件
    profiler = AccelerationProfiler(
        target_position=1000,  # 目標位置
        max_speed=200,         # 最大速度
        acceleration=50        # 加速度
    )
    
    dt = 0.1
    time_values = []
    position_values = []
    speed_values = []
    
    # 模擬執行直到到達目標
    while not profiler.is_finished():
        speed = profiler.update(dt)
        state = profiler.get_state()
        
        time_values.append(state['time'])
        position_values.append(state['position'])
        speed_values.append(state['speed'])
    
    # Plot results
    plt.figure(figsize=(12, 6))
    
    plt.subplot(2, 1, 1)
    plt.plot(time_values, position_values, label='Current Position', linewidth=2, color='b')
    plt.axhline(y=profiler.target_position, color='r', linestyle='--', label='Target Position', linewidth=2)
    plt.title('Position Response (加減速控制)')
    plt.xlabel('Time (s)')
    plt.ylabel('Position')
    plt.legend()
    plt.grid(True)

    plt.subplot(2, 1, 2)
    plt.plot(time_values, speed_values, label='Speed Output', color='g', linewidth=2)
    plt.axhline(y=profiler.max_speed, color='orange', linestyle='--', label='Max Speed', linewidth=2)
    plt.title('Speed Output (加減速)')
    plt.xlabel('Time (s)')
    plt.ylabel('Speed')
    plt.legend()
    plt.grid(True)

    plt.tight_layout()
    plt.show()
    
    # 列印最終狀態
    print(f"最終位置: {profiler.get_state()['position']:.2f}")
    print(f"目標位置: {profiler.target_position}")
    print(f"總耗時: {profiler.get_state()['time']:.2f} 秒")

    plt.tight_layout()
    plt.show()