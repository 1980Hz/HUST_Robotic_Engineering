const int SWITCH_A_PIN = 4;
const int SWITCH_B_PIN = 5;

unsigned long actionCount = 0;
unsigned long stampCount = 0;

bool switchReleased = true;
unsigned long lastChangeTime = 0;

const unsigned long debounceTime = 50;

void setup() {
  Serial.begin(115200);

  pinMode(SWITCH_A_PIN, INPUT_PULLUP);
  pinMode(SWITCH_B_PIN, INPUT_PULLUP);

  Serial.println("Stamp count: 0");
}

void loop() {
  bool switchAActive = digitalRead(SWITCH_A_PIN) == LOW;
  bool switchBActive = digitalRead(SWITCH_B_PIN) == LOW;

  bool released = !switchAActive && !switchBActive;

  if (released && millis() - lastChangeTime >= debounceTime) {
    switchReleased = true;
  }

  if ((switchAActive || switchBActive) &&
      switchReleased &&
      millis() - lastChangeTime >= debounceTime) {
    
    actionCount++;
    switchReleased = false;
    lastChangeTime = millis();

    // 每两次开关动作，冲压计数加一
    if (actionCount % 2 == 0) {
      stampCount++;

      Serial.print("Stamp count: ");
      Serial.println(stampCount);
    }
  }
}