#pragma once
#include <cstring>
#include <cstdint>

class SampleApplication
{
public:
#pragma pack(1)

	struct MSG_HEADER
	{
		uint8_t m_unitCode;
		uint16_t m_opCode;
		uint16_t m_length;
	};

	struct MyData1
	{
		MyData1()
		{
			std::memset(this, 0, sizeof(MyData1));
			header.m_opCode = 0x90;
			header.m_unitCode = 0x03;
			header.m_length = sizeof(MyData1) - sizeof(MSG_HEADER);
		}

		MSG_HEADER header;
		int val1;
		int val2;
		float val3;
		double val4;
	};

	struct MyData2
	{
		MyData2()
		{
			std::memset(this, 0, sizeof(MyData2));
			header.m_opCode = 0x91;
			header.m_unitCode = 0x04;
			header.m_length = sizeof(MyData2) - sizeof(MSG_HEADER);
		}

		MSG_HEADER header;
		int val1;
	};

#pragma pack()
};