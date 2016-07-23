#include <SoftwareSerial.h>

SoftwareSerial monitor_serial(2, 3); // RX, TX

void setup() {
    Serial.begin(9600);

    while(!Serial) {
      ; // we wait for serial port to connect. Needed for native USB support only.
    }

    monitor_serial.begin(115200);
    //sSerial.println("software serial!");

    Serial.println("setup done running!");
}

void loop() {

    //Serial.println("AT+GMR");
    //delay(100);

    while(Serial.available()) {
        monitor_serial.write(Serial.read());
    }

    while(monitor_serial.available()) {
        Serial.write(monitor_serial.read());
    }

    //Serial.println("end of loop");

}
