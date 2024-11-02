Here's a detailed `README.md` for the "Automatic Plant Watering System" project. This guide will help users understand the project concept, set up the environment, and deploy both the hardware and software components effectively.

---

# Automatic Plant Watering System 🌿

This project builds a smart plant watering system that automatically maintains soil moisture at optimal levels without manual intervention. Using a soil moisture sensor and a motor controlled via relay, the system automatically turns on the motor to water plants when the soil moisture drops below a set threshold. Users can also control the motor manually through a web interface.

## 🛠️ Components and Technologies

- **Hardware:**
  - ESP32 (for Wi-Fi connection and motor control)
  - Soil moisture sensor
  - Relay module (to control motor)
  - 5V motor (for water pumping)
  - Optional: Push button and LED for status indication
  
- **Software:**
  - **Frontend:** ReactJS (user interface)
  - **Backend:** WebSocket Server (Node.js)
  - **Communication Protocol:** WebSocket (ESP32 and server communication)

## 📋 System Requirements

1. **Hardware:**
   - ESP32
   - Relay module and 5V motor
   - Soil moisture sensor
   - Jumper wires and breadboard for connections

2. **Software:**
   - Node.js v14+ and npm (for running the WebSocket server and React frontend)
   - Arduino IDE (for programming ESP32)

## 📑 Hardware Wiring Diagram

| Component               | ESP32 Pin | Notes                               |
|-------------------------|-----------|-------------------------------------|
| Soil Moisture Sensor (D0)| D2        | Reads soil moisture level           |
| Relay Module            | D4        | Controls motor on/off               |
| Motor                   | 5V power  | Pumps water when relay is on        |

## 🔧 Setup and Configuration

### Step 1: Program the ESP32

1. **Set up ESP32 support** in Arduino IDE:
   - Go to **File > Preferences** and add the ESP32 boards manager URL: `https://dl.espressif.com/dl/package_esp32_index.json`
   - Go to **Tools > Board** and select **ESP32**.

2. **Install the WebSocket library**:
   - In **Tools > Manage Libraries**, search for and install `ArduinoWebsockets`.

3. **Upload ESP32 code**:

   Open a new Arduino file, paste in the following code, and replace `Your_WiFi_SSID` and `Your_WiFi_Password` with your Wi-Fi credentials. Update `WebSocket_Server_IP` with your server's IP address.

   ```cpp
   #include <WiFi.h>
   #include <ArduinoWebsockets.h>

   using namespace websockets;

   const char* ssid = "Your_WiFi_SSID";
   const char* password = "Your_WiFi_Password";
   const char* websocket_server = "WebSocket_Server_IP";
   const int websocket_port = 5000;

   WebsocketsClient webSocket;
   const int sensorPin = 2;  // Soil moisture sensor
   const int motorPin = 4;   // Relay control pin

   void setup() {
       Serial.begin(115200);
       pinMode(motorPin, OUTPUT);
       digitalWrite(motorPin, LOW); // Initially keep the motor off

       WiFi.begin(ssid, password);
       while (WiFi.status() != WL_CONNECTED) {
           delay(1000);
           Serial.println("Connecting to WiFi...");
       }
       Serial.println("Connected to WiFi");

       webSocket.connect(websocket_server, websocket_port, "/");

       webSocket.onMessage([](WebsocketsMessage message) {
           String data = message.data();
           Serial.println("Message received: " + data);
           if (data == "MOTOR_ON") {
               digitalWrite(motorPin, HIGH);
           } else if (data == "MOTOR_OFF") {
               digitalWrite(motorPin, LOW);
           }
       });
   }

   void loop() {
       int humidity = analogRead(sensorPin);
       String jsonPayload = "{\"type\":\"humidityData\",\"value\":" + String(humidity) + "}";
       webSocket.send(jsonPayload);

       if (webSocket.available()) {
           webSocket.poll();
       }
       delay(1000);
   }
   ```

4. **Upload the code** to ESP32.

### Step 2: Set Up WebSocket Server

1. **Create the WebSocket Server**:
   - In the root of your project folder, create a file named `server.js` and paste the code below:

     ```javascript
     import WebSocket, { WebSocketServer } from "ws";

     const wss = new WebSocketServer({ port: 5000 });
     let motorState = false;

     wss.on("connection", (ws) => {
       console.log("Client connected");

       ws.on("message", (message) => {
         const data = JSON.parse(message);
         if (data.type === "humidityData") {
           console.log(`Humidity: ${data.value}`);
           if (data.value < 300 && !motorState) {
             motorState = true;
             ws.send(JSON.stringify({ type: "motorControl", state: "MOTOR_ON" }));
           }
         } else if (data.type === "manualControl") {
           motorState = data.state === "ON";
           ws.send(JSON.stringify({ type: "motorControl", state: motorState ? "MOTOR_ON" : "MOTOR_OFF" }));
         }
       });
     });

     console.log("WebSocket server running on ws://localhost:5000");
     ```

2. **Run the WebSocket Server**:
   Open a terminal in the project folder and run:
   ```bash
   node server.js
   ```

### Step 3: Set Up Frontend (React)

1. **Create a React project**:
   ```bash
   npx create-react-app plant-watering
   cd plant-watering
   ```

2. **Add Frontend Code**:
   - Replace `App.js` with the code below:

     ```javascript
     import React, { useEffect, useState } from "react";
     import "./App.css";

     function App() {
       const [humidity, setHumidity] = useState(null);
       const [motorState, setMotorState] = useState(false);
       const [ws, setWs] = useState(null);

       useEffect(() => {
         const websocket = new WebSocket("ws://WebSocket_Server_IP:5000");
         setWs(websocket);

         websocket.onopen = () => {
           console.log("Connected to WebSocket server");
         };

         websocket.onmessage = (event) => {
           const data = JSON.parse(event.data);
           if (data.type === "humidityData") {
             setHumidity(data.value);
           } else if (data.type === "motorControl") {
             setMotorState(data.state === "MOTOR_ON");
           }
         };

         return () => {
           websocket.close();
         };
       }, []);

       const handleMotorControl = (state) => {
         if (ws && ws.readyState === WebSocket.OPEN) {
           const controlMessage = JSON.stringify({ type: "manualControl", state });
           ws.send(controlMessage);
           setMotorState(state === "ON");
         }
       };

       return (
         <div className="App">
           <h1>Automatic Plant Watering System</h1>
           <div>Soil Humidity: {humidity ?? "Loading..."}</div>
           <div>Motor State: {motorState ? "ON" : "OFF"}</div>
           <button onClick={() => handleMotorControl("ON")}>Turn ON Motor</button>
           <button onClick={() => handleMotorControl("OFF")}>Turn OFF Motor</button>
         </div>
       );
     }

     export default App;
     ```

3. **Start the frontend**:
   ```bash
   npm start
   ```

## 🧪 Testing and Operation

1. **Check Wi-Fi connection**: Ensure the ESP32 connects to Wi-Fi successfully.
2. **Start the WebSocket Server** and open the frontend in your browser.
3. **Test**: View soil moisture data and control the motor from the interface.

## 📋 Notes

- **Low Moisture**: When soil moisture falls below the threshold, the motor automatically turns on.
- **Manual Control**: Users can also turn the motor on or off through the web interface.

## 📞 Support

If you encounter any issues, please open an **Issue** on GitHub or reach out via email.

Good luck with your project! 🎉