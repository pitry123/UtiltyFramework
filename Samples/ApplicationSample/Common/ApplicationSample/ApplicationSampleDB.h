#pragma once


enum ProjectLogID
{
	NUM_OF_PROJECT_LOGS
};

class ApplicationSampleDB
{
public:
	enum GreenRedTable
	{
		ROW_1, // t:MyData  c:Row 1 of the tester
		ROW_2, // t:int     def:-1 c:Row 2 of the tester
        ROW_3, //ApplicationSampleDB::MyEnum  t:int def:OPTION2
		NUM_OF_GREEN_RED_TABLE_ROWS
	};

#pragma pack(1)
	enum MyEnum
	{
		OPTION1,
		OPTION2
	};

	struct MyArrayStruct
	{
		MyEnum options;  //t:int
		uint8_t bit : 4;
		uint8_t bit1 : 4;
	};
	struct MyInternalData
	{
		int val1;
		ApplicationSampleDB::MyArrayStruct MyArray[10];
		ApplicationSampleDB::MyArrayStruct oneMoreStruct;
	};
	struct MyData
	{
		int val1;
		int val2;
		float val3;
		double val4;
		MyEnum options[3]; //t:int def:OPTION2
		ApplicationSampleDB::MyInternalData structData;
	};

#pragma pack()
};