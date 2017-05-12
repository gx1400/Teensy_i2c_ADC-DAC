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
const uint8_t LED = LED_BUILTIN; //Teensy LED_BUILTIN is pin13
boolean ledstate = 0;
uint16_t voltset = 4095;

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
}

void loop() {
  int16_t results;
  float scale = 0.0001875;
  if (ledstate) {
    ledstate = 0;
  }
  else {
    ledstate = 1;
  }

  if (voltset == 4095) {
    voltset = 2048;
  }
  else {
    voltset = 4095;
  }
  
  digitalWrite(LED, ledstate);
  dac.setVoltage(voltset, false);
  results = ads.readADC_Differential_0_1(); 
  Serial.print("Raw: ");
  Serial.println(results);
  Serial.print("Scaled: ");
  Serial.println(results * scale);
  delay(2000);

  
}
