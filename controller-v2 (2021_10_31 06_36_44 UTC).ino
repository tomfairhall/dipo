#include <Servo.h>
#include <PPMReader.h>

//pins
const byte INA1Pin      = 9;
const byte INA2Pin      = 10;
const byte PWMAPin      = 11;
const byte RxSignalPin  = 3;
const byte servoPin     = 2;

//constant values
const byte channelAmount    = 8;  //#
const byte interval         = 10; //ms

//variable values
int steerChannel = 1500;  //ms
int throtChannel = 1500;  //ms

int setThrot = 0;
int actThrot = 0;

byte actAngle = 90;       //deg
byte setAngle = 90;       //deg

byte forwardsMagnitude  = 0;
byte backwardsMagnitude = 0;
unsigned long previousMillis  = 0;
unsigned long currentMillis   = 0;

//create objects
PPMReader ppm(RxSignalPin, channelAmount);
Servo servo;

//motor functions
void motorCoast() {
  digitalWrite(INA1Pin, LOW);
  digitalWrite(INA1Pin, LOW);
  actThrot = 0;
  analogWrite(PWMAPin, 0);
}

void motorForwards(int speed) {
  digitalWrite(INA1Pin, HIGH);
  digitalWrite(INA2Pin, LOW);
  actThrot = actThrot + speed;
  analogWrite(PWMAPin, actThrot);
}

void motorBackwards(int speed) {
  digitalWrite(INA1Pin, LOW);
  digitalWrite(INA2Pin, HIGH);
  actThrot = actThrot - speed;
  analogWrite(PWMAPin, abs(actThrot));
}

void motorSlow() {
  if (actThrot > setThrot) {
    motorBackwards(1);
  } 
  else if (actThrot < setThrot) {
    motorForwards(1);
  }
}

//servo functions
void servoCoast() {
  actAngle = 90;
  servo.write(90);
}

void servoForwards() {
  actAngle = actAngle + 1;
  servo.write(actAngle);
}

void servoBackwards() {
  actAngle = actAngle - 1;
  servo.write(actAngle);
}

void setup() {
  Serial.begin(115200);

  servo.attach(servoPin);

  //defining pins as inputs/outputs/ect.
  pinMode(INA1Pin, OUTPUT);
  pinMode(INA2Pin, OUTPUT);
  pinMode(PWMAPin, OUTPUT);
  pinMode(RxSignalPin, INPUT);
  pinMode(servoPin, INPUT);
}

void loop() {
  currentMillis = millis();

  if(currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    throtChannel = ppm.latestValidChannelValue(2, 1500);
    steerChannel = ppm.latestValidChannelValue(1, 1500);
    
    //print signal value
    Serial.print("S: " + String(steerChannel) + "\t"); //debug
    Serial.print("T: " + String(throtChannel) + "\t"); //debug

    //set and print the desired angle of the servo based of the input to a degree
    setAngle = constrain(map(steerChannel, 1000, 2000, 50, 120), 0, 180);
    setThrot = constrain(map(throtChannel, 1000, 2000, -255, 255), -255, 255);

    Serial.print("SA: " + String(setAngle) + "\t"); //debug
    Serial.print("ST: " + String(setThrot) + "\t");

    //determine which direction the servo has to move
    if(abs(setAngle-actAngle) < 3 && actAngle < 93 && actAngle > 87) {
      servoCoast(); 
    }
    else if(setAngle > actAngle) {
      servoForwards();
    }
    else if(setAngle < actAngle) {
      servoBackwards();
    }

    //print the current angle of the servo
    Serial.print("AA: " + String(actAngle) + "\t");

    //determine which direction the motor has to move (if any)
    if (setThrot > 15) {
      motorForwards(1);
    }
    else if (setThrot < 10) {
      motorBackwards(1);
    }
    if (setThrot < -15) {
      motorBackwards(1);
    }
    else if (setThrot > 10) {
      motorForwards(1);
    }
    else {
      motorCoast();
    }

    //print the current 
    Serial.print("AT: " + String(actThrot) + "\t");
  
    Serial.println();
  }
}
