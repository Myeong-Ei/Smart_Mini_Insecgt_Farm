#include <EEPROM.h>
#include <MsTimer2.h>
#include <SoftwareSerial.h>
#include <DHT.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>

// pump_motor : D6, soil_moisture sensor : A0, BT_TX : D10, BT_RX : D11, DHT22 : D4, Heater_fan : D8, Heater : D9
// EEPROM의 0번지는 온도 값, 1번지는 습도 값이다.
// Relay pin list
// IN1 : none, IN2 : Humidifier, IN3 : Fan, IN4 : Pump motor
SoftwareSerial BT(10, 11);
DHT dht(4, DHT22);
LiquidCrystal_I2C lcd (0x27, 16, 2);
int humi;
int temp;
int soil;
int temp_read;
int humi_read;
boolean fan_state = false;
boolean pump_motor_state = false;
boolean humidifier_state = false;
String input = "";

//LCD 아이콘
// 0 : fan_icon, 1 : temp_icon, 2 : humi_icon, 3 : heater_icon, 4 : pump_icon
byte fan_icon[8] = {
        B11111,
        B10001,
        B11011,
        B11101,
        B10111,
        B11011,
        B10001,
        B11111
};
byte temp_icon[8] = {
        B00100,
        B01010,
        B01010,
        B01010,
        B01010,
        B10001,
        B10001,
        B01110
};
byte humi_icon[8] = {
        B00000,
        B00100,
        B01010,
        B01010,
        B10001,
        B10001,
        B10001,
        B01110
};
byte heater_icon[8] = {
        B11111,
        B11111,
        B10101,
        B10101,
        B10001,
        B10101,
        B10101,
        B11111
};
byte pump_icon[8] = {
        B00100,
        B00100,
        B00100,
        B11111,
        B11111,
        B11111,
        B11111,
        B00000
};


void start_logo();
String get_string(String input);
void send_temp_value(int temp);
void send_humi_value(int humi);
void send_soil_value(int soil);
void relay_working(int pin, int state);

// 0 : fan_icon, 1 : temp_icon, 2 : humi_icon, 3 : heater_icon, 4 : pump_icon

void setup() {
  pinMode(A0, INPUT);
  pinMode(5, OUTPUT);
  pinMode(6, OUTPUT);
  pinMode(7, OUTPUT);
  pinMode(8, OUTPUT);
  pinMode(13, OUTPUT);
  digitalWrite(5, HIGH);
  digitalWrite(6, HIGH);
  digitalWrite(7, HIGH);
  digitalWrite(8, HIGH);
  Serial.begin(9600);
  BT.begin(9600);
  dht.begin();
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.createChar(0, fan_icon);
  lcd.createChar(1, temp_icon);
  lcd.createChar(2, humi_icon);
  lcd.createChar(3, heater_icon);
  lcd.createChar(4, pump_icon);
  start_logo();
}
void loop() {
  input = get_string(input);
  temp_read = dht.readTemperature();
  humi_read = dht.readHumidity();
  soil = analogRead(A0);
  send_temp_value(temp_read);
  send_humi_value(humi_read);
  send_soil_value(soil);
  Serial.println(input);
  
  if(input.substring(0, 1) == "t"){
    temp = input.substring(1).toInt();
    EEPROM.write(0, temp);
  }
  else if(input.substring(0, 1) == "h"){
    humi = input.substring(1).toInt();
    EEPROM.write(1, humi);
  }
  
  if(EEPROM.read(0) > temp_read){
    relay_working(7, LOW);
    fan_state = true;
  }
  else if(EEPROM.read(0) < temp_read){
    relay_working(7, HIGH);
    fan_state = false;
  }
  
  if(EEPROM.read(1) > humi_read){
    relay_working(6, LOW);
    humidifier_state = true;
  }
  else if(EEPROM.read(1) < humi_read){
    relay_working(6, HIGH);
    humidifier_state = false;
  }
  
  if(input.substring(0, 2) == "ms"){
    relay_working(8, LOW);
    pump_motor_state == true;
    digitalWrite(13, HIGH);
    delay(3000);
    relay_working(8, HIGH);
    digitalWrite(13, LOW);
    pump_motor_state = false;
  }
  else{
      if(soil < 500){
        relay_working(8, LOW);
        pump_motor_state = true;
      }
      else if(soil > 500){
        relay_working(8, HIGH);
        pump_motor_state = false;
      }
  }  
  
  Serial.print("Now Temperature : ");
  Serial.print(temp_read);
  Serial.print(", ");
  Serial.print("Temperature value : ");
  Serial.print(EEPROM.read(0));
  Serial.print(" | ");
  Serial.print("Now Humidity : ");
  Serial.print(humi_read);
  Serial.print(", ");
  Serial.print("Humidity value : ");
  Serial.println(EEPROM.read(1));
  Serial.print("\n");
  Serial.print("Soil moisture sensor : ");
  Serial.println(soil);
  
  
  lcd.setCursor(0, 0);
  lcd.write(1);
  lcd.print(" ");
  lcd.print(temp_read);
  lcd.setCursor(5, 0);
  lcd.write(2);
  lcd.print(" ");
  lcd.print(humi_read);
  lcd.setCursor(10, 0);
  lcd.write(0);
  lcd.print(" ");
  if(fan_state == true) lcd.print("ON ");
  else lcd.print("OFF");
  lcd.setCursor(0, 1);
  lcd.write(2);
  lcd.print(" ");
  if(humidifier_state == true) lcd.print("ON ");
  else lcd.print("OFF");
  lcd.setCursor(6, 1);
  lcd.write(4);
  lcd.print(" ");
  if(pump_motor_state == true) lcd.print("ON ");
  else lcd.print("OFF");
  
  input = "";
}
String get_string(String input){
  String inputString = "";
  while(BT.available()){
    if(BT.available() > 0){
      char c = BT.read();
      inputString += c;
    }
  }
  if(inputString.length() == 0) return input;
  else return inputString;
}
void send_temp_value(int temp){
  BT.print("t");
  BT.println(temp);
}
void send_humi_value(int humi){
  BT.print("h");
  BT.println(humi);
}
void send_soil_value(int soil){
  BT.print("s");
  BT.println(soil);
}
void start_logo(){
  lcd.clear();
  Serial.println("SMART Insect Mini farm START!");
  lcd.setCursor(2, 0);
  lcd.print("SMART Insect");
  delay(700);
  lcd.setCursor(3, 1);
  lcd.print("Mini farm");
  delay(1000);
  lcd.clear();
}
void relay_working(int pin, int state){
  digitalWrite(pin, state);
}
