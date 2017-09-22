/*
ComPort.h (formerly NewSoftSerial.h) - 
*/

#ifndef _ComPort_h
#define _ComPort_h

#include <inttypes.h>
#include <Stream.h>

#define _COMPORT_SS_RXPIN_	2
#define _COMPORT_SS_TXPIN_	3


/******************************************************************************
* Definitions
******************************************************************************/

class ComPort : public Stream
{
private:
#ifndef __arm__
	SoftwareSerial* _SW_Serial;
#endif	
	HardwareSerial* _HW_Serial;

public:
  // public methods
  ComPort();
  ~ComPort();

  void SelectHardwareSerial(HardwareSerial *HW_Serial);
#ifndef __arm__
  void SelectSoftwareSerial(uint8_t receivePin, uint8_t transmitPin);
#endif	
  int peek();
  void begin(long speed);
  virtual size_t write(uint8_t dat);
  virtual int read();
  virtual int available();
  virtual void flush();
  bool overflow();
};

#endif
