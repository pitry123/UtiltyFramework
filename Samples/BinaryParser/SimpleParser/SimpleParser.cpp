// DatabaseSample.cpp : Defines the entry point for the console application.
//
#include <Core.hpp>
#include <Factories.hpp>
#include <Utils.hpp>
#include <Strings.hpp>

#include <cstring>
#include <future>
#include <random>
#include <iostream>
#include <sstream>
#include <Factories.hpp>
#include <Core.hpp>

using namespace Parsers;
static Logging::Logger APP_LOGGER = Core::Framework::CreateLogger("SimpleParser", Logging::Severity::TRACE);

bool TestSimpleStruct_bigEndian()
{
	LOG_FUNC(APP_LOGGER);
	BinaryMetaDataBuilder ipHeader;
	ipHeader = BinaryMetaDataBuilder::Create();

	ipHeader.BigEndian(true).
		Bits<uint8_t>("version", 4).
		Bits<uint8_t>("headerLength",4).
		Simple<uint8_t>("tos").
		Simple<uint16_t>("packetLength").
		Simple<uint16_t>("id").
		Simple<uint16_t>("offset").
		Simple<uint8_t>("ttl").
		Simple<uint8_t>("protocol").
		Simple<uint16_t>("checksum").
		Buffer("src", 4).
		Buffer("dst", 4);

	BinaryParser parser = ipHeader.CreateParser();
	parser.ParseFromString("450002c5939900002c06ef98adc24f6c850186d1");
	
	uint8_t ver = parser.Read<uint8_t>("version");
	uint8_t header = parser.Read<uint8_t>("headerLength");

	Core::Console::ColorPrint(Core::Console::Colors::WHITE, "\nver= %d, header = %d", ver, header);

	if (false == parser.FromJson(parser.ToJson()))
		return false;

	Core::Console::ColorPrint(Core::Console::Colors::WHITE, "\n%s", parser.ToJson(Parsers::JsonDetailsLevel::JSON_ENUM_FULL,false));
	return true;
}
namespace ComplexParser
{
	enum MyEnum
	{
		ENUM1,
		ENUM2,
		ENUM3
	};

#pragma pack(1)
	struct MyInternalStruct
	{
		uint8_t a;
		uint8_t b;
		uint8_t test[5];
	};

	struct MyStruct
	{
		uint32_t a;
		int b;
		MyInternalStruct c;
		int j;
		bool boolian;
		char str[12] = { 0 };
		MyEnum myEnum;
	};
#pragma pack()
	BinaryParser CreateComplexParser()
	{
		BinaryMetadataStore store;
		BinaryMetaData complex = store.Metadata("ComplexParser::MyInternalStruct");
		EnumData enumData = store.Enum("ComplexParser::MyEnum");
		if (enumData.Empty())
		{
			enumData = EnumDataFactory::Create("ComplexParser::MyEnum");
			enumData.AddNewItem(ENUM1,"ENUM1");
			enumData.AddNewItem(ENUM2,"ENUM2");
			enumData.AddNewItem(ENUM3,"ENUM3");
			store.SetEnum("ComplexParser::MyEnum", enumData);
		}
		
		if (complex.Empty())
		{
			complex = BinaryMetaDataBuilder::Create().
				Namely("ComplexParser::MyInternalStruct").
				Simple<uint8_t>("a").
				Simple<uint8_t>("b").
				Buffer("test", 5);
		}

		BinaryMetaDataBuilder metadata = BinaryMetaDataBuilder::Create().
			Simple<uint32_t>("a").
			Simple<int>("b").
			Complex("c", complex).
			Simple<int>("j").
			Simple<bool>("boolian").
			String("str", 12, "Hrllo Worls").
			Enum("myEnum",sizeof(MyEnum),enumData);
		BinaryMetaDataBuilder newMetadata = BinaryMetaDataBuilder::Create(metadata.ToJson());
		
		return newMetadata.CreateParser();
	}
	bool CompareComplex(const MyStruct *myStr, BinaryParser& parser)
	{
		//Compare
		if (myStr->a != parser.Read<uint32_t>("a"))
			return false;
		if (myStr->b != parser.Read<int>("b"))
			return false;
		if (myStr->c.a != parser.ReadComplex("c").Read<uint8_t>("a"))
			return false;
		if (myStr->c.b != parser.ReadComplex("c").Read<uint8_t>("b"))
			return false;
		std::vector<uint8_t> data = parser.ReadComplex("c").Read("test");
		if (std::memcmp(myStr->c.test, data.data(), data.size()))
			return false;
		if (myStr->j != parser.Read<int>("j"))
			return false;
		if (myStr->boolian != parser.Read<bool>("boolian"))
			return false;

		std::string str = parser.ReadString("str");
		if (std::strcmp(str.c_str(), myStr->str))
			return false;
		if (myStr->myEnum != parser.Read<MyEnum>("myEnum"))
			return false;

		return true;

	}
	bool TestComplex()
	{
		LOG_FUNC(APP_LOGGER);

		uint8_t buffer[] = { 1,0,0,0,2,0,0,0,4,5, 0x10,0xa,0xb,0xc,0xd,0x64,0,0,0,true,'H','e','l','l','o',' ','w','o','r','l','d',0,MyEnum::ENUM2 ,0,0,0};
		MyStruct *myStr = (MyStruct*)buffer;

		BinaryParser parser = CreateComplexParser();

		parser.Parse(buffer, sizeof(buffer));
		//Compare with the Original Binary
		if (false == CompareComplex(myStr, parser))
			return false;

		if (false == parser.FromJson(parser.ToJson()))
			return false;

		//Compare after the from JSON
		if (false == CompareComplex(myStr, parser))
			return false;

		Core::Console::ColorPrint(Core::Console::Colors::WHITE, "\n%s", parser.ToJson(Parsers::JsonDetailsLevel::JSON_ENUM_FULL,false));
		parser.Reset();
		Core::Console::ColorPrint(Core::Console::Colors::WHITE, "defaults\n%s", parser.ToJson(Parsers::JsonDetailsLevel::JSON_ENUM_FULL,false));
		std::string str = parser.ReadString("str");
		if (std::strcmp(str.c_str(), "Hrllo Worls"))
			return false;
		
		return true;
	}
}

namespace ComplexWithComplexArray
{
#pragma pack(1)
	struct MyInternalStruct
	{
		uint8_t a;
		uint8_t b;
		uint8_t test[5];
	};

	struct MyStruct
	{
		uint32_t a;
		int b;
		MyInternalStruct c;
		int j;
		char str[12];
		MyInternalStruct internalArray[2];
		int arr[2];
	};
#pragma pack()
	bool TestInternalStruct(const MyInternalStruct &str, BinaryParser parser)
	{
		LOG_FUNC(APP_LOGGER);
		if (str.a != parser.Read<uint8_t>("a"))
			return false;
		if (str.b != parser.Read<uint8_t>("b"))
			return false;
		std::vector<uint8_t> data = parser.Read("test");
		if (std::memcmp(str.test, data.data(), data.size()))
			return false;

		if (false == parser.FromJson(parser.ToJson()))
			return false;
		parser.ToJson(Parsers::JsonDetailsLevel::JSON_ENUM_FULL,false);
		return true;
	}
	BinaryParser CreateComplexWithComplexArray()
	{
		Parsers::SimpleOptions options;
		BinaryMetadataStore store;
		if (store.Metadata("ComplexWithComplexArray::MyInternalStruct").Empty())
		{
			BinaryMetaData internal = BinaryMetaDataBuilder::Create().
				Namely("ComplexWithComplexArray::MyInternalStruct").
				Simple<uint8_t>("a").
				Simple<uint8_t>("b").
				Buffer("test", 5);
		}

		BinaryMetaDataBuilder metadata = BinaryMetaDataBuilder::Create().
			Simple<uint32_t>("a").
			Simple<int>("b").
			Complex("c", "ComplexWithComplexArray::MyInternalStruct").
			Simple<int>("j").
			String("str", 12).
			Array("internalArray", 2, "ComplexWithComplexArray::MyInternalStruct")
			.Array<int>("arr", 2);

		 
		BinaryMetaDataBuilder newMetadata = BinaryMetaDataBuilder::Create(metadata.ToJson(false));
		
		if (newMetadata.Size() != metadata.Size())
		{
			return BinaryParser();
		}
		return newMetadata.CreateParser();
	}
	bool CompareComplexWithComplexArray(const MyStruct *myStr,BinaryParser& parser)
	{
		//see that in bound
		if (!parser.Validate())
			Core::Console::ColorPrint(Core::Console::Colors::RED, "Error on validation ");
		else
			Core::Console::ColorPrint(Core::Console::Colors::GREEN, "Validation OK");

		//Compare
		if (myStr->a != parser.Read<uint32_t>("a"))
			return false;
		//Check id in range
		//////
		if (myStr->b != parser.Read<int>("b"))
			return false;
		if (!TestInternalStruct(myStr->c, parser.ReadComplex("c")))
			return false;
		if (myStr->j != parser.Read<int>("j"))
			return false;
		std::string str = parser.ReadString("str");
		if (std::strcmp(str.c_str(), myStr->str))
			return false;
		size_t i;
		for (i = 0; i < sizeof(myStr->internalArray) / sizeof(MyInternalStruct); i++)
		{
			if (!TestInternalStruct(myStr->internalArray[i], parser.ReadArrayAt<>("internalArray", i)))
				return false;
		}

		i = 0;
		ParserArray<int> arr = parser.ArrayRange<int>("arr");
		for (auto& data : arr)
		{
			if (myStr->arr[i] != data)
				return false;
			i++;
		}
		return true;
	}
	bool TestComplexWithComplexArray()
	{
		LOG_FUNC(APP_LOGGER);
		MyStruct refStr = { 1,2,{ 4,5,{ 0x10,0xa,0xb,0xc,0xd } },100,"Hello world",{ { 5,6,{ 0xf,0xf,0xf,0xf,0xf } },{ 5,6,{ 0xd,0xd,0xd,0xd,0xd } } },{3,4} };
		uint8_t *buffer = (uint8_t*)&refStr;
		MyStruct *myStr = &refStr;
		BinaryParser parser;
		parser = CreateComplexWithComplexArray();

		parser.Parse(buffer, sizeof(MyStruct));

		if (false == CompareComplexWithComplexArray(myStr, parser))
			return false;
		if (false == parser.FromJson(parser.ToJson()))
			return false;
		if (false == CompareComplexWithComplexArray(myStr, parser))
			return false;

		Core::Console::ColorPrint(Core::Console::Colors::WHITE, "\n%s", parser.ToJson(Parsers::JsonDetailsLevel::JSON_ENUM_FULL,false));
		return true;
	}
}
namespace ThreeLayerNestedStruct
{
#pragma pack(1)
	struct MyNested2Struct
	{
		float floatable;
		double dobuleable;
	};
	struct MyNested1Struct
	{
		uint8_t a;
		uint8_t b;
		uint8_t test[5];
		double test2[5];
		MyNested2Struct internal;
	};
	struct MyParentStruct
	{
		uint32_t a;
		int b;
		MyNested1Struct c;
		int j;
		char str[12];
	};
#pragma pack()

	BinaryParser CreateThreeLayerNestedStruct()
	{
		BinaryMetaData internal, nested;
		BinaryMetadataStore store;
		Parsers::SimpleOptions options;
		
		internal = store.Metadata("ThreeLayerNestedStruct::MyNested1Struct");
		if (internal.Empty())
		{
			options.minval<double>(1.3);
			options.maxval<double>(5.54);
			options.defval<double>(1.5);

			internal = BinaryMetaDataBuilder::Create()
				.Namely("ThreeLayerNestedStruct::MyNested1Struct")
				.Simple<uint8_t>("a")
				.Simple<uint8_t>("b")
				.Buffer("test", 5)
				.Array<double>("test2", 5, options)
				.Complex("internal", nested = BinaryMetaDataBuilder::Create()
					.Simple<float>("floatable")
					.Simple<double>("dobuleable",options));
		}

		options.minval<int>(5);
		options.maxval<int>(8);
		options.defval<int>(6);

		return BinaryMetaDataBuilder::Create().
			Simple<uint32_t>("a").
			Simple<int>("b",options).
			Complex("c", internal)
			.Simple<int>("j")
			.String("str", 12).CreateParser();
	}
	bool CompareThreeLayerNestedStruct(MyParentStruct *myStr, BinaryParser& parser)
	{
		//Compare
		if (myStr->a != parser.Read<uint32_t>("a"))
			return false;
		//write out of range val to parser
		if (parser.Validate("a"))
		{
			std::cout << "parameter a is in range";
		}
		else
		{
			std::cout << "parameter a is out of range";
		}
		if (myStr->b != parser.Read<int>("b"))
			return false;
		MyNested1Struct &nested1 = myStr->c;
		BinaryParser nested1Parser = parser.ReadComplex("c");
		if (nested1.a != nested1Parser.Read<uint8_t>("a"))
			return false;
		if (nested1.b != nested1Parser.Read<uint8_t>("b"))
			return false;
		std::vector<uint8_t> data = nested1Parser.Read("test");
		if (std::memcmp(nested1.test, data.data(), data.size()))
			return false;
		MyNested2Struct &nested2 = myStr->c.internal;
		BinaryParser nested2Parser = nested1Parser.ReadComplex("internal");
		if (nested2.floatable != nested2Parser.Read<float>("floatable"))
			return false;
		if (nested2.dobuleable != nested2Parser.Read<double>("dobuleable"))
			return false;
		if (myStr->j != parser.Read<int>("j"))
			return false;

		return true;
	}
	bool TestThreeLayerNestedStruct()
	{
		LOG_FUNC(APP_LOGGER);

		MyParentStruct refStr = { 1,6,{ 4,5,{ 0x10,0xa,0xb,0xc,0xd },{1.3,1.33241,4.54366,2.0,5.4443},{(float)4.31,3.1415926535897932384626433832795}},100,"Hello world" };
		uint8_t *buffer = (uint8_t*)&refStr;
		MyParentStruct *myStr = &refStr;

		BinaryParser parser = CreateThreeLayerNestedStruct();
		parser.Reset();
		if (!parser.Parse(buffer, sizeof(MyParentStruct)))
			return false;

		if (false == CompareThreeLayerNestedStruct(myStr, parser))
			return false;
		
		if (false == parser.FromJson(parser.ToJson()))
			return false;
		
		if (false == CompareThreeLayerNestedStruct(myStr, parser))
			return false;
		
		Core::Console::ColorPrint(Core::Console::Colors::WHITE, "\n%s", parser.ToJson(Parsers::JsonDetailsLevel::JSON_ENUM_FULL,false));

		return true;
	}
}
namespace WriteComplex
{
	bool TestWriteComplex()
	{
		LOG_FUNC(APP_LOGGER);

		uint8_t buffer[] = { 1,0,0,0,2,0,0,0,4,5, 0x10,0xa,0xb,0xc,0xd,0x64,0,0,0,true,'H','e','l','l','o',' ','W','o','r','l','d',0,ComplexParser::MyEnum::ENUM3 ,0,0,0 };
		ComplexParser::MyStruct *myStr = (ComplexParser::MyStruct*)buffer;

		BinaryParser parser = ComplexParser::CreateComplexParser();

		parser.Write<uint32_t>("a", 1);
		parser.Write<int>("b", 2);
		parser.ReadComplex("c").Write<uint8_t>("a", 4);
		parser.ReadComplex("c").Write<uint8_t>("b", 5);
		parser.ReadComplex("c").Write("test", { 0x10,0xa,0xb,0xc,0xd });
		parser.Write<int>("j", 100);
		parser.Write<bool>("boolian", true);
		parser.Write("str", "Hello World");
		parser.Write<ComplexParser::MyEnum>("myEnum", ComplexParser::MyEnum::ENUM3);
		if (false == ComplexParser::CompareComplex(myStr, parser))
			return false;

		if (false == parser.FromJson(parser.ToJson(Parsers::JsonDetailsLevel::JSON_ENUM_FULL,true)))
			return false;

		if (false == ComplexParser::CompareComplex(myStr, parser))
			return false;

		Core::Console::ColorPrint(Core::Console::Colors::WHITE, "\n%s", parser.ToJson(Parsers::JsonDetailsLevel::JSON_ENUM_FULL,false));
		return true;
	}
}
namespace WriteComplexWithComplexArray
{
	bool TestWriteComplexWithComplexArray()
	{
		LOG_FUNC(APP_LOGGER);
		ComplexWithComplexArray::MyStruct refStr = { 1,2,{ 4,5,{ 0x10,0xa,0xb,0xc,0xd } },100,"Hello world",{ { 5,6,{ 0xf,0xf,0xf,0xf,0xf } },{ 5,6,{ 0xd,0xd,0xd,0xd,0xd } } },{ 3,4 } };

		ComplexWithComplexArray::MyStruct *myStr = &refStr;
		BinaryParser parser;
		parser = ComplexWithComplexArray::CreateComplexWithComplexArray();

		parser.Write<uint32_t>("a", 1);
		parser.Write<int>("b", 2);
		parser.ReadComplex("c").Write<uint8_t>("a", 4);
		parser.ReadComplex("c").Write<uint8_t>("b", 5);
		parser.ReadComplex("c").Write("test", { 0x10,0xa,0xb,0xc,0xd });
		parser.Write<int>("j", 100);
		parser.Write("str", "Hello world");
		parser.ReadComplexArrayAt("internalArray", 0).Write<uint8_t>("a", 5);
		parser.ReadComplexArrayAt("internalArray", 0).Write<uint8_t>("b", 6);
		parser.ReadComplexArrayAt("internalArray", 0).Write("test", { 0xf,0xf,0xf,0xf,0xf });
		parser.ReadComplexArrayAt("internalArray", 1).Write<uint8_t>("a", 5);
		parser.ReadComplexArrayAt("internalArray", 1).Write<uint8_t>("b", 6);
		parser.ReadComplexArrayAt("internalArray", 1).Write("test", { 0xd,0xd,0xd,0xd,0xd });
		parser.WriteArrayAt<int>("arr", 0, 3);
		parser.WriteArrayAt<int>("arr", 1, 4);

		if (false == ComplexWithComplexArray::CompareComplexWithComplexArray(myStr, parser))
			return false;
				if (false == parser.FromJson(parser.ToJson(Parsers::JsonDetailsLevel::JSON_ENUM_FULL,false)))
			return false;

		if (false == ComplexWithComplexArray::CompareComplexWithComplexArray(myStr, parser))
			return false;

		Core::Console::ColorPrint(Core::Console::Colors::WHITE, "\n%s", parser.ToJson(Parsers::JsonDetailsLevel::JSON_ENUM_FULL,false));
		
		return true;

	}
}
namespace WriteThreeLayerNestedStruct
{

	bool TestWriteThreeLayerNestedStruct()
	{
		LOG_FUNC(APP_LOGGER);

		ThreeLayerNestedStruct::MyParentStruct refStr = { 1,8,{ 4,5,{ 0x10,0xa,0xb,0xc,0xd },{1.3,1.3241,4.54366,2.0,5.4443},{ (float)4.31,3.1415926535897932384626433832795 } },100,"Hello world" };

		ThreeLayerNestedStruct::MyParentStruct *myStr = &refStr;

		BinaryParser parser = ThreeLayerNestedStruct::CreateThreeLayerNestedStruct();
		parser.Reset();
		parser.Write<uint32_t>("a", 1);
		parser.Write<int>("b", 8);
		parser.ReadComplex("c").Write<uint8_t>("a", 4);
		parser.ReadComplex("c").Write<uint8_t>("b", 5);
		parser.ReadComplex("c").Write("test", { 0x10,0xa,0xb,0xc,0xd });
		parser.Write<int>("j", 100);
		parser.Write("str", "Hello world");
		parser.ReadComplex("c").ReadComplex("internal").Write<float>("floatable", static_cast<float>(4.31));
		parser.ReadComplex("c").ReadComplex("internal").Write<double>("dobuleable", 3.1415926535897932384626433832795);
		if (false == ThreeLayerNestedStruct::CompareThreeLayerNestedStruct(myStr, parser))
			return false;

		if (false == parser.FromJson(parser.ToJson()))
			return false;

		if (false == ThreeLayerNestedStruct::CompareThreeLayerNestedStruct(myStr, parser))
			return false;
		Core::Console::ColorPrint(Core::Console::Colors::WHITE, "\n%s", parser.ToJson(Parsers::JsonDetailsLevel::JSON_ENUM_FULL,false));
		return true;
	}
}
namespace TestLimits
{
	BinaryParser CreateThreeLayerNestedStructWithLimits()
	{
		BinaryMetaData internal, nested;
		BinaryMetadataStore store;
		Parsers::SimpleOptions options;

		internal = store.Metadata("TestLimits::MyInternalStructWithOptions");
		if (internal.Empty())
		{
			internal = BinaryMetaDataBuilder::Create()
				.Namely("TestLimits::MyInternalStructWithOptions")
				.Simple<uint8_t>("a", Parsers::SimpleOptions::create<uint8_t>(1,5,3))
				.Simple<uint8_t>("b", Parsers::SimpleOptions::create<uint8_t>(6,10,7))
				.Buffer("test", 5)
				.Complex("internal", nested = BinaryMetaDataBuilder::Create()
					.Simple<float>("floatable", Parsers::SimpleOptions::create<float>((float)1.3,(float)5.54,(float)1.5))
					.Simple<double>("dobuleable", Parsers::SimpleOptions::create<double>(-3.1415926535897932384626433832795, 3.1415926535897932384626433832795, 3.14))); 
		}

		return BinaryMetaDataBuilder::Create().
			Simple<uint32_t>("a", Parsers::SimpleOptions::create<uint32_t>(8678686,9797979, 9797960)). 
			Simple<int>("b", Parsers::SimpleOptions::create<int>(100,500,300)).
			Complex("c", internal).
			Simple<int>("j", Parsers::SimpleOptions::create<int>(100, 500, 300)).
			String("str", 12).CreateParser();

	}

	BinaryParser CreateStructWithArrays()
	{
		BinaryMetaDataBuilder internal, nested;
		BinaryMetadataStore store;
		Parsers::SimpleOptions options;
		internal = store.Metadata("TestLimits::ArrayStruct");
		if (internal.Empty())
		{
			internal = BinaryMetaDataBuilder::Create();

			internal.Namely("TestLimits::ArrayStruct");
			internal.Simple<uint8_t>("a", Parsers::SimpleOptions::create<uint8_t>(1, 5, 3));
			internal.Simple<uint8_t>("b", Parsers::SimpleOptions::create<uint8_t>(6, 10, 7));
			internal.Buffer("test", 5);
			nested = BinaryMetaDataBuilder::Create();
			internal.Complex("internal", nested 
					.Simple<float>("floatable", Parsers::SimpleOptions::create<float>((float)1.3, (float)5.54, (float)1.5))
					.Simple<double>("dobuleable", Parsers::SimpleOptions::create<double>(-3.1415926535897932384626433832795, 3.1415926535897932384626433832795, 3.141)));
		}

		return BinaryMetaDataBuilder::Create().
			Array("MyInternalStructWithOptionsvar",5,internal).
			Array<uint32_t>("simpleArray",4, Parsers::SimpleOptions::create<uint32_t>(4,7,6)) 
			.CreateParser();

	}

	template<typename T>
	bool TestField(const char* name,BinaryParser parser)
	{
		Parsers::SimpleOptions option;
		option = parser.FieldOptions(name);
		if (option.has_min())
		{
			parser.Write<T>(name, static_cast<T>(option.minval<T>() - 1));
			if (parser.Validate(name)) //should fail
				return false;

			parser.Write<T>(name, option.minval<T>());
			if (false == parser.Validate(name)) //should pass
				return false;
		}
		if(option.has_max())
		{
			parser.Write<T>(name, static_cast<T>(option.maxval<T>() + 1));
			if (parser.Validate(name)) //should fail
				return false;
			parser.Write<T>(name, option.maxval<T>());
			if (false == parser.Validate(name)) //should pass
				return false;

		}
		return true;
	}

	bool TestStructWithLimits()
	{
		LOG_FUNC(APP_LOGGER);

		BinaryParser parser = TestLimits::CreateThreeLayerNestedStructWithLimits();
		//Check that struct has write defaults
		Core::Console::ColorPrint(Core::Console::Colors::CYAN, "Check limits\n");
		
		if (false == TestField<uint32_t>("a",parser))
			return false;
		if (false == TestField<int>("b",parser))
			return false;
		if (false == TestField<uint8_t>("a", parser.ReadComplex("c")))
			return false;
		if (false == TestField<uint8_t>("b", parser.ReadComplex("c")))
			return false;

		parser.ReadComplex("c").Write("test", { 0x10,0xa,0xb,0xc,0xd });
		
		if (false == TestField<int>("j", parser))
			return false;
		parser.Write("str", "Hello world");

		if (false == TestField<float>("floatable", parser.ReadComplex("c").ReadComplex("internal")))
			return false; 
		
		if (false == TestField<double>("dobuleable", parser.ReadComplex("c").ReadComplex("internal")))
			return false;
		
		if (false == parser.FromJson(parser.ToJson())) 
			return false;

		Core::Console::ColorPrint(Core::Console::Colors::WHITE, "\n%s", parser.ToJson(Parsers::JsonDetailsLevel::JSON_ENUM_FULL,false));
		Core::Console::ColorPrint(Core::Console::Colors::CYAN, "Reset to default\n");
		parser.Reset();

		if (false == parser.FromJson(parser.ToJson()))
			return false;
	
		Core::Console::ColorPrint(Core::Console::Colors::WHITE, "\n%s", parser.ToJson(Parsers::JsonDetailsLevel::JSON_ENUM_FULL,false));
		return true;
	}
	bool TestArrayStructDefaults()
	{
		LOG_FUNC(APP_LOGGER);
		ThreeLayerNestedStruct::MyNested1Struct refStr = { 3,7,{ 0,0,0,0,0 }, {0.,0.,0.,0.,0.},{ (float)1.5,(double)3.141 } };

		ThreeLayerNestedStruct::MyNested1Struct *myStr = &refStr;

		BinaryParser parser = TestLimits::CreateStructWithArrays();
		parser.Reset();
		std::cout<< "\n%s"<< parser.ToJson(Parsers::JsonDetailsLevel::JSON_ENUM_FULL, false);

		Parsers::ParserArray<Parsers::BinaryParser> array = parser.ArrayRange<BinaryParser>("MyInternalStructWithOptionsvar");
		//Check Defaults
		uint8_t buffer[5];
		for (BinaryParser &p : array)
		{
			if (p.Read<uint8_t>("a") != myStr->a)
				return false;
			if (p.Read<uint8_t>("b") != myStr->b)
				return false;
			p.Read("test", buffer, sizeof(buffer));
			if (memcmp(buffer, myStr->test, sizeof(buffer)) != 0)
				return false;
			if (p.ReadComplex("internal").Read<float>("floatable") != myStr->internal.floatable)
				return false;
			if (p.ReadComplex("internal").Read<double>("dobuleable") != myStr->internal.dobuleable)
				return false;
		}
		
		Parsers::ParserArray<uint32_t> simpleArray = parser.ArrayRange<uint32_t>("simpleArray");
		for (auto &val : simpleArray)
		{
			if (val != 6)
				return false;
		}
		
		return true;
		
	}
}

namespace Bitwise
{
#pragma pack(1)
	struct IntBit
	{
		uint32_t bit1 : 1;
		uint32_t bit2 : 2;
		uint32_t bit3 : 3;
		uint32_t bit4 : 4;
		uint32_t bit5 : 5;
		uint32_t bit6 : 6;
		uint32_t bit7 : 7;
		uint32_t remain : 4;
	};
	
	struct MyParentBitStruct
	{
		uint8_t a_uint8 :4;
		uint8_t a1_uint8 : 4;
		uint8_t b_uint8 : 6;
		uint8_t b1_uint8 : 2;
		uint8_t c_uint8 : 2;
		uint8_t c1_uint8 : 6;
		int b;
		IntBit c;
		int j;
		float floatable;
	};

	BinaryParser CreateBitwizeNesteddStruct()
	{
		BinaryMetaData internal, nested;
		BinaryMetaData metadata = BinaryMetaDataBuilder::Create().
			Bits<uint8_t>("a_uint8", 4).
			Bits<uint8_t>("a1_uint8", 4).
			Bits<uint8_t>("b_uint8", 6).
			Bits<uint8_t>("b1_uint8", 2).
			Bits<uint8_t>("c_uint8", 2).
			Bits<uint8_t>("c1_uint8", 6).
			Simple<int>("b").
			Complex("c", internal = BinaryMetaDataBuilder::Create()
				.Namely("intBit")
				.Bits<int>("bit1", 1)
				.Bits<int>("bit2", 2)	
				.Bits<int>("bit3", 3)
				.Bits<int>("bit4", 4)
				.Bits<int>("bit5", 5)
				.Bits<int>("bit6", 6)
				.Bits<int>("bit7", 7)
				.Bits<int>("remain", 4)
			)
			.Simple<int>("j")
			.Simple<float>("floatable");
			BinaryMetaDataBuilder metadataFromJson = BinaryMetaDataBuilder::Create(metadata.ToJson());
		return metadataFromJson.CreateParser();
	}

	bool CompareBitParser(BinaryParser& parser, MyParentBitStruct& refStr)
	{
		if (parser.Read<uint8_t>("a_uint8") != refStr.a_uint8)
			return false;
		if (parser.Read<uint8_t>("a1_uint8") != refStr.a1_uint8)
			return false;
		if (parser.Read<uint8_t>("b_uint8") != refStr.b_uint8)
			return false;
		if (parser.Read<uint8_t>("b1_uint8") != refStr.b1_uint8)
			return false;
		if (parser.Read<uint8_t>("c_uint8") != refStr.c_uint8)
			return false;
		if (parser.Read<uint8_t>("c1_uint8") != refStr.c1_uint8)
			return false;
		if(parser.Read<int>("b") != refStr.b)
			return false;
		if(parser.ReadComplex("c").Read<uint32_t>("bit1") != refStr.c.bit1)
			return false;
		if(parser.ReadComplex("c").Read<uint32_t>("bit2")!= refStr.c.bit2)
			return false;										   
		if(parser.ReadComplex("c").Read<uint32_t>("bit3") != refStr.c.bit3)
			return false;										   
		if(parser.ReadComplex("c").Read<uint32_t>("bit4") != refStr.c.bit4)
			return false;										   
		if(parser.ReadComplex("c").Read<uint32_t>("bit5") != refStr.c.bit5)
			return false;										   
		if(parser.ReadComplex("c").Read<uint32_t>("bit6") != refStr.c.bit6)
			return false;
		if(parser.ReadComplex("c").Read<uint32_t>("bit7") != refStr.c.bit7)
			return false;
		
		if (parser.ReadComplex("c").Read<uint32_t>("remain") != refStr.c.remain)
			return false;
		if (parser.Read<int>("j") != refStr.j)
			return false;
		if (parser.Read<float>("floatable") != refStr.floatable)
			return false;

		return true;
	}

	bool TestBitwiseStruct()
	{
		BinaryParser parser = CreateBitwizeNesteddStruct();
		MyParentBitStruct refStr = { 0xa,0xb,0x2f,3,2,0x3f,100,{ 0x1,0x2,0x5,0x6,0x7,0x8,0x9,0xb },100,(float)3.4 };
		
		parser.Write<uint8_t>("a_uint8", 0xa);
		parser.Write<uint8_t>("a1_uint8", 0xb);
		parser.Write<uint8_t>("b_uint8", 0x2f);
		parser.Write<uint8_t>("b1_uint8", 3);
		parser.Write<uint8_t>("c_uint8", 2);
		parser.Write<uint8_t>("c1_uint8", 0x3f);
		parser.Write<int>("b", 100);
		parser.ReadComplex("c").Write<uint32_t>("bit1",1);
		parser.ReadComplex("c").Write<uint32_t>("bit2", 2);
		parser.ReadComplex("c").Write<uint32_t>("bit3", 5);
		parser.ReadComplex("c").Write<uint32_t>("bit4", 6);
		parser.ReadComplex("c").Write<uint32_t>("bit5", 7);
		parser.ReadComplex("c").Write<uint32_t>("bit6", 8);
		parser.ReadComplex("c").Write<uint32_t>("bit7", 9);
		parser.ReadComplex("c").Write<uint32_t>("remain", 0xb);
		parser.Write<int>("j", 100);
		parser.Write<float>("floatable", (float)3.4);

		if (false == parser.FromJson(parser.ToJson()))
			return false;

		if (false == CompareBitParser(parser, refStr))
			return false;

		Core::Console::ColorPrint(Core::Console::Colors::WHITE, "\n%s", parser.ToJson(Parsers::JsonDetailsLevel::JSON_ENUM_FULL, false));
		return true;
		
	}
#pragma pack()
}

namespace TestEnums
{
#pragma pack(1)
	enum MyEnumParsed
	{
		ENUM1,
		ENUM2,
		ENUM3
	};

	enum MyEnumNotParsed
	{
		ENUM_NOT_PARSED1,
		ENUM_NOT_PARSED2,
		ENUM_NOT_PARSED3,
	};

	enum MyEnumParsedWithDefaults
	{
		ENUM_1,
		ENUM_2,
		ENUM_3,
	};

	enum MyEnumParsedNotInt: int16_t
	{
		ENUM_INT16_1,
		ENUM_INT16_2,
		ENUM_INT16_3
	};
	struct MyStruct
	{
		MyEnumParsed myEnumParsed;
		MyEnumNotParsed myEnumNotParsed;
		MyEnumParsedWithDefaults myEnumParsedWithDefaults;
		MyEnumParsedNotInt myEnumParsedNotInt;
		MyEnumParsed myEnumArray[4];

	};
	BinaryParser CreateEnumsStruct()
	{
		BinaryMetaData nested;
		BinaryMetadataStore store;
		EnumData enumData = Parsers::EnumDataFactory::Create("TestEnums::MyEnumParsed").
			AddNewItem(ENUM1, "ENUM1").
			AddNewItem(ENUM2, "ENUM2").
			AddNewItem(ENUM3, "ENUM3");
		store.SetEnum("TestEnums::MyEnumParsed", enumData);
		SimpleOptions options;
		options.defval<MyEnumParsedWithDefaults>(ENUM_2);
		BinaryMetaData metadata = BinaryMetaDataBuilder::Create().
			Enum<MyEnumParsed>("myEnumParsed", enumData).
			Simple<int>("myEnumNotParsed", SimpleOptions()).
			Simple<int>("myEnumParsedWithDefaults", options).
			Simple<int16_t>("myEnumParsedNotInt", SimpleOptions()).
			Array<MyEnumParsed>("myEnumArray", 4, SimpleOptions());

		BinaryMetaDataBuilder metadataFromJson = BinaryMetaDataBuilder::Create(metadata.ToJson());
		return metadataFromJson.CreateParser();
	}

	bool CompareEnumParser(BinaryParser parser, MyStruct& refStr)
	{
		if (parser.Read<MyEnumParsed>("myEnumParsed") != refStr.myEnumParsed)
			return false;
		if (parser.Read<MyEnumNotParsed>("myEnumNotParsed") != refStr.myEnumNotParsed)
			return false;
		if (parser.Read<MyEnumParsedWithDefaults>("myEnumParsedWithDefaults") != refStr.myEnumParsedWithDefaults)
			return false;
		if (parser.Read<MyEnumParsedNotInt>("myEnumParsedNotInt") != refStr.myEnumParsedNotInt)
			return false;
		ParserArray< MyEnumParsed> arr = parser.ArrayRange<MyEnumParsed>("myEnumArray");
		size_t i = 0;
		for (auto& data : arr)
		{
			if (data != refStr.myEnumArray[i])
				return false;
			i++;
		}
		return true;
	}
	bool TestEnumsStruct()
	{
		BinaryParser parser = CreateEnumsStruct();
		parser.Reset();
		MyStruct refStr = { MyEnumParsed::ENUM1,MyEnumNotParsed::ENUM_NOT_PARSED2,MyEnumParsedWithDefaults::ENUM_2,MyEnumParsedNotInt::ENUM_INT16_2,{MyEnumParsed::ENUM1,MyEnumParsed::ENUM2,MyEnumParsed::ENUM3,MyEnumParsed::ENUM1} };
		
		parser.Write<MyEnumParsed>("myEnumParsed", MyEnumParsed::ENUM1);
		parser.Write<MyEnumNotParsed>("myEnumNotParsed", MyEnumNotParsed::ENUM_NOT_PARSED2);
		//parser.Write<MyEnumParsedWithDefaults>("myEnumParsedWithDefaults", MyEnumParsedWithDefaults::ENUM_2);
		parser.Write<MyEnumParsedNotInt>("myEnumParsedNotInt", MyEnumParsedNotInt::ENUM_INT16_2);

		size_t len = parser.ArraySize("myEnumArray");
		for (size_t i = 0 ;i< len;i++)
		{
			parser.WriteArrayAt<MyEnumParsed>("myEnumArray", i, (MyEnumParsed)(((MyEnumParsed::ENUM1 + i) % 3)));
		}
		if (false == parser.FromJson(parser.ToJson(Parsers::JsonDetailsLevel::JSON_ENUM_FULL,false)))
			return false;

		if (false == CompareEnumParser(parser, refStr))
			return false;

		if (false == parser.FromJson(parser.ToJson(Parsers::JsonDetailsLevel::JSON_ENUM_LABLES, false)))
			return false;

		if (false == CompareEnumParser(parser, refStr))
			return false;

		if (false == parser.FromJson(parser.ToJson(Parsers::JsonDetailsLevel::JSON_ENUM_VALUES,false)))
			return false;

		if (false == CompareEnumParser(parser, refStr))
			return false;

		if (false == parser.FromJson(parser.ToJson( Parsers::JsonDetailsLevel::JSON_ENUM_VALUES,false)))
			return false;

		if (false == CompareEnumParser(parser, refStr))
			return false;
		
		if (false == parser.FromJson(parser.ToJson(Parsers::JsonDetailsLevel::JSON_ENUM_LABLES,false)))
			return false;

		if (false == CompareEnumParser(parser, refStr))
			return false;

		if (false == parser.FromJson(parser.ToJson(Parsers::JsonDetailsLevel::JSON_ENUM_FULL,true)))
			return false;

		if (false == CompareEnumParser(parser, refStr))
			return false;

		Core::Console::ColorPrint(Core::Console::Colors::WHITE, "\n%s", parser.ToJson(core::parsers::json_details_level::JSON_ENUM_VALUES,false));
		Core::Console::ColorPrint(Core::Console::Colors::WHITE, "\n%s", parser.ToJson(core::parsers::json_details_level::JSON_ENUM_FULL,false));
		Core::Console::ColorPrint(Core::Console::Colors::WHITE, "\n%s", parser.ToJson(core::parsers::json_details_level::JSON_ENUM_LABLES,false));

		return true;
	}
#pragma pack()
}
int main(int argc, const char* argv[])
{
	//Test Reading from Buffer 
	Core::Console::ColorPrint(Core::Console::Colors::WHITE,"Read Test\n");
	Core::Console::ColorPrint(Core::Console::Colors::WHITE, "\nTestbigEndian and To String Function\n");
	TestSimpleStruct_bigEndian();
	Core::Console::ColorPrint(Core::Console::Colors::WHITE, "\n----------------------------------------------------------------------------");
	if(ComplexParser::TestComplex() == true)
		Core::Console::ColorPrint(Core::Console::Colors::GREEN, "\nTestComplex: Passed ");
	else
		Core::Console::ColorPrint(Core::Console::Colors::RED, "\nTestComplex: failed ");
	Core::Console::ColorPrint(Core::Console::Colors::WHITE, "\n----------------------------------------------------------------------------");
	
	if (ComplexWithComplexArray::TestComplexWithComplexArray() == true)
		Core::Console::ColorPrint(Core::Console::Colors::GREEN, "\nTestComplexWithComplexArray: Passed ");
	else
		Core::Console::ColorPrint(Core::Console::Colors::RED, "\nTestComplexWithComplexArray: failed ");
	Core::Console::ColorPrint(Core::Console::Colors::WHITE, "\n----------------------------------------------------------------------------");
	if (ThreeLayerNestedStruct::TestThreeLayerNestedStruct() == true)
		Core::Console::ColorPrint(Core::Console::Colors::GREEN, "\nTestThreeLayerNestedStruct: Passed ");
	else
		Core::Console::ColorPrint(Core::Console::Colors::RED, "\nTestThreeLayerNestedStruct: failed ");
	
	Core::Console::ColorPrint(Core::Console::Colors::WHITE, "\n----------------------------------------------------------------------------");

	//Test Write to Buffer
	Core::Console::ColorPrint(Core::Console::Colors::WHITE, "\nWrite Test");
	Core::Console::ColorPrint(Core::Console::Colors::WHITE, "\n----------------------------------------------------------------------------");
	if (WriteComplex::TestWriteComplex() == true)
		Core::Console::ColorPrint(Core::Console::Colors::GREEN, "\nTestWriteComplex: Passed ");
	else
		Core::Console::ColorPrint(Core::Console::Colors::RED, "\nTestWriteComplex: failed ");
	Core::Console::ColorPrint(Core::Console::Colors::WHITE, "\n----------------------------------------------------------------------------");

	if (WriteComplexWithComplexArray::TestWriteComplexWithComplexArray() == true)
		Core::Console::ColorPrint(Core::Console::Colors::GREEN, "\nTestComplexWithComplexArray: Passed ");
	else
		Core::Console::ColorPrint(Core::Console::Colors::RED, "\nTestComplexWithComplexArray: failed ");
	Core::Console::ColorPrint(Core::Console::Colors::WHITE, "\n----------------------------------------------------------------------------");
	if (WriteThreeLayerNestedStruct::TestWriteThreeLayerNestedStruct() == true)
		Core::Console::ColorPrint(Core::Console::Colors::GREEN, "\nTestWriteThreeLayerNestedStruct: Passed ");
	else
		Core::Console::ColorPrint(Core::Console::Colors::RED, "\nTestWriteThreeLayerNestedStruct: failed ");
	Core::Console::ColorPrint(Core::Console::Colors::WHITE, "\n----------------------------------------------------------------------------");

	if (TestLimits::TestStructWithLimits() == true)
		Core::Console::ColorPrint(Core::Console::Colors::GREEN, "\nTestStructWithLimits: Passed ");
	else
		Core::Console::ColorPrint(Core::Console::Colors::RED, "\nTestStructWithLimits: failed ");
	Core::Console::ColorPrint(Core::Console::Colors::WHITE, "\n----------------------------------------------------------------------------");
	
	if (TestLimits::TestArrayStructDefaults() == true)
		Core::Console::ColorPrint(Core::Console::Colors::GREEN, "\nTestArrayStructDefaults: Passed ");
	else
		Core::Console::ColorPrint(Core::Console::Colors::RED, "\nTestArrayStructDefaults: failed ");
	Core::Console::ColorPrint(Core::Console::Colors::WHITE, "\n----------------------------------------------------------------------------");

	if (Bitwise::TestBitwiseStruct() == true)
		Core::Console::ColorPrint(Core::Console::Colors::GREEN, "\nTestBitwiseStruct: Passed ");
	else
		Core::Console::ColorPrint(Core::Console::Colors::RED, "\nTestBitwiseStruct: failed ");
	Core::Console::ColorPrint(Core::Console::Colors::WHITE, "\n----------------------------------------------------------------------------");

	if(TestEnums::TestEnumsStruct() == true)
		Core::Console::ColorPrint(Core::Console::Colors::GREEN, "\nTestEnumsStruct: Passed ");
	else
		Core::Console::ColorPrint(Core::Console::Colors::RED, "\nTestEnumsStruct: failed ");
	Core::Console::ColorPrint(Core::Console::Colors::WHITE, "\n----------------------------------------------------------------------------");

	std::getchar();
	return 0;
}
