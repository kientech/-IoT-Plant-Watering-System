import WebSocket, { WebSocketServer } from "ws";
import express from "express";
import http from "http";

const app = express();
const server = http.createServer(app);
const wss = new WebSocketServer({ server });
const PORT = 5000;

let motorState = false; 
let currentHumidity = 0; 

wss.on("connection", (ws) => {
  console.log("New client connected");

  ws.on("message", (message) => {
    const data = JSON.parse(message);
    console.log("Received:", data);

    if (data.type === "sensorData") {
      currentHumidity = data.value;
      console.log(`Current humidity: ${currentHumidity}`);
      broadcastMessage(
        JSON.stringify({ type: "humidityData", value: currentHumidity })
      );
    } else if (data.type === "manualControl") {
      motorState = data.state === "ON";
      const controlMessage = motorState ? "MOTOR_ON" : "MOTOR_OFF";
      broadcastMessage(
        JSON.stringify({ type: "motorControl", state: controlMessage })
      );
    }
  });

  ws.on("close", () => {
    console.log("Client disconnected");
  });
});

function broadcastMessage(message) {
  wss.clients.forEach((client) => {
    if (client.readyState === WebSocket.OPEN) {
      client.send(message);
    }
  });
}

server.listen(PORT, () => {
  console.log(`Server is running on http://localhost:${PORT}`);
});
