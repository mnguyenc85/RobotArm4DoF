/* Board Arduino Uno
*/

#define OLED96 1

#include <Servo.h>
#ifdef OLED96
  #include <SPI.h>
  #include <Wire.h>
  #include <Adafruit_SSD1306.h>
  #include <Adafruit_GFX.h>
#endif

#define MAXSPD 20
#define M1_PWM 4
#define M2_PWM 5
#define M3_PWM 6
#define M4_PWM 8
#define M1_MIN 0
#define M1_MAX 180
#define M2_MIN 0
#define M2_MAX 180
#define M3_MIN 0
#define M3_MAX 180
#define M4_MIN 0x40
#define M4_MAX 0x90

Servo motors[4] = {Servo(), Servo(), Servo(), Servo()};
byte memory[] = {90, 90, 90, 90, 1, 1, 1, 1};
unsigned long motor_tmr;

#ifdef OLED96
  #define SCR_WIDTH 128
  #define SCR_HEIGHT 64
  #define OLED_RESET -1
  #define SCR_ADDR 0x3C
  Adafruit_SSD1306 display(SCR_WIDTH, SCR_HEIGHT, &Wire, OLED_RESET);
  bool OLED_ready = 0;
#endif

char commBuf[128];

byte LED_on = 0;
unsigned long tmr1;

#ifdef OLED96
void OLED_clearLine(int i) {
  display.fillRect(0, i * 8, SCR_WIDTH, 8, 0);
}
void OLED_print(String s, int i) {
  display.setCursor(0, i * 8);
  display.print(s);
}

void handleWiFiCmd(int n) {
  int addr = x2i(&commBuf[1], 2);
  switch (addr) {
    case 0:
      OLED_clearLine(0);
      OLED_print("My IP:", 0);
      OLED_print(str4chars(&commBuf[3], n - 3), 1);
      display.display();
      break;
    case 3:
      OLED_clearLine(0);
      OLED_print("Connecting...", 0);
      display.display();
      break;
    case 4:
      OLED_clearLine(1);
      OLED_print("Disconnected", 1);
      display.display();
      break;
    case 5:
      OLED_print("Client:", 2);
      OLED_print(str4chars(&commBuf[3], n - 3), 3);
      OLED_clearLine(4);
      display.display();
      break;
    case 6:
      OLED_print("disconnected", 4);
      display.display();
      break;
  }
}
#endif

void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);

  motors[0].attach(M1_PWM);
  motors[1].attach(M2_PWM);
  motors[2].attach(M3_PWM);
  motors[3].attach(M4_PWM);

  motor_tmr = millis();
  tmr1 = millis();

#ifdef OLED96
  // initialize OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCR_ADDR)) {
    LED_blink();
  }
  else {
    OLED_ready = 1;
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.clearDisplay();
    OLED_print("Ready!", 0);
    display.display();
  }
#endif
}

void loop() {
  unsigned long t = millis();

  if (t - motor_tmr >= 40) {
    for (byte i = 0; i < 4; i++) Motor_update(i);

    motor_tmr = t;
  }

  if (t - tmr1 >= 200) {
    LED_off();
    RS232_comm();
    tmr1 = t;
  }
}

void Motor_update(byte i) {
  int pos = motors[i].read();
  int target = memory[i];
  if (pos < target) {
    pos += memory[i + 4];
    if (pos > target) pos = target;
    motors[i].write(pos);
  }
  else if (pos > target) {
    pos -= memory[i + 4];
    if (pos < target) pos = target;
    motors[i].write(pos);
  }
}

void LED_blink() {
  if (LED_on <= 0) {
    digitalWrite(LED_BUILTIN, HIGH);
    LED_on = 1;
  }
  LED_on++;
}

void LED_off() {
  if (LED_on > 0) {
    LED_on--;
    if (LED_on <= 0) digitalWrite(LED_BUILTIN, LOW);
  }
}

void RS232_comm() {
  if(Serial.available()) {
    byte i = -1;
    while (Serial.available()) {
      char c = Serial.read();
      if (c == ':') i = 0;
      else if (c == ';') {
        if (i > 0) {
          // Serial.println(i);
          // Serial.write(CommBuf, i);
          // Serial.println();
          process_cmd(i);
          i = 0;
        }      
      }
      else if (i >= 0) {
        commBuf[i] = c;
        i++;
      }
    }
    LED_blink();
  }
}

void process_cmd(int n) {
  switch (commBuf[0]) {
    case '1':
      if (n == 5) {
        int addr = x2i(&commBuf[1], 2);
        int data = x2i(&commBuf[3], 2);
        if (Memory_writeSingle(addr, data)) Serial.print(":a;");
        else Serial.print(":f01;");
      }
      else Serial.print(":f01;");
      break;
    case '4':
      if (n > 2) {
        int addr = x2i(&commBuf[1], 2);
        Memory_readSingle(addr);
      }
      else Serial.print(":f01;");
      break;
    case '5':
      if (n > 4) {
        int addr = x2i(&commBuf[1], 2);
        int n = x2i(&commBuf[3], 2);
        if (addr + n < 24) Memory_readMulti(addr, n);
        else Serial.print(":f02;");
      }
      else Serial.print(":f01;");
      break;
#ifdef OLED96
    case 'e':
      if (n > 2) handleWiFiCmd(n);
      //else Serial.print(":f01;");
      break;
#endif
  }
}

bool Memory_writeSingle(int addr, int data) {
  if (addr < 4) return 0;
  else if (addr < 8) {
    int i = addr - 4;
    // Limit
    switch (i) {
      case 0:
        data = max(min(data, M1_MAX), M1_MIN);
        break;
      case 1:
        data = max(min(data, M2_MAX), M2_MIN);
        break;
      case 2:
        data = max(min(data, M3_MAX), M3_MIN);
        break;
      case 3:
        data = max(min(data, M4_MAX), M4_MIN);
        break;
    }
    memory[i] = data;
    return 1;
  }
  else if (addr < 12) {
    if (data < 1) data = 1;
    else if (data > MAXSPD) data = MAXSPD;
    memory[addr - 4] = data;
    return 1;
  }
  return 0;
}

void Memory_readSingle(int addr) {
  uint8_t buf[] = { 58, 52, 48, 48, 48, 48, 59 };
  int v = readMem(addr);
  buf[2] = commBuf[1];          // i2x1(addr >> 4);
  buf[3] = commBuf[2];          // i2x1(addr);
  buf[4] = i2x1(v >> 4);
  buf[5] = i2x1(v);
  Serial.write(buf, 7);
}

void Memory_readMulti(int addr, int n) {
  int l = n * 2 + 7;
  uint8_t buf[l] = {};
  buf[0] = ':';
  buf[1] = '5';
  buf[2] = commBuf[1];
  buf[3] = commBuf[2];
  buf[4] = commBuf[3];
  buf[5] = commBuf[4];
  int j = 6;
  for (byte i = 0; i < n; i++) {
    int v = readMem(addr + i);
    buf[j++] = i2x1(v >> 4);
    buf[j++] = i2x1(v);
  }
  buf[l - 1] = ';';
  Serial.write(buf, l);
}

int readMem(int i) {
  if (i < 0) return 0;
  else if (i < 4) return motors[i].read();
  else if (i < 8) return memory[i - 4];
  else if (i < 12) return memory[i - 4];
  else if (i < 16) return 0;
  else if (i == 16) return M1_MIN;
  else if (i == 17) return M2_MIN;
  else if (i == 18) return M3_MIN;
  else if (i == 19) return M4_MIN;
  else if (i == 20) return M1_MAX;
  else if (i == 21) return M2_MAX;
  else if (i == 22) return M3_MAX;
  else if (i == 23) return M4_MAX;
  else return 0;
}

String str4chars(char *src, int l) {
  char tmp[l + 1];
  int i = 0;
  while (i < l) {
    tmp[i] = src[i];
    i++;
  }
  tmp[i] = 0;
  return String(tmp);
}

uint8_t i2x1(int v) {
  v = v & 0xf;
  if (v >= 0 && v <= 9) return v + 48;
  else if (v < 17) return v + 87;
  else return 48;
}

int x2i(char *s, byte n) {
  int x = 0;
  for (byte i = 0; i < n; i++) {
    char c = *s;
    if (c >= '0' && c <= '9') {
      x *= 16;
      x += c - '0';
    }
    else if (c >= 'a' && c < 'g') {
      x *= 16;
      x += (c - 'a') + 10;
    }
    else if (c >= 'A' && c < 'G') {
      x *= 16;
      x += (c - 'A') + 10;
    }
    else break;
    s++;
  }
  return x;
}
