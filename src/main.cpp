#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <esp_task_wdt.h>  // Watchdog

// Chân NRF24L01 (bạn thay theo wiring)
#define CE_PIN   5
#define CSN_PIN  17

// Chân LED + Nút
#define LED_PIN  2
#define BTN_PIN  4

// Watchdog timeout (5 giây)
#define WDT_TIMEOUT 5

RF24 radio(CE_PIN, CSN_PIN);

const uint8_t ADDRESS[5] = { 'N', 'o', 'd', 'e', '1' };



// Struct dữ liệu gửi (4 byte)
struct DataPacket {
  uint16_t VD1;
  uint16_t VD2;
} __attribute__((packed));

DataPacket packet;  // biến toàn cục để gửi

// Các tần suất gửi (ms)
int intervals[] = {1000, 500, 250, 125};  // 1Hz, 2Hz, 4Hz, 8Hz
int mode = 0;  // index tần suất
unsigned long lastSend = 0;
unsigned long lastDebounce = 0;
bool ledState = false;

void IRAM_ATTR handleButton() {
  unsigned long now = millis();
  if (now - lastDebounce > 200) {  // chống dội nút
    mode = (mode + 1) % 4;         // chuyển vòng
    lastDebounce = now;
  }
}

void setup() {
  Serial.begin(115200);

  // Watchdog init
  esp_task_wdt_init(WDT_TIMEOUT, true);
  esp_task_wdt_add(NULL);

  // Nút nhấn
  pinMode(BTN_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(BTN_PIN), handleButton, FALLING);

  // LED PWM
  ledcSetup(0, 5000, 8);  // channel 0, freq=5kHz, 8-bit
  ledcAttachPin(LED_PIN, 0);

  // NRF24 init
  radio.begin();
  radio.openWritingPipe(ADDRESS);
  radio.setChannel(100);
  radio.setDataRate(RF24_250KBPS);
  radio.setPALevel(RF24_PA_MAX);
  radio.setPayloadSize(sizeof(DataPacket));  // 4 byte
  radio.stopListening();  // TX mode

  Serial.println("NRF24L01 TX ready");
}

void loop() {
  esp_task_wdt_reset();  // kick watchdog

  unsigned long now = millis();
  if (now - lastSend >= intervals[mode]) {
    lastSend = now;

    // Gán dữ liệu vào struct
    packet.VD1 = random(0, 1024);  // ví dụ giá trị VD1
    packet.VD2 = random(0, 1024);  // ví dụ giá trị VD2

    // Gửi gói struct (4 byte)
    bool ok = radio.write(&packet, sizeof(packet));
    Serial.print("Send: VD1="); Serial.print(packet.VD1);
    Serial.print(" VD2="); Serial.print(packet.VD2);
    Serial.print(" -> "); Serial.println(ok ? "OK" : "Fail");

    // LED nháy minh họa
    ledState = !ledState;
    ledcWrite(0, ledState ? 255 : 0);
  }
}
