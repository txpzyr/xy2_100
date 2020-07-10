/*
    Name:       xy2_100.ino
    Created:	2020/2/19 11:14:37
    Author:     TPC\tp
*/

#define  CLK 2
#define  SYNC 4
#define  DATA_X 16
#define  DATA_Y 17

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

uint8_t parityCheck(uint16_t val) {
	uint8_t cnt = 0;
	while (val) {
		if (val & 0x01)
			cnt++;
		val >>= 1;
	}
	return cnt & 0x01;
}

int ms = 1000;
void loop()
{
	if (Serial.available())
	{
		ms = Serial.parseInt();
		Serial.println(ms);

	}
	unsigned long tick = millis();
	cx = 30000 + 30000 * sin(((tick % ms) / (float)ms) * 2 * PI);
	cy = 30000 + 30000 * cos(((tick % ms) / (float)ms) * 2 * PI);

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
	SYNCH;//ͬ������
	CLKH; //clock����
	
	for (int i = 0; i < 100; ++i) NOP();
	CLKL;
	for (int i = 0; i < 90; ++i)	NOP();
}

void XY2_Send_Last_Bit(bool px, bool py)
{
	PinV(DATA_X, px);
	PinV(DATA_Y, py); 
	SYNCL;//ͬ������
	CLKH; //clock����
	
	for (int i = 0; i < 100; ++i) NOP();
	CLKL;
	for (int i = 0; i < 900; ++i)	NOP();
	SYNCH;
}

void XY2_Send_byte(uint16_t tx, uint16_t ty) //����һ���ַ� char����
{
	uint8_t t;
	SYNCH;
	for (t = 0; t < 16; t++) //��һ���ַ��Ӹ�λ����λ����ת��Ϊ����
	{
		uint16_t x = (tx & 0x8000) >> 7;
		uint16_t y = (ty & 0x8000) >> 7;
		PinV(DATA_X, x);
		PinV(DATA_Y, y);
		tx <<= 1;
		ty <<= 1;
		CLKH;
		for (int i = 0; i < 100; ++i) NOP();
		CLKL;
		for (int i = 0; i < 90; ++i)	NOP();
	}
}