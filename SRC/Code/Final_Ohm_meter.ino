#include<ADS1115_WE.h>
#include<Wire.h>
#include "LCD.h"
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7);
const int MUXEnable = 3;
const int MUXA = 4;
const int MUXB = 5;
const int MUXC = 6;
const int DigiPin = 7;
const int BranchZEnable = 8;
const int BranchZOut = 9;
int calibrateRes = 0;
boolean gained;
float bias = 1.353;
int MUXmode = 9;
float topRes[8] = {113.5231, 401.418232, 1430.46687, 4822.11904, 15821.2380, 43714.80620, 216964.52752, 971693.16312};
float calibrate = 0.50;
#define I2C_ADDRESS 0x48
ADS1115_WE adc = ADS1115_WE(I2C_ADDRESS);
void setup() {
  // put your setup code here, to run once:
  Wire.begin();
  Serial.begin(9600);
  if (!adc.init()) {
    Serial.println("ADS1115 not connected!");

  }
  adc.setConvRate(ADS1115_64_SPS);
  adc.setMeasureMode(ADS1115_CONTINUOUS);
  pinMode(MUXEnable, OUTPUT);
  pinMode(MUXA, OUTPUT);
  pinMode(MUXB, OUTPUT);
  pinMode(MUXC, OUTPUT);
  pinMode(DigiPin, INPUT);
  pinMode(BranchZEnable, INPUT);
  pinMode(BranchZOut, OUTPUT);
  MUX(-1);

  lcd.begin(16, 2);   
  lcd.setBacklightPin(3, POSITIVE);
  flash();
}

void loop() {
  gained = false;
  float resistance = 0.0;
  float voltageTakenA[8] = {0,0,0,0,0,0,0,0};
  readChannel(ADS1115_COMP_2_GND, voltageTakenA);
  
  printVoltageRange();
  float voltageTakenB[8] = {0,0,0,0,0,0,0,0};
  readChannel(ADS1115_COMP_3_GND, voltageTakenB);
  printVoltageRange();
  float resVoltage = resVoltageChoose(voltageTakenA, voltageTakenB);
  Serial.println(voltageTakenA[MUXmode], 6);
  Serial.println(voltageTakenB[MUXmode], 6);
  Serial.println("------");
  Serial.println(gained);
  Serial.println(resVoltage, 6);
  Serial.println(MUXmode);
  if (resVoltage > 3.7 & gained == false) {
    resistance = 100000000;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Open circuit");
  }
  else if (resVoltage < 0.51/ 7.78003113816 & gained == true) {
    resistance = ((bias * (resVoltage - 5) + resVoltage * topRes[MUXmode]) / (5 - resVoltage))+ calibrateRes;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Short circuit");
  } else {
    resistance = ((bias * (resVoltage - 5) + resVoltage * topRes[MUXmode]) / (5 - resVoltage))+ calibrateRes;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(resistance, 5);

  }
  Serial.println(resistance, 5);
  if(digitalRead(DigiPin) == HIGH){
    calibrateResistance(resVoltage);
    }

}

void calibrateResistance(float in){
  if(in < 0.5){
    calibrate = calibrate - in;
    Serial.println(calibrate);
    }
  }
float resVoltageChoose(float A[], float B[]) {
  float out = A[0];
  MUXmode = 0;
  gained = true;
  for (int i = 1; i < 8; ++i) {
    Serial.println(A[i],6);
    if (abs(A[i] - 2.5) < abs(out - 2.5) || out > 3.6) {
      gained = true;
      out = A[i];
      MUXmode = i;
    }
  }
  if(out > 3.6){
    gained = false;
    }
  for (int i = 0; i < 8; ++i) {
    Serial.println(B[i],6);
    if (abs(B[i] - 2.5) < abs(out - 2.5) || out > 3.6) {
      gained = false;
      out = B[i];
      MUXmode = i;
    }
  }
  if (gained == true & out < 3.7) {
    out = out / 7.78003113816;
  }
  return out;
}
void readChannel(ADS1115_MUX channel, float in[]) {

  if (channel == ADS1115_COMP_0_GND) {
    Serial.println("Channel 0: ");
  }
  if (channel == ADS1115_COMP_1_GND) {
    Serial.println("Channel 1: ");
  }
  validResult(channel, in);

  /* setAutoRange() switches to the highest range (+/- 6144 mV), measures the current
     voltage and then switches to the lowest range where the current value is still
     below 80% of the maximum value of the range. The function is only suitable if you
     expect stable or slowly changing voltages. setAutoRange needs roughly the time you
     would need for three conversions.
     If the ADS115 is in single shot mode, setAutoRange() will switch into continuous
     mode to measure a value and switch back again.
     !!! Use EITHER this function whenever needed OR setPermanentAutoRangeMode(true) once !!!
  */
  //adc.setAutoRange(); //use either this or setPermanentAutoRangeMode(true) // alternative: getResult_mV for Millivolt
}

void flash() {
  for (int i = 0; i < 3; i++) {
    lcd.backlight();   
    delay(200);
    lcd.noBacklight();
    delay(200);
  }
  lcd.backlight();
}

int MUX(int choose) {

  lcd.setCursor(0, 1);
  lcd.print(choose);
  MUXmode = choose;
  switch (choose) {
    case 0:
      digitalWrite(MUXEnable, LOW);
      digitalWrite(MUXA, LOW);
      digitalWrite(MUXB, LOW);
      digitalWrite(MUXC, LOW);

      digitalWrite(MUXEnable, HIGH);
      break;
    case 1:
      digitalWrite(MUXEnable, LOW);
      digitalWrite(MUXA, HIGH);
      digitalWrite(MUXB, LOW);
      digitalWrite(MUXC, LOW);

      digitalWrite(MUXEnable, HIGH);
      break;
    case 2:
      digitalWrite(MUXEnable, LOW);
      digitalWrite(MUXA, LOW);
      digitalWrite(MUXB, HIGH);
      digitalWrite(MUXC, LOW);

      digitalWrite(MUXEnable, HIGH);
      break;
    case 3:
      digitalWrite(MUXEnable, LOW);
      digitalWrite(MUXA, HIGH);
      digitalWrite(MUXB, HIGH);
      digitalWrite(MUXC, LOW);

      digitalWrite(MUXEnable, HIGH);
      break;
    case 4:
      digitalWrite(MUXEnable, LOW);
      digitalWrite(MUXA, LOW);
      digitalWrite(MUXB, LOW);
      digitalWrite(MUXC, HIGH);

      digitalWrite(MUXEnable, HIGH);
      break;
    case 5:
      digitalWrite(MUXEnable, LOW);
      digitalWrite(MUXA, HIGH);
      digitalWrite(MUXB, LOW);
      digitalWrite(MUXC, HIGH);

      digitalWrite(MUXEnable, HIGH);
      break;
    case 6:
      digitalWrite(MUXEnable, LOW);
      digitalWrite(MUXA, LOW);
      digitalWrite(MUXB, HIGH);
      digitalWrite(MUXC, HIGH);

      digitalWrite(MUXEnable, HIGH);
      break;
    case 7:
      digitalWrite(MUXEnable, LOW);
      digitalWrite(MUXA, HIGH);
      digitalWrite(MUXB, HIGH);
      digitalWrite(MUXC, HIGH);

      digitalWrite(MUXEnable, HIGH);
      break;
    default:
      digitalWrite(MUXEnable, LOW);
      digitalWrite(MUXA, LOW);
      digitalWrite(MUXB, LOW);
      digitalWrite(MUXC, LOW);

      digitalWrite(MUXEnable, LOW);
      break;
  }
  int Z = digitalRead(BranchZEnable);
  digitalWrite(BranchZOut, !Z);
  delay(20);
  return choose;

}
void validResult(ADS1115_MUX channel, float in[]) {
  for (int i = 0; i < 8; i++) {
    float voltage;
    MUXmode = MUX(i);
    adc.setCompareChannels(channel);
    //adc.setAutoRange();
    adc.setVoltageRange_mV(ADS1115_RANGE_4096);
    voltage = adc.getResult_V() + calibrate;
    in[i] = voltage;
    Serial.print(in[i], 6);
    Serial.print(" | ");
    lcd.setCursor(5, 1);
    lcd.print(in[i], 6);
    //waitTil();
  }

  /* setAutoRange() switches to the highest range (+/- 6144 mV), measures the current
     voltage and then switches to the lowest range where the current value is still
     below 80% of the maximum value of the range. The function is only suitable if you
     expect stable or slowly changing voltages. setAutoRange needs roughly the time you
     would need for three conversions.
     If the ADS115 is in single shot mode, setAutoRange() will switch into continuous
     mode to measure a value and switch back again.
     !!! Use EITHER this function whenever needed OR setPermanentAutoRangeMode(true) once !!!
  */
  //adc.setAutoRange(); //use either this or setPermanentAutoRangeMode(true)

}
void waitTil() {
  int Signal = digitalRead(DigiPin);
  while (Signal == LOW) {
    delay(1);
    Signal = digitalRead(DigiPin);
  }
  delay(1000);
}

void printVoltageRange() {
  unsigned int voltageRange = adc.getVoltageRange_mV();
  Serial.println("Range: ");

  switch (voltageRange) {
    case 6144:
      Serial.print("+/- 6144 mV, Voltage [V]: ");
      break;
    case 4096:
      Serial.print("+/- 4096 mV, Voltage [V]: ");
      break;
    case 2048:
      Serial.print("+/- 2048 mV, Voltage [V]: ");
      break;
    case 1024:
      Serial.print("+/- 1024 mV, Voltage [V]: ");
      break;
    case 512:
      Serial.print("+/- 512 mV, Voltage [V]: ");
      break;
    case 256:
      Serial.print("+/- 256 mV, Voltage [V]: ");
      break;
    default:
      Serial.println("Something went wrong");
  }
}
