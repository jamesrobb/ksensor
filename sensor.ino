#include <OneWire.h>
#include <DallasTemperature.h>
#include <SoftwareSerial.h>
#include <dht.h>

#define UNIT_NAME   "sensorname"
#define HOST_NAME   "hostname"
#define HOST_PORT   80
#define DATA_RELAY_INTERVAL 30000
#define generic_timeout 5000
#define WIFI_ENABLE_PIN 9
#define DHT22_PIN 8
#define ONE_WIRE_WATER_SENSOR_PIN 7

struct
{
    uint32_t total;
    uint32_t ok;
    uint32_t crc_error;
    uint32_t time_out;
    uint32_t connect;
    uint32_t ack_l;
    uint32_t ack_h;
    uint32_t unknown;
} stat = { 0,0,0,0,0,0,0,0};

//SoftwareSerial monitor_serial(2, 3); // RX, TX
dht DHT;
float water_temperature = 0.0;
OneWire waterTemperatureOneWire(ONE_WIRE_WATER_SENSOR_PIN);
DallasTemperature waterSensors(&waterTemperatureOneWire);
bool good_air_temperature_reading;
bool good_water_temperature_reading;

void setup() {
    Serial.begin(9600);
    delay(1000);

    // while(!Serial) {
    //    ; // we wait for serial port to connect. Needed for native USB support only
    // }

    Serial.println("--setup routine starting.");
    Serial.println("");

    Serial1.begin(9600);

    while(!Serial1) {
       ; // we wait for serial1 port to connect to the wifi module
    }

    // we delay/do a single read because we seem to have been getting odd temperature readings otherwise
    waterSensors.begin();
    delay(500);
    get_water_temperature_info();
    delay(500);

    //Serial1.println("AT+UART_DEF=9600,8,1,0,0");

}

void loop() {

    good_air_temperature_reading = false;
    good_air_temperature_reading = false;

    get_air_temperature_info();
    get_water_temperature_info();

    send_temperature_info();

    delay(DATA_RELAY_INTERVAL);

}

bool get_air_temperature_info() {

    int chk = DHT.read22(DHT22_PIN);

    if(chk != DHTLIB_OK) {

        Serial.println("--there was an error getting the air temperature sensor info.");
        return false;

    }

    Serial.println("--temperature info received from air sensor.");
    Serial.print("--temperature: ");
    Serial.println(DHT.temperature);
    Serial.print("--rel humidity: ");
    Serial.println(DHT.humidity);

    good_air_temperature_reading = true;

    return true;
}

bool get_water_temperature_info() {

    good_water_temperature_reading = waterSensors.requestTemperaturesByIndex(0);
    water_temperature = waterSensors.getTempCByIndex(0);

    Serial.println("--temperature info received from water sensor.");
    Serial.print("--temperature: ");
    Serial.println(water_temperature);

    good_water_temperature_reading = true;

    return true;
}

void send_temperature_info() {

    String host_string = String(HOST_NAME);

    Serial.println("");
    Serial.println("--starting send data function.");

    Serial.println("--initializing/reseting wifi module.");
    pinMode(WIFI_ENABLE_PIN, OUTPUT);
    digitalWrite(WIFI_ENABLE_PIN, LOW);
    delay(100);
    digitalWrite(WIFI_ENABLE_PIN, HIGH);
    delay(1000);

    //Serial1.println("AT+CIPMODE=1");
    //delay(2000);

    while(Serial1.available()) {
        String s = Serial1.readString();
        Serial.println(s);
    }

    Serial.println("--finished initializing wifi module.");

    //get_wifi_version();
    if(wifi_set_mode(3)) {
        Serial.println("--wifi mode set.");
    } else {
        Serial.println("--error setting wifi mode.");
    }

    //if(wifi_join_access_point("blurb", "blurb222", 10000)) {
    if(wifi_join_access_point("ssid", "password", 20000)) {
        Serial.println("--connected to ssid.");
    } else {
        Serial.println("--unable to connect to ssid.");
    }

    if(wifi_set_mux(0)) {
        Serial.println("--wifi mux set.");
    } else {
        Serial.println("--error setting wifi mux.");
    }

    if(wifi_create_tcp(host_string, HOST_PORT)) {
        Serial.println("--tcp setup.");
    } else {
        Serial.println("--tcp setup faled.");
    }

    uint8_t buffer[1024] = {0};

    // first we build a string with our request, embedding our information as GET variables
    // it is easier to build a string than a character array, so we simply do that and then
    // convert to a character array
    //String req_string = "GET /ip";
    String req_string = "GET /temp/" + String(UNIT_NAME);
    req_string = req_string + "/" + String(good_air_temperature_reading ? DHT.temperature : 0);
    req_string = req_string + "/" + String(good_air_temperature_reading ? DHT.humidity : 0);
    req_string = req_string + "/" + String(water_temperature);
    req_string = req_string + "/" + String(good_air_temperature_reading ? "1" : "0");
    req_string = req_string + "/" + String(good_water_temperature_reading ? "1" : "0");
    req_string = req_string + " HTTP/1.1\r\nHost: " + host_string + "\r\nConnection: close\r\n\r\n";

    Serial.println("--request string is:");
    Serial.println(req_string);
    Serial.println("");

    int req_len = req_string.length();
    char reqx[req_len+1];

    Serial.println("--request char array is (should be identical to string):");
    req_string.toCharArray(reqx, req_len + 1);
    Serial.println(reqx);

    Serial.println("--attempting to send data. currently no error checking in place");
    wifi_send(reqx);
    Serial.println("--finished attempt. data may or may not have been sent");

    Serial.println("");

    Serial.println("--attempting to close the tcp connection");
    wifi_close_tcp();

    Serial.println("--send data function complete.");
    Serial.println("");
}

void wifi_get_version() {

    Serial1.println("AT+GMR");

    unsigned long start = millis();
    while (millis() - start < generic_timeout) {

        if(Serial1.available()) {
            String s = Serial1.readString();
            Serial.println(s);

            return;
        }

    }

    Serial.println("unable to retrieve version information.");
}

bool wifi_set_mode(int mode) {

    Serial1.print("AT+CWMODE=");
    Serial1.println(mode);

    unsigned long start = millis();
    while (millis() - start < generic_timeout) {

        if(Serial1.available()) {
            String s = Serial1.readString();
            Serial.println(s);

            if(s.indexOf("OK") != -1) {
                return true;
            }
            //Serial.write((char)Serial1.read());
            //delay(10);

        }

    }

    return false;
}

bool wifi_join_access_point(String SSID, String password, int join_timeout) {

    Serial1.print("AT+CWJAP=\"");
    Serial1.print(SSID);
    Serial1.print("\",\"");
    Serial1.print(password);
    Serial1.println("\"");

    unsigned long start = millis();
    while (millis() - start < join_timeout) {

        if(Serial1.available()) {
            String s = Serial1.readString();
            Serial.println(s);

            if(s.indexOf("WIFI GOT IP") != -1) {
                return true;
            }

            //return true;
        }

    }

    return false;

}

bool wifi_set_mux(int mux) {

    Serial1.print("AT+CIPMUX=");
    Serial1.println(mux);

    unsigned long start = millis();
    while (millis() - start < generic_timeout) {

        if(Serial1.available()) {
            String s = Serial1.readString();
            Serial.println(s);

            if(s.indexOf("OK") != -1) {
                return true;
            }

        }

    }

    return false;
}

bool wifi_create_tcp(String host_name, int host_port) {

    Serial1.print("AT+CIPSTART=\"TCP\",\"");
    Serial1.print(host_name);
    Serial1.print("\",");
    Serial1.println(host_port);

    unsigned long start = millis();
    while (millis() - start < generic_timeout) {

        if(Serial1.available()) {
            String s = Serial1.readString();
            Serial.println(s);

            if(s.indexOf("OK") != -1) {
                return true;
            }

        }

    }

    return false;

}

bool wifi_close_tcp() {

    Serial.println("destroying tcp");
    Serial1.println("AT+CIPCLOSE");

    unsigned long start = millis();
    while (millis() - start < generic_timeout) {

        if(Serial1.available()) {
            String s = Serial1.readString();
            Serial.print(s);

            if(s.indexOf("OK") != -1) {

                return true;

            }

            return false;

        }

    }

    return false;

}

bool wifi_send(char *req) {
    // need to add some logic in this functio to know that this was done properly.

    Serial1.print("AT+CIPSEND=");
    Serial1.println(strlen(req));

    delay(1000);
    Serial1.println(req);

    bool req_sent = false;
    unsigned long start = millis();
    while (millis() - start < generic_timeout * 2) {


        while(Serial1.available()) {
            byte s = Serial1.read();
            Serial.write(s);

        }


    }

    return false;
}
