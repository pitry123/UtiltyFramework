#pragma once

#include <map>
#include <string>
#include <sstream>

#include <files/filessystem.h>

#include <Files.hpp>
#include <Factories.hpp>
#include <ConfigurationLoader.hpp>
#include <BinaryParser.hpp>

#define MAX_NESTING_DEPTH 10


namespace ConfigurationLoader
{

	class XmlStoreDB : public StoreDB
	{
	private:
		/// @brief	The file settings map
		std::map<eFILE_HIERARCHY, Files::XmlFile> m_fileSettingsMap;
		bool m_verbose;

		Files::XmlFile CreateUserXmlFile(const char* _factoryPath, const char* _userPath)
		{
			Files::XmlFile file;
			bool fileIsCorrupted = false;

			// Check if xml file exists.
			if (Files::FilesSystem::IsFileExist(_userPath))
			{
				try
				{
					file = Files::XmlFile::Create(_userPath);
				}
				catch (const std::exception& e)
				{
					LOG_WARNING(CFG_LOGGER) << e.what() << " | File is corrupted in" << _userPath << ", trying to copy from " << _factoryPath;
					fileIsCorrupted = true;
				}
			}

			// If xml file does not exists or corrupted.
			if (!Files::FilesSystem::IsFileExist(_userPath) || fileIsCorrupted)
			{
				// Try to copy the file from factory settings to user settings.
				Files::FilesSystem::CopyFile(_factoryPath, _userPath, true);

				if (m_verbose == true)
					Core::Console::ColorPrint(Core::Console::Colors::GREEN, "Copy to user Setting from: %s to %s - Succeeded\n", _factoryPath, _userPath);

				// And create xml file once again.
				file = Files::XmlFile::Create(_userPath);
			}

			if (m_verbose == true)
				Core::Console::ColorPrint(Core::Console::Colors::GREEN, "Open User Setting file: %s - Succeeded\n", _userPath);

			return file;
		}

	protected:
		///-------------------------------------------------------------------
		/// @brief	Adds default data to simple types in metadata
		///
		/// @tparam	T	Generic type parameter.
		/// @param 		   	name		Data name.
		/// @param [in,out]	val			Data default value.
		/// @param [in,out]	metadata	The metadata.
		///-------------------------------------------------------------------
		template<typename T>
		void AddSimpleData(const char* name, T& val, Parsers::BinaryMetaDataBuilder& metadata)
		{
			Parsers::SimpleOptions options;
			options.defval<T>(val);
			metadata.Simple<T>(name, options);
		}

		///-------------------------------------------------------------------
		/// @brief	Loads one xml element, recursive function which load the element and all its suns 
		///
		/// @date	22/05/2018
		///
		/// @exception	std::runtime_error	Raised when a runtime error condition
		/// 								occurs.
		///
		/// @param 		   	element		 	The xml DOM element 
		/// @param [in,out]	o_buffer	 	holder for the element values .
		/// @param [in,out]	o_buffer_size	Size of the buffer.
		///
		/// @return	True if it succeeds, false if it fails.
		///-------------------------------------------------------------------
		virtual bool LoadElement(Files::XmlElement element, char *o_buffer, size_t& o_buffer_size, Parsers::BinaryMetaDataBuilder& metadata, int nestingDepth = 0)
		{
			Parsers::BinaryMetaDataBuilder internal_metadata = metadata;
			if (metadata.Empty())
				internal_metadata = Parsers::BinaryMetaDataBuilder::Create();

			auto attribute = element.QueryAttribute("type");
			if (0 == strcmp(attribute.Value(), "STRUCT"))
			{
				for (auto& child : element.Children())
				{
					if (0 == strcmp(child.QueryAttribute("type").Value(), "STRUCT"))
					{
						nestingDepth++;

						if (nestingDepth >= MAX_NESTING_DEPTH)
						{
							LOG_INFO(CFG_LOGGER) << "Failed to parse the XML file it might be corrupted ";
							throw std::runtime_error("failed to parse xml file");
						}

						size_t o_bSize = 0;
						char o_buff[2048] = {};
						Parsers::BinaryMetaDataBuilder complex_metadata = Parsers::BinaryMetaDataBuilder::Create();
						LoadElement(child, o_buff, o_bSize, complex_metadata, nestingDepth);
						memcpy(o_buffer + o_buffer_size, o_buff, o_bSize);
						o_buffer_size += o_bSize;
						internal_metadata.Complex(element.Name(), complex_metadata);
					}
					else
					{
						LoadElement(child, o_buffer + o_buffer_size, o_buffer_size, metadata);
					}

					//here we need to start new counter
					nestingDepth = 0;

				}
			}
			else if (0 == strcmp(attribute.Value(), "INT"))
			{
				int val = element.QueryChild((size_t)0).ValueAsInt(0);
				memcpy(o_buffer, &val, sizeof(int));
				o_buffer_size += sizeof(int);
				AddSimpleData<decltype(val)>(element.Name(), val, internal_metadata);
			}
			else if (0 == strcmp(attribute.Value(), "INT8_T") || 0 == strcmp(attribute.Value(), "CHAR"))
			{
				int8_t val = (int8_t)element.QueryChild((size_t)0).ValueAsInt(0);
                memcpy(o_buffer, &val, sizeof(int8_t));
                o_buffer_size += sizeof(int8_t);
				AddSimpleData<decltype(val)>(element.Name(), val, internal_metadata);
			}
			else if (0 == strcmp(attribute.Value(), "INT16_T"))
			{
				int16_t val = (int16_t)element.QueryChild((size_t)0).ValueAsInt(0);
                memcpy(o_buffer, &val, sizeof(int16_t));
                o_buffer_size += sizeof(int16_t);
				AddSimpleData<decltype(val)>(element.Name(), val, internal_metadata);
			}
			else if (0 == strcmp(attribute.Value(), "INT32_T"))
			{
				int32_t val = element.QueryChild((size_t)0).ValueAsInt(0);
				memcpy(o_buffer, &val, sizeof(int32_t));
				o_buffer_size += sizeof(int32_t);
				AddSimpleData<decltype(val)>(element.Name(), val, internal_metadata);
			}
			else if (0 == strcmp(attribute.Value(), "INT64_T"))
			{
				int64_t val = element.QueryChild((size_t)0).ValueAsLong(0);
				memcpy(o_buffer, &val, sizeof(int64_t));
				o_buffer_size += sizeof(int64_t);
				AddSimpleData<decltype(val)>(element.Name(), val, internal_metadata);
			}
			else if (0 == strcmp(attribute.Value(), "UINT8_T"))
			{
				uint8_t val = (uint8_t)element.QueryChild((size_t)0).ValueAsUInt(0);
				memcpy(o_buffer, &val, sizeof(uint8_t));
				o_buffer_size += sizeof(uint8_t);
				AddSimpleData<decltype(val)>(element.Name(), val, internal_metadata);
			}
			else if (0 == strcmp(attribute.Value(), "UINT16_T"))
			{
				uint16_t val = (uint16_t)element.QueryChild((size_t)0).ValueAsUInt(0);
				memcpy(o_buffer, &val, sizeof(uint16_t));
				o_buffer_size += sizeof(uint16_t);
				AddSimpleData<decltype(val)>(element.Name(), val, internal_metadata);
			}
			else if (0 == strcmp(attribute.Value(), "UINT32_T"))
			{
				uint32_t val = element.QueryChild((size_t)0).ValueAsUInt(0);
				memcpy(o_buffer, &val, sizeof(uint32_t));
				o_buffer_size += sizeof(uint32_t);
				AddSimpleData<decltype(val)>(element.Name(), val, internal_metadata);
			}
			else if (0 == strcmp(attribute.Value(), "UINT64_T"))
			{
				uint64_t val = (uint64_t)element.QueryChild((size_t)0).ValueAsULong(0);
				memcpy(o_buffer, &val, sizeof(uint64_t));
				o_buffer_size += sizeof(uint64_t);
				AddSimpleData<decltype(val)>(element.Name(), val, internal_metadata);
			}
			else if (0 == strcmp(attribute.Value(), "DOUBLE"))
			{
				double val = element.QueryChild((size_t)0).ValueAsDouble(0);
				memcpy(o_buffer, &val, sizeof(double));
				o_buffer_size += sizeof(double);
				AddSimpleData<decltype(val)>(element.Name(), val, internal_metadata);
			}
			else if (0 == strcmp(attribute.Value(), "FLOAT"))
			{
				float val = element.QueryChild((size_t)0).ValueAsFloat(0);
				memcpy(o_buffer, &val, sizeof(float));
				o_buffer_size += sizeof(float);
				AddSimpleData<decltype(val)>(element.Name(), val, internal_metadata);
			}
			else if (0 == strcmp(attribute.Value(), "STRING"))
			{
				char tempBuffer[2048] = { '\0' };
				size_t strSize = static_cast<size_t>(element.QueryAttribute("size").ValueAsInt(0));
                STRCPY(tempBuffer, sizeof(tempBuffer), element.Value());

				std::string tempString(tempBuffer);
				AutoExpandEnvironmentVariables(tempString);

                memcpy(o_buffer, tempString.c_str(), strSize);
				o_buffer_size += strSize;
				internal_metadata.String(element.Name(), strSize, tempString);
			}
			else if (0 == strcmp(attribute.Value(), "BOOL") || 0 == strcmp(attribute.Value(), "BOOLEAN"))
			{
				bool val = element.QueryChild((size_t)0).ValueAsBool(true);
				memcpy(o_buffer, &val, sizeof(bool));
				o_buffer_size += sizeof(bool);
				AddSimpleData<decltype(val)>(element.Name(), val, internal_metadata);
			}
			else
			{
				LOG_INFO(CFG_LOGGER) << "failed to parse xml file: " <<"for attribute"<< attribute.Value();
				throw std::runtime_error("failed to parse xml file");
			}

			return true;
		}

		///-------------------------------------------------------------------
		/// @brief	Getting the xml root element and iterate until reading all the tree, 
		/// 		use LoadElement as the helper for reading one element 
		///
		/// @date	22/05/2018
		///
		/// @exception	std::runtime_error	Raised when a runtime error condition
		/// 								occurs.
		///
		/// @return	True if it succeeds, false if it fails.
		///-------------------------------------------------------------------
		virtual bool Load() override
		{
			LOG_FUNC(CFG_LOGGER);

			try
			{
				for (auto itr : m_fileSettingsMap)
				{
					std::string root = "/root";
					Files::XmlElement db_node = itr.second.QueryElement(root.c_str());

					if (db_node.Empty() == true)
                        throw std::runtime_error("XMLLoader - Cannot find 'root'");

					Files::XmlElement node;					
					int nCount(0);
					for (auto& elem : db_node.Children())
					{
						char buffer[2048] = {};
						size_t bufSize = 0;

						if (elem.IsComment())
							continue;

						auto indexAttr = elem.QueryAttribute("index");
						auto protoAttr = elem.QueryAttribute("prototype");	// optional
						nCount = indexAttr.ValueAsInt(0);
						Parsers::BinaryMetaDataBuilder metadata;	// empty metadata
						Parsers::BinaryMetadataStore store;	

						const char* typeName = protoAttr.Empty() ? elem.Name() : protoAttr.Value();

						// If metadata is not found create one
						if (store.Metadata(typeName).Empty())
							metadata = Parsers::BinaryMetaDataBuilder::Create();
						
						// In case metadata is found 'LoadElement' will get an empty one so it will not
						// mess with the existing one.
						if (LoadElement(elem, buffer, bufSize, metadata) == false)
							return false;
						
						// LoadElement succeeded. Make sure metadata is referencing to the right one
						if(metadata.Empty())
							metadata = store.Metadata(typeName);
						else
							metadata.Namely(typeName);

						if (itr.first == eFILE_HIERARCHY::FACTORY_SETTINGS)
						{
							// The following if statement means that indexes in the XML must appear in order.
							// Each time m_numberOfRows should match nCount. If not there is a problem in the 
							// elements indexing in the XML
							if (nCount != m_numberOfRows)
							{
								LOG_INFO(CFG_LOGGER) << "XML is not parsed correctly, missing index " << LOG_TO_STRING(m_numberOfRows);
								throw std::runtime_error("Error in XML file");
							}

							m_numberOfRows++;
						}
						else
						{
							// Relevant for DEVELOPER/USER SETTINGS. User may add additional rows to his XML. 
							// Loader must be ready to add rows. Like in the previous case (FACTORY SETTINGS)
							// if nCount equals m_numberOfRows then this is a new line and I need to increment
							// m_numOfRows. If nCount is smaller its OK because I'm reading rows that are already
							// set in FACTORY SETTINGS. If nCount is greater than m_numberOfRows then indexing
							// in the user XML are not ordered and Loader will throw exception
							
							if (nCount > m_numberOfRows || nCount < 0)
							{
								LOG_INFO(CFG_LOGGER) << "XML is not parsed correctly, missing index " << LOG_TO_STRING(m_numberOfRows);
								throw std::runtime_error("Error in XML file");
							}
							else if (nCount == m_numberOfRows)
								m_numberOfRows++;
						}

						Database::Row row;
						if (DB()[DB_INDEX()].TryGet(nCount, row) == false)
						{
							Database::RowInfo rowInfo;
							STRCPY(rowInfo.name, sizeof(rowInfo.name), elem.Name());
							STRCPY(rowInfo.type_name, sizeof(rowInfo.type_name), typeName);
							DB()[DB_INDEX()].AddRow(nCount, bufSize, rowInfo, metadata);
							if (m_verbose == true)
								Core::Console::ColorPrint(Core::Console::Colors::YELLOW, "Metadata for row : %s\n", metadata.ToJson());
						}
						else
						{
							// Safety check
							if (row.DataSize() < bufSize)
							{
								if (m_verbose == true)
									Core::Console::ColorPrint(Core::Console::Colors::GREEN, "XML Row Already exist but different size \n requested size = %d, row size = %d", bufSize, row.DataSize());

								throw std::runtime_error("Row id with different size");
							}

							LOG_DEBUG(CFG_LOGGER) << "XML Row Name" << row.Info().name << " Already exist";

							if (m_verbose == true)
								Core::Console::ColorPrint(Core::Console::Colors::YELLOW, "XML ParamStore Row Name : %s Already exist\n", row.Info().name);
						}

						DB()[DB_INDEX()][nCount].Write(buffer, bufSize, true, 0);
					}
				}
			}
			catch (const std::exception& e)
			{
				std::string str(e.what());

				LOG_INFO(CFG_LOGGER) << "Failed to load file configuration";
				throw std::runtime_error(e.what());
			}

			if (m_fileSettingsMap.end() != m_fileSettingsMap.find(USER_SETTINGS))
			{
				for (auto index = 0; index < m_numberOfRows; ++index)
				{
					DB()[DB_INDEX()][index].Subscribe([this, index](const Database::RowData& data)
					{
						Write(index, data);
					});
				}
			}

			return true;
		}

		///-------------------------------------------------------------------
		/// @brief	Writes one element back to a file, getting one element and write it and its child 
		///
		/// @date	22/05/2018
		///
		/// @exception	std::runtime_error	Raised when a runtime error condition
		/// 								occurs.
		///
		/// @param 		   	element		 	The xml DOM element.
		/// @param 		   	o_buffer	 	The buffer is holding the values that were reading from the DB.
		/// @param [in,out]	o_buffer_size	Size of the buffer.
		///
		/// @return	True if it succeeds, false if it fails.
		///-------------------------------------------------------------------
		virtual bool WriteElement(Files::XmlElement& element, const char *o_buffer, size_t& o_buffer_size)
		{
			auto attribute = element.QueryAttribute("type");

			if (0 == strcmp(attribute.Value(), "STRUCT"))
			{
				for (auto& child : element.Children())
				{
					
					if (0 == strcmp(child.QueryAttribute("type").Value(), "STRUCT"))
					{
						size_t o_bs = 0;
						WriteElement(child, o_buffer + o_buffer_size, o_bs);
						o_buffer_size += o_bs;
					}
					else
					{
						WriteElement(child, o_buffer + o_buffer_size, o_buffer_size);
					}

				}
			}
			else if (0 == strcmp(attribute.Value(), "INT"))
			{
				int val = 0;
				o_buffer_size += sizeof(int);
				memcpy(&val, o_buffer, sizeof(int));
				element.Value(val);
			}
			else if (0 == strcmp(attribute.Value(), "INT8_T") || 0 == strcmp(attribute.Value(), "CHAR"))
			{
				int8_t val = 0;
				o_buffer_size += sizeof(int8_t);
				memcpy(&val, o_buffer, sizeof(int8_t));
				element.Value(static_cast<int32_t>(val));
			}
			else if (0 == strcmp(attribute.Value(), "INT16_T"))
			{
				int16_t val = 0;
				o_buffer_size += sizeof(int16_t);
				memcpy(&val, o_buffer, sizeof(int16_t));
				element.Value(static_cast<int32_t>(val));
			}
			else if (0 == strcmp(attribute.Value(), "INT32_T"))
			{
				int32_t val = 0;
				o_buffer_size += sizeof(int32_t);
				memcpy(&val, o_buffer, sizeof(int32_t));
				element.Value(val);
			}
			else if (0 == strcmp(attribute.Value(), "INT64_T"))
			{
				int64_t val = 0;
				o_buffer_size += sizeof(int64_t);
				memcpy(&val, o_buffer, sizeof(int64_t));
				element.Value(val);
			}
			else if (0 == strcmp(attribute.Value(), "UINT8_T"))
			{
				uint8_t val = 0;
				o_buffer_size += sizeof(uint8_t);
				memcpy(&val, o_buffer, sizeof(uint8_t));
				element.Value(static_cast<uint32_t>(val));
			}
			else if (0 == strcmp(attribute.Value(), "UINT16_T"))
			{
				uint16_t val = 0;
				o_buffer_size += sizeof(uint16_t);
				memcpy(&val, o_buffer, sizeof(uint16_t));
				element.Value(static_cast<uint32_t>(val));
			}
			else if (0 == strcmp(attribute.Value(), "UINT32_T"))
			{
				uint32_t val = 0;
				o_buffer_size += sizeof(uint32_t);
				memcpy(&val, o_buffer, sizeof(uint32_t));
				element.Value(val);
			}
			else if (0 == strcmp(attribute.Value(), "UINT64_T"))
			{
				uint64_t val = 0;
				o_buffer_size += sizeof(uint64_t);
				memcpy(&val, o_buffer, sizeof(uint64_t));
				element.Value(val);
			}
			else if (0 == strcmp(attribute.Value(), "DOUBLE"))
			{
				double val = 0;
				o_buffer_size += sizeof(double);
				memcpy(&val, o_buffer, sizeof(double));
				element.Value(val);
			}
			else if (0 == strcmp(attribute.Value(), "FLOAT"))
			{
				float val = 0;
				o_buffer_size += sizeof(float);
				memcpy(&val, o_buffer, sizeof(float));
				element.Value(val);
			}
			else if (0 == strcmp(attribute.Value(), "STRING"))
			{
				char buff[256] = {};
                o_buffer_size += static_cast<size_t>(element.QueryAttribute("size").ValueAsInt(0));
                memcpy(buff, o_buffer, static_cast<size_t>(element.QueryAttribute("size").ValueAsInt(0)));

				element.Value(buff);
			}
			else if (0 == strcmp(attribute.Value(), "BOOL") || 0 == strcmp(attribute.Value(), "BOOLEAN"))
			{
				bool val = 0;
				o_buffer_size += sizeof(bool);
				memcpy(&val, o_buffer, sizeof(bool));
				element.Value(val);
			}
			else
			{
				LOG_INFO(CFG_LOGGER) << "failed to update xml file unrecognized element type " << LOG_TO_STRING(attribute.Value());
				throw std::runtime_error("failed to parse xml file");
			}

			return true;
		}

		///-------------------------------------------------------------------
		/// @brief	Adds a new element to the xml file, getting an xml DOM element and write it and its childes 
		///
		/// @date	22/05/2018
		///
		/// @exception	std::runtime_error	Raised when a runtime error condition
		/// 								occurs.
		///
		/// @param [in,out]	factoryElement	The element structure as exist in the main file FACTORY_SETTING
		/// @param 		   	o_buffer	  	The values as were taken from the DB
		/// @param [in,out]	o_buffer_size 	Size of the buffer.
		/// @param [in,out]	o_userElement 	The element that need to be written to the USER_SETTING file 
		///
		/// @return	True if it succeeds, false if it fails.
		///-------------------------------------------------------------------
		virtual bool AddElement(Files::XmlElement& factoryElement, const char *o_buffer, size_t& o_buffer_size, Files::XmlElement &o_userElement)
		{
			char childBuffer[2048] = {};
			auto attribute = factoryElement.QueryAttribute("type");

			if (0 == strcmp(attribute.Value(), "STRUCT"))
			{
				o_userElement.AddChild(factoryElement.Name(), nullptr);
				o_userElement.QueryChild(factoryElement.Name()).AddAttribute("index", factoryElement.QueryAttribute("index").Value());
				o_userElement.QueryChild(factoryElement.Name()).AddAttribute("type", factoryElement.QueryAttribute("type").Value());

				for (auto& child : factoryElement.Children())
				{
					auto subAttribute = child.QueryAttribute("type");
					if (0 == strcmp(child.QueryAttribute("type").Value(), "STRUCT"))
					{
						size_t o_bs = 0;

                        auto  userElementChild = o_userElement.QueryChild(factoryElement.Name());
                        AddElement(child, o_buffer + o_buffer_size, o_bs, userElementChild);
						o_buffer_size += o_bs;
					}
					else
					{
                        auto  userElementChild = o_userElement.QueryChild(factoryElement.Name());
                        AddElement(child, o_buffer + o_buffer_size, o_buffer_size, userElementChild);
					}
				}
			}
			else // POD
			{
				if (0 == strcmp(attribute.Value(), "INT"))
				{
					int val = 0;
					o_buffer_size += sizeof(int);
					memcpy(&val, o_buffer, sizeof(int));

#ifdef _WIN32
					sprintf_s(childBuffer, 2048, "%d", val);
#else
					sprintf(childBuffer, "%d", val);
#endif
				}
				else if (0 == strcmp(attribute.Value(), "INT64_T"))
				{
					int64_t val = 0;
					o_buffer_size += sizeof(int64_t);
					memcpy(&val, o_buffer, sizeof(int64_t));

#ifdef _WIN32
					sprintf_s(childBuffer, sizeof(int64_t), "%f", (double)val);
#else
    #ifdef __APPLE__
                    sprintf(childBuffer, "%lld", val);
    #else
                    sprintf(childBuffer, "%lld", (long long int)val);
    #endif
#endif
				}
				else if (0 == strcmp(attribute.Value(), "INT32_T"))
				{
					int32_t val = 0;
					o_buffer_size += sizeof(int32_t);
					memcpy(&val, o_buffer, sizeof(int32_t));

#ifdef _WIN32
					sprintf_s(childBuffer, sizeof(int32_t), "%d", val);
#else
					sprintf(childBuffer, "%d", val);
#endif
				}
				else if (0 == strcmp(attribute.Value(), "DOUBLE"))
				{
					double val = 0;
					o_buffer_size += sizeof(double);
					memcpy(&val, o_buffer, sizeof(double));

#ifdef _WIN32
					sprintf_s(childBuffer, sizeof(double),"%lf", val);
#else
					sprintf(childBuffer, "%lf", val);
#endif					
				}
				else if (0 == strcmp(attribute.Value(), "FLOAT"))
				{
					float val = 0;
					o_buffer_size += sizeof(float);
					memcpy(&val, o_buffer, sizeof(float));

#ifdef _WIN32
					sprintf_s(childBuffer, sizeof(float), "%f", val);
#else
					sprintf(childBuffer, "%f", val);
#endif					
				}
				else if (0 == strcmp(attribute.Value(), "STRING"))
				{
					char val[2048] = {};
                    o_buffer_size += static_cast<size_t>(factoryElement.QueryAttribute("size").ValueAsInt(0));
                    memcpy(&val, o_buffer, static_cast<size_t>(factoryElement.QueryAttribute("size").ValueAsInt(0)));

#ifdef _WIN32
					sprintf_s(childBuffer, 2048,"%s", val);
#else
					sprintf(childBuffer, "%s", val);
#endif					
				}
				else if (0 == strcmp(attribute.Value(), "BOOL") || 0 == strcmp(attribute.Value(), "BOOLEAN"))
				{
					bool val = 0;
					o_buffer_size += sizeof(bool);
					memcpy(&val, o_buffer, sizeof(bool));

#ifdef _WIN32
					sprintf_s(childBuffer, sizeof(bool),"%d", val);
#else
					sprintf(childBuffer, "%d", val);
#endif					
				}
				else
				{
					LOG_INFO(CFG_LOGGER) << "failed to update xml file unrecognized element type " << LOG_TO_STRING(factoryElement.Value());
					throw std::runtime_error("failed to parse xml file");
				}

				o_userElement.AddChild(factoryElement.Name(), childBuffer);
				o_userElement.QueryChild(factoryElement.Name()).AddAttribute("index", factoryElement.QueryAttribute("index").Value());
				o_userElement.QueryChild(factoryElement.Name()).AddAttribute("type", factoryElement.QueryAttribute("type").Value());

				if (0 == strcmp(attribute.Value(), "STRING"))
				{
					o_userElement.QueryChild(factoryElement.Name()).AddAttribute("size", factoryElement.QueryAttribute("size").Value());
				}
			}

			return  true;
		}

		///-------------------------------------------------------------------
		/// @brief	Writes an element to the USER_SETTING file, it use AddElement and WriteElement as an helper
		/// 		it read the structure from the FACTORY_SETTING and write to USER_SETTING 
		///
		/// @date	22/05/2018
		///
		/// @param	index	Zero-based index of the.
		/// @param	data 	The data as taken from the DB
		///
		/// @return	True if it succeeds, false if it fails.
		///-------------------------------------------------------------------
		virtual bool Write(int index, const Database::RowData& data) override
		{
			LOG_FUNC(CFG_LOGGER);

			LOG_INFO(CFG_LOGGER) << "Receive an update on XML file , index " << LOG_TO_STRING(index);

			int childExist = false;
			std::string root = "/root";
			Files::XmlElement db_node = m_fileSettingsMap[USER_SETTINGS].QueryElement(root.c_str());

			Files::XmlElement child;
			for (auto& elem : db_node.Children())
			{
				if (elem.IsComment())
					continue;	// skip xml comment

				if (index == elem.QueryAttribute("index").ValueAsInt(0))
				{
					child = elem;
					childExist = true;
					break;
				}
			}

			const char* buff = static_cast<const char*>(data.Buffer());
			size_t buffSize = 0;

			if (childExist)
			{
				WriteElement(child, buff, buffSize);
			}
			else // child not exist create one 
			{
				Files::XmlElement factory_setting_db_node = m_fileSettingsMap[FACTORY_SETTINGS].QueryElement(root.c_str());
                auto factorySettingChild = factory_setting_db_node.QueryChild(static_cast<size_t>(index));

				AddElement(factorySettingChild, buff, buffSize, db_node);
			}

			return true;
		}

		///-------------------------------------------------------------------
		/// @brief	Print single xml element and its value to the logger 
		/// 		Recursive function that run on the elements and its childes 
		///
		/// @date	22/05/2018
		///
		/// @param 		   	element		 	The xml DOM element.
		/// @param [in,out]	buffer		 	If non-null, hold the values of the element 
		/// @param [in,out]	o_buffer_size	Size of the buffer.
		/// @param [in,out]	o_str		 	The string that will eventfully print to the file 
		///
		/// @return	True if it succeeds, false if it fails.
		///-------------------------------------------------------------------
		virtual bool PrintElement(Files::XmlElement element, char *buffer, size_t &o_buffer_size, std::stringstream& o_str)
		{
			auto attribute = element.QueryAttribute("type");
			o_str << "\n" << element.Name();

			if (0 == strcmp(attribute.Value(), "STRUCT"))
			{
				for (auto& child : element.Children())
				{
					if (0 == strcmp(child.QueryAttribute("type").Value(), "STRUCT"))
					{
						size_t o_bSize = 0;
						PrintElement(child, buffer + o_buffer_size, o_bSize, o_str);
						o_buffer_size += o_bSize;
					}
					else
					{
						PrintElement(child, buffer + o_buffer_size, o_buffer_size, o_str);
					}
				}
			}

			else if (0 == strcmp(attribute.Value(), "INT"))
			{
				o_str << " = " << (*(int *)(buffer));
				o_buffer_size += sizeof(int);
			}
			else if (0 == strcmp(attribute.Value(), "INT64_T"))
			{
				o_str << " = " << (*(int64_t *)(buffer));
				o_buffer_size += sizeof(int);
			}
			else if (0 == strcmp(attribute.Value(), "INT32_T"))
			{
				o_str << " = " << (*(int32_t *)(buffer));
				o_buffer_size += sizeof(int);
			}
			else if (0 == strcmp(attribute.Value(), "UINT32_T"))
			{
				o_str << " = " << (*(uint32_t *)(buffer));
				o_buffer_size += sizeof(int);
			}
			else if (0 == strcmp(attribute.Value(), "INT16_T"))
			{
				o_str << " = " << (*(int16_t *)(buffer));
				o_buffer_size += sizeof(int);
			}
			else if (0 == strcmp(attribute.Value(), "UINT16_T"))
			{
				o_str << " = " << (*(uint16_t *)(buffer));
				o_buffer_size += sizeof(int);
			}
			else if (0 == strcmp(attribute.Value(), "DOUBLE"))
			{
				o_str << " = " << (*(double *)(buffer));
				o_buffer_size += sizeof(double);
			}
			else if (0 == strcmp(attribute.Value(), "FLOAT"))
			{
				o_str << " = " << (*(float *)(buffer));
				o_buffer_size += sizeof(float);
			}
			else if (0 == strcmp(attribute.Value(), "STRING"))
			{
				std::string s(buffer);
				o_str << " = " << s;

				auto sizeAttribute = element.QueryAttribute("size");
                o_buffer_size += static_cast<size_t>(sizeAttribute.ValueAsInt(0));
			}
			else if (0 == strcmp(attribute.Value(), "BOOL") || 0 == strcmp(attribute.Value(), "BOOLEAN"))
			{
				o_str << " = " << (*(bool *)(buffer));
				o_buffer_size += sizeof(bool);

			}
			else
			{
				LOG_INFO(CFG_LOGGER) << "failed to print xml file unrecognized element type " << LOG_TO_STRING(attribute.Value());
			}

			return true;
		}

	public:

		///-------------------------------------------------------------------
		/// @brief	Prints this object to the logger, use PrintElement as an helper for printing a single elements 
		///
		/// @date	22/05/2018
		///
		/// @exception	std::runtime_error	Raised when a runtime error condition
		/// 								occurs.
		///
		/// @return	True if it succeeds, false if it fails.
		///-------------------------------------------------------------------
		virtual bool Print() override
		{
			LOG_FUNC(CFG_LOGGER);

			try
			{
				std::stringstream str;
				std::string root = "/root";

				Files::XmlElement db_node = m_fileSettingsMap[FACTORY_SETTINGS].QueryElement(root.c_str());
				if (db_node.Empty() == true)
					throw;

				int nCount(0);
				char buffer[2048] = {};

				for (auto& elem : db_node.Children())
				{
					str.str("");
					if (elem.IsComment())
						continue;

					auto indexAttribute = elem.QueryAttribute("index");
					nCount = indexAttribute.ValueAsInt(0);

					DB()[DB_INDEX()][nCount].Read(buffer);

					size_t o_buffer_size = 0;
					PrintElement(elem, buffer, o_buffer_size, str);

					str << "\n--------------------------------------------------";
					LOG_INFO(CFG_LOGGER) << LOG_TO_STRING(str.str());
				}
			}
			catch (const std::exception& e)
			{
				std::string str(e.what());

				LOG_INFO(CFG_LOGGER) << "Failed to print configuration parameters , error in the XML file";
				throw std::runtime_error(e.what());
			}

			return true;
		}

		///-------------------------------------------------------------------
		/// @brief	Constructor
		///
		/// @date	22/05/2018
		///
		/// @param [in,out]	i_factorySettingsMap	Zero-based index of the factory
		/// 										settings map.
		/// @param 		   	i_dbIndex				Zero-based index of the database
		/// 										index.
		/// @param 		   	dataset					The dataset.
		/// @param 		   	context					(Optional) The context.
		///-------------------------------------------------------------------
		XmlStoreDB(const Database::AnyKey i_dbIndex,
			const Database::DataSet& dataset,
			std::string i_factory_setting_path,
			std::string i_developer_setting_path,
			std::string i_user_setting_path,
			Files::FilesHandler& i_fileManager,
			std::string i_appName,
			std::string i_groupAppName,
			const Utils::Context& context = nullptr,
			bool verbose = false) :
			StoreDB(dataset, i_dbIndex, i_factory_setting_path, i_developer_setting_path , i_user_setting_path,  i_fileManager, i_appName, i_groupAppName, context),
			m_verbose(verbose)
		{
			if (FILES_MAPS().end() != FILES_MAPS().find(FACTORY_SETTINGS))
			{			
				if (FILES_MAPS()[FACTORY_SETTINGS].find(".xml") == std::string::npos)
				{
					char helperBuffer[2048] = {};
#ifdef _WIN32
					sprintf_s(helperBuffer, 2048, "%s/%s.xml",FILES_MAPS()[FACTORY_SETTINGS].c_str(), i_appName.c_str());
#else
					sprintf(helperBuffer, "%s/%s.xml",FILES_MAPS()[FACTORY_SETTINGS].c_str(), i_appName.c_str());
#endif
					FILES_MAPS()[FACTORY_SETTINGS] = helperBuffer;
				}

				//remove duplicate slashes
				replaceStringInPlace(FILES_MAPS()[FACTORY_SETTINGS], "//", "/");

				m_fileSettingsMap.insert(std::pair<eFILE_HIERARCHY, Files::XmlFile>(FACTORY_SETTINGS, Files::XmlFile::Create(FILES_MAPS()[FACTORY_SETTINGS].c_str())));

				if (m_verbose == true)
					Core::Console::ColorPrint(Core::Console::Colors::GREEN, "Open Factory Setting file: %s - Succeeded\n", FILES_MAPS()[FACTORY_SETTINGS].c_str());
			}
			else
			{
				LOG_INFO(CFG_LOGGER) << "Failed to load xml file configuration, FACTORY_SETTING file is must ";
				throw std::runtime_error("Failed to load xml file configuration, FACTORY_SETTING file is must ");
			}

			if (FILES_MAPS().end() != FILES_MAPS().find(USER_SETTINGS))
			{
				if (FILES_MAPS()[USER_SETTINGS].find(".xml") == std::string::npos)
				{
					char helperBuffer[2048] = {};
#ifdef _WIN32
					sprintf_s(helperBuffer, 2048, "%s/%s.xml",FILES_MAPS()[USER_SETTINGS].c_str(), i_appName.c_str());
#else
					sprintf(helperBuffer, "%s/%s.xml", FILES_MAPS()[USER_SETTINGS].c_str(), i_appName.c_str());
#endif
					FILES_MAPS()[USER_SETTINGS] = helperBuffer;
				}

				Files::XmlFile file;
				//remove duplicate slashes
				replaceStringInPlace(FILES_MAPS()[USER_SETTINGS], "//", "/");

				// Create xml file.
				file = CreateUserXmlFile(FILES_MAPS()[FACTORY_SETTINGS].c_str(), FILES_MAPS()[USER_SETTINGS].c_str());

				if (file.Empty() == false)
				{ //only if files was created insert it to the file map
					m_fileSettingsMap.insert(std::pair<eFILE_HIERARCHY, Files::XmlFile>(USER_SETTINGS, file));
					FILES_MANAGER().SubscribeFile(m_fileSettingsMap[USER_SETTINGS]);
				}
			}

			this->Load();
			this->Print();
		}
	};
}
