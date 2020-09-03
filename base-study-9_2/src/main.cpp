#include <Arduino.h>
#include <avr/wdt.h>//ウォッチドックタイマ
#include <Servo.h>//サーボ

#define m1 5//モーターPWM①
#define m2 6//モーターPWM②
#define sw 10//リセットスイッチ
#define mg 11//電磁石
int sv = 9;//サーボ
#define PWM1 80
#define PWM2 50
#define degree_target 140//サーボの回転角
Servo fuckservo;

bool flag,flag2,flag3,flag4=false;//各フラグ
char buff[10];//モーターPWM取得用文字配列
int  i=0,target=0;// target:目標回転数
int sword=0;//pySerial用変数
int PWM=0;//DCモーターPWM
int degree=0;//サーボの角度変更用
volatile long int value = 0;
volatile int rotation = 0;//エンコーダー回転数÷4　設定分解能
volatile uint8_t prev = 0;

void get_motor();//つかむぜ関数
void Encoder();//エンコーダー読み取り
void software_reset();//コードリセット

void setup() {  
  TCCR0B &= B11111000;
  TCCR0B |= B00000011;
  /* TCCR1B &= B11111000;
  TCCR1B |= B00000001;
  TCCR2B &= B11111000;
  TCCR2B |= B00000001; *///モーター周波数（1kHz）チートコード☆
  
  pinMode(2, INPUT); //割り込み用
  pinMode(3, INPUT); //割り込み用
  attachInterrupt(digitalPinToInterrupt(2), Encoder, CHANGE);//割り込み
  attachInterrupt(digitalPinToInterrupt(3), Encoder, CHANGE);//割り込み
  pinMode(m1,OUTPUT);
  pinMode(m2,OUTPUT);
  pinMode(sw,INPUT);
  pinMode(mg,OUTPUT); 

  fuckservo.attach(sv);
  fuckservo.write(0);//サーボ原点合わせ
  delay(1000);
  fuckservo.detach();
   
  Serial.begin(9600);
//put your setup code here, to run once:
}

void loop() {
/* Serial.println(rotation);
Serial.println(target); */
int SW1;
SW1=digitalRead(sw);
if(SW1==0){
  flag=false;
  flag2=false;
  analogWrite(m1,35);
  digitalWrite(m2,LOW);
  delay(10);
}
if(Serial.available()>0){
  sword=Serial.read();
  buff[i]=sword;
  if (buff[i] == 'a'){
    buff[i] ='\0';
    target = atoi(strtok(buff,","));
     i=0;
  }else{
      i+=1;
    }//文字列受信時の文字分解
  switch (sword){
    case 'a':
      flag=true;//get_motorを呼出　flag
      break;
    case 'b':
      flag2=true;
      break;
    case 'c':
      flag3=true;
      break;
    case 'd':
      flag4=true;
      break;
    case 'r':
      software_reset();
      break;
    }
  }
if(flag){
 get_motor();
}else if(flag2){
 analogWrite(m2,PWM2);
 digitalWrite(m1,LOW);
 if(SW1==0){
   flag2=false;
   Serial.write("B");
 }
}else if(flag3){
  fuckservo.attach(sv);
  while(degree<degree_target){
    delay(5);
    degree++;
    fuckservo.write(degree);
    }
  fuckservo.write(degree_target);
  fuckservo.detach();
  digitalWrite(mg,HIGH); 
  if(flag4){
   flag3=false;
  }
}else if(flag4){
   digitalWrite(mg,LOW);
   delay(500);
   fuckservo.attach(sv);
   fuckservo.write(0);
   fuckservo.detach();
   Serial.write("D");
   delay(500);
   software_reset();
}else{
   digitalWrite(m1,LOW);
   digitalWrite(m2,LOW);
 }
}
void get_motor(){
if (rotation < target){
  analogWrite(m1,PWM1);
  digitalWrite(m2,LOW);
}else if(rotation >= target){
  analogWrite(m1,0);
  digitalWrite(m2,LOW);
  flag=false;
  Serial.write("A");
 }
}
void Encoder(){
  uint8_t a = digitalRead(3);
  uint8_t b = digitalRead(2);
  uint8_t ab = (a << 1) | b;
  uint8_t encoded  = (prev << 2) | ab;
  if(encoded == 0b1101 || encoded == 0b0100 || encoded == 0b0010 || encoded == 0b1011){//パルス波形読み込み（正転）
    value ++;
    rotation = value / 4;
  }else if(encoded == 0b1110 || encoded == 0b0111 || encoded == 0b0001 || encoded == 0b1000) {//パルス波形読み込み（逆転）
    value --;
    rotation = value / 4;
  }
  prev = ab;
}
void software_reset() {
  wdt_disable();
  wdt_enable(WDTO_15MS);
  while (1) {}
}