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
		ROW_2, // t:int    min:5 max:90 def:20 c:Row 2 of the tester
		ROW_3, // t:none    c:Row 3 no type
		ROW_4, // t:none    c:Row 4 no type
		ROW_5, // ApplicationSampleDB::MyEnum t:int
		NUM_OF_GREEN_RED_TABLE_ROWS
	};

	//Empty Table
	enum DynamicTable 
	{
		NUM_DYNAMIC_TABLE_ROWS
	};

#pragma pack(1)
	enum MyEnum
	{
		OPTION1,
		OPTION2 = 0x1
	};

	enum outputEnum
	{
		OUTPUT1,
		OUTPUT2,
		OUTPUT3
	};

	enum inputEnum
	{
		INTPUT1,
		INTPUT2,
		INTPUT3
	};

	struct MyArrayStruct
	{
		ApplicationSampleDB::MyEnum options;  //t:int  def:OPTION2
	};
	struct MyInternalData
	{
		ApplicationSampleDB::MyArrayStruct oneMoreStruct;
		int val1[2]; //def:15 min:10 max:90
		ApplicationSampleDB::MyArrayStruct MyArray[10];
	};
	struct MyData
	{
		int val1;    //min:0 max:120 def:2
		int val2;    //min:-5 max:200 def:-1
		ApplicationSampleDB::MyEnum option;  //t:int  def:OPTION1
		float val3;  //min:0.0 max:3.5 def:2.1211
		double val4; //min:-3.1415926535897932384626433832795 max:6.1415926535897932384626433832795 def:1.23232
		int val5[10]; //def:20
		char str[50]; //def:default string
		ApplicationSampleDB::MyInternalData structData;
	};
	 
#pragma pack()
};