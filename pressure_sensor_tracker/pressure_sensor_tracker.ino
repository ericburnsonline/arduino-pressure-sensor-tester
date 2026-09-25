#include <Wire.h>
#include <U8g2lib.h>
#include <Adafruit_BMP3XX.h>
#include <Adafruit_ADS1X15.h>


// ------------------------------------------------------------
// OLED
//
// "_1_" means page-buffer mode, which uses much less RAM
// than a full 128x64 framebuffer.
// ------------------------------------------------------------

U8G2_SSD1306_128X64_NONAME_1_HW_I2C display(
  U8G2_R0,
  U8X8_PIN_NONE
);


// ------------------------------------------------------------
// Sensors
// ------------------------------------------------------------

Adafruit_BMP3XX bmp;
Adafruit_ADS1115 ads;


// ------------------------------------------------------------
// IDEX sensor constants
//
// Datasheet nominal output at 12 V excitation:
//
// 600 mbar  -> 52.2 mV
// 1100 mbar -> 95.7 mV
//
// Output is ratiometric with supply voltage.
//
// We are powering the IDEX sensor from 5 V.
// ------------------------------------------------------------

const float IDEX_SUPPLY_VOLTAGE = 5.0;

const float IDEX_OUTPUT_AT_600_12V = 52.2;   // mV
const float IDEX_SENSITIVITY_12V   = 0.087;  // mV per mbar

const float IDEX_OUTPUT_AT_600 =
  IDEX_OUTPUT_AT_600_12V *
  (IDEX_SUPPLY_VOLTAGE / 12.0);

const float IDEX_SENSITIVITY =
  IDEX_SENSITIVITY_12V *
  (IDEX_SUPPLY_VOLTAGE / 12.0);


// Expected differential output at 5 V excitation is roughly
// 21.8 mV to 39.9 mV across the rated pressure range.
//
// Use wider bounds for initial testing.

const float IDEX_MIN_VALID_MV = 15.0;
const float IDEX_MAX_VALID_MV = 50.0;


// ------------------------------------------------------------
// Convert IDEX differential voltage to estimated pressure.
//
// This is based on the NOMINAL datasheet transfer function,
// not an actual calibration.
// ------------------------------------------------------------

float idexMillivoltsToPressure(float millivolts)
{
  return 600.0 +
         ((millivolts - IDEX_OUTPUT_AT_600)
          / IDEX_SENSITIVITY);
}


// ------------------------------------------------------------
// Start BMP390 at its known address.
// ------------------------------------------------------------

bool startBMP390()
{
  if (bmp.begin_I2C(0x77)) {
    Serial.println("BMP390 found at 0x77");
    return true;
  }

  return false;
}


// ------------------------------------------------------------
// Draw the OLED screen.
//
// U8g2 page-buffer mode requires the
// firstPage() / nextPage() loop.
// ------------------------------------------------------------

void drawDisplay(
  bool bmpOK,
  float bmpPressure,
  float bmpTemperature,
  bool idexConnected,
  float idexPressure,
  float idexMillivolts,
  float difference
)
{
  display.firstPage();

  do {

    // --------------------------------------------------------
    // Header
    // --------------------------------------------------------

    display.setFont(u8g2_font_6x12_tf);
    display.drawStr(0, 10, "PRESSURE TRACKER");
    display.drawHLine(0, 12, 128);


    // --------------------------------------------------------
    // BMP390
    // --------------------------------------------------------

    display.setFont(u8g2_font_5x8_tf);
    display.drawStr(0, 22, "BMP390");

    if (bmpOK) {

      char buffer[20];

      dtostrf(
        bmpPressure,
        6,
        1,
        buffer
      );

      display.setFont(u8g2_font_7x14B_tf);
      display.drawStr(0, 36, buffer);

      display.setFont(u8g2_font_5x8_tf);
      display.drawStr(58, 36, "hPa");


      // Temperature

      char tempBuffer[12];

      dtostrf(
        bmpTemperature,
        4,
        1,
        tempBuffer
      );

      display.drawStr(88, 22, tempBuffer);
      display.drawStr(113, 22, "C");

    } else {

      display.drawStr(45, 22, "ERROR");
    }


    // --------------------------------------------------------
    // IDEX
    // --------------------------------------------------------

    display.setFont(u8g2_font_5x8_tf);
    display.drawStr(0, 46, "IDEX");

    if (idexConnected) {

      char idexBuffer[20];

      dtostrf(
        idexPressure,
        6,
        1,
        idexBuffer
      );

      display.setFont(u8g2_font_7x14B_tf);
      display.drawStr(0, 61, idexBuffer);

      display.setFont(u8g2_font_5x8_tf);
      display.drawStr(58, 61, "hPa");


      // Difference

      char diffBuffer[12];

      dtostrf(
        difference,
        5,
        1,
        diffBuffer
      );

      display.drawStr(88, 46, "D:");
      display.drawStr(99, 46, diffBuffer);

    } else {

      display.drawStr(31, 46, "NOT CONNECTED");


      // Show raw ADC differential mV while debugging

      char mvBuffer[16];

      dtostrf(
        idexMillivolts,
        7,
        2,
        mvBuffer
      );

      display.drawStr(0, 59, "ADC:");
      display.drawStr(24, 59, mvBuffer);
      display.drawStr(70, 59, "mV");
    }

  } while (display.nextPage());
}


// ------------------------------------------------------------
// SETUP
// ------------------------------------------------------------

void setup()
{
  Serial.begin(115200);

  Wire.begin();


  // ----------------------------------------------------------
  // OLED
  // ----------------------------------------------------------

  display.setI2CAddress(0x3C * 2);

  display.begin();

  display.setFont(u8g2_font_6x12_tf);

  display.firstPage();

  do {
    display.drawStr(0, 12, "Pressure Tracker");
    display.drawStr(0, 30, "Starting...");
  } while (display.nextPage());


  // ----------------------------------------------------------
  // ADS1115
  // ----------------------------------------------------------

  if (!ads.begin()) {

    Serial.println("ADS1115 not found.");

    display.firstPage();

    do {
      display.drawStr(0, 12, "ERROR");
      display.drawStr(0, 30, "ADS1115 not found");
    } while (display.nextPage());

    while (true) {
      delay(1000);
    }
  }

  Serial.println("ADS1115 found.");


  // Highest sensitivity:
  //
  // +/- 0.256 V full-scale differential range.

  ads.setGain(GAIN_SIXTEEN);


  // ----------------------------------------------------------
  // BMP390
  // ----------------------------------------------------------

  if (!startBMP390()) {

    Serial.println("BMP390 not found.");

    display.firstPage();

    do {
      display.drawStr(0, 12, "ERROR");
      display.drawStr(0, 30, "BMP390 not found");
    } while (display.nextPage());

    while (true) {
      delay(1000);
    }
  }


  // BMP390 configuration

  bmp.setTemperatureOversampling(
    BMP3_OVERSAMPLING_8X
  );

  bmp.setPressureOversampling(
    BMP3_OVERSAMPLING_4X
  );

  bmp.setIIRFilterCoeff(
    BMP3_IIR_FILTER_COEFF_3
  );

  bmp.setOutputDataRate(
    BMP3_ODR_25_HZ
  );


  delay(1000);
}


// ------------------------------------------------------------
// LOOP
// ------------------------------------------------------------

void loop()
{
  // ----------------------------------------------------------
  // BMP390 reading
  // ----------------------------------------------------------

  bool bmpOK = bmp.performReading();

  float bmpPressure = 0.0;
  float bmpTemperature = 0.0;

  if (bmpOK) {

    // BMP pressure is reported in Pascals.
    // Convert Pa -> hPa.

    bmpPressure =
      bmp.pressure / 100.0;

    bmpTemperature =
      bmp.temperature;
  }


  // ----------------------------------------------------------
  // IDEX reading through ADS1115
  //
  // Differential measurement:
  //
  // A0 - A1
  // ----------------------------------------------------------

  int16_t adcRaw =
    ads.readADC_Differential_0_1();

  float idexVolts =
    ads.computeVolts(adcRaw);

  float idexMillivolts =
    idexVolts * 1000.0;


  // ----------------------------------------------------------
  // Determine whether the IDEX sensor appears connected.
  // ----------------------------------------------------------

  bool idexConnected =
    idexMillivolts >= IDEX_MIN_VALID_MV &&
    idexMillivolts <= IDEX_MAX_VALID_MV;


  float idexPressure = 0.0;
  float difference = 0.0;

  if (idexConnected) {

    idexPressure =
      idexMillivoltsToPressure(
        idexMillivolts
      );

    difference =
      idexPressure -
      bmpPressure;
  }


  // ----------------------------------------------------------
  // SERIAL OUTPUT
  // ----------------------------------------------------------

  Serial.print("BMP390: ");

  if (bmpOK) {

    Serial.print(
      bmpPressure,
      2
    );

    Serial.print(" hPa");

    Serial.print("  Temp: ");

    Serial.print(
      bmpTemperature,
      1
    );

    Serial.print(" C");

  } else {

    Serial.print("READ ERROR");
  }


  Serial.print(" | IDEX: ");

  if (idexConnected) {

    Serial.print(
      idexPressure,
      2
    );

    Serial.print(" hPa");

    Serial.print(" | Output: ");

    Serial.print(
      idexMillivolts,
      3
    );

    Serial.print(" mV");

    Serial.print(" | Difference: ");

    Serial.print(
      difference,
      2
    );

    Serial.print(" hPa");

  } else {

    Serial.print("NOT CONNECTED");

    Serial.print(" | ADC: ");

    Serial.print(
      idexMillivolts,
      3
    );

    Serial.print(" mV");
  }

  Serial.println();


  // ----------------------------------------------------------
  // OLED
  // ----------------------------------------------------------

  drawDisplay(
    bmpOK,
    bmpPressure,
    bmpTemperature,
    idexConnected,
    idexPressure,
    idexMillivolts,
    difference
  );


  delay(1000);
}
