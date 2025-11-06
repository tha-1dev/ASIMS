#ifndef ASIMS_CONFIG_H
#define ASIMS_CONFIG_H

// ข้อมูลเวอร์ชั่น
#define SYSTEM_VERSION "ASIMS v1.0"
#define BAUD_RATE 115200
#define UPDATE_RATE 10 // Hz

// ค่าคงที่ฮาร์ดแวร์
constexpr float VOLTAGE_DIVIDER_RATIO = 0.409f; // 4.7k/(6.8k+4.7k)
constexpr float ARDUINO_REF_VOLTAGE = 5.0f;
constexpr int ADC_RESOLUTION = 1024;

// ขีดจำกัดการป้องกัน
constexpr float MAX_SAFE_VOLTAGE = 5.0f;
constexpr float ABSOLUTE_MAX_VOLTAGE = 5.5f;

// ระดับขีดจำกัด
namespace Thresholds {
  constexpr float NORMAL = 1.5f;
  constexpr float MEDIUM = 3.0f;
  constexpr float HIGH = 4.0f;
  constexpr float CRITICAL = 4.8f;
}

// การกำหนดพิน
namespace Pins {
  constexpr int BUZZER = 8;
  constexpr int GREEN_LED = 9;
  constexpr int YELLOW_LED = 10;
  constexpr int RED_LED = 11;
  constexpr int BLUE_LED = 12;
  constexpr int ANALOG_IN = A0;
  constexpr int CALIBRATION_BTN = 2;
}

#endif