// 定义蜂鸣器连接的GPIO引脚
const int buzzerPin = 4; 

void setup() {
  // 设置引脚为输出模式
  pinMode(buzzerPin, OUTPUT); 
}

void loop() {
  // 使用tone函数发出指定频率的声音
  tone(buzzerPin, 1000); // 1000Hz，对应中音
  delay(1000);           // 持续1秒
  noTone(buzzerPin);     // 停止发声
  delay(1000);           // 静音1秒
}
