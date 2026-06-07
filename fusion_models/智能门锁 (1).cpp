#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include <WebServer.h>

// ========== 引脚 保持你原来不变 ==========
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 oled(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

#define SERVO_PIN  17
#define BEEP_PIN   15
#define KEY_PIN    7

// ========== WiFi 改成你自己的 ==========
const char* ssid     = "HONOR 200";
const char* password = "1234567890";

WebServer server(80);
bool isLocked = true;

// 舵机底层驱动
void servoWrite(int angle) {
  long us = map(angle, 0, 180, 500, 2400);
  for (int i = 0; i < 25; i++) {
    digitalWrite(SERVO_PIN, HIGH);
    delayMicroseconds(us);
    digitalWrite(SERVO_PIN, LOW);
    delay(20);
  }
}

// 屏幕刷新
void updateScreen() {
  oled.clearDisplay();
  oled.setCursor(25, 22);
  if (isLocked) {
    oled.println("LOCKED");
  } else {
    oled.println("OPEN");
  }
  oled.display();
}

// 执行开锁动作
void unlockDoor() {
  isLocked = false;
  servoWrite(0);
  digitalWrite(BEEP_PIN, LOW);
  delay(200);
  digitalWrite(BEEP_PIN, HIGH);
  updateScreen();
}

// 执行关锁动作
void lockDoor() {
  isLocked = true;
  servoWrite(90);
  digitalWrite(BEEP_PIN, HIGH);
  updateScreen();
}

// 网页主页
void handleRoot() {
  String page = "<html><body style='text-align:center;'>";
  page += "<h2>WiFi 智能门锁</h2>";
  page += "<p><a href='/unlock'><button style='font-size:24px;padding:10px 30px;'>开锁 OPEN</button></a></p>";
  page += "<p><a href='/lock'><button style='font-size:24px;padding:10px 30px;'>关锁 LOCK</button></a></p>";
  page += "</body></html>";
  server.send(200, "text/html", page);
}

void handleUnlock() {
  unlockDoor();
  server.send(200,"text/html","已开锁 <a href='/'>返回</a>");
}

void handleLock() {
  lockDoor();
  server.send(200,"text/html","已关锁 <a href='/'>返回</a>");
}

void setup() {
  Serial.begin(115200);

  // 外设初始化 和你原来一模一样
  pinMode(SERVO_PIN, OUTPUT);
  pinMode(BEEP_PIN, OUTPUT);
  pinMode(KEY_PIN, INPUT_PULLUP);
  digitalWrite(BEEP_PIN, HIGH);

  // OLED
  Wire.begin(8, 9);
  oled.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  oled.clearDisplay();
  oled.setTextSize(2);
  oled.setTextColor(WHITE);

  servoWrite(90);
  updateScreen();

  // 连接WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("WiFi已连上，手机浏览器输入：");
  Serial.println(WiFi.localIP());

  // 网页路由
  server.on("/", handleRoot);
  server.on("/unlock", handleUnlock);
  server.on("/lock", handleLock);
  server.begin();
}

void loop() {
  server.handleClient();

  // 保留你原来的按键功能 不变
  int keyVal = digitalRead(KEY_PIN);
  if (keyVal == LOW) {
    delay(30);
    while(digitalRead(KEY_PIN) == LOW);
    delay(30);

    isLocked = !isLocked;
    if (isLocked) {
      lockDoor();
    } else {
      unlockDoor();
    }
  }
}