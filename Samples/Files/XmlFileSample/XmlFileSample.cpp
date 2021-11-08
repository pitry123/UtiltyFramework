// XmlFileSample.cpp : Defines the entry point for the console application.
//

#include <Files.hpp>
#include <iostream>
#include <iomanip>
#include <map>
#include <limits>


enum NodeTypesEnum
{
	NTYPE_BOOL,
	NTYPE_BYTE,
	NTYPE_SHORT,
	NTYPE_INT,
	NTYPE_LONG,
	NTYPE_FLOAT,
	NTYPE_DOUBLE,
	NTYPE_STRING,
	NTYPE_STRUCT
};

std::map<std::string, NodeTypesEnum> g_node_types;
using dbl = std::numeric_limits<double>;

using namespace Files;

void print_node_data(XmlElement& elem, int spaces)
{
	std::string node_name = elem.Name();
	auto it = g_node_types.find(node_name);
	if (it == g_node_types.end())
		// unknown type
		return;

	std::string sp = "";
	for (int i=0; i<spaces; i++)
		sp += " ";

	switch (it->second)
	{
		case NTYPE_BOOL:
			std::cout << sp << elem.Name() << ": " << elem.ValueAsBool(false) << std::endl;
			return;

		case NTYPE_BYTE:
		case NTYPE_SHORT:
		case NTYPE_INT:
			std::cout << sp << elem.Name() << ": " << elem.ValueAsInt(0) << std::endl;
			return;

		case NTYPE_LONG:
			std::cout << sp << elem.Name() << ": " << elem.ValueAsLong(0) << std::endl;
			return;

		case NTYPE_FLOAT:
			std::cout.precision(16); // to get double precision
			std::cout << sp << elem.Name() << ": " << elem.ValueAsFloat(0.0) << std::endl;
			return;

		case NTYPE_DOUBLE:
			std::cout.precision(32); // to get double precision
			std::cout << sp << elem.Name() << ": " << elem.ValueAsDouble(0.0) << std::endl;
			return;

		case NTYPE_STRING:
			std::cout << sp << elem.Name() << ": " << elem.Value() << std::endl;
			return;

		case NTYPE_STRUCT:
		{
			std::cout << sp << elem.Name() << ": " << std::endl;
			for (auto& attr : elem.Attributes())
			{
				std::cout << sp << "ATTR Name: " << attr.Name() << ", ATTR Value: " << attr.Value() << std::endl;
			}
			for (auto& child : elem.Children())
				print_node_data(child, spaces+2);
			return;
		}

		default:
			// unknown type
			return;
	}
}

int main(int argc, const char* argv[])
{
	std::pair<std::string, NodeTypesEnum> value;

	// Initialize types dictionary:
	value = std::pair<std::string, NodeTypesEnum>("BOOL", NTYPE_BOOL);
	g_node_types.insert(value);
	value = std::pair<std::string, NodeTypesEnum>("BYTE", NTYPE_BYTE);
	g_node_types.insert(value);
	value = std::pair<std::string, NodeTypesEnum>("SHORT", NTYPE_SHORT);
	g_node_types.insert(value);
	value = std::pair<std::string, NodeTypesEnum>("INT", NTYPE_INT);
	g_node_types.insert(value);
	value = std::pair<std::string, NodeTypesEnum>("LONG", NTYPE_LONG);
	g_node_types.insert(value);
	value = std::pair<std::string, NodeTypesEnum>("FLOAT", NTYPE_FLOAT);
	g_node_types.insert(value);
	value = std::pair<std::string, NodeTypesEnum>("DOUBLE", NTYPE_DOUBLE);
	g_node_types.insert(value);
	value = std::pair<std::string, NodeTypesEnum>("STRING", NTYPE_STRING);
	g_node_types.insert(value);
	value = std::pair<std::string, NodeTypesEnum>("STRUCT", NTYPE_STRUCT);
	g_node_types.insert(value);

	XmlFile xmlFile = XmlFile::Create("Suzi.xml");
	FilesHandler filesHandler = FilesHandler::Create(10000);
	filesHandler.SubscribeFile(xmlFile);

	// Make some modifications:
	XmlElement node = xmlFile.QueryElement("/DATABASE/DBENTRY[2]");
	node.AddAttribute("is_hidden", "true");
	node.AddChild("INT", "1");
	node.QueryAttribute("index").Value(57);
	node = xmlFile.QueryElement("/DATABASE/DBENTRY[1]/INT");
	node.Value(23);

	XmlElement db_node = xmlFile.QueryElement("/DATABASE");
	if (db_node.Empty() == true)
		throw;

	// Add new Entry to node
	db_node.AddChild("DBENTRY", nullptr);
	node = xmlFile.QueryElement("/DATABASE/DBENTRY[8]");
	node.AddAttribute("name", "NEW_ENTRY");
	node.AddAttribute("index", "7");
	node.AddChild("STRUCT", nullptr);
	node = xmlFile.QueryElement("/DATABASE/DBENTRY[8]/STRUCT");
	node.AddChild("INT", "65536");
	node.AddChild("STRUCT", nullptr);
	node = xmlFile.QueryElement("/DATABASE/DBENTRY[8]/STRUCT/STRUCT");
	node.AddChild("BOOL", "1");
	node.AddChild("FLOAT", "2.781");
	node.AddChild("STRING", "STAM");

	std::cout << std::endl;
	for (auto& e : node.Children())
		std::cout << "[" << e.Name() << ", " << e.Value() << "]";
	std::cout << std::endl;

	// Print whole XML tree
	
	// Print db element attributes
	std::cout << "DATABASE element name: " << db_node.Name() << std::endl;
	for (auto& attr : db_node.Attributes())
	{
		std::cout << "  ATTR Name: " << attr.Name() << ", ATTR Value: " << attr.Value() << std::endl;
	}

	// Get all 'DATABSE' child elements and print them
	int nCount(1);
	for (auto& elem : db_node.Children())
	{
		if (elem.IsComment())
			continue;

		std::cout << "  =======================" << std::endl;
		std::cout << "  Child #" << nCount++ << ", name: " << elem.Name() << std::endl;
		for (auto& attr : elem.Attributes())
		{
			std::cout << "    ATTR Name: " << attr.Name() << ", ATTR Value: " << attr.Value() << std::endl;
		}

		for (auto& child : elem.Children())
		{
			print_node_data(child, 6);
		}
	}

	// Let Files handler a chance to flush file automatically
	std::cout << "Waiting for files handler to flush... press any key to continue" << std::endl;
	getchar();

	// Modify newly added element
	node = xmlFile.QueryElement("/DATABASE/DBENTRY[8]/STRUCT/STRUCT/STRING");
	node.Value("MAMUNI");

	// Now unsubscribe from handler
	filesHandler.UnsubscribeFile(xmlFile);

	// and make some more modifications :
	node = xmlFile.QueryElement("/DATABASE/DBENTRY[3]");
	node.AddChild("FLOAT", "3.14159");
	node.QueryAttribute("index").Value(314);
	std::cout << "Made some more changes. Now files handler will not flush unregistered file... press any key to continue" << std::endl;
	getchar();

	// Example on how to use Files Handler to flush specific file
	std::cout << "Flushing file manually using files handler.. press any key to continue" << std::endl;
	filesHandler.Flush(xmlFile);
	getchar();
}