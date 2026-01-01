# ESP32 to Raspberry controller Data!!!
## 一，簡介
        ESP32作為樹梅派的從控制器 樹梅派透過UART與ESP32通訊，ESP32被當作樹梅派的擴沖腳位
    提供一組外接的UART、一組CAN、16個角位、硬體計數器(Encoder,PCNT)  
    其中的外接腳位共可以輸出16組PWM，每一個角位也都可以被作為編碼器的訊號輸入

## 二，各模組功能及實現簡介
### 1.GPIO
GPIO的功能直接使用arduino IDE的 `digitalWrite`  
功能基本上跟arduino原生的IO控制一樣  

>[!IMPORTANT]
>由於樹梅派跟ESP32是通訊的問題  
> 不能使用像是 `delayMicroseconds` 這種的控制

### 2.CAN
使用ESP32 idf `TWAI` 能，這個模組就是ESP32的CAN收發器

### 3.PWM
使用LEDC函式庫實作，只有4個Timer，最好所有PWM輸出的頻率都一樣

### 4.外設UART
可以使用原生`Serial`製作，也可以使用ESP32 IDF<UART.h>製作 (尚未製作)

### 5.Encoder
使用PCNT(硬體計數器)功能實作，提供正交編碼器的功能

### 6.通訊UART
負責處理所有來自樹梅派的指令接收與發送，我們設定波特率為`921600`，其中使用了DMA製作非同步接收的功能以增加效率  
使用`driver/UART.h`的DMA


## 三，架構規劃

由多個任務組成，每個任務有不同的功能  
- 指令接收與分析
- IO相關指令執行
- CAN收發器
- 外設UART
  
使用`freeRTOS`的多線程隊列，分別實現個個功能。
|名稱        | 功能      |  
|----------|-----------|  
|指令接收器 | 接收來自樹莓派的指令|
|指令分析器 | 分析指令，並把指令的內容分配給不同的單元執行|
|IO單元    | 處理所有的IO、PWM、Encoder的控制與讀取
|CAN收發單元| 須由指令初始化，進行CAN端口的資料收發
|外設UART收發單元 |須由指令初始化，進行對外UART的資料收發|
|UART回傳單元| 把一些資料從ESP32回傳給樹梅派，包括指令的回傳資料，還有錯誤訊息，及指令丟包等，及指令丟包等  



## 四，封包及指令 
### 封包格式
```
[start][len][cmd][data][data][CRC][CRC][end]
```

- `start` : 起始位，故定為0xAA
- `len` : 資料長度 ( 包含cmd和data )
- `cmd` : 指令ID
- `data` : 指令對應的資料或子命令，長度可變
- `CRC` : 循環冗餘校驗，占用兩位元組(測試階段未啟用)
- `end` : 結束位，固定為0x55


### 指令功能  
- IO
    - pinMode
        - set input
        - set output
        - set 3pin ultrasonic mode
    - PwmSet
    - digitalIO => 數位IO控制
        - read
        - write
        - ultrasonlc read
    - AnalogRead
    - encoder
        - setup
        - readCount
        - reset
- 通訊
    - uart
        - uart setup
        - uart write
        - uart read
        - uart disconnect =>IO與通訊模組斷開連接
    - can
        - can setup
        - can write
        - can read
        - can disconnect => IO與通訊模組斷開連接
- 擴充版控制
    - info => 取得擴充版資料
    - restart => ESP32軟體重啟
    - disable => 忽略所有指令
    - enable => 重新開始接收>

### 指令規劃
`指令ID`前四位為功能ID，後四位為子編號

```
 0123 45678
|0000|0000|
 指令 子編號
```

-IO
function|ID|parameter
---|---|---
pinmode set output | `0x11` | `pin` : uint8_t
pinmode set input  | `0x12` | `pin` : uint8_t
pinmode set 3pin ultrasion| `0x13` | `pin` : uint8_t
pwm setup| `0x14` | `pin` : uint8_t
pwm write dutyCycle| `0x15` | `dutycycle` : float (0~100)
digital read   | `0x16` | `pin` : uint8_t
digital write  | `0x17` | `pin` : uint8_t
analog read    | `0x18` | `pin` : uint8_t
setup encoder  | `0x19` | `pin`,`pin`,`unit` : uint8_t
read encoder count | `0x1A` | `unit` : uint8_t
reset encoder  | `0x1B` | `unit` : uint8_t
3pin ultrasion read | `0x1C` | `pin` : uint8_t