/* @file CustomKeypad.pde
|| @version 1.0
|| @author Alexander Brevig
|| @contact alexanderbrevig@gmail.com
||
|| @description
|| | Demonstrates changing the keypad size and key values.
|| #
*/
#include <EEPROM.h>
#include <Keypad.h>

const byte ROWS = 4; //four rows
const byte COLS = 4; //four columns
//define the cymbols on the buttons of the keypads
char hexaKeys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte rowPins[ROWS] = {5, 4, 3, 2}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {9, 8, 7, 6}; //connect to the column pinouts of the keypad

//initialize an instance of class NewKeypad
Keypad customKeypad = Keypad( makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS); 

#ifndef USE_KEYPAD

#include <Wire.h>
#include "rgb_lcd.h"

rgb_lcd lcd;

const int colorR = 64;
const int colorG = 64;
const int colorB = 64;

String password("12345678");

typedef struct {
  uint8_t Second; 
  uint8_t Minute; 
  uint8_t Hour; 
} HrsMinsSecs_t;

void setup(){
  Serial.begin(9600);
  lcd.begin(16,2);
  lcd.setRGB(colorR, colorG, colorB);
}
  
void loop(){
  char customKey = customKeypad.getKey();
  
  if (customKey){
    Serial.print(customKey);
    switch (customKey) {
    case 'A':
      GetTime();
      break;
    case 'B':
      break;
    case 'C':
      break;
    case 'D':
      break;
    }
  }
}
#endif

boolean CheckPassword(String passwd) 
{
  String guess;
  boolean result = false;
  lcd.clear();
  lcd.setCursor(0,0);
             //0123456789012345
  lcd.print(F("Enter the PIN.  "));
#if 0  
  lcd.setCursor(0,3);
  lcd.print(Get_EEPROM_password());
#endif  
  lcd.setCursor(0,1);
  char done = 0;
  while (!done) {
    char customKey = customKeypad.getKey();
    if (customKey==0) continue;
    if ((customKey>='0')&&(customKey<='9'))
    {
      guess += customKey;
      lcd.write('*');
      if (guess.length()>20) done = true;
    }
    else
    if (customKey=='#') 
    {
      done = true;
      if (guess == passwd) result = true;
    }
    else
    {
      done = true;
    }
  }
  return result;
}

String GetNewPassword(void)
{
  String passwd;
  lcd.clear();
  lcd.setCursor(0,0);
             //01234567890123456789
  lcd.print(F("   Enter new PIN.   "));
  lcd.setCursor(0,1);
  lcd.print(F("    then press #.   "));
#if 1  
  lcd.setCursor(0,3);
  lcd.print(Get_EEPROM_password());
#endif  
  lcd.setCursor(0,2);
  char done = 0;
  while (!done) {
    char customKey = customKeypad.getKey();
    if (customKey==0) continue;
    if ((customKey>='0')&&(customKey<='9'))
    {
      passwd += customKey;
      lcd.write(customKey);
      if (passwd.length()>20) done = true;
    }
    else
    if (customKey=='#') 
    {
      done = true;
    }
    else
    {
      done = true;
      passwd = "";
    }
  }
  return passwd;
}

#ifndef PASSWORD_ADDR
#define PASSWORD_ADDR 0
#endif

String Get_EEPROM_password(void)
{
  String passwd;
  int addr = PASSWORD_ADDR; 
  for (int i = 0; i<20; i++)
  {
    char Key = EEPROM.read(addr++);
    if ((Key>='0')&&(Key<='9'))
    {
      passwd += Key;
    }
    else break;
  }
  return passwd;
}

void Set_EEPROM_password(String passwd)
{
  int len = passwd.length();
  if (len>20) len = 20;
  int addr = PASSWORD_ADDR; 
  for (int i = 0; i<len; i++)
  {
    char Key = passwd[i];
    if ((Key>='0')&&(Key<='9'))
    {
      EEPROM.update(addr++,Key);
    }
    else 
    {
      EEPROM.update(addr++,0xFF);
      break;
    }
  }
  EEPROM.update(addr,0xFF);
}

char GetTime(HrsMinsSecs_t* HrsMinsSecsPtr)
{
  //               "0123456789012345"
  String timestr(F("Hour:Min->"));
  if (HrsMinsSecsPtr->Hour<10) {
    timestr += String('0');
  }
  timestr += String(HrsMinsSecsPtr->Hour)+String(':');
  if (HrsMinsSecsPtr->Minute<10) {
    timestr += String('0');
  }
  timestr += String(HrsMinsSecsPtr->Minute)+String(' ');
  lcd.setCursor(0,1);
  lcd.print(timestr);
  char done = 0;
  char cursor_pos = 10;
  lcd.setCursor(cursor_pos,1);
  lcd.cursor();
  lcd.blink();
  while (!done) {
    char customKey = customKeypad.getKey();
    if (customKey==0) continue;
    if ((customKey>='0')&&(customKey<='9'))
    {
      switch (cursor_pos) {
      default:
        cursor_pos = 10;
      case 10:
        if (customKey<'3') 
        {
          timestr[10] = customKey;
          cursor_pos++;
        }
        break;
      case 11:
        if (timestr[10]<'2') {
          timestr[11] = customKey;
          cursor_pos+=2;
        }
        else if (customKey<'4') {
          timestr[11] = customKey;
          cursor_pos+=2;
        }
        break;
      case 13:
        if (customKey<'6') 
        {
          timestr[13] = customKey;
          cursor_pos++;
        }
        break;
      case 14:
        timestr[14] = customKey;
        cursor_pos = 10;
        break;
      }
      lcd.setCursor(0,1);
      lcd.print(timestr);
      lcd.setCursor(cursor_pos,1);
    }
    else
    if (customKey=='#') 
    {
      HrsMinsSecsPtr->Hour = timestr.substring(10,12).toInt();
      HrsMinsSecsPtr->Minute = timestr.substring(13,15).toInt();
      Serial.println(F("Value accepted"));
      Serial.print(HrsMinsSecsPtr->Hour);
      Serial.print(':');
      Serial.println(HrsMinsSecsPtr->Minute);
      done = true;
    }
    else 
    {
      Serial.println(F("Value accpted"));
      break;
    }
  }
  lcd.noCursor();
  lcd.noBlink();

  return done;
}

String GetNewPhoneNo(void)
{
  String phoneno;
  lcd.clear();
  lcd.setCursor(0,0);
             //01234567890123456789
  lcd.print(F("   Enter Phone No.  "));
  lcd.setCursor(0,1);
  lcd.print(F("    then press #.   "));
  lcd.setCursor(0,2);
  char done = 0;
  while (!done) {
    char customKey = customKeypad.getKey();
    if (customKey==0) continue;
    if ((customKey>='0')&&(customKey<='9'))
    {
      phoneno += customKey;
      lcd.write(customKey);
      if (phoneno.length()>14) done = true;
    }
    else
    if (customKey=='#') 
    {
      done = true;
    }
    else
    {
      done = true;
      phoneno = "";
    }
  }
  return phoneno;
}

