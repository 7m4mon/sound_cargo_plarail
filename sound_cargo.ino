/*
 * 赤外線リモコン＆磁石でプラレール（＋MP3サウンド）をコントロール！　2018/03/17　7M4MON
 * プラレールの貨車に搭載し、リモコンからプラレールの発車・停止・速度(4段階)をコントロールする。
 * 受け取ったリモコンコードに応じてサウンドも鳴らす。
 * 磁石の上を通ったら停止する。ただし、通過モード時は通過。
 * 最大30,720バイトのフラッシュメモリのうち、スケッチが11,148バイト（36%）を使っています。
 * 最大2,048バイトのRAMのうち、グローバル変数が501バイト（24%）を使っていて、ローカル変数で1,547バイト使うことができます。
 */

#include <EEPROM.h>
#include <boarddefs.h>
#include <IRremote.h>
#include <DFPlayer_Mini_Mp3.h>

#define LED_PIN 13
#define RECV_PIN 8
#define MAG_SENS_PIN 3
#define TRAIN_CTRL_PIN 5    // PWM ENABLE PIN = { 3,5,6,9,10,11 }

#define CODE_1   0xFFA25D
#define CODE_2   0xFF629D 
#define CODE_3   0xFFE21D 
#define CODE_4   0xFF22DD   
#define CODE_5   0xFF02FD  
#define CODE_6   0xFFC23D  
#define CODE_7   0xFFE01F 
#define CODE_8   0xFFA857 
#define CODE_9   0xFF906F 
#define CODE_0   0xFF9867 
#define CODE_AS  0xFF6897 
#define CODE_SH  0xFFB04F 
#define CODE_UP  0xFF18E7
#define CODE_DN  0xFF4AB5
#define CODE_OK  0xFF38C7 
#define CODE_L   0xFF10EF 
#define CODE_R   0xFF5AA5 

#define VOL_MAX 30
#define VOL_MIN 1
#define VOL_EEPROM_ADDR 0

#define SPEED_MAX 5
#define SPEED_MIN 0

IRrecv irrecv(RECV_PIN);
decode_results results;
int8_t mp3_volume = 20;
int8_t last_volume;
int8_t train_speed;
bool   stopping = true;
const uint8_t train_speed_list[SPEED_MAX+1] = {0,110,130,170,255,255};  // 停止、スピード1,2,3,4、磁石通過

void led_blilnk(uint8_t on, uint8_t off, uint8_t times){
  uint8_t i;
  for (i=0; i<times; i++){
    digitalWrite(LED_PIN, HIGH);
    delay(on);
    digitalWrite(LED_PIN, LOW);
    delay(off);
  }
}

void setup () {
  pinMode(MAG_SENS_PIN, INPUT_PULLUP);
  irrecv.enableIRIn();      // Start the receiver
	Serial.begin (9600);
	mp3_set_serial (Serial);	//set Serial for DFPlayer-mini mp3 module 
  
  delay (300);
  mp3_volume = (int8_t)EEPROM.read(VOL_EEPROM_ADDR);
  mp3_volume = mp3_volume > VOL_MAX ? VOL_MAX :
               mp3_volume < VOL_MIN ? VOL_MIN :
               mp3_volume;
  mp3_set_volume (mp3_volume);
  last_volume = mp3_volume;
  train_speed = 0;
  delay (300);
  mp3_play (1);
}

void loop() {
  uint8_t play_num = 0;
  if (irrecv.decode(&results)) {
    led_blilnk(10,0,1);
    switch (results.value){
      case CODE_SH:
        play_num += 1;
      case CODE_AS:
        play_num += 1;
      case CODE_0:
        play_num += 1;
      case CODE_9:
        play_num += 1;
      case CODE_8:
        play_num += 1;
      case CODE_7:
        play_num += 1;
      case CODE_6:
        play_num += 1;
      case CODE_5:
        play_num += 1;
      case CODE_4:
        play_num += 1;
      case CODE_3:
        play_num += 1;
      case CODE_2:
        play_num += 1;
      case CODE_1:
        play_num += 1;
        mp3_play ((uint16_t)play_num);
        if(last_volume != mp3_volume){
          EEPROM.write(VOL_EEPROM_ADDR,(byte)mp3_volume);
          last_volume = mp3_volume;
          led_blilnk(10,100,2);
        }
        break;

      /* Volume Control */
      case CODE_UP:
        if (mp3_volume < VOL_MAX) {
          mp3_volume++;
          mp3_set_volume (mp3_volume);
        }else{
          led_blilnk(100,0,1);     
        }
        break;      
      case CODE_DN:
        if (mp3_volume > VOL_MIN) {
          mp3_volume--;
          mp3_set_volume (mp3_volume);
        }else{
          led_blilnk(100,0,1);
        }
        break;
        
      /* Motor Control */
      case CODE_R:
        if (train_speed < SPEED_MAX) {
          if(train_speed == 0){
            analogWrite(TRAIN_CTRL_PIN,255);  //最初だけ喝入れ
            delay(30);
          }
          train_speed++;
          if (train_speed == SPEED_MAX){
            led_blilnk(10,200,3);
          }
          analogWrite(TRAIN_CTRL_PIN,train_speed_list[train_speed]);
        }else{
          led_blilnk(10,200,3);
        }
        break;      
      case CODE_L:
        if (train_speed > SPEED_MIN) {
          train_speed--;
          analogWrite(TRAIN_CTRL_PIN,train_speed_list[train_speed]);
        }else{
          led_blilnk(10,200,2);
        }
        break;
      case CODE_OK:
        train_speed = 0;
        analogWrite(TRAIN_CTRL_PIN,train_speed_list[train_speed]);
        break;
    }
    irrecv.resume(); // Receive the next value
  }

  /* Detect Stop Magnet */
  if(digitalRead(MAG_SENS_PIN) == LOW && !stopping ){
    if(train_speed != SPEED_MAX){    
      train_speed = 0;
      analogWrite(TRAIN_CTRL_PIN,train_speed_list[train_speed]);
      led_blilnk(10,100,6);
    }else{
      /* passing the magnet */
      mp3_play (1);
      led_blilnk(10,200,3); 
    }
  }
  if (train_speed >= 2){
    stopping = false;
  }else if(train_speed == 0){
    stopping = true;
  }
}
