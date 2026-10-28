#include <Arduino.h>
#include <U8g2lib.h>
#include <WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

// ==================== 配置区域 ====================
// 1. 请修改为你的 Wi-Fi 名称和密码
const char* ssid     = "Lc振荡电路";
const char* password = "lc230804";

// 2. OLED 引脚定义（与你的接线一致）
#define OLED_SCL 4
#define OLED_SDA 5

// ==================== 初始化对象 ====================
// 初始化 OLED
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE, /* clock=*/ OLED_SCL, /* data=*/ OLED_SDA);

// 初始化 NTP 客户端
WiFiUDP ntpUDP;
// 使用阿里云 NTP 服务器，更适合国内网络
NTPClient timeClient(ntpUDP, "ntp.aliyun.com", 8 * 3600, 60000); // 8*3600 表示东八区（北京时间）

void setup() {
  Serial.begin(115200);
  
  // 初始化 OLED
  u8g2.begin();
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x10_tn);
  u8g2.drawStr(0, 20, "Connecting WiFi...");
  u8g2.sendBuffer();

  // 连接 Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("\nWiFi Connected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // 初始化 NTP 客户端
  timeClient.begin();
  
  // 首次同步时间（可能需要几秒钟）
  u8g2.clearBuffer();
  u8g2.drawStr(0, 20, "Syncing Time...");
  u8g2.sendBuffer();
  
  timeClient.update();
}

void loop() {
  // 更新 NTP 时间（内部有 60 秒的缓存，不会频繁请求服务器）
  timeClient.update();

  // 获取格式化后的时间字符串，格式为 HH:MM:SS
  String formattedTime = timeClient.getFormattedTime();

  // 开始绘制屏幕
  u8g2.clearBuffer();
  
  // 设置大字体显示时间
  u8g2.setFont(u8g2_font_logisoso24_tn);
  
  // 计算居中位置（屏幕宽 128）
  // logisoso24 字体下，"HH:MM:SS" 大约宽 96 像素
  int x = (128 - 96) / 2; 
  u8g2.drawStr(x, 40, formattedTime.c_str());

  // 可选：在下方显示 Wi-Fi 状态或日期
  u8g2.setFont(u8g2_font_6x10_tn);
  u8g2.drawStr(0, 60, "WiFi: Connected");

  u8g2.sendBuffer();

  // 延时 100ms 刷新一次，避免闪烁
  delay(100);
}
