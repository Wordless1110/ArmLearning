#include<Servo.h>
#include <SoftwareSerial.h>
//板间通讯
SoftwareSerial link(10,11);

Servo base,LArm,RArm,claw;
//每动延迟时间
int DSD =15;
//极限角度
const int baseMin=0;
const int baseMax=180;
const int LArmMin=45;
const int LArmMax=160;
const int RArmMin=0;
const int RArmMax=180;
const int clawClose=100;
const int clawOpen=30;
//初始角度
const int baseInit=90;
const int LArmInit=90;
const int RArmInit=90;
const int clawInit=30;
//当前角度
int basePos=90;
int LArmPos=90;
int RArmPos=90;
int clawPos=30;
//模式变量
int mode=1;//1->串口输入;0->摇杆输入;
int btnmode=0;//0->任务3；1->任务5;
int taskState=0;//0->暂停;1->绘制;
//切换夹取物品
int actionIndex=2;
//摇杆引脚
const int joyLX=A0;//base
const int joyLY=A1;//L
const int joyRX=A2;//R
const int joyRY=A3;//claw
//录制状态
bool recording=false;
bool playing=false;
//录制存储
const int MaxRecordCounts=150;
unsigned char RecordData[MaxRecordCounts][4];
int RecordCounts=0;
//时间
unsigned long lastRecordTime=0;
unsigned long RecordStartTime=0;
unsigned long lastRecordbtnTime=0;
unsigned long lastPlaybtnTime=0;
unsigned long lastInitTime=0;
//长度
const float L1=77;
const float L2=155;
const float H=-85;
const float pen_up=10;
//画线
const int MaxLineNum=10;
float LineX1[MaxLineNum], LineY1[MaxLineNum];
float LineX2[MaxLineNum], LineY2[MaxLineNum];
int LineNum=0,LineIndex=0,stepIndex=0;
const int StepNum=100;
//五点示教
const int teachNum=5;
int teachIndex=0;
float teachX[teachNum];
float teachY[teachNum];

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.setTimeout(50);
  link.begin(9600);
  
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
    dealLetter(c);
    if(mode==1){
      if(c=='x'||c=='y'||c=='z'){
        dealString(cmd);
      }
    }
  }
  if(mode==0){
    Joystick();
  }
  if(link.available()>0){
    char c=link.read();
    if(c!='\n'){
      dealButton(c);
    }
  }
  if(taskState==1)drawLine();
  Recording();
  
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
    case 'r':
      dealReturnButton();
      break;
    case 'm':
      mode=!mode;
      Serial.print("Now the mode is ");
      if(mode==1){
        Serial.println("computer controlling mode.");
      }
      if(mode==0){
        Serial.println("joystick controlling mode.");
      }
      break;
    case 'n':
      btnmode=!btnmode;
      Serial.print("Now the btnmode is ");
      if(btnmode==0){
        Serial.println("task3 mode.");
      }
      if(btnmode==1){
        Serial.println("task5 mode.");
      }
      break;
    case 'a':
      actionA();
      break;
    case 'b':
      actionB();
      break;
    case 'c':
      actionC();
      break;
    case 'Y':                 //画直线
      LineNum=0;
      LineIndex=0;
      stepIndex=0;
      addLine(-50,150, 50,150);
      taskState=1;    //
      break;
    case 'N':                 //画N
      LineNum=0;
      LineIndex=0;
      stepIndex=0;
      addLine(-30,100,-30,170); 
      addLine(-30,170,30,100);
      addLine(30,100,30,170);
      taskState=1;
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
  int deadzone=30;
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

void actionA(void){
  claw.write(clawOpen);
  move(90,140,25);
  delay(600);
  claw.write(clawClose);
  delay(600);
  move(90,90,60);
  delay(600);
  move(165,60,30);
  delay(600);
  //180,60,30
  claw.write(clawOpen);
}

void actionB(void){
  claw.write(clawOpen);
  move(60,130,30);
  delay(600);
  claw.write(clawClose);
  delay(600);
  move(60,90,60);
  delay(600);
  move(180,90,45);
  delay(600);
  claw.write(clawOpen);
}

void actionC(void){
  claw.write(clawOpen);
  move(150,135,10);
  delay(600);
  claw.write(clawClose);//fail to move
  delay(600);
  move(150,100,45);
  delay(200);
  move(150,90,45);
  delay(300);
  move(30,90,30);
  delay(600);
  claw.write(clawOpen);
}

void dealButton(char c){
  if(btnmode==0){
    switch(c){
      case '1':
        dealtask2Button();
        break;
      case '2':
        dealRecordButton();
        break;
      case '3':
        dealPlayButton();
        break;
      case '4':
        dealReturnButton();
        break;
    }
  }
  if(btnmode==1){
    switch(c){
      case '1':
        if(taskState==0){
          LineIndex=0;
          stepIndex=0;
          taskState=1;
          Serial.println("Start");
        }
        break;
      case '2':
        taskState=!taskState;
        if(taskState==0){
          Serial.println("Pause");
        }
        if(taskState==1){
          Serial.println("Continue");
        }
        break;
      case '3':
        taskState=0;
        LineIndex=0;
        stepIndex=0;
        LineNum=0;
        move(90,90,30);
        delay(1000);
        claw.write(clawOpen);
        clawPos=clawOpen;
        Serial.println("Cancell");
        break;
      case '4':
        if(teachIndex<teachNum){
          getXY(teachX[teachIndex],teachY[teachIndex]);
          teachIndex++;
          Serial.print("Set teach point");
          Serial.println(teachIndex);
        }
        if(teachIndex==teachNum){
          LineNum=0;
          LineIndex=0;
          stepIndex=0;
          for(int i=0;i<teachNum-1;i++){
            addLine(teachX[i],teachY[i],teachX[i+1],teachY[i+1]);
          }
        }
        break;
    }
  }
}

void dealtask2Button(void){
  if(recording==true||playing==true)return;
  actionIndex=(actionIndex+1)%3;
  if(actionIndex==0)actionA();
  if(actionIndex==1)actionB();
  if(actionIndex==2)actionC();
}

void dealRecordButton(void){
  if(playing==true)return;
  if(recording==false){
    recording=true;
    mode=0;
    RecordCounts=0;
    RecordStartTime=millis();
    lastRecordTime=millis();
    Serial.println("Start recording");
  }
  else{
    mode=1;
    recording=false;
    unsigned long duration=(millis()-RecordStartTime)/1000;
    Serial.print("Record end,seconds:");
    Serial.println(duration);
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
  if(RecordCounts==MaxRecordCounts){
    Serial.println("Memory is full!Please stop record!");
  }
}

void dealPlayButton(void){
  if(recording==true||playing==true||RecordCounts==0)return;
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

void dealReturnButton(void) {
  if(playing==true||recording==true)return;
  move(baseInit,LArmInit,RArmInit);
  clawPos=clawInit;
  claw.write(clawPos);
  Serial.println("Return to initial position");
}

void moveto(float x,float y,float z){
  float theta1=degrees(atan2(y,x));
  float r=sqrt(x*x+y*y);
  float D=sqrt(r*r+z*z);
  if(D>L1+L2-5||D<fabs(L1-L2)+5){
    Serial.print("Fail to move there");
    return;
  }
  float cos1=(L1*L1+L2*L2-D*D)/(2*L1*L2);
  cos1=constrain(cos1,-1,1);
  float theta2=degrees(acos(cos1));
  float cos2=(L1*L1+D*D-L2*L2)/(2*L1*D);
  cos2=constrain(cos2,-1,1);
  float alpha=degrees(acos(cos2));
  float beta=degrees(atan2(z,r));
  float theta3=alpha+beta;
  float theta4=theta3+theta2-180;

  int sevorbase=constrain((int)theta1,baseMin,baseMax);
  int sevorLArm=constrain((int)(155-theta3),LArmMin,LArmMax);
  int sevorRArm=constrain((int)(90+theta4),RArmMin,RArmMax);
  move(sevorbase,sevorLArm,sevorRArm); 
}

void getXY(float &x,float &y){
  float theta1=basePos;
  float theta3=155-LArmPos;
  float theta4=RArmPos-90;
  float x1=L1*cos(radians(theta3));
  float y1=L1*sin(radians(theta3));
  float x3=L2*cos(radians(theta4));
  float y3=L2*sin(radians(theta4));
  float r=x1+x3;
  float z=y1+y3;
  x=r*cos(radians(theta1));
  y=r*sin(radians(theta1));
}

void addLine(float x1,float y1,float x2,float y2){
  if(LineNum>=MaxLineNum)return;
  LineX1[LineNum]=x1;
  LineY1[LineNum]=y1;
  LineX2[LineNum]=x2;
  LineY2[LineNum]=y2;
  LineNum++;
}

void drawLine(void){
  if(LineIndex>=LineNum){
    move(90,90,30);
    delay(1000);
    claw.write(clawOpen);
    clawPos=clawOpen;
    taskState=0;
    LineIndex=0;
    LineNum=0;
    stepIndex=0;
    teachIndex=0;
    Serial.println("Draw done.");
    return;
  }
  if(stepIndex==0&&LineIndex==0){
    moveto(LineX1[LineIndex],LineY1[LineIndex],H);
    delay(1000);
    claw.write(clawClose);
    clawPos=clawClose;
    delay(500);
  }
  float t=(float)stepIndex/StepNum;
  float x=LineX1[LineIndex]+(LineX2[LineIndex]-LineX1[LineIndex])*t;
  float y=LineY1[LineIndex]+(LineY2[LineIndex]-LineY1[LineIndex])*t;
  moveto(x,y,H);
  delay(5);
  stepIndex++;
  if(stepIndex>StepNum){
    stepIndex=0;
    LineIndex++;
  }
}