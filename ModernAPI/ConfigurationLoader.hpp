#pragma once
#include <Core.hpp>
#include <Files.hpp>
#include <Database.hpp>

#include <sstream>
#include <map>
#include <string>
#include <regex>

#define MAX_NESTING_DEPTH 10


namespace ConfigurationLoader
{
	static Logging::Logger CFG_LOGGER = Core::Framework::CreateLogger("StoreDB", Logging::Severity::DEBUG);

#ifdef _WIN32
    static constexpr char USER_PATH[] = "%PROGRAMDATA%/ElbitSystemsLtd/";
#else
    static constexpr char USER_PATH[] = "~/.config/ElbitSystemsLtd/";
#endif

	class StoreDB : public Database::Dispatcher
	{
	public:
		/// @brief	Values that represent file hierarchies
		enum eFILE_HIERARCHY
		{
			FACTORY_SETTINGS = 0,
			DEVELOPER_SETTINGS,
			USER_SETTINGS,

			FILE_SETTINGS_MAX
		};

	private:
		/// @brief	The dataset
		Database::DataSet m_dataset;		
		/// @brief	map that hold the path of the configuration files 
		std::map<eFILE_HIERARCHY, std::string> m_filePathMap;
		/// @brief	file manager
		Files::FilesHandler m_fileManager;
        /// @brief	Zero-based index of the database
        Database::AnyKey m_dbIndex;

	protected:
        /// @brief	Number of rows we push to the DB
        int m_numberOfRows = 0;
		
		void replaceStringInPlace(std::string& subject, const std::string& search,
			const std::string& replace) {
			size_t pos = 0;
			while ((pos = subject.find(search, pos)) != std::string::npos) {
				subject.replace(pos, search.length(), replace);
				pos += replace.length();
			}
		}


		///-------------------------------------------------------------------
		/// @brief	Gets the m_filePathMap
		///
		/// @date	22/05/2018
		///
		/// @return	A reference to std::map<eFILE_HIERARCHY, std::string>
		///-------------------------------------------------------------------
		std::map<eFILE_HIERARCHY, std::string>& FILES_MAPS()
		{
			return m_filePathMap;
		}

		///-------------------------------------------------------------------
		/// @brief	Gets the m_fileManager
		///
		/// @date	22/05/2018
		///
		/// @return	A reference m_fileManager
		///-------------------------------------------------------------------
		Files::FilesHandler& FILES_MANAGER()
		{
			return m_fileManager;
		}

		///-------------------------------------------------------------------
		/// @brief	Gets the database
		///
		/// @date	22/05/2018
		///
		/// @return	A reference to a Database::DataSet.
		///-------------------------------------------------------------------
		Database::DataSet& DB()
		{
			return m_dataset;
		}

		///-------------------------------------------------------------------
		/// @brief	Database index 
		///
		/// @date	22/05/2018
		///
		/// @return	A reference to a Database::AnyKey (MonitorDBIndex).
		///-------------------------------------------------------------------
		Database::AnyKey& DB_INDEX()
		{
			return m_dbIndex;
		}		

		///-------------------------------------------------------------------
		/// @brief	Interface for the Load function each inheritance will create its own loader
		///
		/// @date	22/05/2018
		///
		/// @return	True if it succeeds, false if it fails.
		///-------------------------------------------------------------------
		virtual bool Load() = 0;

		///-------------------------------------------------------------------
		/// @brief	Prints the parameters after they were insert to the DB
		///
		/// @date	22/05/2018
		///
		/// @return	True if it succeeds, false if it fails.
		///-------------------------------------------------------------------
		virtual bool Print() = 0;

		///-------------------------------------------------------------------
		/// @brief	Writes data back to the file , file can be ini, xml or JSON
		///
		/// @date	22/05/2018
		///
		/// @param	index	Zero-based index of the.
		/// @param	data 	The data.
		///
		/// @return	True if it succeeds, false if it fails.
		///-------------------------------------------------------------------
		virtual bool Write(int index, const Database::RowData& data) = 0;

	public:

		///---------------------------------------------------------------
		/// @brief	Constructor
		///
		/// @date	22/05/2018
		///
		/// @param	Database::DataSet&   reference to the dataset the were build in the main function
		/// @param	Database::AnyKey (MonitorDBIndex)   enum for the database that was assign to this storeDB
		/// @param  std::map<eFILE_HIERARCHY, std::string> hold the path to all the configuration files 
		/// @param  Utils::Context&    is to create new thread or use the context that was given from the main?
		///---------------------------------------------------------------
		StoreDB(const Database::DataSet& dataset,
			Database::AnyKey i_dbIndex,
			std::string i_factory_setting_path,
			std::string i_developer_setting_path,
			std::string i_user_setting_path,
			Files::FilesHandler& i_fileManager,
			std::string i_appName,
			std::string i_groupAppName,
			const Utils::Context& context = nullptr) :
			Database::Dispatcher((context.Empty() == false) ? context : Utils::Context("StoreDB")),
			m_dataset(dataset),
			m_fileManager(i_fileManager),			
            m_dbIndex(i_dbIndex),
            m_numberOfRows(0)
		{
			//Factory Path handling
			//If empty use the default which is the Configuration folder under the running folder 
			if (i_factory_setting_path == "" || i_factory_setting_path == "0" || i_factory_setting_path.empty())
			{
				Application::MainApp mainAapp;
				i_factory_setting_path = mainAapp.ExecutionPath() + "/Configuration/";
				m_filePathMap[FACTORY_SETTINGS] = i_factory_setting_path;
			}
			//if the path include a file name get the file name
			else if (i_factory_setting_path.find(".ini") != std::string::npos || i_factory_setting_path.find(".xml") != std::string::npos)
			{
					m_filePathMap[FACTORY_SETTINGS] = i_factory_setting_path;
			}
			//if does not include files name but command line indicate path - build a path from the i_factory_setting_path/i_groupAppName/i_appName
			else
			{
				if (false == i_groupAppName.empty())
					m_filePathMap[FACTORY_SETTINGS] = i_factory_setting_path + "/" + i_groupAppName;
				else
					m_filePathMap[FACTORY_SETTINGS] = i_factory_setting_path ;
			}
			
			if (i_user_setting_path == "" || i_user_setting_path == "0" || i_user_setting_path.empty())
			{
				if (false == i_groupAppName.empty())
					m_filePathMap[USER_SETTINGS] = USER_PATH + i_groupAppName +"/";
				else
					m_filePathMap[USER_SETTINGS] = USER_PATH ;
			}
			else if (i_user_setting_path.find(".ini") != std::string::npos || i_user_setting_path.find(".xml") != std::string::npos)
			{
				m_filePathMap[USER_SETTINGS] = i_user_setting_path;
			}
			else
			{
				m_filePathMap[USER_SETTINGS] = i_user_setting_path + "/" + i_groupAppName + "/";
			}

			//check if there is environment variable and load them			
			for (auto &itr : m_filePathMap)
				AutoExpandEnvironmentVariables(itr.second);
		}

		///-------------------------------------------------------------------
		/// @brief	Starts the dispatcher 
		///
		/// @date	22/05/2018
		///-------------------------------------------------------------------
		virtual void Start() {}


	};	
}
