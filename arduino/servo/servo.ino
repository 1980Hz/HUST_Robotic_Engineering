// --- 引脚定义 ---
// 旋钮传感器引脚 (对应 STM32 的 PA0 模拟输入)
const int knobPin = PA0; 

// TB6612 控制引脚 (对应 STM32 的 PA1, PA4, PA5)
const int PWMA_Pin = PA1; // PWM 调速引脚
const int AIN1_Pin = PA4; // 方向控制 1
const int AIN2_Pin = PA5; // 方向控制 2

void setup() {
  // 初始化串口，方便在电脑上看数据，波特率 115200
  Serial.begin(115200);

  // 设置引脚模式
  pinMode(knobPin, INPUT_ANALOG); // 旋钮模拟输入
  pinMode(PWMA_Pin, OUTPUT);      // 电机 PWM 输出
  pinMode(AIN1_Pin, OUTPUT);      // 电机方向控制
  pinMode(AIN2_Pin, OUTPUT);      // 电机方向控制

  // 设定初始状态
  digitalWrite(AIN1_Pin, HIGH);   // AIN1 高，AIN2 低 -> 电机正转
  digitalWrite(AIN2_Pin, LOW);    
  analogWrite(PWMA_Pin, 0);       // 初始转速为 0
}

void loop() {
  // 1. 读取旋钮的值
  // STM32 的 ADC 是 12 位的，所以取值范围是 0 ~ 4095
  int knobValue = analogRead(knobPin);

  // 2. 将旋钮的值映射到 PWM 的值 (0~255)
  // map() 函数：将 0-4095 等比例缩放到 0-255
  int pwmValue = map(knobValue, 0, 4095, 0, 255);

  // 3. 为了防止电机在旋钮没转到位时发出微弱的“嗡嗡”声，加一个死区
  // 如果 PWM 值非常小（比如小于 20），就当作 0 处理，让电机停转
  if (pwmValue < 20) {
    pwmValue = 0;
  }

  // 4. 将计算后的 PWM 值输出给 TB6612 (控制转速)
  analogWrite(PWMA_Pin, pwmValue);

  // 5. 串口打印调试信息（方便你看到旋钮的读取状态）
  Serial.print("旋钮原始值: ");
  Serial.print(knobValue);
  Serial.print(" | 映射后 PWM 占空比: ");
  Serial.println(pwmValue);

  // 短暂延时，防止串口打印太快导致卡顿
  delay(50);
}
