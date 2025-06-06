#include <Arduino.h>
#include <FastLED.h>

#define LED_PIN 18       // WS2812 控制信号引脚
#define NUM_LEDS 30       // 灯带灯珠数量
#define BRIGHTNESS 100   // 灯带亮度 (0-255)
#define LED_TYPE WS2812B // 灯带型号
#define COLOR_ORDER GRB  // WS2812 默认排列

CRGB leds[NUM_LEDS];

// 效果模式
enum EffectMode {
  RAINBOW_FLOW,      // 彩虹流动
  BREATHING,         // 呼吸灯
  WAVE,             // 波浪效果
  STATIC_COLOR,     // 静态颜色
  TWINKLE,          // 闪烁效果
  CHASE,            // 追逐效果
  FIRE,             // 火焰效果
  METEOR,           // 流星雨
  RAINBOW_CYCLE,    // 彩虹循环
  PLASMA,           // 等离子效果
  LIGHTNING,        // 闪电效果
  MATRIX_RAIN,      // 矩阵雨
  KNIGHT_RIDER,     // 霹雳游侠
  COMET,            // 彗星效果
  SPARKLE_POP       // 爆裂闪烁
};

EffectMode currentEffect = RAINBOW_FLOW;
unsigned long lastEffectChange = 0;
const unsigned long EFFECT_DURATION = 3000; // 每种效果持续3秒

void setup()
{
  Serial.begin(115200);
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(BRIGHTNESS);  Serial.println("WS2812 华丽灯带控制程序启动");
  Serial.println("包含15种华丽效果，每3秒自动切换");
}

void rainbowFlow() {
  static uint8_t hue = 0;
  static uint8_t direction = 1;
  
  for (int i = 0; i < NUM_LEDS; i++) {
    // 更流畅的彩虹效果，增加方向变化
    uint8_t pixelHue = hue + (i * 256 / NUM_LEDS);
    leds[i] = CHSV(pixelHue, 255, 255);
  }
  
  hue += direction * 2;
  if (hue > 250 || hue < 5) direction *= -1; // 反向流动
}

void breathingEffect() {
  static bool increasing = true;
  static uint8_t brightness = 0;
  
  if (increasing) {
    brightness += 2;
    if (brightness >= 255) {
      brightness = 255;
      increasing = false;
    }
  } else {
    brightness -= 2;
    if (brightness <= 0) {
      brightness = 0;
      increasing = true;
    }
  }
  
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CHSV(160, 255, brightness); // 蓝色呼吸
  }
}

void waveEffect() {
  static float phase = 0;
  phase += 0.1;
  
  for (int i = 0; i < NUM_LEDS; i++) {
    uint8_t brightness = (sin(phase + i * 0.5) + 1) * 127;
    leds[i] = CHSV(96, 255, brightness); // 绿色波浪
  }
}

void staticColor() {
  static uint8_t colorIndex = 0;
  static unsigned long lastColorChange = 0;
  
  if (millis() - lastColorChange > 2000) { // 每2秒换颜色
    colorIndex = (colorIndex + 60) % 255;
    lastColorChange = millis();
  }
  
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CHSV(colorIndex, 255, 255);
  }
}

void twinkleEffect() {
  // 随机闪烁
  if (random8() < 50) { // 约20%的概率
    int led = random8(NUM_LEDS);
    leds[led] = CHSV(random8(), 255, 255);
  }
  
  // 逐渐变暗
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i].fadeToBlackBy(20);
  }
}

void chaseEffect() {
  static int position = 0;
  static unsigned long lastMove = 0;
  static uint8_t hue = 0;
  
  if (millis() - lastMove > 150) { // 每150ms移动一次
    fill_solid(leds, NUM_LEDS, CRGB::Black);
    
    // 多彩追逐效果
    for (int i = 0; i < 5; i++) {
      int pos = (position + i) % NUM_LEDS;
      uint8_t brightness = 255 - (i * 50);
      leds[pos] = CHSV(hue + (i * 30), 255, brightness);
    }
    
    position = (position + 1) % NUM_LEDS;
    hue += 3;
    lastMove = millis();
  }
}

// 新增华丽效果函数
void fireEffect() {
  static byte heat[NUM_LEDS];
  
  // 降温
  for (int i = 0; i < NUM_LEDS; i++) {
    heat[i] = qsub8(heat[i], random8(0, ((55 * 10) / NUM_LEDS) + 2));
  }
  
  // 热量向上传播
  for (int k = NUM_LEDS - 1; k >= 2; k--) {
    heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2]) / 3;
  }
  
  // 底部随机点燃
  if (random8() < 120) {
    int y = random8(7);
    heat[y] = qadd8(heat[y], random8(160, 255));
  }
  
  // 映射到颜色
  for (int j = 0; j < NUM_LEDS; j++) {
    CRGB color = HeatColor(heat[j]);
    leds[j] = color;
  }
}

void meteorEffect() {
  static int meteorPos = -10;
  static uint8_t meteorHue = 0;
  
  // 淡化所有像素
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i].fadeToBlackBy(20);
  }
  
  // 绘制流星
  for (int i = 0; i < 8; i++) {
    int pos = meteorPos - i;
    if (pos >= 0 && pos < NUM_LEDS) {
      uint8_t brightness = 255 - (i * 30);
      leds[pos] = CHSV(meteorHue, 255, brightness);
    }
  }
  
  meteorPos++;
  if (meteorPos > NUM_LEDS + 10) {
    meteorPos = -10;
    meteorHue += 60; // 换颜色
  }
}

void rainbowCycleEffect() {
  static uint16_t j = 0;
  
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CHSV(((i * 256 / NUM_LEDS) + j) & 255, 255, 255);
  }
  j += 2;
}

void plasmaEffect() {
  static float phase1 = 0, phase2 = 0, phase3 = 0;
  
  phase1 += 0.1;
  phase2 += 0.05;
  phase3 += 0.07;
  
  for (int i = 0; i < NUM_LEDS; i++) {
    float v1 = sin(phase1 + i * 0.3);
    float v2 = sin(phase2 + i * 0.2);
    float v3 = sin(phase3 + i * 0.1);
    
    uint8_t brightness = (v1 + v2 + v3 + 3) * 42;
    uint8_t hue = (v1 + v2) * 127 + 128;
    
    leds[i] = CHSV(hue, 255, brightness);
  }
}

void lightningEffect() {
  static unsigned long lastFlash = 0;
  static bool flashing = false;
  static int flashCount = 0;
  
  if (!flashing && millis() - lastFlash > random(2000, 5000)) {
    flashing = true;
    flashCount = random(1, 4);
    lastFlash = millis();
  }
  
  if (flashing) {
    if (flashCount > 0 && millis() - lastFlash > random(50, 200)) {
      // 闪电效果
      fill_solid(leds, NUM_LEDS, CRGB::White);
      FastLED.show();
      delay(random(10, 50));
      fill_solid(leds, NUM_LEDS, CRGB::Black);
      flashCount--;
      lastFlash = millis();
    }
    
    if (flashCount <= 0) {
      flashing = false;
    }
  } else {
    // 平时保持暗蓝色
    for (int i = 0; i < NUM_LEDS; i++) {
      leds[i] = CRGB(0, 0, 20);
    }
  }
}

void matrixRainEffect() {
  static bool drops[NUM_LEDS];
  static uint8_t brightness[NUM_LEDS];
  
  // 随机生成新雨滴
  if (random8() < 30) {
    int pos = random8(NUM_LEDS);
    drops[pos] = true;
    brightness[pos] = 255;
  }
  
  // 更新雨滴
  for (int i = 0; i < NUM_LEDS; i++) {
    if (drops[i]) {
      leds[i] = CRGB(0, brightness[i], 0); // 绿色
      brightness[i] = brightness[i] * 0.9; // 逐渐变暗
      
      if (brightness[i] < 10) {
        drops[i] = false;
      }
    } else {
      leds[i] = CRGB::Black;
    }
  }
}

void knightRiderEffect() {
  static int position = 0;
  static int direction = 1;
  static unsigned long lastMove = 0;
  
  if (millis() - lastMove > 100) {
    fill_solid(leds, NUM_LEDS, CRGB::Black);
    
    // 红色扫描
    for (int i = 0; i < 4; i++) {
      int pos = position + (i * direction);
      if (pos >= 0 && pos < NUM_LEDS) {
        uint8_t brightness = 255 - (i * 60);
        leds[pos] = CRGB(brightness, 0, 0);
      }
    }
    
    position += direction;
    if (position >= NUM_LEDS - 3 || position <= 0) {
      direction *= -1;
    }
    
    lastMove = millis();
  }
}

void cometEffect() {
  static int cometPos = 0;
  static uint8_t cometHue = 0;
  
  // 淡化尾迹
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i].fadeToBlackBy(15);
  }
  
  // 彗星头部
  leds[cometPos] = CHSV(cometHue, 255, 255);
  
  // 彗星尾巴
  for (int i = 1; i < 6 && cometPos - i >= 0; i++) {
    uint8_t brightness = 255 - (i * 40);
    leds[cometPos - i] += CHSV(cometHue, 255, brightness);
  }
  
  cometPos++;
  if (cometPos >= NUM_LEDS) {
    cometPos = 0;
    cometHue += 30;
  }
}

void sparklePopEffect() {
  static unsigned long lastPop = 0;
  
  // 所有LED逐渐变暗
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i].fadeToBlackBy(8);
  }
  
  // 随机爆裂
  if (millis() - lastPop > random(100, 500)) {
    int center = random8(NUM_LEDS);
    uint8_t hue = random8();
    
    // 中心最亮
    leds[center] = CHSV(hue, 255, 255);
    
    // 周围扩散
    for (int i = 1; i < 4; i++) {
      if (center - i >= 0) leds[center - i] = CHSV(hue, 255, 255 - i * 50);
      if (center + i < NUM_LEDS) leds[center + i] = CHSV(hue, 255, 255 - i * 50);
    }
    
    lastPop = millis();
  }
}

void switchEffect() {
  if (millis() - lastEffectChange > EFFECT_DURATION) {
    currentEffect = (EffectMode)((currentEffect + 1) % 15); // 更新为15种效果
    lastEffectChange = millis();
    
    // 清空灯带
    fill_solid(leds, NUM_LEDS, CRGB::Black);
    FastLED.show();
    delay(100);
    
    // 输出当前效果
    Serial.print("切换到效果: ");
    switch (currentEffect) {
      case RAINBOW_FLOW: Serial.println("彩虹流动"); break;
      case BREATHING: Serial.println("呼吸灯"); break;
      case WAVE: Serial.println("波浪效果"); break;
      case STATIC_COLOR: Serial.println("静态颜色"); break;
      case TWINKLE: Serial.println("闪烁效果"); break;
      case CHASE: Serial.println("追逐效果"); break;
      case FIRE: Serial.println("火焰效果"); break;
      case METEOR: Serial.println("流星雨"); break;
      case RAINBOW_CYCLE: Serial.println("彩虹循环"); break;
      case PLASMA: Serial.println("等离子效果"); break;
      case LIGHTNING: Serial.println("闪电效果"); break;
      case MATRIX_RAIN: Serial.println("矩阵雨"); break;
      case KNIGHT_RIDER: Serial.println("霹雳游侠"); break;
      case COMET: Serial.println("彗星效果"); break;
      case SPARKLE_POP: Serial.println("爆裂闪烁"); break;
    }
  }
}

void loop()
{
  switchEffect(); // 检查是否需要切换效果
    // 执行当前效果
  switch (currentEffect) {
    case RAINBOW_FLOW:
      rainbowFlow();
      break;
    case BREATHING:
      breathingEffect();
      break;
    case WAVE:
      waveEffect();
      break;
    case STATIC_COLOR:
      staticColor();
      break;
    case TWINKLE:
      twinkleEffect();
      break;
    case CHASE:
      chaseEffect();
      break;
    case FIRE:
      fireEffect();
      break;
    case METEOR:
      meteorEffect();
      break;
    case RAINBOW_CYCLE:
      rainbowCycleEffect();
      break;
    case PLASMA:
      plasmaEffect();
      break;
    case LIGHTNING:
      lightningEffect();
      return; // 闪电效果自己控制FastLED.show()
    case MATRIX_RAIN:
      matrixRainEffect();
      break;
    case KNIGHT_RIDER:
      knightRiderEffect();
      break;
    case COMET:
      cometEffect();
      break;
    case SPARKLE_POP:
      sparklePopEffect();
      break;
  }
  
  FastLED.show();
  delay(50); // 控制刷新率
}
