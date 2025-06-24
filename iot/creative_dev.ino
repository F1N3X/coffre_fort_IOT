#include <ESP32Servo.h>
#include <Adafruit_NeoPixel.h>

int DataIn = 7;
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(1, DataIn, NEO_GRB + NEO_KHZ800);


const int CODE_LENGTH = 4;
const int Code[CODE_LENGTH] = {1, 2, 3, 1};

int ButtonStep = 0;

const int buttonPins[3] = {2, 3, 4};
bool lastButtonStates[3] = {false, false, false};

  
int temp = 0;
Servo myservo;  // create servo object to control a servo

int val;    // variable to read the value from the analog pin

void setup() {
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);
  pinMode(2, INPUT_PULLUP);
  pinMode(3, INPUT_PULLUP);
  pinMode(4, INPUT_PULLUP);

  Serial.begin(115200);

  myservo.setPeriodHertz(50);
  myservo.attach(44, 500, 2000);
  myservo.write(0);

  pixels.begin();
  pixels.setBrightness(200);

}

void loop() {
  delay(15);
  if (temp == 0) {
    temp = 1;
    Serial.println("COUCOU");
  }
  // for (int pos=0; pos <= 180; pos += 1){
  //   myservo.write(pos);
  //   delay(15);
  // } 
  // for (int pos=180; pos >= 0; pos -= 1){
  //   myservo.write(pos);
  //   delay(15);
  // }
  for (int i = 0; i < 3; i++) {
    bool currentState = digitalRead(buttonPins[i]) == LOW;

    if (currentState && !lastButtonStates[i]) {
      // Appui détecté (front montant)
      handleButtonPress(i + 1); // bouton numéroté de 1 à 3
    }

    lastButtonStates[i] = currentState;
  }

  // for (int i = 2; i<5; i+=1){
  //   if(digitalRead(i) == HIGH){
  //     handleButtonPress(i-1);
  //     Serial.print(i);
  //   }
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
    
    ButtonStep = 0;
    pixels.setPixelColor(0, pixels.Color(255, 10, 10));
    pixels.show();
  }
}


void onCodeSuccess() {
  Serial.println("Code correct !");
  pixels.setPixelColor(0, pixels.Color(10, 255, 10));
  pixels.show();
  myservo.write(90);

}
