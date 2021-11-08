// IniFileSample.cpp : Defines the entry point for the console application.

#include <list>

#include <Files.hpp>
#include <cstring>
#include <fstream>

using namespace Files;

int main(int argc, const char* argv[])
{
	IniFile file = IniFile::Create("Suzi.ini");

	int32_t input_ch(0);
	bool is_act(false);
	float fov(0.0f);
	bool found(false);
	char str[1024] = {};
	size_t size(1024);

	found = file.ReadInt("VideoSwitch", "InputChannels", input_ch, 4);
	found ? printf("Video Switch Input Channels number: %d\n", input_ch) : printf("Not Found\n");
	found = file.ReadBool("in0", "is_active", is_act, false);
	found ? printf("Input Channel 0 is %s\n", is_act ? "active" : "inactive") : printf("Not Found\n");
	found = file.ReadBool("in1", "is_active", is_act, false);
	found ? printf("Input Channel 1 is %s\n", is_act ? "active" : "inactive") : printf("Not Found\n");
	found = file.ReadFloat("in0", "NFOV_AZ", fov, -1.0f);
	found ? printf("NFOV AZ Angle for Input Channel 0 is %f\n", fov) : printf("Not Found\n");
	found = file.ReadFloat("in1", "WFOV_ELV", fov, -1.0f);
	found ? printf("WFOV EL Angle for Input Channel 1 is %f\n", fov) : printf("Not Found\n");
	found = file.ReadString("in1", "ChannelName", str, size, "Default");
	found ? printf("Input Channel 1 name is %s\n", str) : printf("Not Found\n");

	// Erase an entry and try to find it later
	file.DeleteKey("out0", "ChannelName");
	found = file.ReadString("out0", "ChannelName", str, size, "Default");
	found ? printf("Output Channel 0 name is %s\n", str) : printf("Output Channel 0 name not found. Using \"%s\"\n", str);

	// Change value to existing key_value
	file.WriteInt("out0", "r-coax", 4);

	// Add new section-key
	file.WriteInt("New section", "Int Key", 57);

	// Add some other values to new section
	file.WriteBool("New section", "Bool Key", true);
	file.WriteFloat("New section", "Float Key", 5.7f);
	file.WriteString("New section", "Str Key", "Nisim Aytek");


	std::list<IniFile::Entry> myList;
	file.ReadAllAsString<1024>(myList);

	for (auto& itr : myList)
	{
		printf("\n%s.%s=%s", itr.m_section.c_str(), itr.m_key.c_str(), itr.m_value.c_str());
	}

	// Flush file
	file.Flush();

	// wait for any key...
	getchar();
	return 0;
}
