#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <esp_task_wdt.h>
// Định nghĩa chân kết nối
#define CE_PIN   9
#define CSN_PIN  10
#define RF24_DEFAULT_CHANNEL 100
#define WDT_TIMEOUT 5       // Timeout watchdog = 5 giây
// Khởi tạo đối tượng RF24
// SPI hardware mặc định ESP32: SCK=18, MISO=19, MOSI=23
RF24 radio(CE_PIN, CSN_PIN);

// Địa chỉ pipe 
/*
Địa chỉ (Address width) có thể chọn 3, 4 hoặc 5 bytes.
Mặc định nhiều thư viện (như RF24) dùng 5 bytes vì nó đảm bảo tính duy nhất và độ nhiễu thấp.
Địa chỉ phải là dãy byte nhị phân → không bắt buộc phải là chuỗi ASCII (có thể là 0xE7, 0xE7, 0xE7, 0xE7, 0xE7).
*/
const uint8_t ADDRESS[5] = { 'N', 'o', 'd', 'e', '1' };
struct DataPacket {
  uint16_t VD1; // tốc độ động cơ trái
  uint16_t VD2; // tốc độ động cơ phải
};
void setup() {
  esp_task_wdt_init(WDT_TIMEOUT, true); 
  esp_task_wdt_add(NULL);      

  Serial.begin(115200);
  if (!radio.begin()) {
    Serial.println("Radio không khởi động được!");
    while (1);
  }
  else {
    Serial.println("Radio khởi động OK!");
  }


  /*
  Gán địa chỉ 5 byte cho TX pipe (pipe 0).
  */
  radio.openReadingPipe(1, ADDRESS); 

  // Cấu hình công suất phát (E01-ML01DP5 có PA → dùng MAX)
  // (LOW/MED/HIGH/MAX)
  radio.setPALevel(RF24_PA_MAX);

  // Chọn tốc độ truyền (250kbps = xa, 2Mbps = nhanh)
  radio.setDataRate(RF24_250KBPS);
  // Chọn kênh truyền (tần số 2400 + kênh MHz)
  // Kênh có thể chọn từ 0 → 125 (tương ứng 2400 → 2525 MHz)
  // Cần tránh các kênh WiFi (ví dụ kênh 6 = 2437 MHz)
  // Mặc định thư viện dùng kênh 76 (2476 MHz)  
  radio.setChannel(RF24_DEFAULT_CHANNEL);
  // Bất chế độ lắng nghe → bật RX mode;
  radio.setPayloadSize(sizeof(DataPacket)); // cố định 4 bytes

  radio.startListening();

}

void loop() {
  esp_task_wdt_reset(); 
  if (radio.available()) {
    DataPacket packet;
    // Đọc dữ liệu từ buffer
    radio.read(&packet, sizeof(packet));
    // In ra màn hình Serial
    Serial.print("Nhận được: VD1 = ");
    Serial.print(packet.VD1);
    Serial.print(" | VD2 = ");
    Serial.println(packet.VD2);
  }
}
