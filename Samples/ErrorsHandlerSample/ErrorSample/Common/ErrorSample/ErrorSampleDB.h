#pragma once


enum ProjectLogID
{
	NUM_OF_PROJECT_LOGS
};

class ApplicationSampleDB
{
public:
	enum InternalErrorsDB
	{
		ERROR1, // CommonTypes::StatusEnum t:int def:STATUS_NONE c:Row 1 of the tester
		CC_ROM_ERROR, // CommonTypes::StatusEnum t:int     def:STATUS_NONE c:Row 2 of the tester
        ERROR3, // CommonTypes::StatusEnum t:int  def:STATUS_NONE
		NUM_OF_GREEN_RED_TABLE_ROWS
	};

	//Empty Table
	enum ErrorGroupsDB
	{
		NUM_OF_ERRORGROUP_ROWS
	};
	//Empty Table
	enum ErrorStoreDBEnum
	{ 
		NUM_OF_ERRORSTORE_ROWS
	};
	//Empty Table
	enum ErrorDBEnum
	{
		NUM_OF_ERRORS_ROWS
	};

	
#pragma pack(1)
	enum MyEnum
	{
		OPTION1,
		OPTION2
	};

	

#pragma pack()
};

