#include <bluefruit.h>


#define VBAT_MV_PER_LSB   (0.73242188F)   // 3.0V ADC range and 12-bit ADC resolution = 3000mV/4096

#ifdef NRF52840_XXAA
#define VBAT_DIVIDER      (0.5F)          // 150K + 150K voltage divider on VBAT
#define VBAT_DIVIDER_COMP (2.0F)          // Compensation factor for the VBAT divider
#else
#define VBAT_DIVIDER      (0.71275837F)   // 2M + 0.806M voltage divider on VBAT = (2M / (0.806M + 2M))
#define VBAT_DIVIDER_COMP (1.403F)        // Compensation factor for the VBAT divider
#endif

#define REAL_VBAT_MV_PER_LSB (VBAT_DIVIDER_COMP * VBAT_MV_PER_LSB)


float readVBAT(void) {
  float raw;

  // Set the analog reference to 3.0V (default = 3.6V)
  analogReference(AR_INTERNAL_3_0);

  // Set the resolution to 12-bit (0..4095)
  analogReadResolution(12); // Can be 8, 10, 12 or 14

  // Let the ADC settle
  delay(1);

  // Get the raw 12-bit, 0..3000mV ADC value
  raw = analogRead(PIN_VBAT);

  // Set the ADC back to the default settings
  analogReference(AR_DEFAULT);
  analogReadResolution(10);

  // Convert the raw value to compensated mv, taking the resistor-
  // divider into account (providing the actual LIPO voltage)
  // ADC range is 0..3000mV and resolution is 12-bit (0..4095)
  return raw * REAL_VBAT_MV_PER_LSB;
}

uint8_t mvToPercent(float mvolts) {
  if(mvolts < 3300)
    return 0;

  if(mvolts < 3600) {
    mvolts -= 3300;
    return mvolts / 30;
  }

  mvolts -= 3600;
  return 10 + (mvolts * 0.15F );  // thats mvolts /6.66666666
}

void setup() 
{
  ble_gap_addr_t gaddr;
  gaddr.addr_type = 0;
  // BT MAC address, in reverse order
  gaddr.addr[5] = 'Z';
  gaddr.addr[4] = 'E';
  gaddr.addr[3] = 'U';
  gaddr.addr[2] = 'S';
  gaddr.addr[1] = 0;
  gaddr.addr[0] = 0x00;

  Bluefruit.begin();
  Bluefruit.autoConnLed(true); // We can turn this LED of to reduce power consumption
  Bluefruit.setTxPower(4);     // Check bluefruit.h for supported values
  Bluefruit.setName("baton");  // Not actually used
  Bluefruit.setAddr(&gaddr);
  startAdvertising();
}

uint8_t advertisingbuffer[13] = {
  12,   // 12 bytes after this: the type (one byte), manufacturer ID (2 bytes) and then the actual data (nine bytes)
  0xFF, // type 0xFF means "manufacturer specific data"
  0xFF, 0xFF, // manufacturer ID 0xFFFF because we're not a manufacturer in https://www.bluetooth.com/specifications/assigned-numbers/company-identifiers/
  0     // rest of array contains the actual data: first 8 bytes big-endian timestamp in millis since boot, then 1 byte battery percentage
};

// Time since boot as unsigned 64 bit uint to avoid rollover
uint64_t millis64() {
  static uint32_t low32, high32;
  uint32_t new_low32 = millis();
  if (new_low32 < low32) high32++;
  low32 = new_low32;
  return (uint64_t) high32 << 32 | low32;
}

void updateAdvertisingData() {
  uint64_t uptime = millis64();
  for (uint8_t idx = 0; idx < 8; idx++) {
    advertisingbuffer[4+idx] = (uptime >> (8*(7-idx))) & 0xFF;
  }
  float vbat_mv = readVBAT();
  uint8_t vbat_per = mvToPercent(vbat_mv);
  advertisingbuffer[4+8] = vbat_per;
  Bluefruit.Advertising.setData(advertisingbuffer, sizeof(advertisingbuffer));
}
  
void startAdvertising(void)
{  
  Bluefruit.Advertising.setType(BLE_GAP_ADV_TYPE_NONCONNECTABLE_SCANNABLE_UNDIRECTED);
  Bluefruit.Advertising.restartOnDisconnect(true);
  Bluefruit.Advertising.setInterval(160, 160);    // in units of 0.625 ms
  Bluefruit.Advertising.setFastTimeout(30);       // number of seconds in fast mode (since fast and slow mode have same interval, this is not important)
  updateAdvertisingData();
  Bluefruit.Advertising.start(0);                 // 0 = Don't stop advertising after n seconds  
}
 

void loop() 
{
  delay(100);
  updateAdvertisingData();
}
