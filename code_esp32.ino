#include <WiFi.h>
#include <ArduinoWebsockets.h>

using namespace websockets;

const char* ssid = "TrungKien";         
const char* password = "trungkien"; 
const char* websocket_server = "192.168.100.170";  
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
        Serial.println("Nhận được tin nhắn: " + data);

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
        Serial.println("Mất kết nối WebSocket. Đang kết nối lại...");
        connectWebSocket();
    }
}

void connectWebSocket() {
    Serial.println("Đang kết nối WebSocket...");
    if (webSocket.connect(websocket_server, websocket_port, "/")) {
        Serial.println("Đã kết nối WebSocket thành công!");
    } else {
        Serial.println("Kết nối WebSocket thất bại. Thử lại...");
        delay(2000); 
        connectWebSocket();  
    }
}
