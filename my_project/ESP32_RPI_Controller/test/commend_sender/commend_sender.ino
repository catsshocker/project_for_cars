void send_commend(uint8_t command, uint8_t *data, int datalen){
    const uint8_t start = 0xAA;
    const uint8_t end = 0x55;
    uint8_t pack[256];
    int pack_pos = 0;

    pack[pack_pos++] = start;
    pack[pack_pos++] = datalen + 1; // length of data + 1(command)
    pack[pack_pos++] = command;
    for(int i = 0; i < datalen; i++){
        pack[pack_pos++] = data[i];
    }
    pack[pack_pos++] = 0x00;
    pack[pack_pos++] = 0x00;
    pack[pack_pos++] = end;

    Serial.write(pack, pack_pos);
}

void setup(){
    Serial.begin(115200);
    send_commend(0x11, new uint8_t[1]{0x01}, 1);
    send_commend(0x12, new uint8_t[1]{0x02}, 1);
    send_commend(0x13, new uint8_t[1]{0x03}, 1);
    send_commend(0x14, new uint8_t[1]{0x04}, 1);
    send_commend(0x15, new uint8_t[4]{0x05,0x06,0x07,0x08}, 4);
    send_commend(0x19, new uint8_t[3]{0x09,0x0A,0x0B}, 3);
}

void loop(){

}