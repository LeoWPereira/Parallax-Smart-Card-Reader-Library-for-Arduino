#include <IS24C16A.h>

IS24C16A::IS24C16A(int IOPin, int CLKPin, int CDPin, int RSTPin, int Delay)
	: SmartCardReader(IOPin, CLKPin, CDPin, RSTPin, Delay)
{
}

bool IS24C16A::IsDevicePresent(uint8_t Page)
{
	_Start();
	bool ackbit = _Write(DEVICE_ADDRESS + (Page << 1) | 0);
	_Stop();

	if (ackbit == ACK)
		return true;
	return false;
}

void IS24C16A::Initialize()
{
  Output(CLK);
  Input(IO);
  Set_High(CLK);
  
  for (int i = 0; i < 9; i++)
  {
    Set_Low(CLK);
    Set_High(CLK);
    
    if (Get(IO) != 0)
      break;
  }
}

uint8_t IS24C16A::Read(uint8_t Page, uint8_t Location)
{
	uint8_t value = 0;
  
	_Start();
	_Write(DEVICE_ADDRESS + (Page << 1));
	_Write(Location);
  
	_Start();
	_Write(DEVICE_ADDRESS + (Page << 1) + 1);
	value = _Read(NAK);
	_Stop();

	return value;
}

void IS24C16A::Write(uint8_t Page, uint8_t Location, uint8_t Val)
{
	DelayAmt = 400;
	_Start();
	DelayAmt = 50;
	_Write(DEVICE_ADDRESS + (Page << 1));
	_Write(Location);
	_Write(Val);
	DelayAmt = 400;
	_Stop();
	_Process();
	while (!IsDevicePresent(Page));
	DelayAmt = 50;
}

void IS24C16A::ReadString(uint8_t Page, uint8_t Location, int Len, char *String)
{
	for (int i = 0; i < Len; i++)
	{
		if (Location < 256)
		{
			*String++ = (char) Read(Page, Location++);
			*String = NULL;
		}
	}
}

void IS24C16A::WriteString(uint8_t Page, uint8_t Location, const char *String)
{
	for (int i = 0; i < strlen(String); i++)
	{
		if (Location < 256)
		{
			Write(Page, Location++, (uint8_t) String[i]);
			if (Location < 256)
				Write(Page, Location, (uint8_t) NULL);
		}
	}
}

void IS24C16A::ClearString(uint8_t Page, uint8_t Location, int Len)
{
	for (int i = 0; i < Len; i++)
	{
		if (Location < 256)
		{
			Write(Page, Location++, (uint8_t) NULL);
		}
	}
}

void IS24C16A::_Start()
{
  Output(IO);
  Set_High(IO);
  Output(CLK);
  Set_High(CLK);
  Set_Low(IO);
  Set_Low(CLK);
}

void IS24C16A::_Stop()
{
  Output(CLK);
  Set_Low(CLK);
  Output(IO);
  Set_Low(IO);
  Set_High(CLK);
  Set_High(IO); 
  Input(CLK);
  Input(IO);
}

void IS24C16A::_Process()
{
  Output(CLK);
  Input(IO);
  
  if (Get(IO) != 0)
  {
    Set_Low(CLK);
    Set_High(CLK);
  }
  
  Input(CLK);
  Input(IO);
  
  delayMicroseconds(1000);
}

uint8_t IS24C16A::_Read(bool ackbit)
{
  uint8_t data = 0;
  
  Input(IO);
  Output(CLK);
  
  data = shiftIn(IO, CLK, MSBFIRST);

  if (ackbit == ACK)
    Set_Low(IO);
  else
    Set_High(IO); 
  
  Output(IO);
  Set_High(CLK);
  Set_Low(CLK);
  Set_Low(IO);
  
  return data;
}

bool IS24C16A::_Write(uint8_t data)
{
  bool ackbit = 0;
  
  Output(IO);
  Output(CLK);
  
  shiftOut(IO, CLK, MSBFIRST, data);
  
  Input(IO);
  
  do
  {
    Set_High(CLK);
    ackbit = Get(IO);
    Set_Low(CLK);
  } while (ackbit != ACK);

  Output(IO);
  Set_Low(IO);
  
  return ackbit;
}

