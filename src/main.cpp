#include <Arduino.h>
#include <U8g2lib.h>
#include <RadioLib.h>
#include <string.h>

SX1262 radio = new Module(8, 3, 5, 4);
U8G2_SH1106_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, /* clock=*/ 1, /* data=*/ 0, /* reset=*/ U8X8_PIN_NONE);
unsigned char page=0;
int readValue=0;
unsigned char lastbuttonpress=8;
unsigned char type=0;
bool retranslatormode = false;
bool recievemode = true;
const char* menuelements[]={"reciever mode","retranslator mode","transmit"};
struct __attribute__((__packed__)) packagesingle{
  uint8_t kind;
  uint8_t networkid;
  uint32_t messageid;
  uint32_t recieveid;
  uint8_t hops;
  char message[]; //I don't know how long message will be
};

void sendsingle(uint32_t targetId, const char* text);

void setup() {
  Serial.begin(115200);
  u8g2.begin();
  SPI.begin(10, 6, 7, 8);
  radio.begin(868.0);
}

void loop() {
  readValue=analogRead(2);
  if (readValue > 0 && readValue < 250) {
    if (lastbuttonpress == 8 && page > 0 && type == 0) {
      page -= 1;
    }
    lastbuttonpress = 0;
  } 
  else if (readValue > 650 && readValue < 870) {
    lastbuttonpress = 1;
    if (page ==2 && type==0){
      sendsingle(2, "Hello world, This is Summer. Test of new lora chip. This UTF-8 String is compacted as much as possible");
    }
    if (page==0 && type==0){
      recievemode=!recievemode;
    }
    if (page==1 && type==0){
      retranslatormode=!retranslatormode;
    }
  } 
  else if (readValue > 350 && readValue < 600) {
    if (lastbuttonpress == 8 && page < 2 && type == 0) {
      page += 1;
    }
    lastbuttonpress = 2;
  } 
  else {
    lastbuttonpress = 8; 
  }

  
//rendering stage
 u8g2.clearBuffer();
 u8g2.setFont(u8g2_font_ncenB08_tr);
  if (type==0){
    for(int i=0;i<3;i++){
      u8g2.setCursor(10, (20*(i+1))-10);
      if (i==page){
        u8g2.setDrawColor(1);
        u8g2.drawBox(0, (i * 20), 108, 20);
        u8g2.setDrawColor(0);
      }
      else{
        u8g2.setDrawColor(1);
      }
    u8g2.print(menuelements[i]);
    }
  }
 u8g2.setDrawColor(1);
 u8g2.drawDisc(116, 10, 9);
 u8g2.drawDisc(116, 30, 9);
 u8g2.setDrawColor(0);
 if (recievemode==false){
  u8g2.drawDisc(116, 10, 8);
 }
 if (retranslatormode==false){
  u8g2.drawDisc(116, 30, 8);
 }
 u8g2.sendBuffer();
 Serial.println(readValue);
 delay(10);
}

void sendsingle(uint32_t targetid, const char* message){
    uint8_t msglen=strlen(message);
    uint8_t buffer[sizeof(packagesingle)+msglen];//creating buffer with size of struct + message
    struct packagesingle* pkgs = (struct packagesingle*)buffer;
    pkgs -> kind = (uint8_t)0;
    pkgs -> networkid = (uint32_t)10;
    pkgs -> messageid = (uint32_t)20;
    pkgs -> recieveid = (uint32_t)targetid;
    pkgs -> hops = (uint8_t)2;

    memcpy(pkgs -> message ,message,msglen);
    int state = radio.transmit(buffer, sizeof(buffer));
    if (state == RADIOLIB_ERR_NONE){
      Serial.println("sended");
    }
    else{
      Serial.println(state);
    }
}   