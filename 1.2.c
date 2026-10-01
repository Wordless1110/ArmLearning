#include<Servo.h>
Servo base,LArm,RArm,claw;
int DSD =15;//每动延迟时间
//极限角度
const int baseMin=0;
const int baseMax=180;
const int LArmMin=45;
const int LArmMax=135;
const int RArmMin;
const int RArmMax;
const int clawMin=130;
const int clawMax=95;
//当前角度
int basePos=90;
int LArmPos=90;
int RArmPos=90;
int clawPos=105;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  //链接对应引脚
  base.attach(9);
  delay(10);
  LArm.attach(8);
  delay(10);
  RArm.attach(7);
  delay(10);
  claw.attach(6);
  delay(10);

  base.write(basePos);
  delay(100);
  LArm.write(LArmPos);
  delay(100);
  RArm.write(RArmPos);
  delay(100);
  claw.write(clawPos);
  delay(100);

}

void loop() {
  // put your main code here, to run repeatedly:
  if(Serial.available()>0){
    char servoName=Serial.read();
    dealInput(servoName);
  }
  delay(50);
}

//处理输入字母
void dealInput(int servoName){
  int ServoPos;
  switch(servoName){
    case 'x':
      ServoPos=Serial.parseInt();
      if(ServoPos<baseMax&&ServoPos>baseMin){
        if(ServoPos>basePos){
          for(;basePos<ServoPos;basePos++){
            base.write(basePos);
            delay(DSD);
          }
        }
        if(ServoPos<basePos){
          for(;basePos>ServoPos;basePos--){
            base.write(basePos);
            delay(DSD);
          }
        }
        basePos=ServoPos;
        base.write(basePos);
      }
      break;
    case 'y':
      ServoPos=Serial.parseInt();
      if(ServoPos<LArmMax&&ServoPos>LArmMin){
        if(ServoPos>LArmPos){
          for(;LArmPos<ServoPos;LArmPos++){
            LArm.write(LArmPos);
            delay(DSD);
          }
        }
        if(ServoPos<LArmPos){
          for(;LArmPos>ServoPos;LArmPos--){
            LArm.write(LArmPos);
            delay(DSD);
          }
        }
        LArmPos=ServoPos;
        LArm.write(LArmPos);
      }
      break;
    case 'z':
      ServoPos=Serial.parseInt();
      if(ServoPos<RArmMax&&ServoPos>RArmMin){
        if(ServoPos>RArmPos){
          for(;RArmPos<ServoPos;RArmPos++){
            RArm.write(RArmPos);
            delay(DSD);
          }
        }
        if(ServoPos<RArmPos){
          for(;RArmPos>ServoPos;RArmPos--){
            RArm.write(RArmPos);
            delay(DSD);
          }
        }
        RArmPos=ServoPos;
        RArm.write(RArmPos);
      }
      break;
    case 'o':
      claw.write(clawMin);
     break;
    case 's':
      claw.write(clawMax);
      break;
    case 'h':
      if(DSD<40){
        DSD=DSD+5;
      }
      break;
    case 'l':
      if(DSD>5){
        DSD=DSD-5;
      }
      break;
  }
}