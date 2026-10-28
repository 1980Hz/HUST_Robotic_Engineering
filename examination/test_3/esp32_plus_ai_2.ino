const int SWITCH_A_PIN = 4;
const int SWITCH_B_PIN = 5;

unsigned long actionCount = 0;
unsigned long stampCount = 0;

unsigned long lastActionTime = 0;
unsigned long lastStampTime = 0;
unsigned long stampPeriod = 0;

unsigned long lastPrintTime = 0;

bool switchReleased = true;
unsigned long lastChangeTime = 0;

const unsigned long debounceTime = 50;
const unsigned long printInterval = 5000;

void setup() {
  Serial.begin(115200);

  pinMode(SWITCH_A_PIN, INPUT_PULLUP);
  pinMode(SWITCH_B_PIN, INPUT_PULLUP);

  Serial.println("Stamp counter and period measurement started");
  Serial.println("Waiting for stamping motion...");
}

void loop() {
  unsigned long currentTime = millis();

  bool switchAActive = digitalRead(SWITCH_A_PIN) == LOW;
  bool switchBActive = digitalRead(SWITCH_B_PIN) == LOW;

  // 两个触点都没有接地，表示开关已经释放
  bool released = !switchAActive && !switchBActive;

  if (released &&
      currentTime - lastChangeTime >= debounceTime) {
    switchReleased = true;
  }

  // 检测一次有效开关动作
  if ((switchAActive || switchBActive) &&
      switchReleased &&
      currentTime - lastChangeTime >= debounceTime) {

    actionCount++;
    switchReleased = false;
    lastChangeTime = currentTime;

    // 每两次动作计为一次冲压
    if (actionCount % 2 == 0) {
      stampCount++;

      // 从第二次冲压开始计算周期
      if (lastStampTime != 0) {
        stampPeriod = currentTime - lastStampTime;
      }

      lastStampTime = currentTime;
    }
  }

  // 每5秒输出一次数据
  if (currentTime - lastPrintTime >= printInterval) {
    lastPrintTime = currentTime;

    Serial.print("Stamp count: ");
    Serial.println(stampCount);

    if (stampPeriod > 0) {
      Serial.print("Stamp period: ");
      Serial.print(stampPeriod / 1000.0, 2);
      Serial.println(" s");
    } else {
      Serial.println("Stamp period: waiting for next complete cycle");
    }
  }
}