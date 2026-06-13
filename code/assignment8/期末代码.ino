#include <Adafruit_NeoPixel.h>

// --- 引脚定义与灯带参数配置 ---
#define LED_PIN    18          // WS2812 数据引脚连接到 GPIO18
#define LED_COUNT  30           // 需要控制的灯珠数量，如果是灯带请改为实际数量
#define LED_BRIGHTNESS 25      // 总体亮度 (0 - 255)，建议从较低亮度开始测试

// 创建 NeoPixel 对象，参数：灯珠数, 引脚, 灯珠类型
// NEO_GRB + NEO_KHZ800 是 WS2812B 最常用的规格，具体可查你购买灯带的说明
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

#define LDR_PIN     4         // 亮度传感器OUT引脚
#define FSR_PIN    5      // fsr薄膜压力传感器引脚

// 控制状态变量
bool ledIsOn = false;              // LED当前状态
bool pressureDetected = false;     // 用于检测压力的上升沿
unsigned long lastPressureTime = 0;// 最后一次检测到较大压力的时间
const unsigned long AUTO_OFF_TIME = 30000; // 自动熄灭时间30秒

void turnOnLED() {
  for (int i = 0; i < LED_COUNT; i++) {
    strip.setPixelColor(i, strip.Color(255, 140, 0));
  }
  strip.show();
  ledIsOn = true;
  lastPressureTime = millis();
  Serial.println("LED已点亮");
}

void turnOffLED() {
  strip.clear();
  strip.show();
  ledIsOn = false;
  Serial.println("LED已熄灭");
}

void setup() {
  strip.begin();                // 初始化 LED 灯带
  strip.setBrightness(LED_BRIGHTNESS); // 设置亮度
  strip.clear();                // 初始时熄灭LED
  strip.show();

  analogReadResolution(13);
  Serial.begin(115200);
  Serial.println("系统启动，LED初始熄灭");
}

void loop() {
  int ldrValue = analogRead(LDR_PIN);
  int brightness = ldrValue - 4096;
  if(brightness<0) brightness = 0;
  Serial.print("亮度传感器数值: ");
  Serial.print(brightness);
  
  bool isDark = false;
  if (brightness > 3000) {
    Serial.println("  —— 强光");
  } 
  else if (brightness > 1500) {
    Serial.println("  —— 正常光");
  } 
  else {
    Serial.println("  —— 弱光/黑暗");
    isDark = true;
  }
  
  int fsrValue = analogRead(FSR_PIN);
  bool isHighPressure = false;
  
  // 简单的压力等级判断
  if (fsrValue < 100) {
    Serial.println("压力状态: 无压力");
    pressureDetected = false;
  } else if (fsrValue < 2000) {
    Serial.println("压力状态: 轻微压力");
    pressureDetected = false;
  } else if (fsrValue < 5000) {
    Serial.println("压力状态: 中等压力");
    pressureDetected = false;
  } else {
    Serial.println("压力状态: 较大压力");
    isHighPressure = true;
  }

  // 控制逻辑
  if (isHighPressure && !pressureDetected) {
    // 检测到较大压力的上升沿
    pressureDetected = true;
    
    if (!ledIsOn) {
      // LED熄灭时，只有在弱光/黑暗下才点亮
      if (isDark) {
        turnOnLED();
      } else {
        Serial.println("环境太亮，不点亮LED");
      }
    } else {
      // LED点亮时，直接熄灭（不需要弱光条件）
      turnOffLED();
    }
  }

  // 自动熄灭检查
  if (ledIsOn && (millis() - lastPressureTime > AUTO_OFF_TIME)) {
    Serial.println("30秒无操作，自动熄灭");
    turnOffLED();
  }

  delay(500);
}
