#pragma once
#include "ConfigurationLoader.hpp"

#include <Files.hpp>
#include <Factories.hpp>

#include <list>
#include <string>
#include <sstream>

#define MAX_LINE_SIZE 1024

namespace ConfigurationLoader
{
	class INIStoreDB : public StoreDB
	{
	private:
		/// @brief	The file settings map
		std::map<eFILE_HIERARCHY, Files::IniFile> m_fileSettingsMap;

    protected:
		///-------------------------------------------------------------------
		/// @brief	Getting the INI root element and iterate until reading all the tree, 
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
				int number_of_rows(0);
				std::list < Files::IniFile::Entry > FACTORY_SETTING_DATA;

				auto itr = m_fileSettingsMap.find(FACTORY_SETTINGS);
				if (itr != m_fileSettingsMap.end())
				{
					itr->second.ReadAllAsString<MAX_LINE_SIZE>(FACTORY_SETTING_DATA);
					
					for (auto itr_data : FACTORY_SETTING_DATA)
					{
						Common::CommonTypes::CHARBUFFER charBuffer;

						std::string temp = itr_data.m_value.c_str();
						AutoExpandEnvironmentVariables(temp);
#ifdef _WIN32
						sprintf_s(charBuffer.buffer, 256, "%s", temp.c_str());
#else
						sprintf(charBuffer.buffer, "%s", temp.c_str());
#endif

						Database::Row row;
						if (DB()[DB_INDEX()].TryGet(number_of_rows, row) == false)
						{
							DB()[DB_INDEX()].AddRow<Common::CommonTypes::CHARBUFFER>(number_of_rows);
							row = DB()[DB_INDEX()][number_of_rows];
						}

						//safety check
						if (row.DataSize() != sizeof(Common::CommonTypes::CHARBUFFER))
						{
							Core::Console::ColorPrint(Core::Console::Colors::GREEN, "XML ParamStore Row Name : %s Already exist with different size \n", row.Info().name);
							throw std::runtime_error("paramStore row id with different size");
						}

						LOG_DEBUG(CFG_LOGGER) << "XML ParamStore Row Name";// << row.Info().name << " Already exist";
						Core::Console::ColorPrint(Core::Console::Colors::GREEN, "XML ParamStore Row Name : %s Already exist\n", row.Info().name);
						row.Write<Common::CommonTypes::CHARBUFFER>(charBuffer, true, 0);
						number_of_rows++;
					}
				}	
				
				itr = m_fileSettingsMap.find(DEVELOPER_SETTINGS);
				if (itr != m_fileSettingsMap.end())
				{
					std::list < Files::IniFile::Entry > DATA;
					itr->second.ReadAllAsString<MAX_LINE_SIZE>(DATA);

					for (auto itr_data : DATA)
					{
						int index(0);

						for (auto itr_data_factory : FACTORY_SETTING_DATA)
						{
							if (itr_data.ParameterExist(itr_data_factory))
							{
								Common::CommonTypes::CHARBUFFER charBuffer;
								std::string temp = itr_data.m_value.c_str();
								AutoExpandEnvironmentVariables(temp);
#ifdef _WIN32
								sprintf_s(charBuffer.buffer, sizeof(decltype(charBuffer.buffer)), "%s", temp.c_str());
#else
								sprintf(charBuffer.buffer, "%s", temp.c_str());
#endif
								DB()[DB_INDEX()][index].Write<Common::CommonTypes::CHARBUFFER>(charBuffer, true, 0);
								break;
							}

							index++;
						}
					}
				}
				
				itr = m_fileSettingsMap.find(USER_SETTINGS);
				if (itr != m_fileSettingsMap.end())
				{
					std::list < Files::IniFile::Entry > DATA;
					itr->second.ReadAllAsString<MAX_LINE_SIZE>(DATA);

					for (auto itr_data : DATA)
					{
						int index(0);

						for (auto itr_data_factory : FACTORY_SETTING_DATA)
						{
							if (itr_data.ParameterExist(itr_data_factory))
							{
								Common::CommonTypes::CHARBUFFER charBuffer;
								std::string temp = itr_data.m_value.c_str();
								AutoExpandEnvironmentVariables(temp);

#ifdef _WIN32
								sprintf_s(charBuffer.buffer, 256, "%s", temp.c_str());
#else
								sprintf(charBuffer.buffer, "%s", temp.c_str());
#endif

								DB()[DB_INDEX()][index].Write<Common::CommonTypes::CHARBUFFER>(charBuffer, true, 0);
								break;
							}

							index++;
						}
					}
				}

				for (int i = 0; i < number_of_rows; ++i)
				{
					DB()[DB_INDEX()][i].Subscribe([this, i](const Database::RowData& data)
					{
						Write(i, data);
					});
				}
			}
			catch (const std::exception& e)
			{
                LOG_INFO(CFG_LOGGER) << "Failed to load INI file configuration: " << e.what();
				throw std::runtime_error(e.what());
			}

			return true;
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
			LOG_INFO(CFG_LOGGER) << "Receive an update on INI file , index " << LOG_TO_STRING(index);
			
			std::list < Files::IniFile::Entry > FACTORY_SETTING_DATA;			
			m_fileSettingsMap[FACTORY_SETTINGS].ReadAllAsString<MAX_LINE_SIZE>(FACTORY_SETTING_DATA);

			auto itr = FACTORY_SETTING_DATA.begin();
			for (int i = 0; i < index; ++i)
			{
				itr++;
			}			

			std::list < Files::IniFile::Entry > DATA;
			m_fileSettingsMap[USER_SETTINGS].ReadAllAsString<MAX_LINE_SIZE>(DATA);

			m_fileSettingsMap[USER_SETTINGS].DeleteKey(itr->m_section.c_str(), itr->m_key.c_str());

			Common::CommonTypes::CHARBUFFER buff = data.Read<Common::CommonTypes::CHARBUFFER>();
            m_fileSettingsMap[USER_SETTINGS].WriteString(itr->m_section.c_str(), itr->m_key.c_str(), buff.buffer);
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

			std::list < Files::IniFile::Entry >  FACTORY_SETTING_DATA;
			m_fileSettingsMap[FACTORY_SETTINGS].ReadAllAsString<MAX_LINE_SIZE>(FACTORY_SETTING_DATA);

			std::stringstream sstr;
			sstr << "\n-----------------------------------\nLoaded Parameters:\n";

			int index(0);
			for (auto itr : FACTORY_SETTING_DATA)
			{
				Common::CommonTypes::CHARBUFFER charBuffer = DB()[DB_INDEX()][index].Read<Common::CommonTypes::CHARBUFFER>();
				sstr << index << ". " << itr.m_section << ":" << itr.m_key << "=" << charBuffer.buffer << "\n";
				index++;
			}

            sstr << "-----------------------------------\n" << std::endl;

			LOG_INFO(CFG_LOGGER) << sstr.str().c_str();
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
		INIStoreDB(const Database::AnyKey i_dbIndex,
			const Database::DataSet& dataset,
			std::string i_factory_setting_path,
			std::string i_developer_setting_path,
			std::string i_user_setting_path,
			Files::FilesHandler& i_fileManager,
			std::string i_appName,
			std::string i_groupAppName,
			const Utils::Context& context = nullptr) :
			StoreDB(dataset, i_dbIndex, i_factory_setting_path, i_developer_setting_path, i_user_setting_path, i_fileManager, i_appName, i_groupAppName, context)
		{
			//On ini file only read the FACTORY SETTING
			if (FILES_MAPS()[FACTORY_SETTINGS].find(".ini") == std::string::npos)
			{
#ifdef _WIN32
				std::string file = FILES_MAPS()[FACTORY_SETTINGS] + "Windows" + i_appName +".ini";
#else
				std::string file = FILES_MAPS()[FACTORY_SETTINGS]  + "Linux" + i_appName + ".ini";
#endif
				m_fileSettingsMap.insert(std::pair<eFILE_HIERARCHY, Files::IniFile>(FACTORY_SETTINGS, Files::IniFile::Create(file.c_str())));
			}
			else
			{
				m_fileSettingsMap.insert(std::pair<eFILE_HIERARCHY, Files::IniFile>(FACTORY_SETTINGS, Files::IniFile::Create(FILES_MAPS()[FACTORY_SETTINGS].c_str())));
			}
			

			if (m_fileSettingsMap.find(USER_SETTINGS) != m_fileSettingsMap.end())
				i_fileManager.SubscribeFile(m_fileSettingsMap[USER_SETTINGS]);

			this->Load();
			this->Print();
		}
	};
}
