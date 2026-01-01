#include "Hbridge.h"

MotorController mc(26); //26號腳位控制馬達啟動與否

void setup(){
    mc.motorA.setPin(GPIO_NUM_25, GPIO_NUM_33, GPIO_NUM_32);
    mc.motorB.setPin(GPIO_NUM_27, GPIO_NUM_14, GPIO_NUM_12);
    mc.motorA.setStopMode(BRAKE); //設定停止模式為煞車
    mc.motorB.setStopMode(COAST);
    mc.motorB.reverse = true;
    delay(300);//上電緩衝
    mc.begin();
    mc.enable();
    for(int i=0;i<255;i++){
        mc.motorA.setSpeed(-i); //設定馬達速度，範圍-255~255，負值為反轉
        mc.motorB.setSpeed(i);
        delay(20);
    }
    delay(50);
    for(int i=0;i<512;i++){
        mc.motorA.setSpeed(-255+i); //設定馬達速度，範圍-255~255，負值為反轉
        mc.motorB.setSpeed(255-i);
        delay(20);
    }
    delay(50);
    for(int i=0;i<255;i++){
        mc.motorA.setSpeed(255-i); //設定馬達速度，範圍-255~255，負值為反轉
        mc.motorB.setSpeed(-255+i);
        delay(20);
    }
    mc.motorA.setSpeed(0); //停止馬達
    mc.motorB.stop();
    mc.disable();
}
void loop(){

}