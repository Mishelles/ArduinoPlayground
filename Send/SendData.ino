#define APP_KEY "f126ab86-cb1c-4f9a-be33-e282afe4a401"
#define THING_NAME "Etu2"
#define SERVICE_NAME "GetProp"

#define BUFF_LENGTH 128
#define sensor_count 4

#define temp 0
#define light 1
#define sound 2
#define terminal_state 3

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
