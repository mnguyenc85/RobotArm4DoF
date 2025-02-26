/*
  Kết nối WiFi
  Chuyển tiếp bản tin từ WiFi sang cổng COM1
*/

#include <SimpleTimer.h>
#include <ESP8266WiFi.h>

// #define DEBUG 1

char wifi_ssid[16] = "NCM_Test";
char wifi_pw[16] = "0846056084";
WiFiServer server(8888);
WiFiClient client;
bool has_client = 0;
byte wifi_delay = 1;

byte LED_on = 0;
SimpleTimer timer1(500);
SimpleTimer timer2(500);

char commBuf[128];

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);

  Serial.begin(115200);

#ifdef DEBUG
  while (!Serial) { ; }
  Serial.println("Ready");
#else
  delay(1000);
#endif
}

void loop() {
  if (timer1.isReady()) {
    LED_off();
    RS232_routine();
    timer1.reset();
  }

  if (timer2.isReady()) {
    // WiFi routine
    if (WiFi.isConnected()) {
      if (server.status() == CLOSED) {
#ifdef DEBUG        
        Serial.print("\nWiFi ");
        Serial.println(WiFi.localIP());
#endif
        server.begin();
      }
      else WiFi_handle();
    }
    else {
      if (wifi_delay > 0) wifi_delay--;
      else WiFi_connect();
    }

    timer2.reset();
  }
}

void WiFi_connect() {
  if (WiFi.getMode() != WIFI_STA) {
    WiFi.mode(WIFI_STA);
    WiFi.begin(wifi_ssid, wifi_pw);
#ifdef DEBUG
    Serial.print("Connecting");
#endif
  }
#ifdef DEBUG
  if (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
  }
#endif
}

void WiFi_stop() {
  server.stop();
  WiFi.disconnect();
  wifi_delay = 6;         // 6 * 500ms = 3s
#ifdef DEBUG
  Serial.print("WiFi off");
#endif
}

void WiFi_handle() {
  if (!client) { 
    if (has_client) {
#ifdef DEBUG      
      Serial.println("Client off");
#endif
      LED_blink();
      has_client = 0;
    }
    client = server.available();
  }
  
  if (client) {
    if (client.connected()) {
      if (!has_client) {
#ifdef DEBUG
        Serial.print("Client ");
        Serial.println(client.remoteIP());
#endif
        LED_blink();
        has_client = 1;
      }
      WiFi_read_msg();
    }
  }
}

void WiFi_read_msg() {
  if (client.available()) {
    int count = 0;
    while (client.available()) {
      Serial.write(client.read());
      count++;
    }
  }
}

void RS232_routine() {
  if(Serial.available()) {
    byte i = -1;
    while (Serial.available()) {
      char c = Serial.read();
      if (c == ':') i = 0;
      else if (c == ';') {
        if (i > 0) {
          // Serial.println(i);
          // Serial.write(commBuf, i);
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
  if (commBuf[0] == 'e') {
    int addr = x2i(&commBuf[1], 2);
    switch (addr) {
      case 0:
        Serial.println(WiFi.localIP());
        Serial.println(wifi_ssid);
        Serial.println(wifi_pw);
        break;
      case 1:
        string4chars(wifi_ssid, &commBuf[3], n - 3);
        break;
      case 2:
        string4chars(wifi_pw, &commBuf[3], n - 3);
        break;
      case 3:
        WiFi_stop();
        break;
    }
  }
  else {
    if (client && client.connected()) {
      client.write(commBuf, n);
    }
  }
}

void string4chars(char *dst, char *src, int l) {
  int i = 0;
  while (i < l) {
    dst[i] = src[i];
    i++;
  }
  dst[i] = 0;
}

/* HEX String to int */
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
    digitalWrite(LED_BUILTIN, LOW);
    LED_on = 1;
  }
  LED_on++;
}

void LED_off() {
  if (LED_on > 0) {
    LED_on--;
    if (LED_on <= 0) digitalWrite(LED_BUILTIN, HIGH);
  }
}
