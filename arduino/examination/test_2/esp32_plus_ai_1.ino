#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <DHT.h>

#define DHT_PIN 4
#define DHT_TYPE DHT11

const char* WIFI_SSID = "****";
const char* WIFI_PASSWORD = "****";
const char* DEEPSEEK_API_KEY = "****";

const char* DEEPSEEK_URL =
  "https://api.deepseek.com/chat/completions";

DHT dht(DHT_PIN, DHT_TYPE);

// 保存最近几轮对话
DynamicJsonDocument conversation(12000);

void connectWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.print("Connecting WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi connected");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
}

void clearConversation() {
  conversation.clear();

  JsonArray messages = conversation["messages"].to<JsonArray>();

  JsonObject systemMessage = messages.add<JsonObject>();
  systemMessage["role"] = "system";
  systemMessage["content"] =
    "你是一个温湿度监测助手。"
    "用户会提供实时温度和湿度数据，也会提出普通问题。"
    "只有在用户明确给出温湿度正常范围、判断标准或要求时，"
    "才按照用户提供的标准判断。"
    "如果用户没有给出标准，不要擅自假设正常范围，"
    "应询问用户希望采用什么标准。"
    "回答使用中文，简洁准确，不要编造传感器数据。";
}

void askDeepSeek(const String& question,
                 float temperature,
                 float humidity) {
  WiFiClientSecure client;
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

  JsonArray messages = conversation["messages"];

  JsonObject userMessage = messages.add<JsonObject>();
  userMessage["role"] = "user";

  String prompt =
    "用户问题：\n" + question +
    "\n\n当前实时传感器数据：\n" +
    "温度：" + String(temperature, 1) + " 摄氏度\n" +
    "湿度：" + String(humidity, 1) + " %\n\n" +
    "请结合当前对话上下文回答。";

  userMessage["content"] = prompt;

  DynamicJsonDocument requestDoc(12000);
  requestDoc["model"] = "deepseek-chat";
  requestDoc["temperature"] = 0.2;
  requestDoc["messages"] = messages;

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
      Serial.println("响应解析失败");
      http.end();
      return;
    }

    const char* answer =
      responseDoc["choices"][0]["message"]["content"];

    Serial.println();
    Serial.println("DeepSeek：");
    Serial.println(answer);
    Serial.println();

    // 保存模型回复，使后续问题可以继续上下文对话
    JsonObject assistantMessage = messages.add<JsonObject>();
    assistantMessage["role"] = "assistant";
    assistantMessage["content"] = answer;
  } else {
    Serial.print("请求失败，HTTP 状态码：");
    Serial.println(httpCode);
    Serial.println(response);

    // 请求失败时删除刚刚添加的用户消息
    messages.remove(messages.size() - 1);
  }

  http.end();
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  dht.begin();
  connectWiFi();
  clearConversation();

  Serial.println();
  Serial.println("可以开始与 DeepSeek 对话");
  Serial.println("输入 /clear 可以清除当前对话");
}

void loop() {
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

    float humidity = dht.readHumidity();
    float temperature = dht.readTemperature();

    if (isnan(temperature) || isnan(humidity)) {
      Serial.println("DHT11 读取失败，请检查接线");
      return;
    }

    askDeepSeek(question, temperature, humidity);
  }

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi 已断开，正在重新连接...");
    connectWiFi();
  }
}