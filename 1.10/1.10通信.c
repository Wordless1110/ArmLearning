#include<SoftwareSerial.h>
SoftwareSerial link(1,0);
//引脚连接
const int btn1=A0;
const int btn2=A1;
const int btn3=A2;
const int btn4=A3;

const int gap_time=300;
long lastbtntime[4]={0};

void setup() {
  // put your setup code here, to run once:
  link.begin(9600);

  pinMode(btn1, INPUT_PULLUP);
  pinMode(btn2, INPUT_PULLUP);
  pinMode(btn3, INPUT_PULLUP);
  pinMode(btn4, INPUT_PULLUP);
}

void loop() {
  // put your main code here, to run repeatedly:
  dealButton(btn1,1,0);
  dealButton(btn2,2,1);
  dealButton(btn3,3,2);
  dealButton(btn4,4,3);
}

void dealButton(int pin,int code,int index){
  if(digitalRead(pin)==LOW&&millis()-lastbtntime[index]>gap_time){
    lastbtntime[index]=millis();
    link.print(code);
  }
}