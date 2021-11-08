// LogFileSample.cpp : Defines the entry point for the console application.
//

#include <Files.hpp>
#include <cstring>
#include <fstream>

using namespace Files;

int main(int argc, const char* argv[])
{
    
    LogFile file = LogFile::Create("Suzi.log", true, 20, 200, "STAM Header\n", 1);

    // Write something
    file.WriteLine("0123456789012", 13);
    file.WriteLine("0123456789012", 13);
    file.Flush();
    
    // Read something
    char read_content[64] = {};
    file.ReadFile(read_content, 45, false);
    printf("%s\n", read_content);

    // wait for any key...
    getchar();
    return 0;
}