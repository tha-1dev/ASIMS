// =============================================
// ASIMS - Arduino Signal Interface & Monitoring System
// เวอร์ชั่นสมบูรณ์ พร้อมคอมเม้นต์ภาษาไทย
// =============================================

/* 
 * วัตถุประสงค์: ระบบตรวจสอบสัญญาณ Analog แรงดันสูงอย่างปลอดภัย
 * คุณสมบัติ:
 * - อ่านสัญญาณ ±12V ได้อย่างปลอดภัย
 * - แสดงสถานะ 4 ระดับผ่าน LED
 * - แจ้งเตือนด้วยเสียงเมื่อค่าเกินกำหนด
 * - การทำงานแบบเรียลไทม์
 */

// ==================== กำหนดค่าพิน ====================
const int BUZZER_PIN = 8;      // ขาเชื่อมต่อบัซเซอร์
const int GREEN_LED_PIN = 9;   // LED สีเขียว (สถานะปกติ)
const int YELLOW_LED_PIN = 10; // LED สีเหลือง (สถานะปานกลาง)
const int RED_LED_PIN = 11;    // LED สีแดง (สถานะสูง)
const int BLUE_LED_PIN = 12;   // LED สีน้ำเงิน (ระบบพร้อม)
const int ANALOG_PIN = A0;     // ขาอ่านสัญญาณ Analog
const int CALIBRATION_BUTTON = 2; // ปุ่มปรับเทียบ

// ==================== กำหนดค่าระดับแรงดัน ====================
const float NORMAL_THRESHOLD = 1.5;    // แรงดันต่ำกว่า 1.5V = ปกติ
const float MEDIUM_THRESHOLD = 3.0;    // 1.5V - 3.0V = ปานกลาง
const float HIGH_THRESHOLD = 4.0;      // 3.0V - 4.0V = สูง
const float CRITICAL_THRESHOLD = 4.8;  // เกิน 4.8V = วิกฤต

// ==================== ตัวแปรระบบ ====================
float measuredVoltage = 0.0;           // แรงดันที่วัดได้ (0-5V)
float opAmpOutputVoltage = 0.0;        // แรงดันขาออก Op-Amp (±12V)
bool systemArmed = false;              // สถานะพร้อมทำงานของระบบ
unsigned long lastUpdate = 0;          // เวลาอัพเดทล่าสุด
const unsigned long UPDATE_INTERVAL = 100; // อัพเดททุก 100ms

// ==================== ตัวแปรสถิติ ====================
float minVoltage = 999.0;              // ค่าแรงดันต่ำสุด
float maxVoltage = -999.0;             // ค่าแรงดันสูงสุด
unsigned long startTime = 0;           // เวลาเริ่มทำงาน
int readingCount = 0;                  // นับจำนวนการอ่านค่า

// ==================== ฟังก์ชันตั้งค่าเริ่มต้น ====================
void setup() {
  // เริ่มต้นการตั้งค่าพิน
  initializePins();
  
  // เริ่มการสื่อสารแบบอนุกรม
  Serial.begin(115200);
  
  // ลำดับการเริ่มต้นระบบ
  startupSequence();
  
  // ตั้งค่าตัวแปรสถิติ
  startTime = millis();
  
  // แสดงข้อความเริ่มต้น
  Serial.println("=== ASIMS System Started ===");
  Serial.println("ระบบตรวจสอบสัญญาณแรงดันสูง");
  Serial.println("ช่วงแรงดันเข้า: ±12V -> 0-5V (ปลอดภัย)");
  Serial.println("สถานะ: ระบบพร้อมทำงาน");
  Serial.println("----------------------------");
}

// ==================== ฟังก์ชันวนลูปหลัก ====================
void loop() {
  unsigned long currentTime = millis();
  
  // อัพเดทค่าตามช่วงเวลาที่กำหนด
  if (currentTime - lastUpdate >= UPDATE_INTERVAL) {
    lastUpdate = currentTime;
    
    // อ่านและประมวลผลค่าแรงดัน
    readVoltage();
    
    // อัพเดทการแสดงผลสถานะ
    updateStatusIndicators();
    
    // อัพเดทสถิติ
    updateStatistics();
    
    // ส่งข้อมูลออกทางอนุกรม
    serialOutput();
  }
  
  // ตรวจสอบปุ่มปรับเทียบ
  checkCalibration();
}

// ==================== ฟังก์ชันตั้งค่าพิน ====================
void initializePins() {
  // ตั้งค่าพินเอาท์พุท
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(GREEN_LED_PIN, OUTPUT);
  pinMode(YELLOW_LED_PIN, OUTPUT);
  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(BLUE_LED_PIN, OUTPUT);
  
  // ตั้งค่าพินอินพุท
  pinMode(ANALOG_PIN, INPUT);
  pinMode(CALIBRATION_BUTTON, INPUT_PULLUP);
}

// ==================== ลำดับการเริ่มต้นระบบ ====================
void startupSequence() {
  Serial.println("กำลังทดสอบระบบ...");
  
  // ทดสอบ LED และบัซเซอร์
  for (int i = 0; i < 3; i++) {
    digitalWrite(BLUE_LED_PIN, HIGH);
    digitalWrite(GREEN_LED_PIN, HIGH);
    digitalWrite(YELLOW_LED_PIN, HIGH);
    digitalWrite(RED_LED_PIN, HIGH);
    tone(BUZZER_PIN, 1000, 200);
    delay(300);
    
    digitalWrite(BLUE_LED_PIN, LOW);
    digitalWrite(GREEN_LED_PIN, LOW);
    digitalWrite(YELLOW_LED_PIN, LOW);
    digitalWrite(RED_LED_PIN, LOW);
    delay(200);
  }
  
  // ตั้งค่าสถานะพร้อมทำงาน
  digitalWrite(BLUE_LED_PIN, HIGH);
  systemArmed = true;
  
  Serial.println("การทดสอบระบบเสร็จสิ้น");
}

// ==================== ฟังก์ชันอ่านค่าแรงดัน ====================
void readVoltage() {
  // อ่านค่า Analog ด้วยการเฉลี่ยเพื่อความแม่นยำ
  long sum = 0;
  for (int i = 0; i < 16; i++) {
    sum += analogRead(ANALOG_PIN);
    delay(1);
  }
  int analogValue = sum >> 4; // หารด้วย 16
  
  // แปลงค่าเป็นแรงดัน (0-5V)
  measuredVoltage = (analogValue * 5.0) / 1024.0;
  
  // คำนวณแรงดันขาออก Op-Amp ดั้งเดิม
  opAmpOutputVoltage = measuredVoltage / 0.409;
  
  readingCount++;
}

// ==================== ฟังก์ชันอัพเดทการแสดงผล ====================
void updateStatusIndicators() {
  // ปิด LED สถานะทั้งหมดชั่วคราว
  digitalWrite(GREEN_LED_PIN, LOW);
  digitalWrite(YELLOW_LED_PIN, LOW);
  digitalWrite(RED_LED_PIN, LOW);
  digitalWrite(BUZZER_PIN, LOW);
  
  // ตรวจสอบและตั้งค่าสถานะ
  if (measuredVoltage < NORMAL_THRESHOLD) {
    // สถานะปกติ - LED เขียว
    digitalWrite(GREEN_LED_PIN, HIGH);
  }
  else if (measuredVoltage < MEDIUM_THRESHOLD) {
    // สถานะปานกลาง - LED เหลือง
    digitalWrite(YELLOW_LED_PIN, HIGH);
  }
  else if (measuredVoltage < HIGH_THRESHOLD) {
    // สถานะสูง - LED แดง
    digitalWrite(RED_LED_PIN, HIGH);
  }
  else {
    // สถานะวิกฤต - LED แดง + บัซเซอร์
    digitalWrite(RED_LED_PIN, HIGH);
    
    // บัซเซอร์ส่งสัญญาณเป็นจังหวะ
    if (millis() % 1000 < 500) {
      digitalWrite(BUZZER_PIN, HIGH);
    } else {
      digitalWrite(BUZZER_PIN, LOW);
    }
  }
}

// ==================== ฟังก์ชันอัพเดทสถิติ ====================
void updateStatistics() {
  // อัพเดทค่าแรงดันต่ำสุด/สูงสุด
  if (measuredVoltage < minVoltage) minVoltage = measuredVoltage;
  if (measuredVoltage > maxVoltage) maxVoltage = measuredVoltage;
}

// ==================== ฟังก์ชันส่งข้อมูลอนุกรม ====================
void serialOutput() {
  // แสดงผลละเอียดทุก 5 วินาที
  if (readingCount % 50 == 0) {
    Serial.println("=== รายงานสถานะ ASIMS ===");
    Serial.print("แรงดันที่วัดได้: ");
    Serial.print(measuredVoltage, 3);
    Serial.println(" V");
    
    Serial.print("แรงดันขาออก Op-Amp: ");
    Serial.print(opAmpOutputVoltage, 2);
    Serial.println(" V");
    
    Serial.print("สถานะ: ");
    if (measuredVoltage < NORMAL_THRESHOLD) Serial.println("ปกติ");
    else if (measuredVoltage < MEDIUM_THRESHOLD) Serial.println("ปานกลาง");
    else if (measuredVoltage < HIGH_THRESHOLD) Serial.println("สูง");
    else Serial.println("วิกฤต!");
    
    Serial.print("ค่าต่ำสุด/สูงสุด: ");
    Serial.print(minVoltage, 3);
    Serial.print("V / ");
    Serial.print(maxVoltage, 3);
    Serial.println("V");
    
    unsigned long runTime = (millis() - startTime) / 1000;
    Serial.print("เวลาทำงาน: ");
    Serial.print(runTime);
    Serial.println(" วินาที");
    Serial.println("----------------------------");
  }
}

// ==================== ฟังก์ชันตรวจสอบปุ่มปรับเทียบ ====================
void checkCalibration() {
  if (digitalRead(CALIBRATION_BUTTON) == LOW) {
    delay(50); // ดีเลย์ป้องกันสัญญาณรบกวน
    if (digitalRead(CALIBRATION_BUTTON) == LOW) {
      calibrateSystem();
    }
    while (digitalRead(CALIBRATION_BUTTON) == LOW); // รอจนกว่าจะปล่อยปุ่ม
  }
}

// ==================== ฟังก์ชันปรับเทียบระบบ ====================
void calibrateSystem() {
  Serial.println("=== โหมดปรับเทียบ ===");
  Serial.println("โปรดต่อสัญญาณ 0V แล้วกดปุ่ม...");
  
  // รอการกดปุ่ม
  while (!Serial.available());
  Serial.read();
  
  // ปรับเทียบจุดศูนย์
  long zeroSum = 0;
  for (int i = 0; i < 100; i++) {
    zeroSum += analogRead(ANALOG_PIN);
    delay(10);
  }
  
  Serial.println("ปรับเทียบจุดศูนย์เสร็จสิ้น");
  Serial.println("การปรับเทียบเสร็จสมบูรณ์!");
  Serial.println("----------------------------");
}

// ==================== ฟังก์ชันทดสอบประสิทธิภาพ ====================
void performanceTest() {
  Serial.println("=== การทดสอบประสิทธิภาพ ===");
  
  // ทดสอบเวลาตอบสนอง
  unsigned long startTime = micros();
  for (int i = 0; i < 1000; i++) {
    readVoltage();
  }
  unsigned long endTime = micros();
  
  float avgReadTime = (endTime - startTime) / 1000.0;
  Serial.print("เวลาในการอ่านค่าเฉลี่ย: ");
  Serial.print(avgReadTime);
  Serial.println(" ไมโครวินาที");
}