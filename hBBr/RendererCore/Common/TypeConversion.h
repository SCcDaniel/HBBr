#pragma once
#include "Common.h"
typedef unsigned char   byte;
typedef short   int16;
typedef unsigned short   uint16;

#define RGB888_RED      0x00ff0000
#define RGB888_GREEN    0x0000ff00
#define RGB888_BLUE     0x000000ff

#define RGB565_RED      0xf800
#define RGB565_GREEN    0x07E0
#define RGB565_BLUE     0x001f

/*
Ҫ����ʾ:
1. float��unsigned long������ͬ�����ݽṹ����
2. union������������ݴ������ͬ������ռ�
*/
typedef union
{
	float fdata;
	unsigned long ldata;
}FloatLongType;

namespace TypeConversion
{
	int bytesToInt(uint8_t* bytes);

	unsigned int bytesToUInt(uint8_t* bytes);

	int16 bytesToInt16(uint8_t* bytes);

	uint16 bytesToUInt16(uint8_t* bytes);

	uint8_t* UIntToBytes(unsigned int ui);

	uint8_t* IntToBytes(int ui);

	/*
	��������fת��Ϊ4���ֽ����ݴ����uint8_t[4]��
	*/
	void Float_to_Byte(float f, unsigned char uint8_t[]);
	/*
	��4���ֽ�����byte[4]ת��Ϊ�����������*f��
	*/
	void Byte_to_Float(float* f, unsigned char byte[]);

	uint8_t* UInt16ToBytes(uint16 ui);

	uint8_t* Int16ToBytes(int16 ui);

	////////////////
	struct RGBA
	{
		uint8_t R = 0xff;
		uint8_t G = 0xff;
		uint8_t B = 0xff;
		uint8_t A = 0xff;
	};

	struct BYTE_16bit
	{
		uint8_t B1 = 0x0;
		uint8_t B2 = 0x0;
	};

	RGBA ConversionByteToRGB555A1(uint8_t* b5g5r5a1);

	BYTE_16bit ConversionRGB555A1ToByte(RGBA rgb5);

}