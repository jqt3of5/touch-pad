#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include "BLE2902.h"
#include "BLEHIDDevice.h"
#include "HIDTypes.h"
#include "HIDKeyboardTypes.h"
//#include <WiFi.h>

#include <Arduino.h>

#define ARDUINOHA_SWITCH
//#include <ArduinoHA.h>
//#include <ArduinoOTA.h>

#include "hidReportDescriptor.h"

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/i

#ifndef _BV
#define _BV(bit) (1 << (bit))
#endif

static const uint8_t LED_PIN = 2;

const int TOUCH_PIN_COUNT = 8;
const uint8_t TOUCH_PINS[] = {T0/*, T1, T2*/, T3, T4, T5, T6, T7, T8, T9};


// You can have up to 4 on one i2c bus but one is enough for testing!
//Adafruit_MPR121 cap = Adafruit_MPR121();

//HADevice device("touch_pad");
//WiFiClient client;
//HAMqtt mqtt(client, device);

BLEHIDDevice* hid;
BLECharacteristic* input;
BLECharacteristic* output;
BLECharacteristic* inputVolume;
BLECharacteristic* outputVolume;
bool connected = false;

//
class MyCallbacks : public BLEServerCallbacks {
    void onConnect(BLEServer* pServer){
        connected = true;
        BLE2902* desc = (BLE2902*)input->getDescriptorByUUID(BLEUUID((uint16_t)0x2902));
        desc->setNotifications(true);

        BLE2902* descv = (BLE2902*)inputVolume->getDescriptorByUUID(BLEUUID((uint16_t)0x2902));
        descv->setNotifications(true);

        digitalWrite(LED_PIN, HIGH);
    }

    void onDisconnect(BLEServer* pServer){
        connected = false;
        BLE2902* desc = (BLE2902*)input->getDescriptorByUUID(BLEUUID((uint16_t)0x2902));
        desc->setNotifications(false);

        BLE2902* descv = (BLE2902*)inputVolume->getDescriptorByUUID(BLEUUID((uint16_t)0x2902));
        descv->setNotifications(false);
        digitalWrite(LED_PIN, LOW);
    }
};
//
void taskServer(void*){
    BLEDevice::init("TouchPad");
    BLEServer *pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MyCallbacks());

    hid = new BLEHIDDevice(pServer);
    inputVolume = hid->inputReport(1); // <-- input REPORTID from report map
    outputVolume = hid->outputReport(1); // <-- output REPORTID from report map

    input = hid->inputReport(2); // <-- input REPORTID from report map
    output = hid->outputReport(2); // <-- output REPORTID from report map

    std::string name = "SubstantiveTech";
    hid->manufacturer()->setValue(name);

    hid->pnp(0x02, 0xe502, 0xa111, 0x0210);
    hid->hidInfo(0x00,0x02);

    hid->reportMap((uint8_t*)keyboardHidDescriptor, sizeof(keyboardHidDescriptor));
    hid->startServices();


    BLESecurity *pSecurity = new BLESecurity();
//  pSecurity->setKeySize();
    pSecurity->setAuthenticationMode(ESP_LE_AUTH_BOND);


    BLEAdvertising *pAdvertising = pServer->getAdvertising();
    pAdvertising->setAppearance(HID_KEYBOARD);
    pAdvertising->addServiceUUID(hid->hidService()->getUUID());
    pAdvertising->start();
    hid->setBatteryLevel(98);

    delay(portMAX_DELAY);
}

void blink(int pin, int count, bool fast)
{
    for (int i = 0; i < count; ++i)
    {
        digitalWrite(pin, HIGH);
        delay(fast ? 250 : 500);
        digitalWrite(pin, LOW);
        delay(fast ? 250 : 500);
    }
}

void setup() {
    Serial.begin(115200);

    while (!Serial) { // needed to keep leonardo/micro from starting too fast!
        delay(10);
    }

//    WiFi.begin("WaitingOnComcast", "1594N2640W");
//    WiFi.setAutoReconnect(true);
//
//    while (WiFi.waitForConnectResult() != WL_CONNECTED) {
//        Serial.println("Connection Failed! Rebooting...");
//        delay(5000);
//        ESP.restart();
//    }

    // Port defaults to 3232
    // ArduinoOTA.setPort(3232);

    // Hostname defaults to esp3232-[MAC]
    // ArduinoOTA.setHostname("myesp32");

    // No authentication by default
    // ArduinoOTA.setPassword("admin");

    // Password can be set with it's md5 value as well
    // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
    // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

//    ArduinoOTA
//            .onStart([]() {
//                String type;
//                if (ArduinoOTA.getCommand() == U_FLASH)
//                    type = "sketch";
//                else // U_SPIFFS
//                    type = "filesystem";
//
//                // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
//                Serial.println("Start updating " + type);
//            })
//            .onEnd([]() {
//                Serial.println("\nEnd");
//            })
//            .onProgress([](unsigned int progress, unsigned int total) {
//                Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
//            })
//            .onError([](ota_error_t error) {
//                Serial.printf("Error[%u]: ", error);
//                if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
//                else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
//                else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
//                else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
//                else if (error == OTA_END_ERROR) Serial.println("End Failed");
//            });
//
//    ArduinoOTA.begin();

//    device.setName("Blinds Controller");
//    device.setSoftwareVersion("0.9.2");
//    device.enableLastWill();
//
//    if (!mqtt.begin("tiltpi.equationoftime.tech", 1883))
//    {
//        Serial.println("Failed to connect to mqtt broker");
//    }
//
//    while (!mqtt.isConnected())
//    {
//        Serial.println("Connecting to mqtt");
//        mqtt.loop();
//        delay(500);
//    }

    // Default address is 0x5A, if tied to 3.3V its 0x5B
    // If tied to SDA its 0x5C and if SCL then 0x5D
//    if (!cap.begin(0x5A)) {
//        Serial.println("MPR121 not found, check wiring?");
//        while (1);
//    }
    xTaskCreate(taskServer, "server", 20000, NULL, 5, NULL);

    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);

//    touch_pad_set_voltage()

    touch_high_volt_t high;
    touch_low_volt_t low;
    touch_volt_atten_t attn;
    touch_pad_get_voltage(&high, &low, &attn);
    uint16_t sleep_cycle;
    uint16_t meas_cycle;

//    Serial.printf("voltage high: %d voltage low: %d voltage attn: %d\n", high, low, attn);

    touch_pad_get_meas_time(&sleep_cycle, &meas_cycle);
//    Serial.printf("sleep time: %d meas cycle: %d\n", sleep_cycle, meas_cycle);

    touch_cnt_slope_t slope;
    touch_tie_opt_t tie;
    touch_pad_get_cnt_mode(static_cast<touch_pad_t>(0), &slope, &tie);

//    Serial.printf("cnt slope: %d tie opt: %d\n", slope, tie);

//    touch_pad_set_voltage(TOUCH_HVOLT_2V7, TOUCH_LVOLT_0V5, TOUCH_HVOLT_ATTEN_0V);

    for (int i = 0; i < 9; ++i)
    {
        touch_pad_set_cnt_mode(static_cast<touch_pad_t>(i), TOUCH_PAD_SLOPE_1, TOUCH_PAD_TIE_OPT_LOW);
    }

    for (int i = 0; i < TOUCH_PIN_COUNT; ++i)
    {
        pinMode(TOUCH_PINS[i], INPUT);
    }

//    Serial.println("index, pad, value");
}

const int readingMax = 100;
uint16_t touchReadings[TOUCH_PIN_COUNT][readingMax] = {0};

const int touchMax = 5;
uint16_t currentReadings[TOUCH_PIN_COUNT][touchMax] = {0};

unsigned long lastCalibration = 0;
float channelAverages[TOUCH_PIN_COUNT] = {0};
float channelStandardDeviations[TOUCH_PIN_COUNT] = {0};

int touchTotal = 0;
int readingTotal = 0;
int readingIndex = 0;

float average (uint16_t values[], int size)
{
    float total = 0;
    for (int i = 0; i < size; ++i)
    {
       total += values[i];
    }
    return total/size;
}

float average(uint16_t values[], int size, int start, int stop)
{
    int count = stop < start ? (stop + size - start) : stop - start;
    float total = 0;
   for (int i = 0; i < count; ++i)
   {
      total += values[(start + i)%size];
   }

   return total/count;
}

float standardDeviation(uint16_t values[], int size)
{
    float avg = average(values, size);

    float total = 0;
    for(int i = 0; i < size; ++i)
    {
        float v = values[i] - avg;
        total += v*v;
    }

    return sqrt(total/(size - 1));
}

void loop() {
//    mqtt.loop();
//    ArduinoOTA.handle();

    if (readingTotal < readingMax)
    {
        readingTotal += 1;
    }

    if (touchTotal < touchMax)
    {
        touchTotal += 1;
    }

    bool hit = false;
    for (int i = 2; i < TOUCH_PIN_COUNT; ++i)
    {
        uint16_t read = touchRead(TOUCH_PINS[i]);
        //TODO: Normalize readings, because we have a narrow range
        currentReadings[i][readingIndex%touchTotal] = read;

        float touchAverage = average(currentReadings[i], touchTotal);

        if (i == 2)
        {
            Serial.printf("%f %f %f %f %d", channelAverages[i], channelAverages[i] + 4*channelStandardDeviations[i], channelAverages[i] - 4*channelStandardDeviations[i], touchAverage, read);
        }

        //averages and std default to 0, so this will never be true if we aren't calibrated
        if (touchAverage < channelAverages[i] - 4*channelStandardDeviations[i])
        {
            hit = true;
//            Serial.printf("%f ", touchAverage);
        } else{
//            Serial.printf("%f ", 0);
        }

        if (readingTotal < readingMax) {
            touchReadings[i][readingIndex%readingTotal] = read;
        }

        //Only calibrates once per second, and only if we have a full reading array, and if we aren't currently being tapped
        if (read >= channelAverages[i] - 4*channelStandardDeviations[i] && readingTotal == readingMax)
        {
            touchReadings[i][readingIndex%readingTotal] = read;
            lastCalibration = millis();
//            for (int i = 0; i < TOUCH_PIN_COUNT; ++i)
//            {
                channelAverages[i] = average(touchReadings[i], readingTotal);
                channelStandardDeviations[i] = standardDeviation(touchReadings[i], readingTotal);
//            }
        }
    }

    Serial.println();

    digitalWrite(LED_PIN, hit ? HIGH : LOW);
    readingIndex += 1;

//    for (int i = 0; i < TOUCH_PIN_COUNT; ++i)
//    {
//        float total = 0;
//        for(int j = 0; j < readingTotal; ++j)
//        {
//            total += touchReadings[j][i];
//        }
////        Serial.printf("pad: %d value: %f\n", i, total/readingTotal);
//        if (touchReadings[(readingIndex+readingTotal-1)%readingTotal][i] < .90*total/readingTotal)
//        {
//            blink(LED_PIN, 2, true);
//        }
//    }

    delay(20);

//    inputKeyboard_t a{};
//    a.Key[i%6] = 0x02 + i;
//       a.reportId = 0x02;
//    input->setValue((uint8_t*)&a,sizeof(a));
//    input->notify();
}