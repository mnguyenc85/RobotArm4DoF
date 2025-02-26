#include <Servo.h>
#include <SimpleTimer.h>

#define MAXSPD 20
SimpleTimer timerMotor(40);        // max speed = 1deg/2ms == 20 => 40ms = 20 * 2ms

#define M1_PWM 4
#define M2_PWM 5
#define M3_PWM 6
#define M4_PWM 8

#define LED1_PIN 7

#define M1_MIN 0
#define M1_MAX 180
#define M2_MIN 16
#define M2_MAX 175
#define M3_MIN 32
#define M3_MAX 175
#define M4_MIN 0
#define M4_MAX 180

int mpos[4] = { 90, 90, 90, 90};
int mspd[4] = { 1, 1, 1, 1 };
Servo motors[4] = {Servo(), Servo(), Servo(), Servo()};

byte LED_on = 0;
byte LED1 = 0;
SimpleTimer timer1(200);

char commBuf[128];

void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(LED1_PIN, OUTPUT);

  motors[0].attach(M1_PWM);
  motors[1].attach(M2_PWM);
  motors[2].attach(M3_PWM);
  motors[3].attach(M4_PWM);

  delay(1000);
  LED_blink();

  timerMotor.reset();
  timer1.reset();
}

void loop() {
  if (timerMotor.isReady()) {
    for (byte i = 0; i < 4; i++) Motor_update(i);

    LED1++;
    if (LED1 == 10) {
      digitalWrite(LED1_PIN, HIGH);
    }
    else if (LED1 >= 20) {
      digitalWrite(LED1_PIN, LOW);
      LED1 = 0;
    }

    timerMotor.reset();
  }

  if (timer1.isReady()) {
    LED_off();
    RS232_comm();
    timer1.reset();
  }
}

void Motor_update(byte i) {
  int pos = motors[i].read();
  int target = mpos[i];
  if (pos < target) {
    pos += mspd[i];
    if (pos > target) pos = target;
    motors[i].write(pos);
  }
  else if (pos > target) {
    pos -= mspd[i];
    if (pos < target) pos = target;
    motors[i].write(pos);
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
        if (Motor_writeSingle(addr, data)) Serial.print(":a;");
        else Serial.print(":f01;");
      }
      else Serial.print(":f01;");
      break;
    case '4':
      if (n > 2) {
        int addr = x2i(&commBuf[1], 2);
        Motor_readSingle(addr);
      }
      else Serial.print(":f01;");
      break;
    case '5':
      if (n > 4) {
        int addr = x2i(&commBuf[1], 2);
        int n = x2i(&commBuf[3], 2);
        if (addr + n < 24) Motor_readMulti(addr, n);
        else Serial.print(":f02;");
      }
      else Serial.print(":f01;");
      break;
    case '7':
      timerMotor.reset();
      Serial.print(":a;");
      break;
  }
}

bool Motor_writeSingle(int addr, int data) {
  if (addr < 4) return 0;
  else if (addr < 8) {
    int i = addr - 4;
    // Limit
    if (i == 0) {
      if (data < M1_MIN) data = M1_MIN;
      else if (data > M1_MAX) data = M1_MAX;
    }
    if (i == 1) {
      if (data < M2_MIN) data = M2_MIN;
      else if (data > M2_MAX) data = M2_MAX;
    }
    if (i == 2) {
      if (data < M3_MIN) data = M3_MIN;
      else if (data > M3_MAX) data = M3_MAX;
    }
    if (i == 3) {
      if (data < M4_MIN) data = M4_MIN;
      else if (data > M4_MAX) data = M4_MAX;
    }
    mpos[i] = data;
    return 1;
  }
  else if (addr < 12) {
    if (data < 1) data = 1;
    else if (data > MAXSPD) data = MAXSPD;
    mspd[addr - 8] = data;
    return 1;
  }
  return 0;
}

void Motor_readSingle(int addr) {
  uint8_t buf[] = { 58, 52, 48, 48, 48, 48, 59 };
  int v = readMem(addr);
  buf[2] = commBuf[1];          // i2x1(addr >> 4);
  buf[3] = commBuf[2];          // i2x1(addr);
  buf[4] = i2x1(v >> 4);
  buf[5] = i2x1(v);
  Serial.write(buf, 7);
}

void Motor_readMulti(int addr, int n) {
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
  else if (i < 8) return mpos[i - 4];
  else if (i < 12) return mspd[i - 8];
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