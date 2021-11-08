// BinFileSample.cpp : Defines the entry point for the console application.
//

#include <Files.hpp>
#include <cstring>

using namespace Files;

struct MyData
{
    int val1;
    int val2;
    float val3;
    double val4;
};

int main(int argc, const char* argv[])
{	 
    BinFile file = BinFile::Create("Suzi.bin", 2048);
    MyData data = {};
    data.val1 = 10;
    data.val2 = 57;
    data.val3 = 3.1459f;
    data.val4 = 40.5;

    printf("Write to file:\t val1=%d, val2=%d, val3 = %f, val4 = %lf\n", data.val1, data.val2, data.val3, data.val4);
    file.WriteFile(data);
    file.Flush();

    MyData data2 = {}; 
    file.ReadFile(data2);
    printf("Read from file:\t val1=%d, val2=%d, val3 = %f, val4 = %lf\n", data2.val1, data2.val2, data2.val3, data2.val4);

	// wait for any key...
	getchar();
	return 0;
}
