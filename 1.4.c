#include<Servo.h>
Servo base,LArm,RArm,claw;
//每动延迟时间
int DSD =15;
//极限角度
const int baseMin=0;
const int baseMax=180;
const int LArmMin=45;
const int LArmMax=160;
const int RArmMin=10;
const int RArmMax=90;
const int clawClose=130;
const int clawOpen=95;
//当前角度
int basePos=90;
int LArmPos=90;
int RArmPos=75;
int clawPos=105;
//摇杆引脚
const int joyLX=A0;//base
const int joyLY=A1;//L
const int joyRX=A2;//R
const int joyRY=A3;//claw
//模式变量
int mode=1;//1->串口输入;0->摇杆输入;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.setTimeout(50);
  //链接对应引脚
  base.attach(9);
  delay(10);
  LArm.attach(8);
  delay(10);
  RArm.attach(7);
  delay(10);
  claw.attach(6);
  delay(10);
  //初始化舵机位置
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
  //读取输入
  if(Serial.available()>0){
    String cmd=Serial.readString();
    cmd.trim();
    char c=cmd[0];
    if(c=='m'){
      mode=!mode;
    };
    if(mode==1){
      if(c=='x'||c=='y'||c=='z'){
        dealString(cmd);
      }
      else{
        dealLetter(c);
      }
    }
  }
  if(mode==0){
    Joystick();
  }
}

//处理输入字母
void dealLetter(char Letter){
  switch(Letter){
    case 'o':
      claw.write(clawClose);
     break;
    case 's':
      claw.write(clawOpen);
      break;
    case 'h':
      if(DSD<40){
        DSD=DSD+5;
        Serial.print("The DSD is ");
        Serial.println(DSD);
      }
      break;
    case 'l':
      if(DSD>5){
        DSD=DSD-5;
        Serial.print("The DSD is");
        Serial.println(DSD);
      }
      break;
  }
}
//处理输入字符串
void dealString(String cmd){
  int tbase=basePos;
  int tLArm=LArmPos;
  int tRArm=RArmPos;
  int start=0;
  for(int i=0;i<=cmd.length();i++){
    //分隔逗号之间内容
    if(i==cmd.length()||cmd[i]==','){
      String part=cmd.substring(start,i);
      part.trim();
      if(part.length()>1){
        char ServoName=part[0];
        int ServoPos=part.substring(1).toInt();
        switch(ServoName){
          case 'x':
            if(ServoPos<=baseMax&&ServoPos>=baseMin){
              tbase=ServoPos;
            }
            break;
          case 'y':
            if(ServoPos<=LArmMax&&ServoPos>=LArmMin){
              tLArm=ServoPos;
            }
            break;
          case 'z':
            if(ServoPos<=RArmMax&&ServoPos>=RArmMin){
              tRArm=ServoPos;
            }
            break;
        }
      }
      start=i+1;
    }
  }
  move(tbase,tLArm,tRArm);
}
//一同运行
void move(int tbase,int tLArm,int tRArm){
  int stepBase=abs(tbase-basePos);
  int stepLArm=abs(tLArm-LArmPos);
  int stepRArm=abs(tRArm-RArmPos);
  //获取最多步数
  int max_step=stepBase;
  if(stepLArm>max_step)max_step=stepLArm;
  if(stepRArm>max_step)max_step=stepRArm;
  if(max_step==0)return;
  //计算每步角度(可正可负)
  float everyBase=(float)(tbase-basePos)/max_step;
  float everyLArm=(float)(tLArm-LArmPos)/max_step;
  float everyRArm=(float)(tRArm-RArmPos)/max_step;
  float tempBase=basePos;
  float tempLArm=LArmPos;
  float tempRArm=RArmPos;
  for(int i=0;i<max_step;i++){
    tempBase+=everyBase;
    tempLArm+=everyLArm;
    tempRArm+=everyRArm;
    base.write(tempBase);
    LArm.write(tempLArm);
    RArm.write(tempRArm);
    delay(DSD);
  }
  basePos=tbase;
  LArmPos=tLArm;
  RArmPos=tRArm;
  base.write(basePos);
  LArm.write(LArmPos);
  RArm.write(RArmPos);
}

void Joystick(void){
  int tempbase=analogRead(joyLX);
  int tempLArm=analogRead(joyLY);
  int tempRArm=analogRead(joyRX);
  int tempclaw=analogRead(joyRY);
  int offsetbase=tempbase-512;
  int offsetLArm=tempLArm-512;
  int offsetRArm=tempRArm-512;
  int offsetclaw=tempclaw-512;
  int deadzone=50;
  int everystep=2;
  //base
  if(offsetbase>deadzone){
    basePos+=everystep;
  }
  if(offsetbase<-deadzone){
    basePos-=everystep;
  }
  if(basePos<baseMin)basePos=baseMin;
  if(basePos>baseMax)basePos=baseMax;
  base.write(basePos);
  //LArm
  if(offsetLArm>deadzone){
    LArmPos+=everystep;
  }
  if(offsetLArm<-deadzone){
    LArmPos-=everystep;
  }
  if(LArmPos<LArmMin)LArmPos=LArmMin;
  if(LArmPos>LArmMax)LArmPos=LArmMax;
  LArm.write(LArmPos);
  //RArm
  if(offsetRArm>deadzone){
    RArmPos+=everystep;
  }
  if(offsetRArm<-deadzone){
    RArmPos-=everystep;
  }
  if(RArmPos<RArmMin)RArmPos=RArmMin;
  if(RArmPos>RArmMax)RArmPos=RArmMax;
  RArm.write(RArmPos);
  //claw
  if(offsetclaw>deadzone){
    clawPos+=everystep;
  }
  if(offsetclaw<-deadzone){
    clawPos-=everystep;
  }
  if(clawPos<clawOpen)clawPos=clawOpen;
  if(clawPos>clawClose)clawPos=clawClose;
  claw.write(clawPos);
  delay(15);
}