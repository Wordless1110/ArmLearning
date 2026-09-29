#include<Servo.h>
Servo base,LArm,RArm,claw;
int DSD =15;//每动延迟时间
//极限角度
const int baseMin;
const int baseMax;
const int LArmMin;
const int LArmMax;
const int RArmMin;
const int RArmMax;
const int clawMin;
const int clawMax;
//当前角度
int basePos=90;
int LArmPos=90;
int RArmPos=90;
int clawPos=90;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  //链接对应引脚
  base.attach(9);
  delay(100);
  LArm.attach(8);
  delay(100);
  RArm.attach(7);
  delay(100);
  claw.attach(6);
  delay(100);

}

void loop() {
  // put your main code here, to run repeatedly:
  if(Serial.available()>0){
    char servoName=Serial.read();
    getPos(servoName);
  }
  //转动舵机
  base.write(basePos);
  delay(20);
  LArm.write(LArmPos);
  delay(20);
  RArm.write(RArmPos);
  delay(20);
  claw.write(clawPos);
  delay(20);
}

//获取对应角度
void getPos(int servoName){
  switch(servoName){
    case 'x':
      basePos=Serial.parseInt();
      break;
    case 'y':
      LArmPos=Serial.parseInt();
      break;
    case 'z':
      RArmPos=Serial.parseInt();
      break;
    case 'e':
      clawPos=Serial.parseInt();
      break;
  }
}