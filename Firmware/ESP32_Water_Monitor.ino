/*******************************************************
 * ESP32 Smart Water & Air Quality Monitoring System
 * 
 * Sensors:
 * - pH Sensor
 * - Turbidity Sensor
 * - TDS Sensor
 * - MQ-7 CO Sensor
 * - MQ-135 Air Quality Sensor
 * - DHT22 Temperature/Humidity
 *
 * Cloud:
 * - Blynk IoT
 * - Firebase Realtime Database
 *
 * Board:
 * ESP32 DevKit
 *******************************************************/


// ===================== LIBRARIES =====================

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>
#include <MQUnifiedsensor.h>
#include <Firebase_ESP_Client.h>


// ===================== WIFI =====================

#define WIFI_SSID       "YOUR_WIFI_NAME"
#define WIFI_PASSWORD   "YOUR_WIFI_PASSWORD"

// ===================== BLYNK =====================

#define BLYNK_TEMPLATE_ID   "YOUR_TEMPLATE_ID"
#define BLYNK_TEMPLATE_NAME "YOUR_TEMPLATE_NAME"
#define BLYNK_AUTH_TOKEN    "YOUR_AUTH_TOKEN"


// ===================== FIREBASE =====================

#define DATABASE_URL "YOUR_DATABASE_URL"


FirebaseData fbdo;
FirebaseConfig config;


// ===================== SENSOR PINS =====================

// ADC1 ONLY (WiFi compatible)

#define PH_PIN       34
#define TURB_PIN     35
#define TDS_PIN      32
#define MQ7_PIN      33
#define MQ135_PIN    36

#define DHT_PIN      4



// ===================== DHT =====================

#define DHT_TYPE DHT22

DHT dht(DHT_PIN, DHT_TYPE);


// ===================== MQ SENSORS =====================

#define BOARD "ESP32"

#define VOLTAGE_RESOLUTION 3.3
#define ADC_RESOLUTION 12


#define MQ7_RATIO_CLEAN_AIR 27.5
#define MQ135_RATIO_CLEAN_AIR 3.6


MQUnifiedsensor MQ7(
  BOARD,
  VOLTAGE_RESOLUTION,
  ADC_RESOLUTION,
  MQ7_PIN,
  "MQ-7"
);


MQUnifiedsensor MQ135(
  BOARD,
  VOLTAGE_RESOLUTION,
  ADC_RESOLUTION,
  MQ135_PIN,
  "MQ-135"
);



// ===================== VARIABLES =====================


unsigned long lastUpdate = 0;

const unsigned long UPDATE_INTERVAL = 5000;


// pH calibration values
// Change after calibration with pH buffers

float PH_SLOPE  = -5.70;
float PH_OFFSET = 21.34;



// ===================== ADC FUNCTIONS =====================


float readADC(uint8_t pin)
{

  uint32_t total = 0;


  for(int i = 0; i < 64; i++)
  {
    total += analogRead(pin);
    delay(2);
  }


  return total / 64.0;

}



float adcToVoltage(float adc)
{
  return adc * (3.3 / 4095.0);
}



// ===================== PH SENSOR =====================


float readPH()
{

  float voltage =
  adcToVoltage(readADC(PH_PIN));


  float ph =
  PH_SLOPE * voltage + PH_OFFSET;


  return constrain(ph,0,14);

}



// ===================== TURBIDITY SENSOR =====================


float readTurbidity()
{

  float voltage =
  adcToVoltage(readADC(TURB_PIN));


  float ntu =
  -1120.4 * voltage * voltage
  + 5742.3 * voltage
  - 4352.9;


  if(ntu < 0)
    ntu = 0;


  return ntu;

}



// ===================== TDS SENSOR =====================


float readTDS(float temperature)
{


  float voltage =
  adcToVoltage(readADC(TDS_PIN));



  // temperature compensation

  float compensation =
  1.0 + 0.02 * (temperature - 25.0);



  voltage /= compensation;



  float tds =
  (133.42 * pow(voltage,3)
  -255.86 * pow(voltage,2)
  +857.39 * voltage)
  *0.5;



  if(tds < 0)
    tds = 0;



  return tds;

}



// ===================== MQ INITIALIZATION =====================


void setupMQ()
{

  Serial.println("MQ Sensors warming up...");


  MQ7.init();
  MQ135.init();


  delay(60000);



  Serial.println("Calibrating MQ sensors...");


  float mq7R0 =
  MQ7.calibrate(MQ7_RATIO_CLEAN_AIR);


  float mq135R0 =
  MQ135.calibrate(MQ135_RATIO_CLEAN_AIR);



  MQ7.setR0(mq7R0);

  MQ135.setR0(mq135R0);



  Serial.println("MQ calibration finished");

}

// ===================== SETUP =====================

void setup()
{

  Serial.begin(115200);


  // ADC Configuration

  analogReadResolution(12);

  analogSetPinAttenuation(PH_PIN, ADC_11db);
  analogSetPinAttenuation(TURB_PIN, ADC_11db);
  analogSetPinAttenuation(TDS_PIN, ADC_11db);
  analogSetPinAttenuation(MQ7_PIN, ADC_11db);
  analogSetPinAttenuation(MQ135_PIN, ADC_11db);



  // DHT Start

  dht.begin();



  // WiFi

  Serial.println();
  Serial.print("Connecting to WiFi");


  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);


  while(WiFi.status() != WL_CONNECTED)
  {

    delay(500);
    Serial.print(".");

  }


  Serial.println();
  Serial.println("WiFi Connected");



  // Blynk

  Blynk.begin(
    BLYNK_AUTH_TOKEN,
    WIFI_SSID,
    WIFI_PASSWORD
  );



  // Firebase

  config.database_url = DATABASE_URL;


  Firebase.begin(
    &config,
    nullptr
  );


  Firebase.reconnectWiFi(true);



  // MQ Setup

  setupMQ();



  Serial.println("-----------------------");
  Serial.println("SYSTEM READY");
  Serial.println("-----------------------");

}



// ===================== SEND FIREBASE =====================


void sendFirebase(
float temperature,
float humidity,
float ph,
float turbidity,
float tds,
float co,
float airQuality
)
{


  if(!Firebase.ready())
  {
    Serial.println("Firebase not ready");
    return;
  }



  Firebase.RTDB.setFloat(
    &fbdo,
    "/water_monitoring/temperature",
    temperature
  );


  Firebase.RTDB.setFloat(
    &fbdo,
    "/water_monitoring/humidity",
    humidity
  );


  Firebase.RTDB.setFloat(
    &fbdo,
    "/water_monitoring/ph",
    ph
  );


  Firebase.RTDB.setFloat(
    &fbdo,
    "/water_monitoring/turbidity",
    turbidity
  );


  Firebase.RTDB.setFloat(
    &fbdo,
    "/water_monitoring/tds",
    tds
  );


  Firebase.RTDB.setFloat(
    &fbdo,
    "/water_monitoring/co",
    co
  );


  Firebase.RTDB.setFloat(
    &fbdo,
    "/water_monitoring/air_quality",
    airQuality
  );


}



// ===================== LOOP =====================


void loop()
{


  Blynk.run();



  if(millis() - lastUpdate >= UPDATE_INTERVAL)
  {


    lastUpdate = millis();



    // -------- DHT --------

    float temperature =
    dht.readTemperature();


    float humidity =
    dht.readHumidity();



    if(isnan(temperature) || isnan(humidity))
    {

      Serial.println("DHT Error");

      return;

    }



    // -------- Water Sensors --------


    float ph =
    readPH();


    float turbidity =
    readTurbidity();


    float tds =
    readTDS(temperature);



    // -------- MQ Sensors --------


    MQ7.update();

    MQ135.update();



    float co =
    MQ7.readSensor();



    float airQuality =
    MQ135.readSensor();




    // ================= SERIAL MONITOR =================


    Serial.println();

    Serial.println("======================");

    Serial.println("SENSOR DATA");


    Serial.print("Temperature: ");
    Serial.print(temperature);
    Serial.println(" C");


    Serial.print("Humidity: ");
    Serial.print(humidity);
    Serial.println(" %");


    Serial.print("pH: ");
    Serial.println(ph);



    Serial.print("Turbidity: ");
    Serial.print(turbidity);
    Serial.println(" NTU");


    Serial.print("TDS: ");
    Serial.print(tds);
    Serial.println(" ppm");


    Serial.print("CO: ");
    Serial.print(co);
    Serial.println(" ppm");


    Serial.print("Air Quality: ");
    Serial.println(airQuality);


    Serial.println("======================");




    // ================= BLYNK =================


    Blynk.virtualWrite(V0, tds);

    Blynk.virtualWrite(V1, ph);

    Blynk.virtualWrite(V2, turbidity);

    Blynk.virtualWrite(V3, temperature);

    Blynk.virtualWrite(V4, humidity);

    Blynk.virtualWrite(V5, co);

    Blynk.virtualWrite(V6, airQuality);





    // ================= FIREBASE =================


    sendFirebase(
      temperature,
      humidity,
      ph,
      turbidity,
      tds,
      co,
      airQuality
    );


  }


}
