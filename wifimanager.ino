#include <WiFi.h>
#include <WebServer.h>
#include <EEPROM.h>

const char* apSSID = "Node Farm";

WebServer server(80);

void setup() {
  Serial.begin(115200);
  EEPROM.begin(512);

  String ssid = readEEPROM(0);
  String password = readEEPROM(50);

 if (ssid.length() > 0 && password.length() > 0) {
    WiFi.begin(ssid.c_str(), password.c_str());
    if (testWifi()) {
      // Kết nối thành công, tiếp tục với setup Blynk
      Blynk.begin(BLYNK_AUTH_TOKEN, ssid.c_str(), password.c_str());
    } else {
      // Không thể kết nối, chuyển sang chế độ AP
      setupAP();
    }
  } else {
    setupAP();
  }
}

void loop() {
  // Do nothing in the loop
}

void setupAP() {
  IPAddress local_IP(8, 8, 8, 8); // Địa chỉ IP tĩnh bạn muốn đặt cho ESP32
  IPAddress gateway(8, 8, 8, 8);
  IPAddress subnet(255, 255, 255, 0);

  // Kiểm tra nếu thiết lập IP tĩnh không thành công
  if (!WiFi.softAPConfig(local_IP, gateway, subnet)) {
    Serial.println("Cấu hình IP tĩnh cho AP thất bại!");
  }
  
  WiFi.softAP(apSSID);
  startWebServer();
  while (true) {
    server.handleClient();
    delay(10); // Small delay to prevent watchdog reset
  }
}

void startWebServer() {
  server.on("/", handleRoot);
  server.on("/save", handleSave);
  server.begin();
  Serial.println("HTTP server đã được khởi động");
}

void handleRoot() {
  String html = R"rawliteral(
<!DOCTYPE html>
<html lang="vi">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ESP32 WiFi Manager</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            background-color: #f0f0f0;
            margin: 0;
            padding: 20px;
            display: flex;
            justify-content: center;
            align-items: center;
            min-height: 100vh;
        }
        .container {
            background-color: white;
            padding: 30px;
            border-radius: 8px;
            box-shadow: 0 0 10px rgba(0,0,0,0.1);
            width: 100%;
            max-width: 400px;
        }
        h1 {
            color: #333;
            text-align: center;
            margin-bottom: 20px;
        }
        form {
            display: flex;
            flex-direction: column;
        }
        label {
            margin-bottom: 5px;
            color: #666;
        }
        input[type="text"], input[type="password"] {
            padding: 10px;
            margin-bottom: 15px;
            border: 1px solid #ddd;
            border-radius: 4px;
            font-size: 16px;
        }
        input[type="submit"] {
            background-color: #4CAF50;
            color: white;
            border: none;
            padding: 12px;
            font-size: 16px;
            cursor: pointer;
            border-radius: 4px;
            transition: background-color 0.3s;
        }
        input[type="submit"]:hover {
            background-color: #45a049;
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>Node Farm</h1>
        <form action="/save" method="POST">
            <label for="ssid">Tên mạng (SSID):</label>
            <input type="text" id="ssid" name="ssid" required>
            
            <label for="password">Mật khẩu:</label>
            <input type="password" id="password" name="password" required>
            
            <input type="submit" value="Lưu và Kết nối">
        </form>
    </div>
</body>
</html>
)rawliteral";

  server.send(200, "text/html", html);
}

void handleSave() {
  String ssid = server.arg("ssid");
  String password = server.arg("password");
  
  writeEEPROM(0, ssid);
  writeEEPROM(50, password);
  
  String html = R"rawliteral(
<!DOCTYPE html>
<html lang="vi">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Đã lưu cấu hình</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            background-color: #f0f0f0;
            margin: 0;
            padding: 20px;
            display: flex;
            justify-content: center;
            align-items: center;
            min-height: 100vh;
        }
        .container {
            background-color: white;
            padding: 30px;
            border-radius: 8px;
            box-shadow: 0 0 10px rgba(0,0,0,0.1);
            width: 100%;
            max-width: 400px;
            text-align: center;
        }
        h1 {
            color: #4CAF50;
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>Đã lưu cấu hình</h1>
        <p>Hệ thống sẽ khởi động lại và thử kết nối với mạng WiFi mới.</p>
    </div>
</body>
</html>
)rawliteral";

  server.send(200, "text/html", html);
  delay(3000);
  ESP.restart();
}

bool testWifi() {
  int c = 0;
  Serial.println("Đang chờ kết nối WiFi");
  while (c < 20) {
    if (WiFi.status() == WL_CONNECTED) {
      return true;
    }
    delay(500);
    Serial.print(".");
    c++;
  }
  Serial.println("");
  Serial.println("Không thể kết nối, chuyển sang chế độ AP");
  return false;
}

String readEEPROM(int address) {
  String data = "";
  for (int i = 0; i < 32; ++i) {
    char c = EEPROM.read(address + i);
    if (c == 0) break;
    data += c;
  }
  return data;
}

void writeEEPROM(int address, String data) {
  for (int i = 0; i < data.length(); ++i) {
    EEPROM.write(address + i, data[i]);
  }
  EEPROM.write(address + data.length(), 0);
  EEPROM.commit();
}
