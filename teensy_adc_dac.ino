// Using adafruit MCP4725 & ADS1115 development boards
// Using github libraries for chips.
// - For ADS1115: https://learn.adafruit.com/adafruit-4-channel-adc-breakouts/programming
//   Had to be updated to change references in .c and .h to Wire.h to i2c_t3.h
// - For MCP4725: https://github.com/jopiek/Arduino/tree/master/Teensy_MCP4725
//
//
// Starting scan...
// Addr: 0x48 Ack - ADC
// Addr: 0x62 Ack - DAC

  // The ADC input range (or gain) can be changed via the following
  // functions, but be careful never to exceed VDD +0.3V max, or to
  // exceed the upper and lower limits if you adjust the input range!
  // Setting these values incorrectly may destroy your ADC!
  //                                                                ADS1015  ADS1115
  //                                                                -------  -------
  // ads.setGain(GAIN_TWOTHIRDS);  // 2/3x gain +/- 6.144V  1 bit = 3mV      0.1875mV (default)
  // ads.setGain(GAIN_ONE);        // 1x gain   +/- 4.096V  1 bit = 2mV      0.125mV
  // ads.setGain(GAIN_TWO);        // 2x gain   +/- 2.048V  1 bit = 1mV      0.0625mV
  // ads.setGain(GAIN_FOUR);       // 4x gain   +/- 1.024V  1 bit = 0.5mV    0.03125mV
  // ads.setGain(GAIN_EIGHT);      // 8x gain   +/- 0.512V  1 bit = 0.25mV   0.015625mV
  // ads.setGain(GAIN_SIXTEEN);    // 16x gain  +/- 0.256V  1 bit = 0.125mV  0.0078125mV

#include <i2c_t3.h>
#include <Teensy_MCP4725.h>
#include <Adafruit_ADS1015.h>

Teensy_MCP4725 dac;
Adafruit_ADS1115 ads;
IntervalTimer tmrChange;

const uint8_t LED = LED_BUILTIN; //Teensy LED_BUILTIN is pin13
const float MAX_CURRENT = 5.00;
const float MIN_CURRENT = 0.00;
const float MAX_VOLTAGE = 3.00;
const float MIN_VOLTAGE = 0.00;

boolean ledstate = 0;
uint16_t voltset = 4095;
String inputString;
boolean strComplete = false;
float scale = 0.0001875;
int16_t results = 0.0;
float fResult = 0.0;

float vset = 0.00;
float iset = 0.00;

void setup() {
  // put your setup code here, to run once:
  pinMode(LED, OUTPUT);

  Serial.begin(9600);
  Serial.println("Starting program...");
  delay(1000);
 
  // For Adafruit MCP4725A1 the address is 0x62 (default) or 0x63 (ADDR pin tied to VCC)
  // For MCP4725A0 the address is 0x60 or 0x61
  // For MCP4725A2 the address is 0x64 or 0x65

  Wire.begin(I2C_MASTER, 0x00, I2C_PINS_18_19, I2C_PULLUP_INT, I2C_RATE_400);
  dac.begin(0x62);
  ads.begin();

  delay(500);
  ReadDAC();
  vset = fResult - 0.0045; // keep previous value, but DAC has a non-zero offset, so won't ever match
  tmrChange.begin(Change, 1000000);
}

void loop() {
  if (strComplete) {
    Serial.println("Read serial data!");
    Serial.print(inputString);
    String strOut = "";
    char cComm = inputString[0];
    String sValue = "";
    float fValue = 0;
    
    switch (cComm) {
      case 'v':
        fValue = inputString.substring(1).toFloat();
        fValue = checkVoltage(fValue);
        strOut = "Setting voltage..." + String(fValue, 4);
        setVoltage(fValue);
        break;
      case 'i':
        fValue = inputString.substring(1).toFloat();
        fValue = checkCurrent(fValue);
        strOut = "Setting current..." + String(fValue, 4);;
        setCurrent(fValue);
        break;
      case 's':
        strOut = "Printing setpoints...";
        break;
      default:
        strOut = "Command not recognized";
      break;
    }
    Serial.println(strOut);

    
    Serial.print("Vset: ");
    strOut = String(vset, 4);
    Serial.print(strOut);
    Serial.println("V");

    Serial.print("Iset: ");
    strOut = String(iset, 4);
    Serial.print(strOut);
    Serial.println("A");
    
    inputString = "";
    strComplete = false;
  }
}

void Change(void) {
  
  if (ledstate) {
    ledstate = 0;
  }
  else {
    ledstate = 1;
  }
  
  digitalWrite(LED, ledstate);
  uint16_t uSetpoint = scaleVoltage(vset);
  dac.setVoltage(uSetpoint, true);
  ReadDAC();
}

void serialEvent() {
  while (Serial.available()) {
    char inChar = (char)Serial.read();

    inputString += inChar;

    if (inChar == '\n') {
      strComplete = true;
    }
  }
}

void setVoltage(float volts) {
  vset = volts;
}

void setCurrent(float current) {
  iset = current;
}

float checkVoltage(float volts) {
  float out = vset;
  if (volts <= MAX_VOLTAGE && volts >= MIN_VOLTAGE) {
    Serial.println("Voltage checked ok!");
    out = volts;
  }
  else {
    Serial.println("ERROR: Voltage out of range!");
    out = vset;
  }

  return out;
}

float checkCurrent(float amps) {
  float out = iset;
  if (amps <= MAX_CURRENT && amps >= MIN_CURRENT) {
    Serial.println("Current checked ok!");
    out = amps;
  }
  else {
    Serial.println("ERROR: Current out of range!");
    out = iset;
  }

  return out;
}

uint16_t scaleVoltage(float setpoint){
  uint16_t out = 0;
  float scaler = 3.3313 / 4096;
  out = setpoint / scaler;
  //Serial.print("Setpoint: ");
  //Serial.println(setpoint);
  //Serial.print("Scaler: ");
  //Serial.println(out);
  
  return out;
}

void ReadDAC(void) {
  results = ads.readADC_Differential_0_1(); 
  Serial.print("Raw: ");
  Serial.println(results);

  String strOut = "";
  fResult = results * scale;
  strOut = String(fResult, 4);
  Serial.print("Scaled: ");
  Serial.print(strOut);
  Serial.println("V");
}

