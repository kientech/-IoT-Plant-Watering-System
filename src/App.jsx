import React, { useEffect, useState } from "react";
import "./App.css";

function App() {
  const [humidity, setHumidity] = useState(null);
  const [motorState, setMotorState] = useState(false);
  const [ws, setWs] = useState(null);

  useEffect(() => {
    const websocket = new WebSocket("ws://192.168.100.170:5000");
    setWs(websocket);

    websocket.onopen = () => {
      console.log("Connected to WebSocket server");
    };

    websocket.onmessage = (event) => {
      const data = JSON.parse(event.data);
      console.log("Message from server:", data);

      if (data.type === "humidityData") {
        setHumidity(data.value); // Cập nhật độ ẩm
      } else if (data.type === "motorControl") {
        setMotorState(data.state === "MOTOR_ON"); // Cập nhật trạng thái motor
      }
    };

    websocket.onclose = () => {
      console.log("Disconnected from WebSocket server");
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
    } else {
      console.log("WebSocket connection is not open");
    }
  };

  return (
    <div className="App">
      <header className="header">
        <h1>Plant Watering System</h1>
      </header>
      <main className="main-content">
        <div className="status-container">
          <div className="status-item">
            <h2>Soil Humidity</h2>
            <div className="humidity-display">{humidity ?? "Loading..."}</div>
          </div>
          <div className="status-item">
            <h2>Motor State</h2>
            <div className={`motor-state ${motorState ? "on" : "off"}`}>
              {motorState ? "ON" : "OFF"}
            </div>
          </div>
        </div>
        <div className="controls">
          <button
            className="control-button on-button"
            onClick={() => handleMotorControl("ON")}
          >
            Turn ON Motor
          </button>
          <button
            className="control-button off-button"
            onClick={() => handleMotorControl("OFF")}
          >
            Turn OFF Motor
          </button>
        </div>
      </main>
    </div>
  );
}

export default App;
