#include <SPI.h>
#include <WiFi.h>
#include <math.h>
#include <rgb_lcd.h>
#include <Servo.h>

#define NETWORK_SSID "5411_CNOT"
#define PASS "ILikeIoT"

#define APP_KEY "f126ab86-cb1c-4f9a-be33-e282afe4a401"
#define THING_NAME "Etu2"
#define SERVICE_NAME "GetProp"

#define BUFF_LENGTH 128
#define sensor_count 4

#define TEMP_PIN A0
#define SOUND_PIN A1
#define LIGHT_PIN A2
#define BUZ_PIN 2
#define SERVO_PIN 3
#define BUT_PIN 4
#define ANGLE_PIN 5

#define temp 0
#define light 1
#define sound 2
#define terminal_state 3

rgb_lcd lcd;

Servo servo;

const int colorR = 255;
const int colorG = 0;
const int colorB = 0;

int buttonState = 0;
int tempMax = 27;
int servoState = 0;

char buff[BUFF_LENGTH] = "";

char ssid[] = NETWORK_SSID;
char pass[] = PASS;
int status = WL_IDLE_STATUS;
char iot_server[] = "https://jsciclpcompany5864.cloud.thingworx.com";
IPAddress iot_address(50, 19, 39, 46);
char appKey[] = APP_KEY;
char thingName[] = THING_NAME;
char serviceName[] = SERVICE_NAME;
WiFiClient client;

char* sensor_names[] = {
        "temp",
        "light",
        "sound",
        "terminal_state"
};

int sensor_values[sensor_count];

void buz(){
  for (int i = 0; i < 10; ++i){
    digitalWrite(BUZ_PIN, HIGH);
    delay(10*i);
    digitalWrite(BUZ_PIN, LOW);
    delay(10*i);
  }
}

int readLuminosity(){
  int value = analogRead(LIGHT_PIN);
  value = map(value, 0, 800, 0, 10);
  return value;
}

int readSound(){
    long sum = 0;
    for(int i=0; i<32; i++)
    {
        sum += analogRead(SOUND_PIN);
    }

    sum >>= 5;
    return sum;
}

int readTemperature(){
    const int B=4275;                 // B value of the thermistor
    const int R0 = 100000;            // R0 = 100k
    int read_temp = analogRead(TEMP_PIN);

    float R = 1023.0/((float)read_temp)-1.0;
    R = 100000.0*R;

    int temperature=(int)1.0/(log(R/100000.0)/B+1/298.15)-273.15;//convert to temperature via datasheet ;
    return temperature;
}

void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}

void readSensors() {
    sensor_values[temp] = readTemperature();
    sensor_values[sound] = readSound();
    sensor_values[light] = readLuminosity();
    sensor_values[terminal_state] = digitalRead(BUT_PIN);
}

void readSensorsEmulated(){
    sensor_values[temp] = 100500;
    sensor_values[sound] = 500100;
    sensor_values[light] = 1004;
    sensor_values[terminal_state] = -2;
}

void parseStr() {
    char* tok = strtok(buff, " ");
    Serial.println("\n\n");
   //pumpState = atoi(tok);
    tok = strtok(NULL, " ");
    //servoState = atoi(tok);
    tok = strtok(NULL, " ");
}


void wifiSetup(){
    // check for the presence of the shield:
    if (WiFi.status() == WL_NO_SHIELD) {
     Serial.println("WiFi shield not present");
    // don't continue:
    while (true);
    }

    String fv = WiFi.firmwareVersion();
     if (fv != "1.1.0") {
      Serial.println("Please upgrade the firmware");
    }

    // attempt to connect to Wifi network:
    while (status != WL_CONNECTED) {
      Serial.print("Attempting to connect to SSID: ");
      Serial.println(ssid);
      // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
      status = WiFi.begin(ssid, pass);

      // wait 10 seconds for connection:
      delay(10000);
    }
    Serial.println("Connected to wifi");
    printWifiStatus();
}

void sendData() {
    Serial.println("Connecting to server...");
    client.stop();
    if(client.connect(iot_address, 443)) {
        if(client.connected()) {
            Serial.println("Sending data to server...\n");
            Serial.print("POST /Thingworx/Things/");
            client.print("POST /Thingworx/Things/");
            Serial.print(thingName);
            client.print(thingName);
            Serial.print("/Services/");
            client.print("/Services/");
            Serial.print(serviceName);
            client.print(serviceName);
            Serial.print("?appKey=");
            client.print("?appKey=");
            Serial.print(appKey);
            client.print(appKey);
            Serial.print("&method=post&x-thingworx-session=true");
            client.print("&method=post&x-thingworx-session=true");

            for(int i = 0; i < sensor_count; ++i) {
                Serial.print("&");
                client.print("&");
                Serial.print(sensor_names[i]);
                client.print(sensor_names[i]);
                Serial.print("=");
                client.print("=");
                Serial.print(sensor_values[i]);
                client.print(sensor_values[i]);
            }

            Serial.println(" HTTP/1.1");
            client.println(" HTTP/1.1");
            Serial.println("Accept: application/json");
            client.println("Accept: application/json");
            Serial.print("Host: ");
            client.print("Host: ");
            Serial.println(iot_server);
            client.println(iot_server);
            Serial.println("Content-Type: application/json");
            client.println("Content-Type: application/json");
            Serial.println();
            client.println();
            int i = 0;
            bool flagSkip = false;
            bool flagStart = false;
            while(client.connected()) {
                while(client.available() > 0) {
                    char symb = client.read();
                    Serial.print(symb);
                    if(symb == '|' && flagStart != true) {
                        flagStart = true;
                        continue;
                    }
                    if(flagStart == true) {
                        if(symb == '|') {
                            flagStart = false;
                            buff[i] = '\0';
                            parseStr();
                            return;
                        }
                        buff[i] = symb;
                        i++;
                    }
                }
                client.stop();
            }
        }
    }
}

void wtf(){
  Serial.println("Triggered");
}

void buttonHandler(){
  int buttonNow=digitalRead(BUT_PIN);
  if ((buttonState == 0) && (buttonNow)){
    buttonState = 1;
    buz();
    lcd.display();
  }else if ((buttonState == 1) && (buttonNow)){
    buttonState = 0;
    lcd.noDisplay();
    buz();
    }
}

void servoControl(){
  if ((sensor_values[0] > tempMax) && (servoState == 0)){
    servoState = 1;
  }else if ((sensor_values[0] < tempMax) && (servoState)){
    servoState = 0;
  }
  if (servoState){
    servo.attach(SERVO_PIN);
  }else{
    servo.attach(SERVO_PIN);
  }
}

void setup() {
  attachInterrupt(BUT_PIN, wtf, CHANGE);
  pinMode(BUZ_PIN, OUTPUT);
  pinMode(BUT_PIN, INPUT);
  pinMode(SERVO_PIN, OUTPUT);
  Serial.begin(9600);
  while (!Serial) {
    ;
  }
  lcd.begin(16, 2);
  lcd.setRGB(colorR, colorG, colorB);
  lcd.print("Welcome!");
  buz();
  wifiSetup();
  lcd.setCursor(0, 1);
  lcd.print("Initialisation complete!");
  delay(500);
  lcd.clear();
}

void loop() {
  buttonHandler();
  buttonState = 1;
  if (buttonState){
    Serial.println("--------------------");
    Serial.println("Loop begin:");
    //readSensors();
    readSensorsEmulated();
    sendData();
   // servoControl();
    lcd.setCursor(0, 0);
    lcd.print("Temperature: ");
    lcd.print(sensor_values[0]);
    lcd.setCursor(0, 1);
    lcd.print("Luminosity: ");
    lcd.print(sensor_values[1]);
    Serial.println("--------------------");
    delay(100);
  }
}
