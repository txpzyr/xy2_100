/*
    Name:       xy2_100.ino
    Created:	2020/2/19 11:14:37
    Author:     TPC\TangPeng
*/

#define  CLK 2
#define  SYNC 4
#define  DATA_X 16
#define  DATA_Y 17

#define T_UP 15
#define T_DOWN 7

//pin1-4 需要相对GND有1.2v

void setup()
{
	Serial.begin(115200);
	Serial.println("init...");

	pinMode(CLK, OUTPUT);
	pinMode(SYNC, OUTPUT);
	pinMode(DATA_X, OUTPUT);
	pinMode(DATA_Y, OUTPUT);

	digitalWrite(SYNC, 0);
	digitalWrite(CLK, 0);
	disableCore0WDT();
	xTaskCreatePinnedToCore(reader, "ADC_reader", 2048, NULL, 0, NULL, 0);
	Serial.println("go...");
}

uint8_t px, py; 
uint16_t cx = 0, cy = 0;
void reader(void* pvParameters) 
{
	while (1)
	{
		XY2_Send_First_Bit(0);
		XY2_Send_First_Bit(0);
		XY2_Send_First_Bit(1);

		XY2_Send_byte(cx, cy);
		XY2_Send_Last_Bit(px, py);
		
	}
}

uint8_t parityCheck(uint16_t val)
{
	uint8_t cnt = 0;
	while (val) 
	{
		if (val & 0x01) cnt++;
		val >>= 1;
	}
	return cnt & 0x01;
}

int ms = 1000;
int maxA = 32000;
void loop()
{
	if (Serial.available())
	{
		String line = Serial.readString();
		Serial.println(line);
		int index = 0;
		ms = GetFloat(index, line);
		maxA = GetFloat(index, line);
	}

	unsigned long tick = millis();
	cx = 32000 + maxA * sin(((tick % ms) / (float)ms) * 2 * PI);
	cy = 32000 + maxA * cos(((tick % ms) / (float)ms) * 2 * PI);

	px = !parityCheck(cx);
	py = !parityCheck(cy);
	//Serial.println(cx);


	//delay(1);
	//cx += 1;
	//cy += 1;
	//Serial.println(cx);
}

#define PinH(pin) GPIO.out_w1ts = ((uint32_t)1 << pin)
#define PinL(pin) GPIO.out_w1tc = ((uint32_t)1 << pin)

#define PinV(pin, value) value==0?PinL(pin):PinH(pin)

#define CLKH PinH(CLK)
#define CLKL PinL(CLK)

#define SYNCH PinH(SYNC)
#define SYNCL PinL(SYNC)


void XY2_Send_First_Bit(bool x)
{
	PinV(DATA_X, x);
	PinV(DATA_Y, x);
	SYNCH;//同步拉高
	CLKH; //clock拉高
	
	for (int i = 0; i < T_UP; ++i) NOP();
	CLKL;
	for (int i = 0; i < T_DOWN; ++i)	NOP();
}


void XY2_Send_byte(uint16_t tx, uint16_t ty) //发送一个字符 char类型
{
	uint8_t t;
	SYNCH;
	for (t = 0; t < 16; t++) //将一个字符从高位到低位依次转化为脉冲
	{
		uint16_t x = (tx & 0x8000) >> 7;
		uint16_t y = (ty & 0x8000) >> 7;
		PinV(DATA_X, x);
		PinV(DATA_Y, y);
		tx <<= 1;
		ty <<= 1;
		CLKH;
		for (int i = 0; i < T_UP; ++i) NOP();
		CLKL;
		for (int i = 0; i < T_DOWN; ++i)	NOP();
	}
}

void XY2_Send_Last_Bit(bool px, bool py)
{
	PinV(DATA_X, px);
	PinV(DATA_Y, py); 
	SYNCL;//同步拉低
	CLKH; //clock拉高
	
	for (int i = 0; i < T_UP; ++i) NOP();
	CLKL;
	for (int i = 0; i < T_DOWN; ++i)	NOP();
	SYNCH;
}



double GetFloat(int& index, String line)
{
	int data = 0;
	double left = 0, right = 0;
	double pos = 10;
	bool point = false;
	bool neg = false;
	while ((data = line[index]) != ' ' && data != '\n' && data > 0)
	{
		if (data == '-')
		{
			neg = true;
		}
		else if (data == '.')
		{
			point = true;
		}
		else if (point == false)
		{
			left = left * 10 + (data - '0');
		}
		else
		{
			right = right + (data - '0') / pos;
			pos *= 10;
		}
		index++;
	}
	index++;
	double ret = left + right;
	return neg ? -ret : ret;
}