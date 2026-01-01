#include "myQueue.h"

Queue<int> queue(10);

void setup(){
    Serial.begin(9600);
    queue.push(4);
    queue.push(5);
    Serial.println(queue.pop());
    Serial.println(queue.pop());
}

void loop(){

}