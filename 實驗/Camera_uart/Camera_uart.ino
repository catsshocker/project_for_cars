#include "esp_camera.h"

#define CAMERA_MODEL_AI_THINKER
#include "camera_pins.h"

camera_config_t config;
sensor_t *s;

void setup() {
    Serial.begin(961200);
    Serial.setDebugOutput(false);

    pinMode(4,OUTPUT);
    
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pin_d0 = Y2_GPIO_NUM;
    config.pin_d1 = Y3_GPIO_NUM;
    config.pin_d2 = Y4_GPIO_NUM;
    config.pin_d3 = Y5_GPIO_NUM;
    config.pin_d4 = Y6_GPIO_NUM;
    config.pin_d5 = Y7_GPIO_NUM;
    config.pin_d6 = Y8_GPIO_NUM;
    config.pin_d7 = Y9_GPIO_NUM;
    config.pin_xclk = XCLK_GPIO_NUM;
    config.pin_pclk = PCLK_GPIO_NUM;
    config.pin_vsync = VSYNC_GPIO_NUM;
    config.pin_href = HREF_GPIO_NUM;
    config.pin_sscb_sda = SIOD_GPIO_NUM;
    config.pin_sscb_scl = SIOC_GPIO_NUM;
    config.pin_pwdn = PWDN_GPIO_NUM;
    config.pin_reset = RESET_GPIO_NUM;
    config.xclk_freq_hz = 20000000;
    config.pixel_format = PIXFORMAT_JPEG;
    config.frame_size = FRAMESIZE_VGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;

    if (esp_camera_init(&config) != ESP_OK) {  //初始化 啟動相機
        Serial.println("Camera init failed!");
        while (true)
            ;
    }

    s = esp_camera_sensor_get();  //相機設定參數
    s->set_framesize(s, FRAMESIZE_VGA); // 解析度：VGA (640x480)
    s->set_quality(s, 10);              // 畫質：1(最高)~63(最低)
    s->set_brightness(s, 1);            // 亮度：-2~2
    s->set_contrast(s, 1);              // 對比r：-2~2
    s->set_saturation(s, 0);            // 飽和度：-2~2

    Serial.println("ESP32-CAM ready. Send 'cap' to capture.");
}

void loop() {
    if (Serial.available()) {
        String cmd = Serial.readStringUntil('\n');
        cmd.trim();
        if (cmd == "cap") {
            digitalWrite(4,1);
            delay(200);

            // esp_camera_fb_get();
            // delay(1);
            camera_fb_t *fb = esp_camera_fb_get();
            digitalWrite(4,0);
            
            if (!fb) {
                Serial.println("Capture failed");
                return;
            }
            delay(10);

            // 傳送影像資料 (包上標記)
            Serial.println("<START>");
            Serial.println((int)(fb->len));
            Serial.write(fb->buf, fb->len);
            Serial.println("<END>");

            esp_camera_fb_return(fb);
        }
    }
}
