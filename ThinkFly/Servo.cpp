#include <Servo.h>

Servo myServo;
int angle = 90; // 初始角度

const int button1 = PB0; // 按键1接PB0和GND
const int button2 = PB1; // 按键2接PB1和GND

// 记录按键上一次的状态，用来做边沿检测
int lastButton1State = HIGH;
int lastButton2State = HIGH;

void setup() {
  myServo.attach(PA0); // 舵机信号线接PA0
  pinMode(button1, INPUT_PULLUP);
  pinMode(button2, INPUT_PULLUP);
  myServo.write(angle);
  delay(500);
}

void loop() {
  // 读取当前按键状态
  int currentButton1State = digitalRead(button1);
  int currentButton2State = digitalRead(button2);

  // --- 检测按键1按下的一瞬间 ---
  if (currentButton1State == LOW && lastButton1State == HIGH) {
    angle += 45;
    if (angle > 180) angle = 180; // 限制最大角度
    myServo.write(angle);
    delay(50); // 简单的按键消抖
  }
  lastButton1State = currentButton1State; // 更新状态

  // --- 检测按键2按下的一瞬间 ---
  if (currentButton2State == LOW && lastButton2State == HIGH) {
    angle -= 45;
    if (angle < 0) angle = 0; // 限制最小角度
    myServo.write(angle);
    delay(50);
  }
  lastButton2State = currentButton2State; // 更新状态
  
  delay(10);
}