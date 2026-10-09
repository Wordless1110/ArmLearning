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
const int clawClose=140;
const int clawOpen=90;
//初始角度
const int baseInit=90;
const int LArmInit=90;
const int RArmInit=55;
const int clawInit=105;
//当前角度
int basePos=90;
int LArmPos=90;
int RArmPos=55;
int clawPos=105;
//模式变量
int mode=1;//1->串口输入;0->摇杆输入;
//摇杆引脚
const int joyLX=A0;//base
const int joyLY=A1;//L
const int joyRX=A2;//R
const int joyRY=A3;//claw
//按键引脚
const int task2=2;
const int Recordbtn=3;
const int Playbtn=4;
const int PosInit=5;
//录制状态
bool recording=false;
bool playing=false;
//录制存储
const int MaxRecordCounts=200;
unsigned char RecordData[MaxRecordCounts][4];
int RecordCounts=0;
//时间
unsigned long lastRecordTime=0;
unsigned long RecordStartTime=0;
unsigned long lastRecordbtnTime=0;
unsigned long lastPlaybtnTime=0;
unsigned long lastInitTime=0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.setTimeout(50);
  
  pinMode(task2, INPUT_PULLUP);
  pinMode(Recordbtn, INPUT_PULLUP);
  pinMode(Playbtn, INPUT_PULLUP);
  pinMode(PosInit, INPUT_PULLUP);
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
  delay(500);
  LArm.write(LArmPos);
  delay(500);
  RArm.write(RArmPos);
  delay(500);
  claw.write(clawPos);
  delay(500);
}

void loop() {
  // put your main code here, to run repeatedly:
  //读取输入
  if(Serial.available()>0){
    String cmd=Serial.readString();
    cmd.trim();
    if(cmd.length()==0)return; 
    char c=cmd[0];
    if(c=='m'){
      mode=!mode;
      Serial.print("Now the mode is ");
      if(mode==1){
        Serial.println("computer controlling mode.");
      }
      if(mode==0){
        Serial.println("joystick controlling mode.");
      }
    }
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
  dealRecordButton();
  Recording();
  dealPlayButton();
  dealReturnButton();
}

//处理输入字母
void dealLetter(char Letter){
  switch(Letter){
    case 'o':case 'O':
      claw.write(clawClose);
     break;
    case 's':case 'S':
      claw.write(clawOpen);
      break;
    case 'h':case 'H':
      if(DSD<40){
        DSD=DSD+5;
        Serial.print("The DSD is ");
        Serial.println(DSD);
      }
      break;
    case 'l':case 'L':
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
  if(playing==true)return;
  int tempbase=analogRead(joyLX);
  int tempLArm=analogRead(joyLY);
  int tempRArm=analogRead(joyRX);
  int tempclaw=analogRead(joyRY);
  int offsetbase=tempbase-512;
  int offsetLArm=tempLArm-512;
  int offsetRArm=tempRArm-512;
  int offsetclaw=tempclaw-512;
  int deadzone=50;
  int everystep=1;
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
    LArmPos-=everystep;
  }
  if(offsetLArm<-deadzone){
    LArmPos+=everystep;
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

void dealRecordButton(void){
  if(playing==true)return;
  if(digitalRead(Recordbtn)==LOW&&millis()-lastRecordbtnTime>300){
    lastRecordbtnTime=millis();
    if(recording==false){
      recording=true;
      mode=0;
      RecordCounts=0;
      RecordStartTime=millis();
      lastRecordTime=millis();
      Serial.println("Start recording");
    }
    else{
      recording=false;
      unsigned long duration=(millis()-RecordStartTime)/1000;
      Serial.print("Record end,seconds:");
      Serial.println(duration);
    }
  }
}

void Recording(void){
  if(recording==false||playing==true||millis()-lastRecordTime<100)return;
  lastRecordTime=millis();
  if(RecordCounts<MaxRecordCounts){
    RecordData[RecordCounts][0]=basePos;
    RecordData[RecordCounts][1]=LArmPos;
    RecordData[RecordCounts][2]=RArmPos;
    RecordData[RecordCounts][3]=clawPos;
    RecordCounts++;
  }
  if(RecordCounts==200){
    Serial.println("Memory is full!Please stop record!");
  }
}

void dealPlayButton(void){
  if(recording==true||RecordCounts==0)return;
  if(digitalRead(Playbtn)==LOW&&playing==false&&millis()-lastPlaybtnTime>300){
    lastPlaybtnTime=millis();
    playing=true;
    Serial.println("Start playing");
    for(int i=0;i<RecordCounts;i++){
      base.write(RecordData[i][0]);
      LArm.write(RecordData[i][1]);
      RArm.write(RecordData[i][2]);
      claw.write(RecordData[i][3]);
      delay(100);
    }
    playing=false;
    Serial.println("Play end");
    basePos=RecordData[RecordCounts-1][0];
    LArmPos=RecordData[RecordCounts-1][1];
    RArmPos=RecordData[RecordCounts-1][2];
    clawPos=RecordData[RecordCounts-1][3];
  }
}

void dealReturnButton(void) {
  if(playing==true||recording==true)return;
  if(digitalRead(PosInit)==LOW&&millis()-lastInitTime>300){
    lastInitTime=millis();
    move(baseInit,LArmInit,RArmInit);
    clawPos=clawInit;
    claw.write(clawPos);
    Serial.println("Return to initial position");
  }
}