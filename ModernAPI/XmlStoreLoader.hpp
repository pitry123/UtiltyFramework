#pragma once

#include <BinaryParser.hpp>
#include <Files.hpp>
#include <Database.hpp>
#include <Core.hpp>


class XmlStoreLoader
{
public:
	static bool Load(Database::Table& table, Files::XmlElement& dbElement, bool verbose = false)
	{
		XmlStoreLoader store_loader(table, verbose);
		return store_loader.LoadTable(dbElement);
	}

	static bool Load(Database::Table& table, const std::string& dataSetPath, bool verbose = false)
	{
		
		Files::XmlFile xmlFile;

		try
		{
			xmlFile = Files::XmlFile::Create(dataSetPath.c_str());
		}
		catch (std::exception &e)
		{
			if (verbose)
				Core::Console::ColorPrint(Core::Console::Colors::RED, "Error Reading dataset file : %s, error %s\n", dataSetPath.c_str(), e.what());
			return false;
		}

		Files::XmlElement dbElement = xmlFile.QueryElement("DATABASE");
		if (dbElement.Empty())
		{
			if (verbose)
				Core::Console::ColorPrint(Core::Console::Colors::YELLOW, "No 'DATABASE' defined in XML file\n");
			return false;
		}
		return XmlStoreLoader::Load(table, dbElement, verbose);
	}


private:
	bool m_verbose;
	Database::Table m_table;
	XmlStoreLoader(Database::Table& table, bool verbose) : m_verbose(verbose), m_table(table)
	{
		// Nothing to do for now...
	}

	

	bool LoadTable(Files::XmlElement& element)
	{
		// Build all prototype types from XML
		for (auto& child : element.Children("PROTOTYPE"))
		{
			std::string name = child.QueryAttribute("name").Value();
			BuildPrototype(name, child);
		}
		
		// Now read all DB entries from XML and fill given table
		size_t size = 0;
		int db_entry_indx = 0;
		Parsers::BinaryParser parser;
		Parsers::BinaryMetadataStore store;
			
		for (auto& db_entry : element.Children("DBENTRY"))
		{
			Database::RowInfo info;
			Parsers::BinaryMetaDataBuilder metadata;
			STRCPY(info.name, sizeof(info.name), db_entry.QueryAttribute("name").Value());

			// See if entry has prototype definition
			auto db_entry_proto = db_entry.QueryAttribute("prototype");
			if (db_entry_proto.Empty() == false)
			{
				// DB entry has prototype. Load prototype from metadata store
				metadata = store.Metadata(db_entry_proto.Value());
				if (metadata.Empty())
				{
					// Prototype undefined - Error
					if (m_verbose)
						Core::Console::ColorPrint(Core::Console::Colors::RED, "DBENTRY %s prototype does not exist. Invalid XML\n", info.name);
					return false;
				}
				STRCPY(info.type_name, sizeof(info.type_name), db_entry_proto.Value());
			}

			if (metadata.Empty())
			{
				// DB entry does not have prototype. Create metadata for it
				for (auto& child : db_entry.Children())
				{
					if (child.IsComment())
						continue;	// skip xml comment

					TypeEnum type = utils::types::get_type(ToLowerCase(child.Name()).c_str());
					std::string name; // Name is optional but try to get it anyway
					auto attr = child.QueryAttribute("name");
					if (attr.Empty() == false)
						name = std::string(attr.Value());

					if (type == TypeEnum::COMPLEX)
					{
						metadata = BuildComplex(name, child);
					}
					else if (type == TypeEnum::ARRAY)
					{
						BuildArray(name, child, metadata);
					}
					else
					{
						metadata = Parsers::BinaryMetaDataBuilder::Create();
						if (type == TypeEnum::STRING)
						{
                            size = static_cast<size_t>(std::stoi(child.QueryAttribute("size").Value()));
						}
						BuildField(name, size, type, child, metadata);
					}
				}

				//if (m_verbose)
				//	PrintMetadata(metadata);
			}
			
			// OK, metadata is ready. Now use parser
			parser = metadata.CreateParser();
			parser.Reset(); //Set the defaults
			// Fill DB entry Values
			size_t ch_index = 0;
			for (auto& child : db_entry.Children())
			{
				if (child.IsComment())
					continue;	// skip xml comment

				TypeEnum ch_type = utils::types::get_type(ToLowerCase(child.Name()).c_str());
				std::string ch_name; // Name is optional but try to get it anyway
				auto ch_attr = child.QueryAttribute("name");
				if (ch_attr.Empty() == false)
					ch_name = std::string(ch_attr.Value());
				else
					ch_name = std::to_string(ch_index);
				if (ch_type == TypeEnum::COMPLEX)
				{
					ParseComplex(ch_name, child, parser);
				}
				else if (ch_type == TypeEnum::ARRAY)
				{
					ParseArray(ch_name, child, parser);
				}
				else
					ParseField(ch_name, ch_type, child, parser);

				ch_index++;
			}

			//Add row to table if necessary.
			Database::Row row;
			m_table.TryGet(db_entry_indx, row);
			if (row.Empty())
				m_table.AddRow(db_entry_indx, metadata.Size(), info, metadata);
			Buffers::Buffer buffer = parser.Buffer();
			m_table[db_entry_indx].Write((const void*)buffer.Data(), buffer.Size());
			
			if (m_verbose)
			{
				Core::Console::ColorPrint(Core::Console::Colors::YELLOW, "DBENTRY %d - %s. Data:\n", db_entry_indx, info.name);
				//Core::Console::ColorPrint(Core::Console::Colors::WHITE, "%s\n", parser.ToJson(false));
			}

			db_entry_indx++;
		}
		
		return true;
	}

	bool BuildPrototype(const std::string& name, Files::XmlElement& pt_element)
	{
		// Prototype usually has one complex child. No sense in defining prototype with primitive type
		// in it or multiple primitive types without complex (struct) type surrounding them.
		auto pt_child = pt_element.QueryChild(static_cast<size_t>(0));
		if (pt_child.Empty())
		{
			if (m_verbose)
				Core::Console::ColorPrint(Core::Console::Colors::YELLOW, "Prototype '%s' has no children. Invalid.\n", name.c_str());
			return false;
		}

		const char* type_name = pt_child.Name();
		if (utils::types::get_type(ToLowerCase(type_name).c_str()) != TypeEnum::COMPLEX)
		{
			if (m_verbose)
				Core::Console::ColorPrint(Core::Console::Colors::YELLOW, "Prototype '%s' is not complex type. Invalid.\n", name.c_str());
			return false;
		}
		auto attr = pt_child.QueryAttribute("name");
		std::string structName = attr.Empty() ? name : attr.Value();	// use prototype name if no name attribute to (struct) child
		if (m_verbose)
			Core::Console::ColorPrint(Core::Console::Colors::GREEN, "Found valid prototype - '%s'\n", name.c_str());
		
		// Build prototype metadata
		Parsers::BinaryMetaData metadata = BuildComplex(structName, pt_child);
		
		// Name the new prototype metadata in store
		Parsers::BinaryMetadataStore store;
		store.SetMetadata(name.c_str(), metadata);
		
        //if (m_verbose)
        //{
        //    Parsers::BinaryMetaDataBuilder mb = store.Metadata(name.c_str());
        //    PrintMetadata(mb);
        //}
		return true;
	}

	Parsers::BinaryMetaData BuildComplex(const std::string& pt_name, Files::XmlElement& pt_child)
	{
		size_t size = 0;
		Parsers::BinaryMetadataStore store;
		Parsers::BinaryMetaDataBuilder metadata;

		// Look for the prototype in store.
		if (pt_name.empty() == false)
			metadata = store.Metadata(pt_name.c_str());

		if (metadata.Empty())
		{
			// Prototype is not found in store. Need to create new metadata for it
			metadata = Parsers::BinaryMetaDataBuilder::Create();

			for (auto& child : pt_child.Children())
			{
				if (child.IsComment())
					continue;	// skip xml comment

				TypeEnum type = utils::types::get_type(ToLowerCase(child.Name()).c_str());
				std::string name; // Name is optional but try to get it anyway
				auto attr = child.QueryAttribute("name");
				if (attr.Empty() == false)
					name = std::string(attr.Value());

				if (type == TypeEnum::COMPLEX)
				{
					metadata.Complex(name.c_str(), BuildComplex(name, child));
				}
				else if (type == TypeEnum::ARRAY)
				{
					BuildArray(name, child, metadata);
				}
				else
				{
					if (type == TypeEnum::STRING)
					{
                        size = static_cast<size_t>(std::stoi(child.QueryAttribute("size").Value()));
					}
					BuildField(name, size, type, child, metadata);
				}
			}

			// Metadata for prototype is complete. Save it for later use in store
			if (pt_name.empty() == false)
				metadata.Namely(pt_name.c_str());
		}
		return std::move(metadata);
	}

	bool BuildArray(const std::string& ar_name, Files::XmlElement element, Parsers::BinaryMetaDataBuilder& metadata)
	{
		bool child_found(false);
		size_t ar_size(0), loop_indx(0), first_child_indx(0);
		Files::XmlAttribute attr = element.QueryAttribute("size");
		if (attr.Empty())
		{
			// No information of array size in array tag. No choice but to iterate on children and count
			for (auto& child : element.Children())
			{
				if (child.IsComment() == false)
				{
					ar_size++;
					if (child_found == false)
					{
						first_child_indx = loop_indx;
						child_found = true;
					}
				}
				loop_indx++;
			}
		}
		else
            ar_size = static_cast<size_t>(std::stoi(attr.Value()));

		// Nothing to build if array is empty
		if (child_found == false)
			return false;

		// Get type of array members from the first real child of the array.
		auto ar_child = element.QueryChild(static_cast<size_t>(first_child_indx));
		TypeEnum type = utils::types::get_type(ToLowerCase(ar_child.Name()).c_str());

		// No support for strings
		if (type == TypeEnum::STRING)
			return false;

		if (type == TypeEnum::COMPLEX)
		{
			std::string ch_name; // Name is optional but try to get it anyway
			attr = ar_child.QueryAttribute("name");
			if (attr.Empty() == false)
				ch_name = std::string(attr.Value());

			metadata.Array(ar_name.c_str(), ar_size, BuildComplex(ch_name, ar_child));
		}
		else
		{
			metadata.Array(ar_name.c_str(), type, ar_size);
		}
		return false;
	}

	bool BuildField(const std::string& name, size_t size, TypeEnum type, Files::XmlElement element, Parsers::BinaryMetaDataBuilder& metadata)
	{
		Parsers::BinaryMetadataStore store;
		switch (type)
		{
			case TypeEnum::UINT8:
			case TypeEnum::BYTE:
			case TypeEnum::BOOL:
				AddSimple<uint8_t>(name, sizeof(uint8_t), element, metadata);
				break;

			case TypeEnum::INT8:
			case TypeEnum::CHAR:
				AddSimple<char>(name, sizeof(char), element, metadata);
				break;

			case TypeEnum::USHORT:
			case TypeEnum::UINT16:
				AddSimple<uint16_t>(name, sizeof(uint16_t), element, metadata);
				break;

			case TypeEnum::SHORT:
			case TypeEnum::INT16:
				AddSimple<int16_t>(name, sizeof(int16_t), element, metadata);
				break;

			case TypeEnum::UINT32:
				AddSimple<uint32_t>(name, sizeof(uint32_t), element, metadata);
				break;

			case TypeEnum::INT32:
				AddSimple<int32_t>(name, sizeof(int32_t), element, metadata);
				break;

			case TypeEnum::FLOAT:
				AddSimple<float>(name, sizeof(float), element, metadata);
				break;
		
			case TypeEnum::DOUBLE:
				AddSimple<double>(name, sizeof(double), element, metadata);
				break;
		
			case TypeEnum::INT64:
				AddSimple<int64_t>(name, sizeof(int64_t), element, metadata);
				break;
		
			case TypeEnum::UINT64:
				AddSimple<uint64_t>(name, sizeof(uint64_t), element, metadata);
				break;

			case TypeEnum::STRING:
				metadata.String(name.c_str(), size, element.Value());
				break;

			default:
				if (m_verbose)
					Core::Console::ColorPrint(Core::Console::Colors::YELLOW, "Field '%s' is unknown or complex\n", name.c_str());
				return false;
		}

		return true;
	}

	template<typename T> void AddSimple(const std::string& name, size_t size, Files::XmlElement& element, Parsers::BinaryMetaDataBuilder& metadata)
	{
		Parsers::SimpleOptions options;
		options.defval(element.Value(), utils::types::get_type<T>());

		if (size == sizeof(T))
		{
			metadata.Simple<T>(name.c_str(), options);
		}
		else
		{
			//Check if it is an array
			if (CheckIfArray(size, sizeof(T)))
			{
				metadata.Array<T>(name.c_str(), size, options);
			}
		}
	}

	bool ParseComplex(const std::string& elm_name, Files::XmlElement& elm_child, Parsers::BinaryParser& parser)
	{
		size_t ch_index = 0;
		for (auto& child : elm_child.Children())
		{
			if (child.IsComment())
				continue;	// skip xml comment

			TypeEnum ch_type = utils::types::get_type(ToLowerCase(child.Name()).c_str());
			std::string ch_name; // Name is optional but try to get it anyway
			auto ch_attr = child.QueryAttribute("name");
			if (ch_attr.Empty() == false)
				ch_name = std::string(ch_attr.Value());
			else
				ch_name = std::to_string(ch_index);
			if (ch_type == TypeEnum::COMPLEX)
			{
				Parsers::BinaryParser ch_parser = parser.ReadComplex(ch_name.c_str());
				ParseComplex(ch_name, child, ch_parser);
			}
			else if (ch_type == TypeEnum::ARRAY)
			{
				ParseArray(ch_name, child, parser);
			}
			else
				ParseField(ch_name, ch_type, child, parser);

			ch_index++;
		}
		return true;
	}

	bool ParseArray(const std::string& name, Files::XmlElement element, Parsers::BinaryParser& parser)
	{
		size_t count(0);
		for (auto& child : element.Children())
		{
			if (child.IsComment())
				continue;	// skip xml comment

			TypeEnum ch_type = utils::types::get_type(ToLowerCase(child.Name()).c_str());
			if (ch_type == TypeEnum::STRING)
				return false;

			std::string ch_name; // Name is optional but try to get it anyway
			auto ch_attr = child.QueryAttribute("name");
			if (ch_attr.Empty() == false)
				ch_name = std::string(ch_attr.Value());

			if (ch_type == TypeEnum::COMPLEX)
			{
				//Parsers::BinaryParser ch_parser = parser.ReadComplex(ch_name.c_str());
				//ParseComplex(ch_name, child, ch_parser);
			}

			switch (ch_type)
			{
				case TypeEnum::UINT8:
				case TypeEnum::BYTE:
				case TypeEnum::BOOL:
				{
					uint8_t val = static_cast<decltype(val)>(child.ValueAsInt(0));
					parser.WriteArrayAt<decltype(val)>(name.c_str(), count, val);
				}
				break;

				case TypeEnum::INT8:
				case TypeEnum::CHAR:
				{
					int8_t val =  static_cast<decltype(val)>(child.ValueAsInt(0));
					parser.WriteArrayAt<decltype(val)>(name.c_str(), count, val);
				}
				break;

				case TypeEnum::USHORT:
				case TypeEnum::UINT16:
				{
					uint16_t val = static_cast<decltype(val)>(child.ValueAsInt(0));
					parser.WriteArrayAt<decltype(val)>(name.c_str(), count, val);
				}
				break;

				case TypeEnum::SHORT:
				case TypeEnum::INT16:
				{
					int16_t val = static_cast<decltype(val)>(child.ValueAsInt(0));
					parser.WriteArrayAt<decltype(val)>(name.c_str(), count, val);
				}
				break;

				case TypeEnum::UINT32:
				{
					uint32_t val = static_cast<decltype(val)>(child.ValueAsInt(0));
					parser.WriteArrayAt<decltype(val)>(name.c_str(), count, val);
				}
				break;

				case TypeEnum::INT32:
				{
					int32_t val = static_cast<decltype(val)>(child.ValueAsInt(0));
					parser.WriteArrayAt<decltype(val)>(name.c_str(), count, val);
				}
				break;

				case TypeEnum::FLOAT:
				{
					float val = child.ValueAsFloat(0);
					parser.WriteArrayAt<float>(name.c_str(), count, val);
				}
				break;

				case TypeEnum::DOUBLE:
				{
					double val = child.ValueAsDouble(0);
					parser.WriteArrayAt<double>(name.c_str(), count, val);
				}
				break;

				case TypeEnum::INT64:
				{
					int64_t val = static_cast<decltype(val)>(child.ValueAsLong(0));
					parser.WriteArrayAt<decltype(val)>(name.c_str(), count, val);
				}
				break;

				case TypeEnum::UINT64:
				{
					uint64_t val = static_cast<decltype(val)>(child.ValueAsLong(0));
					parser.WriteArrayAt<decltype(val)>(name.c_str(), count, val);
				}
				break;

				default:
					break;
			}

			count++;
		}
		return true;
	}

	bool ParseField(const std::string& name, TypeEnum type, Files::XmlElement element, Parsers::BinaryParser& parser)
	{
		switch (type)
		{
			case TypeEnum::UINT8:
			case TypeEnum::BYTE:
			case TypeEnum::BOOL:
			{
				uint8_t val = static_cast<decltype(val)>(element.ValueAsInt(0));
				parser.Write<decltype(val)>(name.c_str(), val);
			}
			break;

			case TypeEnum::INT8:
			case TypeEnum::CHAR:
			{
				int8_t val =  static_cast<decltype(val)>(element.ValueAsInt(0));
				parser.Write<decltype(val)>(name.c_str(), val);
			}
			break;

			case TypeEnum::USHORT:
			case TypeEnum::UINT16:
			{
				uint16_t val = static_cast<decltype(val)>(element.ValueAsInt(0));
				parser.Write<decltype(val)>(name.c_str(), val);
			}
			break;

			case TypeEnum::SHORT:
			case TypeEnum::INT16:
			{
				int16_t val = static_cast<decltype(val)>(element.ValueAsInt(0));
				parser.Write<decltype(val)>(name.c_str(), val);
			}
			break;

			case TypeEnum::UINT32:
			{
				uint32_t val = static_cast<decltype(val)>(element.ValueAsInt(0));
				parser.Write<decltype(val)>(name.c_str(), val);
			}
			break;

			case TypeEnum::INT32:
			{
				int32_t val = static_cast<decltype(val)>(element.ValueAsInt(0));
				parser.Write<decltype(val)>(name.c_str(), val);
			}
			break;

			case TypeEnum::FLOAT:
			{
				float val = element.ValueAsFloat(0);
				parser.Write<decltype(val)>(name.c_str(), val);
			}
			break;

			case TypeEnum::DOUBLE:
			{
				double val = element.ValueAsDouble(0);
				parser.Write<decltype(val)>(name.c_str(), val);
			}
			break;

			case TypeEnum::INT64:
			{
				int64_t val = static_cast<decltype(val)>(element.ValueAsLong(0));
				parser.Write<decltype(val)>(name.c_str(), val);
			}
			break;

			case TypeEnum::UINT64:
			{
				uint64_t val = static_cast<decltype(val)>(element.ValueAsLong(0));
				parser.Write<decltype(val)>(name.c_str(), val);
			}
			break;

			case TypeEnum::STRING:
			{
				const char* val = element.Value();
				parser.Write(name.c_str(), val);
			}
			break;

			default:
				return false;
		}

		return true;
	}

	std::string ToLowerCase(const std::string& str)
	{
		std::string lowerCase = str;
		std::transform(lowerCase.begin(), lowerCase.end(), lowerCase.begin(), [](unsigned char c) 
		{
			std::locale loc;
			return std::tolower(static_cast<char>(c), loc);
		});
		return lowerCase;
	}

	bool CheckIfArray(size_t size, size_t type_size)
	{
		if (size > type_size && size % type_size == 0)
			return true;

		return false;
	}

	void PrintMetadata(Parsers::BinaryMetaDataBuilder& metadata)
	{
		int i = 0;
		bool cont_prnt = false;
		do
		{
			std::string str = metadata.ToJson(false) + i;
			if (str.length() >= 1023)
			{
				cont_prnt = true;
				str[1023] = '\0';
			}
			else
				cont_prnt = false;
			Core::Console::ColorPrint(Core::Console::Colors::WHITE, "%s", str.c_str());
			i += 1023;
		} while (cont_prnt);
		Core::Console::ColorPrint(Core::Console::Colors::WHITE, "\n");
	}

	
};
