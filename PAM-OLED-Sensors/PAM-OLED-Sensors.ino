//
// LCD
//
#include "U8glib.h"

U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_NONE);

//
// SI7021
//
#include <Wire.h>
#include <SI7021.h>

#if defined(ESP8266)
//
// For the ESP8266 we need these to be defined for I2C
//
#define SDA 4
#define SCL 5
#endif

SI7021 sensor;

float temperature;
int humidity;

//
// TSL2561
//
#include "TSL2561.h"

TSL2561 tsl(TSL2561_ADDR_FLOAT); 

boolean hasTSL = false;
uint16_t lux1;

//
// BH1750
//
#include <BH1750.h>

BH1750 lightMeter;

uint16_t lux2;

//
// BMP180
//
#include <SFE_BMP180.h>
#define ALTITUDE 0

SFE_BMP180 pressure;

boolean hasBMP = false;
double temperature2,pressure2,altitude2;

//
// LCD, the function that is called to draw to the OLED
//
void draw() {
  //
  // Set the correct values for drawing with the u8g library
  //
  u8g.setFont(u8g_font_6x10);
  u8g.setFontRefHeightExtendedText();
  u8g.setDefaultForegroundColor();
  u8g.setFontPosTop();
  char printThis[14];
  String t;
  //
  // Temperature from SI7021 and if we have it
  // the BMP180 (which is than averaged)
  //
  float tmp2 = temperature;
  if (hasBMP) {
    tmp2 = (tmp2+temperature2)/2;
  }
  t = "Temp:";
  t.toCharArray(printThis,13);
  u8g.drawStr(0,0,printThis);
  //
  // The temperature with degrees and C added
  //
  t = String(tmp2,1)+" "+(char)176+"C";
  t.toCharArray(printThis,13);
  u8g.drawStr(50,0,printThis);
  //
  // Humidity from the SI7021
  //
  t = "Humi:";
  t.toCharArray(printThis,14);
  u8g.drawStr(0,11,printThis);
  //
  // The humidity and % added
  //
  t = String(humidity)+" %";
  t.toCharArray(printThis,14);
  u8g.drawStr(50,11,printThis);
  //
  // The lux from the BH1750 and from the
  // TSL2561 if we have it (it is averaged if we have it)
  //
  t = "Lux:";
  t.toCharArray(printThis,14);
  u8g.drawStr(0,22,printThis);
  uint16_t tmp = lux2;
  if (hasTSL) {
    tmp = (tmp+lux1)/2;
  }
  //
  // The lux with lx added
  //
  t = String(tmp)+" lx";
  t.toCharArray(printThis,8);
  u8g.drawStr(50,22,printThis);
  //
  // The pressure from the BMP180
  //
  t = "Press:";
  t.toCharArray(printThis,14);
  u8g.drawStr(0,33,printThis);
  //
  // The pressure with mb added
  //
  t = String(pressure2,0)+" mb";
  t.toCharArray(printThis,14);
  u8g.drawStr(50,33,printThis);
  //
  // The altitude from BMP180
  //
  t = "Alti:";
  t.toCharArray(printThis,14);
  u8g.drawStr(0,44,printThis);
  //
  // The altitude with m added
  //
  t = String(altitude2,0)+" m";
  t.toCharArray(printThis,14);
  u8g.drawStr(50,44,printThis);
}

void setup() {
  //
  // I2C startup
  //
#if defined(ESP8266)
  //
  // For the ESP8266 we need to start the sensor with SDA and SCL pin numbers
  //
  sensor.begin(SDA,SCL);
#else
  //
  // For Arduino we can just call begin.
  //
  sensor.begin();
#endif
  //
  // SI7021
  //
  temperature = 0;
  humidity = 0;
  
  //
  // TSL2561
  //
  if (tsl.begin()) {
    hasTSL = true;
    tsl.setGain(TSL2561_GAIN_16X);
    tsl.setTiming(TSL2561_INTEGRATIONTIME_13MS);
  }

  //
  // BH1750
  //
  lightMeter.begin();
  //
  // BMP180
  //
  if (pressure.begin()) {
    hasBMP = true;
  }
}

void loop() {
  //
  // LCD, the draw routine is handled by the u8g library
  //
  u8g.firstPage();  
  do {
     draw();
  } while(u8g.nextPage());

  //
  // SI7021
  //
  temperature = sensor.getCelsiusHundredths();
  temperature = temperature / 100;
  humidity = sensor.getHumidityPercent();

  //
  // TSL2561
  //
  if (hasTSL) {
    uint32_t lum = tsl.getFullLuminosity();
    uint16_t ir, full;
    ir = lum >> 16;
    full = lum & 0xFFFF;
    lux1 = tsl.calculateLux(full, ir);
  }

  //
  // BH1750
  //
  lux2 = lightMeter.readLightLevel();

  //
  // BMP180
  //
  if (hasBMP) {
    char status;
    status = pressure.startTemperature();
    if (status != 0) {
      delay(status);
      status = pressure.getTemperature(temperature2);
      if (status != 0) {
        status = pressure.startPressure(3);
        if (status != 0) {
          delay(status);
          status = pressure.getPressure(pressure2,temperature2);
          if (status != 0) {
            double p0 = pressure.sealevel(pressure2,ALTITUDE);
            altitude2 = pressure.altitude(pressure2,p0);
          }
        }
      }
    }
  }
  
  delay(50);
}

