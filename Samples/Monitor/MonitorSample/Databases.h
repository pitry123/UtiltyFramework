
class MonitorSample
{

public: 
	enum Table1DBEnum
	{
		ROW_1,  // t:int
		ROW_2,	// t:int
		ROW_3,  // t:int
		ROW_4,  // t:int		
		ROW_5,  // t:int
		ROW_6,  // t:int
		ROW_7,  // t:int
		ROW_8,  // t:int
		ROW_9,  // t:int
		ROW_10,  // t:int
		ROW_11,  // t:int
		ROW_12,  // t:int
		ROW_13,  // t:int
		ROW_14,  // t:int
		ROW_15,  // t:int
		ROW_16,  // t:int
		ROW_17,  // t:int
		ROW_18,  // t:int
		ROW_19,  // t:int
		ROW_20,  // t:int

		NUM_OF_TABLE1_ROWS
	};

	enum Table2DBEnum
	{
		ROW_21,  // t:float
		ROW_22,	// t:float
		ROW_23,	// t:float
		ROW_24,	// t:float
		ROW_25,	// t:float

		NUM_OF_TABLE2_ROWS
	};
#pragma pack(1)

	struct MyData_struct
	{
		int val1; 
		int val2; 
		int val3; 
		int val4; 
	};
#pragma pack()
};