#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <DHT.h>

// -------------------- 引脚定义 --------------------
const int SWITCH_A_PIN = 4;
const int SWITCH_B_PIN = 5;

#define DHT_PIN 6
#define DHT_TYPE DHT11

DHT dht(DHT_PIN, DHT_TYPE);

// -------------------- WiFi 和 DeepSeek --------------------
const char* WIFI_SSID = "****";
const char* WIFI_PASSWORD = "****";

// 填入重新生成的 API Key
const char* DEEPSEEK_API_KEY = "****";

const char* DEEPSEEK_URL =
  "https://api.deepseek.com/chat/completions";

// -------------------- 计数和周期 --------------------
unsigned long actionCount = 0;
unsigned long stampCount = 0;

unsigned long lastStampTime = 0;
unsigned long stampPeriod = 0;

unsigned long lastChangeTime = 0;
unsigned long lastPrintTime = 0;

bool switchReleased = true;

const unsigned long debounceTime = 50;
const unsigned long printInterval = 5000;

// -------------------- 对话记录 --------------------
// 保存系统提示词和最近几轮对话
DynamicJsonDocument conversation(12000);

// 连接 WiFi
void connectWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.print("正在连接 WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi connected");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
}

// 初始化对话
void clearConversation() {
  conversation.clear();

  JsonArray messages = conversation.to<JsonArray>();

  JsonObject systemMessage = messages.add<JsonObject>();
  systemMessage["role"] = "system";
  systemMessage["content"] =
    "你是工业冲压设备状态诊断助手。"
    "用户会提供冲压机构运动周期、DHT11温度和湿度数据。"
    "你必须根据用户在对话中给出的判断标准进行分析。"
    "如果用户没有给出温度、湿度或运动周期的正常范围，"
    "不要擅自假设标准，应先询问用户。"
    "如果数据异常，请明确指出异常项目、当前值、用户设定的正常范围，"
    "并给出简短的检查建议。"
    "如果缺少运动周期数据，也要明确说明。"
    "回答使用中文，内容简洁，不要编造数据。";
}

// 读取当前设备数据
bool readDeviceData(float& temperature, float& humidity) {
  humidity = dht.readHumidity();
  temperature = dht.readTemperature();

  if (isnan(temperature) || isnan(humidity)) {
    return false;
  }

  return true;
}

// 发送问题和实时设备数据给 DeepSeek
void askDeepSeek(const String& question,
                 float temperature,
                 float humidity) {
  WiFiClientSecure client;

  // 测试阶段使用；正式产品建议配置服务器证书
  client.setInsecure();

  HTTPClient http;

  if (!http.begin(client, DEEPSEEK_URL)) {
    Serial.println("HTTP 初始化失败");
    return;
  }

  http.addHeader("Content-Type", "application/json");
  http.addHeader(
    "Authorization",
    String("Bearer ") + DEEPSEEK_API_KEY
  );

  String sensorPrompt =
    "用户问题：\n" + question +
    "\n\n本次采集到的设备实时数据：\n"
    "温度：" + String(temperature, 1) + " 摄氏度\n"
    "湿度：" + String(humidity, 1) + " %\n";

  if (stampPeriod > 0) {
    sensorPrompt +=
      "冲压运动周期：" + String(stampPeriod / 1000.0, 2) + " 秒\n";
  } else {
    sensorPrompt +=
      "冲压运动周期：暂无数据，尚未完成两次冲压测量\n";
  }

  sensorPrompt +=
    "\n请结合之前的对话上下文和以上最新数据回答。";

  JsonArray history = conversation.as<JsonArray>();

  // 保存用户消息
  JsonObject userMessage = history.add<JsonObject>();
  userMessage["role"] = "user";
  userMessage["content"] = sensorPrompt;

  DynamicJsonDocument requestDoc(14000);

  requestDoc["model"] = "deepseek-chat";
  requestDoc["temperature"] = 0.2;

  JsonArray requestMessages =
    requestDoc["messages"].to<JsonArray>();

  // 将历史对话复制到本次请求
  for (JsonVariant message : history) {
    requestMessages.add(message);
  }

  String requestBody;
  serializeJson(requestDoc, requestBody);

  Serial.println("正在请求 DeepSeek...");

  int httpCode = http.POST(requestBody);
  String response = http.getString();

  if (httpCode == HTTP_CODE_OK) {
    DynamicJsonDocument responseDoc(8192);

    DeserializationError error =
      deserializeJson(responseDoc, response);

    if (error) {
      Serial.println("DeepSeek 响应解析失败");
      Serial.println(response);

      history.remove(history.size() - 1);
      http.end();
      return;
    }

    const char* answer =
      responseDoc["choices"][0]["message"]["content"];

    if (answer == nullptr) {
      Serial.println("DeepSeek 返回内容为空");
      history.remove(history.size() - 1);
      http.end();
      return;
    }

    Serial.println();
    Serial.println("DeepSeek：");
    Serial.println(answer);
    Serial.println();

    // 保存模型回复，支持连续对话
    JsonObject assistantMessage = history.add<JsonObject>();
    assistantMessage["role"] = "assistant";
    assistantMessage["content"] = answer;

    // 保留系统消息，删除最早的旧消息，避免内存持续增长
    while (history.size() > 13) {
      history.remove(1);
    }
  } else {
    Serial.print("请求失败，HTTP 状态码：");
    Serial.println(httpCode);
    Serial.println(response);

    // 请求失败时删除刚才添加的用户消息
    history.remove(history.size() - 1);

    if (httpCode == 401) {
      Serial.println("API Key 无效或已失效");
    } else if (httpCode == 402) {
      Serial.println("DeepSeek API 余额不足");
    }
  }

  http.end();
}

// 检测开关，并完成计数和周期测量
void updateStampMeasurement() {
  unsigned long currentTime = millis();

  bool switchAActive =
    digitalRead(SWITCH_A_PIN) == LOW;

  bool switchBActive =
    digitalRead(SWITCH_B_PIN) == LOW;

  // 两个输入均为高电平，表示开关处于释放或切换状态
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

      if (lastStampTime != 0) {
        stampPeriod = currentTime - lastStampTime;
      }

      lastStampTime = currentTime;
    }
  }
}

// 每 5 秒输出一次冲压状态
void printStampStatus() {
  unsigned long currentTime = millis();

  if (currentTime - lastPrintTime < printInterval) {
    return;
  }

  lastPrintTime = currentTime;

  Serial.print("Stamp count: ");
  Serial.println(stampCount);

  if (stampPeriod > 0) {
    Serial.print("Stamp period: ");
    Serial.print(stampPeriod / 1000.0, 2);
    Serial.println(" s");
  } else {
    Serial.println("Stamp period: waiting for two complete stamps");
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  pinMode(SWITCH_A_PIN, INPUT_PULLUP);
  pinMode(SWITCH_B_PIN, INPUT_PULLUP);

  dht.begin();

  clearConversation();
  connectWiFi();

  Serial.println();
  Serial.println("设备诊断系统已启动");
  Serial.println("输入问题后按回车，例如：");
  Serial.println("我的设备状态正常吗？");
  Serial.println();
  Serial.println("可以在对话中设定正常标准，例如：");
  Serial.println("温度20到30度、湿度40%到60%、周期小于3秒才算正常");
  Serial.println();
  Serial.println("输入 /clear 可以清除对话记录");
}

void loop() {
  updateStampMeasurement();
  printStampStatus();

  if (Serial.available() > 0) {
    String question = Serial.readStringUntil('\n');
    question.trim();

    if (question.length() == 0) {
      return;
    }

    if (question == "/clear") {
      clearConversation();
      Serial.println("对话记录已清除");
      return;
    }

    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("WiFi 未连接，正在重新连接...");
      connectWiFi();
    }

    float temperature;
    float humidity;

    if (!readDeviceData(temperature, humidity)) {
      Serial.println("DHT11 读取失败，请检查 GPIO6 接线");
      return;
    }

    askDeepSeek(question, temperature, humidity);
  }

  if (WiFi.status() != WL_CONNECTED) {
    connectWiFi();
  }
}
