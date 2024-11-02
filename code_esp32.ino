#include <WiFi.h>
#include <ArduinoWebsockets.h>

using namespace websockets;

const char* ssid = "your_SSID";         
const char* password = "your_PASSWORD"; 
const char* websocket_server = "your_SERVER_IP";  
const int websocket_port = 5000;

WebsocketsClient webSocket;

const int sensorPin = 34;  
const int motorPin = 4;    
unsigned long previousMillis = 0;
const long interval = 1000; 
bool motorState = false;    

void setup() {
    Serial.begin(115200);
    pinMode(motorPin, OUTPUT);
    digitalWrite(motorPin, LOW); 

    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }
    Serial.println("Connected to WiFi");

    connectWebSocket();

    webSocket.onMessage([](WebsocketsMessage message) {
        String data = message.data();
        Serial.println("Received message: " + data);

        if (data == "{\"type\":\"motorControl\",\"state\":\"MOTOR_ON\"}") {
            motorState = true; 
            digitalWrite(motorPin, HIGH); 
            Serial.println("Motor turned ON");
        } else if (data == "{\"type\":\"motorControl\",\"state\":\"MOTOR_OFF\"}") {
            motorState = false; 
            digitalWrite(motorPin, LOW);  
            Serial.println("Motor turned OFF");
        }
    });
}

void loop() {
    unsigned long currentMillis = millis();

    if (currentMillis - previousMillis >= interval) {
        previousMillis = currentMillis;
        
        int humidity = analogRead(sensorPin);
        String jsonPayload = "{\"type\":\"sensorData\",\"value\":" + String(humidity) + "}";
        
        webSocket.send(jsonPayload);
        Serial.println("Sent data to WebSocket: " + jsonPayload);
    }

    if (webSocket.available()) {
        webSocket.poll();
    } else {
        Serial.println("WebSocket disconnected. Reconnecting...");
        connectWebSocket();
    }
}

void connectWebSocket() {
    Serial.println("Connecting to WebSocket...");
    if (webSocket.connect(websocket_server, websocket_port, "/")) {
        Serial.println("WebSocket connection successful!");
    } else {
        Serial.println("WebSocket connection failed. Retrying...");
        delay(2000); 
        connectWebSocket();  
    }
}
