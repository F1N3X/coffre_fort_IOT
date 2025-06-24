#include <ESP32Servo.h>
#include <Adafruit_NeoPixel.h>
#include <WiFi.h>
#include <WebServer.h>
#include <Adafruit_MPU6050.h>

int DataIn = 7;
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(1, DataIn, NEO_GRB + NEO_KHZ800);

const char* ssid = "TestESP32";
const char* ssidpassword = "motdepasse";
WebServer server(80);

const int CODE_LENGTH = 4;
const int Code[CODE_LENGTH] = {1, 2, 3, 1};
const char* finalCode = "1231";
int ButtonStep = 0;

const int buttonPins[3] = {2, 3, 4};
bool lastButtonStates[3] = {false, false, false};

Servo myservo;  // create servo object to control a servo

Adafruit_MPU6050 mpu;


void setup() {
  Serial.begin(115200);

  WiFi.softAP(ssid, ssidpassword);
  Serial.println("Serveur web actif !");
  Serial.println(WiFi.softAPIP());
  server.on("/", handleRoot);
  server.on("/code", handleCode);
  server.begin();

  pinMode(2, INPUT_PULLUP);
  pinMode(3, INPUT_PULLUP);
  pinMode(4, INPUT_PULLUP);


  myservo.setPeriodHertz(50);
  myservo.attach(44, 500, 2000);
  myservo.write(0);

  pixels.begin();
  pixels.setBrightness(200);
  pixels.setPixelColor(0, pixels.Color(10, 10, 200));
  pixels.show();

   if (!mpu.begin()) { // Change address if needed
 			Serial.println("Failed to find MPU6050 chip");
 			while (1) {
 					delay(10);
 			}
 	}
  Serial.println("MPU6050 found");

  mpu.setAccelerometerRange(MPU6050_RANGE_16_G);
 	mpu.setGyroRange(MPU6050_RANGE_250_DEG);
 	mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

}

void loop() {
  server.handleClient(); // <-- Appels HTTP gérés ici
  Serial.print(".");
  delay(15);
  for (int i = 0; i < 3; i++) {
    bool currentState = digitalRead(buttonPins[i]) == LOW;

    if (currentState && !lastButtonStates[i]) {
      // Appui détecté (front montant)
      handleButtonPress(i + 1); // bouton numéroté de 1 à 3
    }

    lastButtonStates[i] = currentState;
  }
  sensors_event_t a, g, temp;
 	mpu.getEvent(&a, &g, &temp);
  if(g.gyro.x > 0.3 || g.gyro.y > 0.3 || g.gyro.z > 0.3 || g.gyro.x < -0.3 || g.gyro.y < -0.3 || g.gyro.z < -0.3 ){
    pixels.setPixelColor(0, pixels.Color(255, 10, 10));
    pixels.show();
    delay(800);

  } else {
    pixels.setPixelColor(0, pixels.Color(10, 10, 255));
    pixels.show();
  }

  // Serial.println("formater");
  // if (a.acceleration.x <0){
  //   Serial.print("x < 0 ");
  // } else {
  //   Serial.print("x > 0 ");
  // }

  // if (a.acceleration.y <0){
  //   Serial.print("y < 0 ");
  // } else {
  //   Serial.print("y > 0 ");
  // }

  // if (a.acceleration.z <0){
  //   Serial.print("z < 0 ");
  // } else {
  //   Serial.print("z > 0 ");
  // }

}


void handleButtonPress(int ButtonPressed) {
  Serial.println(ButtonPressed);
  if (ButtonPressed == Code[ButtonStep]) {
    ButtonStep++;
    pixels.setPixelColor(0, pixels.Color(10, 10, 10));
    pixels.show();
    if (ButtonStep >= CODE_LENGTH) {
      onCodeSuccess();
    }
  } else {
    onCodeError()
  }
}


void onCodeSuccess() {
  Serial.println("Code correct !");
  pixels.setPixelColor(0, pixels.Color(10, 255, 10));
  pixels.show();
  myservo.write(90);
  delay(800);

}
void onCodeError() {
    ButtonStep = 0;
    pixels.setPixelColor(0, pixels.Color(255, 10, 10));
    pixels.show();
    pixels.setPixelColor(0, pixels.Color(255, 10, 10));
    pixels.show();
    myservo.write(0);
    delay(800);
}
void handleCode() {
  if (server.hasArg("code")) {
    String userCode = server.arg("code");
    if (userCode == finalCode) {
      server.send(200, "text/plain", "Code correct ! Coffre ouvert.\nÉtat: ");
      onCodeSuccess();
    } else {
      server.send(403, "text/plain", "Code incorrect !");
      onCodeError()
    }
  } else {
    server.send(400, "text/plain", "Paramètre 'code' manquant.");
  }
}

void handleRoot() {
  server.send(200, "text/html", R"rawliteral(
<!DOCTYPE html>
<html lang="fr">
<head>
  <meta charset="UTF-8">
  <title>Coffre connecté</title>
  <style>
    body {
      margin: 0; padding: 0;
      font-family: Arial, sans-serif;
      background: linear-gradient(135deg, #2c3e50, #3498db);
      display: flex; justify-content: center; align-items: center;
      height: 100vh; color: #fff;
    }
    .box {
      background: #ffffffcc; color: #222;
      padding: 20px 30px; border-radius: 12px;
      box-shadow: 0 0 20px rgba(0,0,0,0.3);
      text-align: center; width: 90%; max-width: 400px;
    }
    input {
      padding: 10px; width: 100%;
      border: 2px solid #3498db;
      border-radius: 8px; margin-bottom: 15px;
      font-size: 16px;
    }
    button {
      padding: 10px 20px;
      background: #3498db; color: white;
      border: none; border-radius: 8px;
      font-size: 16px; cursor: pointer;
    }
    button:hover { background: #2980b9; }
    #reponse { margin-top: 15px; font-weight: bold; }
  </style>
</head>
<body>
  <div class="box">
    <h2>Déverrouiller le coffre</h2>
    <input type="text" id="code" placeholder="Entrez le code" />
    <button onclick="envoyerCode()">Valider</button>
    <p id="reponse"></p>
  </div>
  <script>
    function envoyerCode() {
      const code = document.getElementById("code").value;
      fetch("/code?code=" + encodeURIComponent(code))
        .then(res => res.text())
        .then(txt => document.getElementById("reponse").innerText = txt)
        .catch(err => document.getElementById("reponse").innerText = "Erreur : " + err);
    }
  </script>
</body>
</html>
  )rawliteral");
}