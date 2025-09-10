#include <Arduino.h>

int led = PB9;
int button = PB8;

int pin_adc[8] = {PA0, PA1, PB0, PB1, PA4, PA5, PA6, PA7};
int pin_tx[4] = {PA9, PB3, PB10, PA2};
int pin_rx[4] = {PA10, PA15, PB11, PA3};
int baud = 115200;

HardwareSerial serial_ecm(pin_rx[0], pin_tx[0]);
HardwareSerial serial_psm1(pin_rx[1], pin_tx[1]);
HardwareSerial serial_psm2(pin_rx[2], pin_tx[2]);
HardwareSerial serial_psm3(pin_rx[3], pin_tx[3]);

HardwareSerial* serials[4] = {&serial_ecm, &serial_psm1, &serial_psm2, &serial_psm3};


uint32_t last_release = 0;
int is_released = 0;


void setup() {
    Serial.begin(9600);
    analogReadResolution(12);
    pinMode(led, OUTPUT);
    pinMode(button, INPUT);
    digitalWrite(led, 1);
    // serial_psm1.begin(baud);
    for (int i = 0; i < 4; i++) {
      serials[i]->begin(baud);
    }
    for (int i = 0; i < 8; i++) {
      pinMode(pin_adc[i], INPUT);
    }
}

void loop () {
    uint32_t t = millis();
    for (int i = 0; i < 8; i++) {
      Serial.print(analogRead(pin_adc[i]));
      Serial.print(',');
    }
    Serial.print('\n');
    // serial_psm1.println("ecm");
    for (int i = 0; i < 4; i++) {
      serials[i]->println(i);
    }
    delay(100);
}
