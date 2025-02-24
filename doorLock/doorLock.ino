// Define the board type
#define ESP8266_BOARD 1
#define ESP32_BOARD 2

// Select the board type here
#define BOARD_TYPE ESP8266_BOARD // Change to ESP32_BOARD for ESP32

#if BOARD_TYPE == ESP8266_BOARD
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266mDNS.h>
ESP8266WebServer server(80); // Define server for ESP8266
ESP8266WiFiMulti WiFiMulti;  // Define WiFiMulti for ESP8266
#elif BOARD_TYPE == ESP32_BOARD
#include <WiFi.h>
#include <WebServer.h>
#include <WiFiMulti.h>
#include <ESPmDNS.h>
WebServer server(80); // Define server for ESP32
WiFiMulti WiFiMulti;  // Define WiFiMulti for ESP32
#endif

#include <WebSocketsServer.h>
#include <ArduinoJson.h>
#include <Servo.h>
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>CyberLock Control</title>
    <style>
        body {
            margin: 0;
            padding: 0;
            background: #0a0a0a;
            display: flex;
            justify-content: center;
            align-items: center;
            min-height: 80vh;
            font-family: 'Arial', sans-serif;
        }

        .lock-container {
            background: linear-gradient(45deg, #1a1a1a, #2d2d2d);
            padding: 2rem;
            border-radius: 15px;
            box-shadow: 0 0 20px rgba(0, 255, 255, 0.3);
            border: 2px solid #00ffff;
            display: flex;
            flex-direction: column;
            align-items: center;
        }

        .lock-title {
            color: #00ffff;
            text-align: center;
            text-shadow: 0 0 10px rgba(0, 255, 255, 0.5);
            margin-bottom: 1rem;
        }

        .status-indicator {
            margin-bottom: 2rem;
            display: flex;
            justify-content: center;
            align-items: center;
            gap: 1rem;
        }

        .led {
            width: 20px;
            height: 20px;
            border-radius: 50%;
            background: #ff0000;
            box-shadow: 0 0 5px rgba(0, 255, 255, 0.3);
        }

        .lock-button {
            background: linear-gradient(45deg, #003333, #006666);
            width: 150px;
            height: 150px;
            border-radius: 50%;
            border: 3px solid #00ffff;
            cursor: pointer;
            position: relative;
            transition: all 0.2s ease;
            box-shadow: 0 0 20px rgba(0, 255, 255, 0.3);
        }

        .lock-button.active {
            background: linear-gradient(45deg, #006666, #009999);
            box-shadow: 0 0 30px rgba(0, 255, 255, 0.5);
            transform: scale(0.95);
        }

        .lock-button::before {
            content: '';
            position: absolute;
            top: 50%;
            left: 50%;
            transform: translate(-50%, -50%);
            width: 80%;
            height: 80%;
            border: 2px solid #00ffff;
            border-radius: 50%;
            opacity: 0.5;
        }

        .status-text {
            color: #00ffff;
            text-align: center;
            margin-top: 1rem;
        }

        .connection-status {
            position: fixed;
            top: 20px;
            right: 20px;
            display: flex;
            align-items: center;
            gap: 10px;
        }

        .lock-button.disabled {
            opacity: 0.5;
            cursor: not-allowed;
            filter: grayscale(0.8);
            pointer-events: none; /* Add this line to disable interactions */
        }

        @keyframes pulse {
            0% { box-shadow: 0 0 5px rgba(0, 255, 255, 0.3); }
            50% { box-shadow: 0 0 20px rgba(0, 255, 255, 0.6); }
            100% { box-shadow: 0 0 5px rgba(0, 255, 255, 0.3); }
        }
    </style>
</head>
<body>
    <div class="connection-status">
        <span style="color: #00ffff;">CONNECTION:</span>
        <div class="led" id="connection-led"></div>
    </div>

    <div class="lock-container">
        <h1 class="lock-title">CYBERLOCK</h1>
        <div class="status-indicator">
            <div class="led" id="status-led"></div>
            <!-- <div class="led"></div>
            <div class="led"></div> -->
        </div>
        <div class="lock-button" id="lockButton"></div>
        <p class="status-text" id="statusText">STATUS: LOCKED</p>
    </div>

    <script>
        const lockButton = document.getElementById('lockButton');
        const connectionLed = document.getElementById('connection-led');
        const statusLed = document.getElementById('status-led');
        const statusText = document.getElementById('statusText');
        
        let pressTimer = null;
        let isButtonActive = true;
        let unlockInProgress = false;
        let isLocked = true; // Track lock state

        // WebSocket connection
        const socket = new WebSocket('ws://' + window.location.hostname + ':81');

        // Connection status
        socket.onopen = function() {
            console.log('Connected to WebSocket server');
            connectionLed.style.background = '#00ff00';
        };

        socket.onerror = socket.onclose = function() {
            connectionLed.style.background = '#ff0000';
            statusText.textContent = 'CONNECTION LOST';
        };

        const enableButton = () => {
            isButtonActive = true;
            lockButton.classList.remove('disabled');
        };

        const disableButton = () => {
            isButtonActive = false;
            lockButton.classList.add('disabled');
        };

        const handlePress = () => {
            if (!isButtonActive || !isLocked) return; // Prevent if already unlocked

            statusLed.style.background = '#ffff00';
            statusText.textContent = 'HOLD TO UNLOCK...';
            
            pressTimer = setTimeout(() => {
                lockButton.classList.add('active');
                const data = JSON.stringify({ type: 'door', x: Math.round(1)});
                socket.send(data);
                statusText.textContent = 'STATUS: UNLOCKED';
                statusLed.style.background = '#00ff00';
                unlockInProgress = true;
                isLocked = false; // Update lock state
                disableButton();
            }, 1000);
        };

        const handleRelease = () => {
            clearTimeout(pressTimer);
            
            if (unlockInProgress) {
                lockButton.classList.remove('active');
                statusLed.style.background = '#00ff00';
                statusText.textContent = 'STATUS: UNLOCKED';
                
                setTimeout(() => {
                    const data = JSON.stringify({ type: 'door', x: Math.round(0)});
                    socket.send(data);
                    statusLed.style.background = '#ff0000';
                    statusText.textContent = 'STATUS: LOCKED';
                    unlockInProgress = false;
                    isLocked = true; // Update lock state
                    enableButton();
                }, 3000);
            } else {
                statusLed.style.background = '#ff0000';
                statusText.textContent = 'STATUS: LOCKED';
                if (isLocked) enableButton();
            }
        };

        // Event listeners (keep the same)
        lockButton.addEventListener('mousedown', handlePress);
        lockButton.addEventListener('mouseup', handleRelease);
        lockButton.addEventListener('mouseleave', handleRelease);
        lockButton.addEventListener('touchstart', (e) => {
            e.preventDefault();
            handlePress();
        });
        lockButton.addEventListener('touchend', (e) => {
            e.preventDefault();
            handleRelease();
        });
    </script>
</body>
</html>
)rawliteral";

#if BOARD_TYPE == ESP8266_BOARD
unsigned long lastMDNSUpdate = 0;
const unsigned long MDNS_UPDATE_INTERVAL = 100; // Update mDNS every 100 mili second
#endif

WebSocketsServer webSocket = WebSocketsServer(81);
Servo myServo; // Create a servo object

void handleRoot()
{
    server.send(200, "text/html", index_html);
}

void handleServo(int status)
{
  if (status == 0) {
    myServo.write(40);
    digitalWrite(LED_BUILTIN, LOW);
    Serial.println("Servo angle 40");
  }
  else if ( status == 1) {
    myServo.write(0);
    digitalWrite(LED_BUILTIN, HIGH);
    Serial.println("Servo angle 0");
  }
}

void handleWebSocketMessage(uint8_t *data, size_t len, uint8_t num)
{
    // Allocate the JSON document
    StaticJsonDocument<200> doc;

    // Deserialize the JSON document
    DeserializationError error = deserializeJson(doc, data, len);

    // Test if parsing succeeds
    if (error)
    {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
        return;
    }

    // Handle the received data based on type
    if (doc["type"] == "door")
    {
        int x = doc["x"].as<int>();
        Serial.println(x);
        handleServo(x);
    }
    else
    {
        Serial.println("Unknown data received");
        webSocket.sendTXT(num, "Unknown data received");
    }
}

void onWebSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length)
{
    switch (type)
    {
    case WStype_DISCONNECTED:
        Serial.printf("[%u] Disconnected!\n", num);
        break;
    case WStype_CONNECTED:
    {
        IPAddress ip = webSocket.remoteIP(num);
        Serial.printf("[%u] Connection from ", num);
        Serial.println(ip.toString());
        break;
    }
    case WStype_TEXT:
        //webSocket.sendTXT(num, payload);
        handleWebSocketMessage(payload, length, num);
        break;
    case WStype_BIN:
        Serial.printf("[%u] get binary length: %u\n", num, length);
        break;
    case WStype_PING:
        Serial.printf("[%u] Received PING\n", num);
        break;
    case WStype_PONG:
        Serial.printf("[%u] Received PONG\n", num);
        break;
    case WStype_ERROR:
        Serial.printf("[%u] WebSocket Error!\n", num);
        break;
    }
}

void setup()
{
    Serial.begin(115200);

    // Connect to the external WiFi network (STA mode)
    WiFiMulti.addAP("WiFiName", "WiFiPassword");
    myServo.attach(0,650, 2400);
    pinMode(LED_BUILTIN, OUTPUT);

    // Wait for the ESP to connect to WiFi
    while (WiFiMulti.run() != WL_CONNECTED)
    {
        delay(100);
    }

    // Print the IP address
    Serial.print("Connected to ");
    Serial.println(WiFi.SSID());
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    // Start mDNS service
    String hostname = "doorlock";
    if (MDNS.begin(hostname.c_str()))
    {
        Serial.printf("mDNS responder started with hostname: %s.local\n", hostname.c_str());
    }
    else
    {
        Serial.println("Error setting up mDNS responder!");
    }

    // Set up AP mode
    WiFi.softAP("doorlock", "password123"); // Set your AP SSID and password
    IPAddress myIP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(myIP);

    // Start the HTTP server
    server.on("/", handleRoot);
    server.begin();
    Serial.println("HTTP server started");

    // Start the WebSocket server
    webSocket.begin();
    webSocket.onEvent(onWebSocketEvent);
    Serial.println("WebSocket server started");
}

void loop()
{
    server.handleClient();
    webSocket.loop();

#if BOARD_TYPE == ESP8266_BOARD
    // Handle mDNS queries at a reduced frequency
    unsigned long currentMillis = millis();
    if (currentMillis - lastMDNSUpdate >= MDNS_UPDATE_INTERVAL)
    {
        lastMDNSUpdate = currentMillis;
        MDNS.update();
    }
#endif
}