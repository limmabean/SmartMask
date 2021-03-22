#include <ArduinoBLE.h>

/* 
// This file is subject to the terms and conditions defined in
// file 'LICENSE.md', which is part of this source code package.
*/

#include "EEPROM.h"

#define EEPROM_PREFS_IDX  (0)
#define EEPROM_VALID_CODE (0xAB)
#define DEFAULT_VER_MAJOR (0)
#define DEFAULT_VER_MINOR (0)
#define DEFAULT_VER_PATCH (0)

#define RESOLUTION_BITS (10)
#ifdef ADCPIN
#define EXTERNAL_ADC_PIN ADCPIN
#endif

typedef struct {
  uint8_t valid = EEPROM_VALID_CODE;
  uint8_t ver_major = DEFAULT_VER_MAJOR;
  uint8_t ver_minor = DEFAULT_VER_MINOR;
  uint8_t ver_patch = DEFAULT_VER_PATCH;
} preferences_t;

preferences_t prefs;

void setup() {
  Serial.begin(115200);
  Serial.println("SmartMask - Debug Output");
  EEPROM.init();

  analogReadResolution(RESOLUTION_BITS);    // set the resolution of analogRead results
                                            //  - maximum: 16 bits (padded with trailing zeroes)
                                            //  - ADC:     14 bits (maximum ADC resolution)
                                            //  - default: 10 bits (standard Arduino setting)
                                            //  - minimum:  1 bit
  analogWriteResolution(RESOLUTION_BITS);   // match resolution for analogWrite

  // use EEPROM.get(int index, T type) to retrieve
  // an arbitrary type from flash memory
  prefs.valid = 0x00;
  EEPROM.get(EEPROM_PREFS_IDX, prefs);

  if(prefs.valid != EEPROM_VALID_CODE){
    Serial.println("EEPROM was invalid");
    
    // use EEPROM to store the default structure
    preferences_t default_prefs;
    EEPROM.put(EEPROM_PREFS_IDX, default_prefs);
    
    Serial.println("EEPROM initialized");
  }

  // verify that the prefs are valid
  EEPROM.get(EEPROM_PREFS_IDX, prefs);
  if(prefs.valid != EEPROM_VALID_CODE){
    Serial.println("ERROR");
    while(1){};
  }

  Serial.println("EEPROM is valid");

  Serial.printf("version: %d.%d.%d\n", prefs.ver_major, prefs.ver_minor, prefs.ver_patch);

  Serial.printf("\nany characters received over SERIAL will increment the patch version and be stored after power-down\n");
}

void loop() {
  // Store EEPROM
  if (Serial.available()) {
    while (Serial.available()) {
      Serial.read();
      prefs.ver_patch++;
    }
    EEPROM.put(EEPROM_PREFS_IDX, prefs);
    Serial.printf("version: %d.%d.%d\n", prefs.ver_major, prefs.ver_minor, prefs.ver_patch);
  }

  // Read Analog
  #ifdef ADCPIN
  int external = analogRead(EXTERNAL_ADC_PIN); // reads the analog voltage on the selected analog pin
  Serial.printf("external (counts): %d, ", external);
  analogWrite(LED_BUILTIN, external);
  #endif

  int vcc_3 = analogReadVCCDiv3();    // reads VCC across a 1/3 voltage divider
  int vss = analogReadVSS();          // ideally 0
  int temp_raw = analogReadTemp();    // raw ADC counts from die temperature sensor
  
  float temp_f = getTempDegF();       // computed die temperature in deg F
  float vcc_v = getVCCV();            // computed supply voltage in V

  Serial.printf("temp (counts): %d, vcc/3 (counts): %d, vss (counts): %d, time (ms) %d\n", temp_raw, vcc_3, vss, millis());
  
}
