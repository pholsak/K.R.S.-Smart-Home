// โครงงานวิทยาศาสตร์ ประเภทสิ่งประดิษฐ์ เรื่อง K.R.S. Smart Home
#include <Wire.h>
#define BLYNK_PRINT Serial
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <DHT.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <MCP3008.h>
#include <Adafruit_MCP23017.h>    // ขยาย Digital เพิ่มเป็น 16 ขา โดยใช้ MCP23017 


char auth[] = "b0cc3200bb5049b79a3221c48cd2ccc0";
char ssid[] = "Alpha Metro";
char pass[] = "soaps7522";

const int LDR = 0;        // ต่อกับเซ็นเซอร์ LDR เพื่ออ่านค่าระดับแสง
const int LEDPin1 = D0;   // ต่อหลอดไฟรั้วบ้าน

const int LPG = 1;
const int BUZZER = D2;    // กำหนดขาเพื่อต่อกับลำโพงแจ้งเตือนแก๊สรั่ว

const int MOISE = 2;      // ต่อสายสัญญาณเข้าขา 2 ADC ของ MCP3008 (ขาสัญญาณ dht 111)
const int PUMP = D3;


int ADC_LDR_Read = 300;     // กำหนดระดับความแข้งแสงที่จะสั่งให้ เปิด-ปิด 
int ADC_LPG_Read = 300;     // กำหนดค่าระดับแก๊สเพื่อให้แจ้งเดือน
int ADC_MOISE_Read1 = 300;  // กำหนดระดับความชื้นในดินเพื่อส่งให้ปั้มรดน้ำ
int ADC_MOISE_Read2 = 500;  // กำหนดระดับความชื้นในดินเพื่อสั่งให้ปั้มหยุดทำงาน

#define DHTTYPE DHT11     // กำหนดเป็นเซ็นเซอร์ DHT 11
#define DOOR D4           // กำหนดขาเพื่อควบคุมเปิด ปิด ประตูบ้าน
#define DHTPIN D1         // ต่อสายสัญญาณเข้าขา D1 ของ NodeMCU (สัญญาณ dht 111)
#define FAN D9            // กำหนดขาเพื่อควบคุมเปิด ปิด พัดลมระบายความร้อน
#define CS_PIN D8         //
#define CLOCK_PIN D5      //
#define MOSI_PIN D7       //
#define MISO_PIN D6       //

//-------------------------------------------------
MCP3008 adc(CLOCK_PIN, MOSI_PIN, MISO_PIN, CS_PIN);
Adafruit_MCP23017 mcp;

DHT dht(DHTPIN, DHTTYPE);

BlynkTimer timer;

//-------------------------------------------------
void sendSensor()
{
  float h = dht.readHumidity();
  float t = dht.readTemperature(); // or dht.readTemperature(true) for Fahrenheit
  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
  Serial.println("Humidity : ");
  Serial.println(h, 1);
  Serial.println(" %\t ");
  Serial.println("Temperature : ");
  Serial.println(t, 1);  
  Serial.println(" *C ");
  
  Blynk.virtualWrite(V0, h);      // ต่อกับ Blynk ที่ขา V0 เพื่อดึง ค่า %ความชื้น ขึ้นแสดงในแอพ Blynk
  Blynk.virtualWrite(V1, t);      // ต่อกับ Blynk ที่ขา V1 เพื่อดึง ค่าอุณหภูมิ ขึ้นแสดงในแอพ Blynk
}

//-------------------------------------------------
void setup() 
{
  Serial.begin(9600);
  
  pinMode(LEDPin1, OUTPUT);       // LEDPin1 แทนไฟรั้วบ้านนะครับ
  pinMode(BUZZER, OUTPUT);
  pinMode(PUMP, OUTPUT);

  Blynk.begin(auth, ssid, pass);
  dht.begin();
  digitalWrite(D1, HIGH);
  timer.setInterval(500, sendSensor);
}

//-------------------------------------------------
void loop()  {
  
  // ส่วนของ LDR Sensor เปิด-ปิด ไฟรั้วบ้าน
  int LDR_Sensor = adc.readADC(0);   // read Channel 0 from MCP3008 ADC (pin 1)
  Serial.print("Detect Light Level: ");
  Serial.println(LDR_Sensor);

  if (LDR_Sensor > ADC_LDR_Read) {
    delay(5000);
    digitalWrite(LEDPin1, LOW);
    } else {
      digitalWrite(LEDPin1, HIGH);
    }
  
  //----------------------------------------------
  // ส่วนของ LPG Gas Sensor
  int LPG_Sensor = adc.readADC(1);   // read Channel 0 from MCP3008 ADC (pin 2)

  Serial.print("LPG Gas Detect: ");
  Serial.println(LPG_Sensor);

  if (LPG_Sensor > ADC_LPG_Read) {
    digitalWrite(BUZZER, LOW);
    } else {
      digitalWrite(BUZZER, HIGH);
    }
    delay(500);

  //-----------------------------------------------
  // ส่วนของเซ็นเซอร์วัดความชื้น + รดน้ำต้นไม้
  int MOISE_Sensor = adc.readADC(2);   // read Channel 0 from MCP3008 ADC (pin 3)

  Serial.print("Moisture Level: ");
  Serial.println(MOISE_Sensor);

  if (MOISE_Sensor > ADC_MOISE_Read1) {
    digitalWrite(PUMP, HIGH);
    } if (MOISE_Sensor <= ADC_MOISE_Read2) {
    digitalWrite(PUMP, LOW);
    }
    delay(2000);
    
  //-----------------------------------------------
  // ส่วนของ Blynk
  Blynk.run();
  timer.run();
}
