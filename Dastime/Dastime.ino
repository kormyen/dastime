#include "TimeLib.h"
#include "Adafruit_GFX.h"
#include "Arduino_ST7789.h"
#include "SPI.h"

#define TFT_DC    8
#define TFT_RST   9 
#define TFT_MOSI  11   // for hardware SPI data pin (all of available pins)
#define TFT_SCLK  13   // for hardware SPI sclk pin (all of available pins)

Arduino_ST7789 _tft = Arduino_ST7789(TFT_DC, TFT_RST, TFT_MOSI, TFT_SCLK);

// REFERENCE
#define DEG2RAD 0.0174532925
#define ANGLE_OFFSET 90
#define SECONDS_PER_DAY 86400
#define MAGIC_LUNATION_OFFSET 0.73
int CENTER_SCREEN_X = 120;
int CENTER_SCREEN_Y = 120;

// Synodic month is the count of days between full moon to full moon.
// Synodic month in days: 29.530588853 + 0.000000002162 × Y
// Y is years since epoch J2000.0 (1 January 2000 12:00 TT), expressed in Julian years of 365.25 days
#define SYNODIC_MONTH_BASE 29.530588853 // Length of a Synodic month in solar days (J2000 epoch)
#define SYNODIC_MONTH_OFFSET 0.000000002162 // Yearly (J2000 epoch) offset to synodic month due to the Moon's drag
#define SECS_BETWEEN_J1970_AND_J2000 946684800 // UTC 1st January 2000 12:00:00 AM in seconds since now()'s epoch of Jan 1 1970

// SETTINGS
#define EARTH_SIZE 64
//#define MOON_ORBIT 16
//#define MOON_SIZE 10
#define MARKER_ORBIT 8
#define MARKER_SIZE 6
#define TIMEZONE_OFFSET 12
#define MARKER_Y_FIX 2

#define LINE_MAIN 8
#define LINE_MINOR 4
#define LINE_MAIN_THICK 4
#define LINE_STEP 0.041666666667

int _moonSize;
int _moonOrbit;

// STATE
float _angle;
float _yearPerc;
float _yearLight;
float _monthPerc;
float _dayPerc;
float _lunationPerc;
time_t _now;

void setup() 
{
  Serial.begin(9600);

  setTime(1, 40, 0, 24, 6, 2018);
  
  _tft.init(240, 240);
  _tft.fillScreen(BLACK);

  _moonSize = EARTH_SIZE * 0.27;
  _moonOrbit = MARKER_ORBIT * 2 + MARKER_SIZE;
  
  doFullRecalc();
  //drawDebugText();
  drawEarth();
  //drawOrbitMarkerCirc();
  drawOrbitMarkerTriangle();
  drawEarthNotches();
  drawMoon();
}

void drawEarth()
{
  _tft.drawCircle(CENTER_SCREEN_X, CENTER_SCREEN_Y, EARTH_SIZE, WHITE);
  _yearLight = 1 - _yearPerc;
  float linePerc = 1;
  for(int i = 0; linePerc >= 0; i++)
  {
    linePerc = _monthPerc - 0.01 * i;
    drawSpherePart(CENTER_SCREEN_X, CENTER_SCREEN_Y, EARTH_SIZE, _yearLight, linePerc);
  }
}

void drawOrbitMarkerCirc()
{
  int markerDistance = EARTH_SIZE + MARKER_ORBIT;
  float markerRotation = (_dayPerc * -360) + 360 + (_yearLight * 360);
  while (markerRotation > 360)
  {
    markerRotation -= 360;
  }
  int markerX = (int) (CENTER_SCREEN_X + markerDistance * cos((markerRotation + 90) * DEG2RAD));
  int markerY = (int) (CENTER_SCREEN_Y + markerDistance * sin((markerRotation + 90) * DEG2RAD)) + MARKER_Y_FIX;
  //_tft.drawCircle(markerX, markerY, MARKER_SIZE, WHITE);

  float markerPerc = (markerRotation / 360) + 0.5;
  while(markerPerc > 1)
  {
    markerPerc -= 1;
  }
  
  float linePerc = 1;
  for(int i = 0; linePerc >= 0; i++)
  {
    linePerc = 0.5 - 0.01 * i;
    drawSpherePart(markerX, markerY, MARKER_SIZE, markerPerc, linePerc);
  }
}

void drawOrbitMarkerTriangle()
{
  int markerTipDistance = EARTH_SIZE + MARKER_ORBIT;
  float markerTipRotation = (_dayPerc * -360) + 360 + (_yearLight * 360);
  while (markerTipRotation > 360)
  {
    markerTipRotation -= 360;
  }
  int markerTipX = (int) (CENTER_SCREEN_X + markerTipDistance * cos((markerTipRotation + 90) * DEG2RAD));
  int markerTipY = (int) (CENTER_SCREEN_Y + markerTipDistance * sin((markerTipRotation + 90) * DEG2RAD)) + MARKER_Y_FIX;

  int markerLeftX = (int) (CENTER_SCREEN_X + (markerTipDistance + LINE_MAIN) * cos((markerTipRotation - 5 + 90) * DEG2RAD));
  int markerLeftY = (int) (CENTER_SCREEN_Y + (markerTipDistance + LINE_MAIN) * sin((markerTipRotation - 5 + 90) * DEG2RAD)) + MARKER_Y_FIX;

  int markerRightX = (int) (CENTER_SCREEN_X + (markerTipDistance + LINE_MAIN) * cos((markerTipRotation + 5 + 90) * DEG2RAD));
  int markerRightY = (int) (CENTER_SCREEN_Y + (markerTipDistance + LINE_MAIN) * sin((markerTipRotation + 5 + 90) * DEG2RAD)) + MARKER_Y_FIX;

  _tft.fillTriangle(markerTipX, markerTipY, markerLeftX, markerLeftY, markerRightX, markerRightY, WHITE); 
}

void drawEarthNotches()
{
  drawEarthNotch(0, true, WHITE); // Midnight
//  drawEarthNotch(LINE_STEP * 1, false, WHITE); // 23
//  drawEarthNotch(LINE_STEP * 2, false, WHITE); // 22
//  drawEarthNotch(LINE_STEP * 3, true, BLACK); // 21
//  drawEarthNotch(LINE_STEP * 4, false, BLACK); // 20
//  drawEarthNotch(LINE_STEP * 5, false, BLACK); // 19
//  drawEarthNotch(0.25, true, BLACK); // 18
//  drawEarthNotch(LINE_STEP * 7, false, BLACK); // 17
//  drawEarthNotch(LINE_STEP * 8, false, BLACK); // 16
//  drawEarthNotch(LINE_STEP * 9, true, BLACK); // 15
//  drawEarthNotch(LINE_STEP * 10, false, BLACK); // 14
//  drawEarthNotch(LINE_STEP * 11, false, BLACK); // 13
//  drawEarthNotch(0.5, true, BLACK); // 12
//  drawEarthNotch(LINE_STEP * 13, false, BLACK); // 11
//  drawEarthNotch(LINE_STEP * 14, false, BLACK); // 10
//  drawEarthNotch(LINE_STEP * 15, true, BLACK); // 9
//  drawEarthNotch(LINE_STEP * 16, false, BLACK); // 8
//  drawEarthNotch(LINE_STEP * 17, false, BLACK); // 7
//  drawEarthNotch(0.75, true, BLACK); // 6
//  drawEarthNotch(LINE_STEP * 19, false, BLACK); // 5
//  drawEarthNotch(LINE_STEP * 20, false, BLACK); // 4
  drawEarthNotch(LINE_STEP * 21, true, BLACK); // 3
  drawEarthNotch(LINE_STEP * 22, false, WHITE); // 2
  drawEarthNotch(LINE_STEP * 23, false, WHITE); // 1
}

void drawEarthNotch(float rotationPerc, bool mainLine, uint16_t color)
{
  // Overlap offset for black lines to overcover the earth circle line
  int offset = 0;
  if (color == BLACK)
  {
    offset = 5;
  }
  
  int lineLength = LINE_MINOR;
  if (mainLine)
  {
    lineLength = LINE_MAIN;
  }
  
  float rotation = (_yearLight * 360) + (rotationPerc * 360);
  while (rotation > 360)
  {
    rotation -= 360;
  }

  if (mainLine)
  {
    rotation -= (LINE_MAIN_THICK / 2);
  }
  while (rotation < 0)
  {
    rotation += 360;
  }
  
  int StartX = (int) (CENTER_SCREEN_X + (EARTH_SIZE - lineLength) * cos((rotation + 90) * DEG2RAD));
  int StartY = (int) (CENTER_SCREEN_Y + (EARTH_SIZE - lineLength) * sin((rotation + 90) * DEG2RAD));
  int EndX = (int) (CENTER_SCREEN_X + (EARTH_SIZE + offset) * cos((rotation + 90) * DEG2RAD));
  int EndY = (int) (CENTER_SCREEN_Y + (EARTH_SIZE + offset) * sin((rotation + 90) * DEG2RAD));

  if (!mainLine)
  {
    _tft.drawLine(StartX, StartY, EndX, EndY, color);
  }
  else
  {
    float rotation2 = rotation + (LINE_MAIN_THICK / 2);
    while (rotation2 > 360)
    {
      rotation -= 360;
    }
    int Start2X = (int) (CENTER_SCREEN_X + (EARTH_SIZE - lineLength) * cos((rotation2 + 90) * DEG2RAD));
    int Start2Y = (int) (CENTER_SCREEN_Y + (EARTH_SIZE - lineLength) * sin((rotation2 + 90) * DEG2RAD));
    int End2X = (int) (CENTER_SCREEN_X + (EARTH_SIZE + offset) * cos((rotation2 + 90) * DEG2RAD));
    int End2Y = (int) (CENTER_SCREEN_Y + (EARTH_SIZE + offset) * sin((rotation2 + 90) * DEG2RAD));

    _tft.fillTriangle(StartX, StartY, Start2X, Start2Y, EndX, EndY, color); 
    _tft.fillTriangle(EndX, EndY, End2X, End2Y, Start2X, Start2Y, color); 
    _tft.fillTriangle(StartX, StartY, EndX, EndY, Start2X, Start2Y, color); 
    _tft.fillTriangle(Start2X, Start2Y, End2X, End2Y, StartX, StartY, color); 
  }
}

void drawMoon()
{
  int moonDistance = EARTH_SIZE + _moonOrbit + _moonSize;
  float lunarRotation = (_lunationPerc * -360) + 360;
  int moonX = (int) (CENTER_SCREEN_X + moonDistance * cos((lunarRotation - 270) * DEG2RAD));
  int moonY = (int) (CENTER_SCREEN_Y + moonDistance * sin((lunarRotation - 270) * DEG2RAD));
  _tft.drawCircle(moonX, moonY, _moonSize, WHITE);
  
  float moonFill = 0;
  if (_lunationPerc < 0.5)
  {
    moonFill = _lunationPerc * 2;
  }
  else
  {
      moonFill = 1 - ((_lunationPerc - 0.5) * 2);
  }
  
  float linePerc = 1;
  for(int i = 0; linePerc >= 0; i++)
  {
    linePerc = moonFill - 0.01 * i;
    drawSpherePart(moonX, moonY, _moonSize, _yearLight, linePerc);
  }
}

void loop() 
{
}

void doFullRecalc()
{
  _now = now();
  _yearPerc = calcYearPerc(day(), month(), year());
  _monthPerc = calcMonthPerc(day(), month(), year());
  Serial.println(_monthPerc);
  _dayPerc = (second() + (minute() * 60.0) + (hour() * 60.0 * 60.0)) / SECONDS_PER_DAY;

  // Calculate current Synodic month length
  time_t secondsSinceJ2000 = _now - (TIMEZONE_OFFSET * 60 * 60) - SECS_BETWEEN_J1970_AND_J2000;
  float daysSinceJ2000 = secondsSinceJ2000 / 60.0 / 60.0 / 24.0;
  float julianYearsSinceJ2000 = daysSinceJ2000 / 365.25;
  float synMonthDays = SYNODIC_MONTH_BASE + (SYNODIC_MONTH_OFFSET * julianYearsSinceJ2000);
  float synMonthSeconds = synMonthDays * 24 * 60 * 60;

  // Calculate how many seconds since a reference full moon
  float utcSeconds = _now - (TIMEZONE_OFFSET * 60 * 60);
  _lunationPerc = normalize(utcSeconds / synMonthSeconds) + MAGIC_LUNATION_OFFSET;
  while(_lunationPerc > 1) { _lunationPerc -= 1; }
  //float lunationDay = _lunationPerc * synMonthDays;
}

void drawSpherePart(int orig_x, int orig_y, int radius, float rotationPerc, float fillPerc)
{
  float baseAngle = rotationPerc * 360.0;
  float offsetAngle = fillPerc * 180.0;
  
  _angle = baseAngle + offsetAngle;
  while (_angle > 360)
  {
    _angle -= 360;
  }
  
  int leftX = (int) (orig_x + radius * cos((_angle - 90) * DEG2RAD));
  int leftY = (int) (orig_y + radius * sin((_angle - 90) * DEG2RAD));

  _angle = baseAngle - offsetAngle;
  while (_angle > 360)
  {
    _angle -= 360;
  }

  int rightX = (int) (orig_x + radius * cos((_angle - 90) * DEG2RAD));
  int rightY = (int) (orig_y + radius * sin((_angle - 90) * DEG2RAD));
  
  int threeX = (int) (orig_x + radius * cos((baseAngle - 90) * DEG2RAD));
  int threeY = (int) (orig_y + radius * sin((baseAngle - 90) * DEG2RAD));
  
  _tft.fillTriangle(leftX, leftY, rightX, rightY, threeX, threeY, WHITE); 
//  _tft.drawLine(leftX, leftY, rightX, rightY, WHITE); 
}

void drawDebugText()
{
  _tft.setTextColor(WHITE,BLACK);
  _tft.setTextSize(2);
  _tft.setCursor(30, 115);
  _tft.setCursor(30, 20);
  
  char buf[3]; 
  sprintf(buf, "%02d",(int)(_yearPerc*100)); 
  _tft.print("Y"); _tft.print(buf); 
  
  sprintf(buf, "%02d",(int)(_monthPerc*100)); 
  _tft.print(" M"); _tft.print(buf); 
  
  sprintf(buf, "%02d",(int)(_dayPerc*100)); 
  _tft.print(" D"); _tft.print(buf); 
  
  sprintf(buf, "%02d",(int)(_lunationPerc*100)); 
  _tft.print(" L"); _tft.println(buf); 
}

float calcYearPerc(int day, int month, int year)
{
  // Given a day, month, and year (4 digit), returns 
  // the day of year. Errors return 999.
  
  int daysInMonth[] = {31,28,31,30,31,30,31,31,30,31,30,31};
  
  // Verify we got a 4-digit year
  if (year < 1000) {
    return 999;
  }
  
  // Check if it is a leap year, this is confusing business
  // See: https://support.microsoft.com/en-us/kb/214019
  if (year%4  == 0) {
    if (year%100 != 0) {
      daysInMonth[1] = 29;
    }
    else {
      if (year%400 == 0) {
        daysInMonth[1] = 29;
      }
    }
   }

  // Make sure we are on a valid day of the month
  if (day < 1) 
  {
    return 999;
  } else if (day > daysInMonth[month-1]) {
    return 999;
  }

  int doy = 0;
  for (int i = 0; i < month - 1; i++) {
    doy += daysInMonth[i];
  }

  float total = 0.0;
  for (int i = 0; i < 12; i++) {
    total += daysInMonth[i];
  }
  
  doy += day;
  return doy / total;
}

float calcMonthPerc(int day, int month, int year)
{
  // Given a day, month, and year (4 digit), returns the month perc
  
  int daysInMonth[] = {31,28,31,30,31,30,31,31,30,31,30,31};
  
  // Verify we got a 4-digit year
  if (year < 1000) {
    return 999;
  }
  
  // Check if it is a leap year, this is confusing business
  // See: https://support.microsoft.com/en-us/kb/214019
  if (year%4  == 0) {
    if (year%100 != 0) {
      daysInMonth[1] = 29;
    }
    else {
      if (year%400 == 0) {
        daysInMonth[1] = 29;
      }
    }
   }

  // Make sure we are on a valid day of the month
  if (day < 1) 
  {
    return 999;
  } else if (day > daysInMonth[month-1]) {
    return 999;
  }

  float total = daysInMonth[month - 1];
  Serial.print("total: "); Serial.println(total);
  return day / total;
}

float normalize(float v)
{
  v -= floor(v); 
  if (v < 0) v += 1;
  return v;
}
