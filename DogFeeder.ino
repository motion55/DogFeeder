 
  //
  #define USE_GSM     1
  #define USE_KEYPAD  1
  #define USE_FEEDER  1
  #define USE_RGB_LCD 1
  #define USE_YIELD   1
  
  // include the library code:
#if USE_GSM  
  #include "src/SIMCOM.h"
  #include "src/sms.h"
  SMSGSM sms;
  boolean started=false;
  #define DEFAULT_NUMBER  1 // 0 or 1
  #define SMS_TARGET0 "09297895641"
  #define SMS_TARGET1 "09217755043" //<-use your own number 
  #define SMS_TARGET2 "00000000000" //spare

  typedef char phone_number_t[14];
  phone_number_t phone_book[3] = { SMS_TARGET0, SMS_TARGET1, SMS_TARGET2 };
#endif
  
#if USE_FEEDER
  #include <EEPROM.h>
  #include <Servo.h>
  
	#define dispenser 11
  Servo servo;

  const int SwitchLowPin = A0;
  const int SwitchHighPin = A1;
  const int SolenoidValve = A2;
	
  #define FEED1_HOUR_ADDR 30
  #define FEED1_MIN_ADDR  31
  #define FEED2_HOUR_ADDR 32
  #define FEED2_MIN_ADDR  33
  
  #define FEED_HOUR1  8
  #define FEED_HOUR2  18
  #define FEED_MIN    0

  #define CLEAN_HOUR  18
  #define CLEAN_MINS  32
  
  char feed_dog = 0;
  char dog_fed = 0;
  char cage_clean = 0;
  
  char FeedHours1 = FEED_HOUR1;
  char FeedMins1 = FEED_MIN;
  
  char FeedHours2 = FEED_HOUR2;
  char FeedMins2 = FEED_MIN;

#endif
  
#include <Wire.h>
#if USE_RGB_LCD
  #include "src/rgb_lcd.h"
#else
  //#include <LiquidCrystal_I2C.h>
  #include "LiquidCrystal_PCF8574.h"
#endif

  // initialize the library with the numbers of the interface pins
#if defined(LiquidCrystal_I2C_h)
  LiquidCrystal_I2C lcd(0x27,16,2);
#elif defined(LiquidCrystal_PCF8574_h)
  LiquidCrystal_PCF8574 lcd(0x27);
#else   
  rgb_lcd lcd;
#endif  
  int i, j, k;
  long previousMillis1 = 0;
  const long period1 = 300;
  long previousMillis2 = 0;
  const long period2 = 1000;
  
#if USE_GSM  
  #ifndef HAVE_HWSERIAL1
  const int RX_pin = 2;
  const int TX_pin = 3;
  #endif
  const int GSM_ON_pin = A3;
#endif  

#if USE_KEYPAD
  typedef struct {
    uint8_t Second; 
    uint8_t Minute; 
    uint8_t Hour; 
  } HrsMinsSecs_t;

  #include <Keypad.h>
  extern Keypad customKeypad; 
  boolean Unlocked; 
  String password("1234567890");
  boolean CheckPassword(String passwd);
  String GetNewPassword(void);
  String Get_EEPROM_password(void);
  void Set_EEPROM_password(String passwd);
  char GetTime(HrsMinsSecs_t* HrsMinsSecsPtr);
  #endif  

  #include <Time.h>
  
  char DateText[] = "02/22/2017 \0";
  //                 01234567890
  char TimeText[] = "00:00:00 \0";
  //                 123456789
  
  #if USE_YIELD
  boolean bYieldEnable = false;
  #endif
  
  void setup() {
    // put your setup code here, to run once:
    Serial.begin(9600);
    i = 0;
    j = 0;
    k = 0;
    LCDInit();
    lcd.setCursor(0,0);
    //          "0123456789012345"
    lcd.print(F("   Dog Feeder   "));
    lcd.setCursor(0,1);
    lcd.print(F("                "));
    
    DS3231_setup();
  
  #if USE_KEYPAD
    Unlocked = false;
    String pw_eeprom = Get_EEPROM_password();
    if (pw_eeprom.length()>0)
    {
      password = pw_eeprom;    
    }
    Set_EEPROM_password(password);
  #endif    
  
  #if USE_FEEDER
    FeedHours1 = EEPROM.read(FEED1_HOUR_ADDR);
    FeedMins1 = EEPROM.read(FEED1_MIN_ADDR);
    if ((FeedHours1>=24)||(FeedMins1>=60))
    {
      FeedHours1 = FEED_HOUR1;
      FeedMins1 = FEED_MIN;
      EEPROM.update(FEED1_HOUR_ADDR, FeedHours1);
      EEPROM.update(FEED1_MIN_ADDR, FeedMins1);
    }

    FeedHours2 = EEPROM.read(FEED2_HOUR_ADDR);
    FeedMins2 = EEPROM.read(FEED2_MIN_ADDR);
    if ((FeedHours2>=24)||(FeedMins2>=60))
    {
      FeedHours2 = FEED_HOUR2;
      FeedMins2 = FEED_MIN;
      EEPROM.update(FEED2_HOUR_ADDR, FeedHours2);
      EEPROM.update(FEED2_MIN_ADDR, FeedMins2);
    }

    servo.attach(dispenser);
    pinMode (SwitchLowPin, INPUT_PULLUP);
    pinMode (SwitchHighPin, INPUT_PULLUP);
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(SolenoidValve, OUTPUT);
  #endif    
  
#if USE_GSM  
    lcd.setCursor(0,1);
    //          "0123456789012345"
    lcd.print(F("Initializing GSM"));
  #if defined(HAVE_HWSERIAL1)
    gsm.SelectHardwareSerial(&Serial1, GSM_ON_pin);
  #else
    gsm.SelectSoftwareSerial(RX_pin, TX_pin, GSM_ON_pin);
  #endif
    if (gsm.begin(9600))
    {
      started=true;  
      //Send a message to indicate successful connection
      String hello(F("Dog Feeder is now Online."));
      sms.SendSMS(phone_book[DEFAULT_NUMBER], hello.c_str());
    }
    else
    {
      lcd.print(F("Initializing failed."));
      delay(3000);
    }
#endif  
    previousMillis1 = millis();
    previousMillis2 = millis();

    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);
    
    LCD_refresh();
    
    #if USE_YIELD
    bYieldEnable = true;
    #endif
  }
  
  void loop() {
   // put your main code here, to run repeatedly:
    unsigned long currentMillis = millis();

    if (currentMillis - previousMillis2 > period2) {
      previousMillis2 = currentMillis;
      LCD_refresh();
      #if USE_GSM  
      SMS();
      #endif
      #if USE_YIELD
    } else {
      yield();
      #endif
    }
    
    #if USE_KEYPAD
    #if USE_YIELD
    char customKey = GetKey();
    #else
    char customKey = customKeypad.getKey();
    #endif
    
    if (customKey){
      Serial.print(customKey);
      switch (customKey) {
      case 'A':
        if (CheckPassword(password))
        {
          lcd.setCursor(0,0);
          //                  0123456789012345
          String PromptStr(F(" Set Feed1 Time."));
          lcd.print(PromptStr);
          HrsMinsSecs_t HrsMinsSecs;
          HrsMinsSecs.Hour = FeedHours1;
          HrsMinsSecs.Minute = FeedMins1;
          if (GetTime(&HrsMinsSecs))
          {
            FeedHours1 = HrsMinsSecs.Hour;
            FeedMins1 = HrsMinsSecs.Minute;
            EEPROM.update(FEED1_HOUR_ADDR, FeedHours1);
            EEPROM.update(FEED1_MIN_ADDR, FeedMins1);
          }
        }
        else
        {
          LCD_PIN_reject();
        }
        break;
      case 'B':
        if (CheckPassword(password))
        {
          lcd.setCursor(0,0);
          //                  0123456789012345
          String PromptStr(F(" Set Feed2 Time."));
          lcd.print(PromptStr);
          HrsMinsSecs_t HrsMinsSecs;
          HrsMinsSecs.Hour = FeedHours2;
          HrsMinsSecs.Minute = FeedMins2;
          if (GetTime(&HrsMinsSecs))
          {
            FeedHours2 = HrsMinsSecs.Hour;
            FeedMins2 = HrsMinsSecs.Minute;
            EEPROM.update(FEED2_HOUR_ADDR, FeedHours2);
            EEPROM.update(FEED2_MIN_ADDR, FeedMins2);
          }
        }
        else
        {
          LCD_PIN_reject();
        }
        break;
      case 'C':
        if (CheckPassword(password))
        {
          lcd.setCursor(0,0);
          //                  0123456789012345
          String PromptStr(F(" Adjust the RTC "));
          lcd.print(PromptStr);
          time_t tm = now();
          HrsMinsSecs_t HrsMinsSecs;
          HrsMinsSecs.Hour = hour(tm);
          HrsMinsSecs.Minute = minute(tm);
          if (GetTime(&HrsMinsSecs))
          {
            int HourVal = HrsMinsSecs.Hour;
            if ((HourVal<0)||(HourVal>23)) HourVal = hour();
            int MinVal = HrsMinsSecs.Minute;
            if ((MinVal<0)||(MinVal>59)) MinVal = minute();
            setTime(HourVal,MinVal,second(),day(),month(),year());
            DS3231_setDateTime(year(),month(),day(),hour(),minute(),second());
          }
        }
        else
        {
          LCD_PIN_reject();
        }
        break;
      case 'D':
        if (CheckPassword(password))
        {
          String passnew = GetNewPassword();
          if (passnew.length()>0)
          {
            password = passnew;
            Set_EEPROM_password(password);
          }
        }
        else
        {
          LCD_PIN_reject();
        }
        break;
      }
    }
    #endif
  }
    
  const int colorR = 64;
  const int colorG = 64;
  const int colorB = 64;
  
  void LCDInit()
  {
  #if defined(LiquidCrystal_I2C_h)
    lcd.init();
    lcd.backlight();
  #elif defined(LiquidCrystal_PCF8574_h)
    lcd.begin(20,4);
    lcd.setBacklight(255);
  #else
    lcd.begin(16,2);
    lcd.setRGB(colorR, colorG, colorB);
  #endif    
  }

#if USE_GSM  
  void SMS()
  {
    if(started)
    {
      char pos = sms.IsSMSPresent(SMS_ALL);
      if(pos>0&&pos<=20)       //if message from 1-20 is found
      { 
        char smsbuffer[160];
        char phone_n[20];
        
        //Read if there are messages on SIM card and print them.
        if(sms.GetSMS(pos, phone_n, 20, smsbuffer, 160))
        {
          if (CheckPhonebook(String(phone_n)))
          {
            if(strstr(smsbuffer,"DATE"))
            {
              char *pDateStr = strstr(smsbuffer,"DATE");
              pDateStr += 4; //move pointer to strring after "DATE"
              String sMonth(pDateStr);  //07/23
              if ((sMonth.length()>=5)&&(pDateStr[2]=='/'))
              {
                int MonthVal = sMonth.toInt();
                if ((MonthVal<1)||(MonthVal>12)) MonthVal = month();
                String sDay(pDateStr+3);
                int DayVal = sDay.toInt();
                if ((DayVal<1)||(DayVal>31)) DayVal = day();
                setTime(hour(),minute(),second(),DayVal,MonthVal,year());
                DS3231_setDateTime(year(),month(),day(),hour(),minute(),second());
              }
            }
            else if(strstr(smsbuffer,"TIME"))
            {
              char *pTimeStr = strstr(smsbuffer,"TIME");
              pTimeStr += 4; //move pointer to strring after "TIME"
              String sHour(pTimeStr);  //08:31
              if ((sHour.length()>=5)&&(pTimeStr[2]==':'))
              {
                int HourVal = sHour.toInt();
                if ((HourVal<0)||(HourVal>23)) HourVal = hour();
                String sMinute(pTimeStr+3);
                int MinVal = sMinute.toInt();
                if ((MinVal<0)||(MinVal>59)) MinVal = minute();
                setTime(HourVal,MinVal,second(),day(),month(),year());
                DS3231_setDateTime(year(),month(),day(),hour(),minute(),second());
              }
            }
          #if USE_FEEDER
            else if(strstr(smsbuffer,"FEED"))
            {
              char *pReport = strstr(smsbuffer,"FEED");
              pReport += 4;
              String sHour(pReport);
              if (sHour.length()>=5)
              {
                if (sHour[2]==':')
                {
                  int tempHour = sHour.toInt();
                  pReport += 3;
                  String sMin(pReport);
                  int tempMin = sMin.toInt();
                  if ((tempHour<24)&&(tempMin<60))
                  {
                    String stringOne(F("Set "));
                    if (tempHour<12) {
                      FeedHours1 = tempHour;
                      FeedMins1 = tempMin;
                      EEPROM.update(FEED1_HOUR_ADDR, FeedHours1);
                      EEPROM.update(FEED1_MIN_ADDR, FeedMins1);
                      stringOne.concat(F("Feed1 Time"));
                    } else {
                      FeedHours2 = tempHour;
                      FeedMins2 = tempMin;
                      EEPROM.update(FEED2_HOUR_ADDR, FeedHours2);
                      EEPROM.update(FEED2_MIN_ADDR, FeedMins2);
                      stringOne.concat(F("Feed2 Time"));
                    }
                    if (tempHour<10) {
                      stringOne.concat('0');
                    }
                    stringOne.concat(String(tempHour));
                    if (tempMin<10) {
                      stringOne.concat(F(":0"));
                    } else {
                      stringOne.concat(':');
                    }
                    stringOne.concat(String(tempMin));
                    stringOne.toCharArray(smsbuffer,160);
                    sms.SendSMS(phone_n, smsbuffer);
                  }
                }
              }
              else
              {
                feed_dog = true;
              }
              dog_fed = false;
            }
          #endif  
            else 
            {
              char *pPIN = strstr(smsbuffer, "PIN");
              if (pPIN)
              {
                pPIN += 3;
                String passnew(pPIN);
                if (passnew.length() > 0)
                {
                  String stringOne = "New PIN:" + passnew;
                  lcd.setCursor(0,1);
                  lcd.print(F("PIN was changed."));
                  stringOne.toCharArray(smsbuffer, 160);
                  sms.SendSMS(phone_n, smsbuffer);
                  password = passnew;
                  Set_EEPROM_password(password);
                }
              }
            }
          }
          sms.DeleteSMS(pos); //after reading, delete SMS
        }  
      }
    }
  }
  
  boolean CheckPhonebook(String number1)
  {
    for (int i=0; i<3; i++)
    {
      String number2(phone_book[i]);
      int len = number2.length() - 10;
      if (len>0)
      {
        number2 = number2.substring(len);
      }
      if (number1.endsWith(number2)) return true;
    }
    return false;
  }
#endif

  char bRestart;
  
  void LCD_refresh()
  {
    if (bRestart)
    {
      bRestart = 0;
      LCDInit();
    }

    lcd.clear();
    UpdateTime();
    
    #if USE_FEEDER
    lcd.setCursor(0,1);
    lcd.print(F("Feed:"));
    lcd.print(FeedTimeStr());
    DogFeeder();
    #endif
  }

  void UpdateTime(void)
  {
    time_t tm = now();
      
    char hr = hour(tm);
    if (hr < 10)
    {
      TimeText[0] = ' ';
      TimeText[1] = '0' + hr;
    }
    else
    {
      TimeText[0] = '1';
      TimeText[1] = '0' + (hr - 10);
    }
  
    char min = minute(tm);
    char min10 = min / 10;
    TimeText[3] = '0' + min10;
    TimeText[4] = '0' + min - (min10 * 10);
  
    char sec = second(tm);
    char sec10 = sec / 10;
    TimeText[6] = '0' + sec10;
    TimeText[7] = '0' + sec - (sec10 * 10);
    
    int mon = month(tm);
    if (mon > 9)
    {
      DateText[0] = '1';
      mon -= 10;
    }
    else
    {
      DateText[0] = '0';
    }
    DateText[1] = '0' + mon;

    int dayOnes = day(tm);
    int dayTens = dayOnes / 10;
    dayOnes -= dayTens * 10;
    DateText[3] = '0' + dayTens;
    DateText[4] = '0' + dayOnes;

    //String yearstr(year(tm));
    String yearstr(F("2017"));
    DateText[6] = yearstr.charAt(0);
    DateText[7] = yearstr.charAt(1);
    DateText[8] = yearstr.charAt(2);
    DateText[9] = yearstr.charAt(3);

    lcd.setCursor(0,0);
    lcd.print(F("Time:"));
    lcd.print(TimeText);
  }

  void LCD_PIN_reject(void)
  {
    lcd.clear();
    lcd.setCursor(0,1);
    //          "0123456789012345" 
    lcd.print(F(" PIN rejected..."));
    delay(2000);
  }

  #if USE_FEEDER
 
  void PumpOn() {
    digitalWrite(LED_BUILTIN, HIGH);
  }

  void PumpOff() {
    digitalWrite(LED_BUILTIN, LOW);
  }
  
  const int stop_position = 95;
  const int velocity = 20;
  int stopservo = 90;

  void DogFeeder(void)
  {
    time_t tm = now();
    char hrs = hour(tm);
    char min = minute(tm);
    unsigned long currentMillis = millis();
    
    if (((hrs==FeedHours1)&&(min==FeedMins1))
    || ((hrs==FeedHours2)&&(min==FeedMins2))
    || (feed_dog))
    {
      static unsigned long previousMillis = 0;
      if (!dog_fed)  {
        dog_fed = true;
        previousMillis = currentMillis;
        servo.write(stop_position + velocity);  //Feeder open
        lcd.setCursor(0,1);
        lcd.print(F("Feeding Dog..."));
        char smsbuffer[160];
        String stringOne = String(F("Dog was fed on "))+String(TimeText);
        stringOne.toCharArray(smsbuffer,160);
        sms.SendSMS(phone_book[DEFAULT_NUMBER],smsbuffer);
      } else {
        if (currentMillis - previousMillis >= 1500L) {
          servo.write(stopservo);   //Close feeder
          feed_dog = false;
        }
      }
    }
    else
    {
      dog_fed = false;
    }

    if ((hrs==CLEAN_HOUR)&&(min==CLEAN_MINS)) 
    {
      static unsigned long previousMillis = 0;
      if (!cage_clean) {
        cage_clean = true;
        previousMillis = currentMillis;
        PumpOn();
        char smsbuffer[160];
        String stringOne = String(F("Dog cagec was cleaned"));
        stringOne.toCharArray(smsbuffer,160);
        sms.SendSMS(phone_book[DEFAULT_NUMBER],smsbuffer);
      } else {
        if (currentMillis - previousMillis >= 15000L) {
          PumpOff();
        }
      }
    }
    else 
    {
      cage_clean = false;
    }
}

  String FeedTimeStr(void)
  {
    int timesec = (now()/60)%1440;
    int FeedTime1 = (FeedHours1*60)+FeedMins1;
    int FeedTime2 = (FeedHours2*60)+FeedMins2;
    if (FeedTime1<=FeedTime2) {
      if (timesec<FeedTime1) {
        timesec = FeedTime1;
      } else if (timesec<=FeedTime2) {
        timesec = FeedTime2;
      } else {
        timesec = FeedTime1;
      }
    } else {
      if (timesec<=FeedTime2) {
        timesec = FeedTime2;
      } else if (timesec<=FeedTime1) {
        timesec = FeedTime1;
      } else {
        timesec = FeedTime2;
      }
    }
    int FeedHours = timesec / 60;
    int FeedMins = timesec % 60;
    String stringOne;
    if (FeedHours<10) 
    {
      stringOne.concat('0');
    }
    stringOne.concat(String((int)FeedHours));
    if (FeedMins<10)
    {
      stringOne.concat(F(":0"));
    }
    else
    {
      stringOne.concat(':');
    }
    stringOne.concat(String((int)FeedMins));
    return stringOne;
  }
  #endif
  
  #if USE_YIELD
  char _KeyChar = 0;
  
  char GetKey(void)
  {
    char KeyChar = _KeyChar;
    _KeyChar = 0;
    return KeyChar;
  }

  void yield(void)
  {
    if (bYieldEnable) {
      bYieldEnable = false;
      unsigned long currentMillis = millis();
      if (currentMillis - previousMillis1 > period1) {
        previousMillis1 = currentMillis;
        UpdateTime();
        FloatSensor();
      }
      char KeyChar = customKeypad.getKey();
      if (KeyChar!=0) _KeyChar = KeyChar;

      bYieldEnable = true;
    }
  }
  #endif

  void FloatSensor()
  {
    if (digitalRead(SwitchLowPin) == LOW) {
      //digitalWrite(LED_BUILTIN, LOW);
      digitalWrite(SolenoidValve, LOW);
      Serial.println ("SOLENOID CLOSED.");
    }
    else if (digitalRead(SwitchHighPin) == LOW)
    {
      //digitalWrite(LED_BUILTIN, HIGH);
      digitalWrite(SolenoidValve, HIGH);
      Serial.println("SOLANOID OPEN.");
    }
  }
  