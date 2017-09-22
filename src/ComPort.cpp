/*
ComPort.cpp
*/

// 
// Includes
// 
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <Arduino.h>
#ifndef __arm__
#include <SoftwareSerial.h>
#include <util/delay_basic.h>
#endif
#include "ComPort.h"

//
// Constructor
//

ComPort::ComPort()
{
#ifndef __arm__
	_SW_Serial = NULL;
#endif
	_HW_Serial = NULL;
}

//
// Destructor
//
ComPort::~ComPort()
{
#ifndef __arm__
	if (_SW_Serial != NULL)
	{
		delete _SW_Serial;
		_SW_Serial = NULL;
	}
#endif
}

int ComPort::peek()
{
	return 0;
}

//
// Public methods
//

void ComPort::SelectHardwareSerial(HardwareSerial* HW_Serial)
{
	if (HW_Serial!=NULL) _HW_Serial = HW_Serial;
#ifndef __arm__
	if (_SW_Serial != NULL)
	{
		delete _SW_Serial;
		_SW_Serial = NULL;
	}
#endif
}

#ifndef __arm__
void ComPort::SelectSoftwareSerial(uint8_t receivePin, uint8_t transmitPin)
{
	if (_SW_Serial == NULL)
	{
		_SW_Serial = new SoftwareSerial(receivePin, transmitPin);
	}
}
#endif	

void ComPort::begin(long speed)
{
#ifndef __arm__
	if ((_SW_Serial == NULL) && (_HW_Serial == NULL))
	{
		SelectSoftwareSerial(_COMPORT_SS_RXPIN_, _COMPORT_SS_TXPIN_);
		#if defined(HAVE_HWSERIAL0)
		if (_SW_Serial == NULL) _HW_Serial = &Serial;
		#elif defined(HAVE_HWSERIAL1)
		if (_SW_Serial == NULL) _HW_Serial = &Serial1;
		#endif
	}

	if (_SW_Serial != NULL)
	{
		_SW_Serial->begin(speed);
	}
	else
#endif	
	if (_HW_Serial != NULL)
	{
		_HW_Serial->begin(speed);
	}

}

size_t ComPort::write(uint8_t dat)
{
#ifndef __arm__
	if (_SW_Serial != NULL)
	{
		return _SW_Serial->write(dat);
	}
	else
#endif	
	if (_HW_Serial != NULL)
	{
		return _HW_Serial->write(dat);
	}
	return 0;
}

int ComPort::read()
{
#ifndef __arm__
	if (_SW_Serial != NULL)
	{
		return _SW_Serial->read();
	}
	else
#endif	
	if (_HW_Serial != NULL)
	{
		return _HW_Serial->read();
	}
	return 0;
}

int ComPort::available()
{
#ifndef __arm__
	if (_SW_Serial != NULL)
	{
		return _SW_Serial->available();
	}
	else
#endif	
	if (_HW_Serial != NULL)
	{
		return _HW_Serial->available();
	}
	return 0;
}

void ComPort::flush()
{
#ifndef __arm__
	if (_SW_Serial != NULL)
	{
		_SW_Serial->flush();
		_SW_Serial->overflow();
	}
	else
#endif	
	if (_HW_Serial != NULL)
	{
		_HW_Serial->flush();
	}
}

bool ComPort::overflow()
{
#ifndef __arm__
	if (_SW_Serial != NULL)
	{
		return _SW_Serial->overflow();
	}
#endif	
	return false;
}
