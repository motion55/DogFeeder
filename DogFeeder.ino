 
  //
  #define USE_GSM     1
  #define USE_KEYPAD  1
  #define USE_REPORT  0
  #define USE_RGB_LCD 1
  
  // include the library code:
#if USE_GSM  
  #include "src/SIMCOM.h"
  #include "src/sms.h"
  SMSGSM sms;
  boolean started=false;
  #define DEFAULT_NUMBER  1 // 0 or 1
  #define SMS_TARGET0 "09065069294" //<-use your own number 
  #define SMS_TARGET1 "09297895641"
  #define SMS_TARGET2 "00000000000" //spare

  typedef char phone_number_t[14];
  phone_number_t phone_book[3] = { SMS_TARGET0, SMS_TARGET1, SMS_TARGET2 };
#endif
  
#if USE_REPORT
  #include <EEPROM.h>

  #define REPORT_HOUR_ADDR 30
  #define REPORT_MIN_ADDR  31
  
  #define REPORT_HOUR 0
  #define REPORT_MIN  0
  #define REPORT_SEC  0

  char send_report = 0;
  char report_sent = 0;
  
  char ReportHours = REPORT_HOUR;
  char ReportMins = REPORT_MIN;
#endif
  
  #include <Wire.h>
#if USE_RGB_LCD
  #include "rgb_lcd.h"
  #else
  //#include <LiquidCrystal_I2C.h>
  #include "LiquidCrystal_PCF8574.h"
#endif

  /*The circuit:
  * LCD RS pin to digital pin 10
  * LCD R/W pin to GND
  * LCD Enable pin to digital pin 9
  * LCD D4 pin to digital pin 7
  * LCD D5 pin to digital pin 6
  * LCD D6 pin to digital pin 5
  * LCD D7 pin to digital pin 4
  * LCD VSS pin to ground
  * LCD VCC pin to 5V
  * 10K resistor:
  * ends to +5V and ground
  * wiper to LCD VO pin (pin 3)
  */
  // initialize the library with the numbers of the interface pins
#if defined(LiquidCrystal_I2C_h)
  LiquidCrystal_I2C lcd(0x27,16,2);
#elif defined(LiquidCrystal_PCF8574_h)
  LiquidCrystal_PCF8574 lcd(0x27);
#else   
  rgb_lcd lcd;
#endif  
  int i, j, k;
  long previousMillis = 0;
  const long period1 = 5000;
  long previousMillis2 = 0;
  const long period2 = 1000;
  
#if USE_GSM  
  #ifndef HAVE_HWSERIAL1
  const int RX_pin = 2;
  const int TX_pin = 3;
  #endif
  const int GSM_ON_pin = 0;
#endif  

#if USE_KEYPAD
  #include <Keypad.h>
  extern Keypad customKeypad; 
  boolean Unlocked; 
  String password("1234567890");
  boolean CheckPassword(String passwd);
  String GetNewPassword(void);
  String Get_EEPROM_password(void);
  void Set_EEPROM_password(String passwd);
#endif  

  #include <Time.h>
  
  char DateText[] = "02/22/2017 \0";
  //                 01234567890
  char TimeText[] = "00:00:00a\0";
  //                 123456789
  
  void setup() {
    // put your setup code here, to run once:
    //SerialUSB.begin(9600);
    i = 0;
    j = 0;
    k = 0;
    LCDInit();
    lcd.setCursor(0,0);
    lcd.print(F("    Energy Meter    "));
    lcd.setCursor(0,1);
    lcd.print(F("   Billing System   "));
    lcd.setCursor(0,2);
    lcd.print(F("  By Nery And Jasa  "));
    
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
  
  #if USE_REPORT
    unsigned char HourVal = EEPROM.read(REPORT_HOUR_ADDR);
    unsigned char MinVal = EEPROM.read(REPORT_MIN_ADDR);
    if ((HourVal<24)&&(MinVal<60))
    {
      ReportHours = HourVal;
      ReportMins = MinVal;
    }
    else
    {
      HourVal = REPORT_HOUR;
      MinVal = REPORT_MIN;
      EEPROM.update(REPORT_HOUR_ADDR, HourVal);
      EEPROM.update(REPORT_MIN_ADDR, MinVal);
    }
  #endif    
  
#if USE_GSM  
    lcd.setCursor(0,3);
    lcd.print(F(" Initializing  GSM. "));
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
    previousMillis = millis();
    previousMillis2 = millis();

    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);
    
    LCD_refresh();
  }
  
  void loop() {
   // put your main code here, to run repeatedly:
    unsigned long currentMillis = millis();

    if (currentMillis - previousMillis2 > period2) {
      previousMillis2 = currentMillis;

      UpdateTime();
      
    #if USE_GSM  
      SMS();
    #endif    
    }
    
    #if USE_KEYPAD
    char customKey = customKeypad.getKey();
    
    if (customKey){
      switch (customKey) {
      case 'A':
        if (CheckPassword(password))
        {
        }
        else
        {
          LCD_PIN_reject();
        }
        break;
      case 'B':
        if (CheckPassword(password))
        {
        }
        else
        {
          LCD_PIN_reject();
        }
        break;
      case 'C':
        if (CheckPassword(password))
        {
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
    
  void LCDInit()
  {
  #if defined(LiquidCrystal_I2C_h)
    lcd.init();
    lcd.backlight();
  #elif defined(LiquidCrystal_PCF8574_h)
    lcd.begin(16,2);
    lcd.setBacklight(255);
  #else
    lcd.begin(16,2);
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
          #if USE_REPORT
            else if(strstr(smsbuffer,"REPORT"))
            {
              char *pReport = strstr(smsbuffer,"REPORT");
            #if 1
              lcd.setCursor(0,2);
              lcd.print(pReport);
              delay(1000);
            #endif
              pReport += 6;
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
                    ReportHours = tempHour;
                    ReportMins = tempMin;
                    String stringOne(F("Set Report Time "));
                    AddReportTime(stringOne);
                    stringOne.toCharArray(smsbuffer,160);
                    sms.SendSMS(phone_n, smsbuffer);
                    EEPROM.update(REPORT_HOUR_ADDR, ReportHours);
                    EEPROM.update(REPORT_MIN_ADDR, ReportMins);
                  }
                }
              }
              else
              {
                send_report = 0xFF;
              }
              report_sent = 0;
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
                  lcd.setCursor(0, 2);
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
      else
      {
        #if USE_REPORT
        if (send_report)
        {
          send_report = 0;
          report_sent = 0xFF;
          lcd.setCursor(0,2);
          lcd.print(F("Sending report..."));
        }
        #endif
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
    lcd.setCursor(0,0);
    // lcd.print(F("Total KWHr ="));
    // lcd.print(String(Total_WattHr/1000.0f,5));

    lcd.setCursor(0,1);
  
    UpdateTime();
  }

  void UpdateTime(void)
  {
    time_t tm = now();
      
    int hour = hourFormat12(tm);
    if (hour < 10)
    {
      TimeText[0] = ' ';
      TimeText[1] = '0' + hour;
    }
    else
    {
      TimeText[0] = '1';
      TimeText[1] = '0' + (hour - 10);
    }
  
    int min = minute(tm);
    int min10 = min / 10;
    TimeText[3] = '0' + min10;
    TimeText[4] = '0' + min - (min10 * 10);
  
    int sec = second(tm);
    int sec10 = sec / 10;
    TimeText[6] = '0' + sec10;
    TimeText[7] = '0' + sec - (sec10 * 10);
    
    if (isAM(tm))
    {
      TimeText[8] = 'a';
    }
    else
    {
      TimeText[8] = 'p';
    }
    
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

    lcd.setCursor(0,3);
    lcd.print(DateText);
    lcd.setCursor(11,3);
    lcd.print(TimeText);

    #if USE_REPORT
    DailyReport();
    #endif
  }

  void LCD_PIN_reject(void)
  {
    lcd.clear();
    lcd.setCursor(0,2);
    lcd.print(F(" Password rejected. "));
    delay(2000);
  }

  #if USE_REPORT
  void DailyReport(void)
  {
    time_t tm = now();
    
    if (hour(tm)==ReportHours)
    {
      if (minute(tm)==ReportMins)
      {
        if (!report_sent) send_report = 0xFF;
      }
      else
      {
        report_sent = 0;
      }
    }
    else
    {
      report_sent = 0;
    }
  }

  void AddReportTime(String &stringOne)
  {
    if (ReportHours<10) 
    {
      stringOne.concat('0');
    }
    stringOne.concat(String((int)ReportHours));
    if (ReportMins<10)
    {
      stringOne.concat(F(":0"));
    }
    else
    {
      stringOne.concat(':');
    }
    stringOne.concat(String((int)ReportMins));
  }
  #endif
  

