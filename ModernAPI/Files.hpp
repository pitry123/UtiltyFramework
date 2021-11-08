#pragma once
#include <files/bin_file_interface.h>
#include <files/log_file_interface.h>
#include <files/ini_file_interface.h>
#include <files/xml_file_interface.h>
#include <files/filessystem.h>
#include <files/files_handler.h>
#include <utils/ref_count_ptr.hpp>
#include <utils/files.hpp>
#include <Common.hpp>

#include <stdexcept>
#include <functional>
#include <limits>
#include <list>

namespace Files
{
    // To commonly wrap flush method
    class FileBase : public Common::CoreObjectWrapper<core::files::file_access_interface>
    {
    public:
        FileBase() {} // Empty object
        FileBase(core::files::file_access_interface* file) : Common::CoreObjectWrapper<core::files::file_access_interface>(file) {}

        virtual ~FileBase() = default;

        bool Flush()
        {
            this->ThrowOnEmpty("FileBase");
            return this->m_core_object->flush();
        }
    };

    // To wrap bin_file API
    class BinFile : public FileBase
    {    
    private:
        BinFile(files::bin_file_interface* binFile) : FileBase(binFile)
        {
            // Nothing to do for now...
        }

		inline files::bin_file_interface* core_bin_file() const
		{
			if (Empty() == true)
				throw std::runtime_error("Empty BinFile");

			return static_cast<files::bin_file_interface*>(static_cast<core::files::file_access_interface*>(m_core_object));
		}

    public:
        BinFile() {} // Empty object

        void WriteFile(const void* buffer, size_t size)
        {
            core_bin_file()->write_file(buffer, size);
        }

        template <typename T>
        void WriteFile(const T& val)
        {
            WriteFile(&val, sizeof(val));
        }

        size_t ReadFile(void* buffer, size_t size) const
        {            
            return core_bin_file()->read_file(buffer, size);
        }

        template <typename T>
        void ReadFile(T& val)
        {
            ReadFile(&val, sizeof(val));
        }

        // Wrapper for factory method
        static BinFile Create(const char* filePath, size_t maxFileSize)
        {
            utils::ref_count_ptr<files::bin_file_interface> instance;
            if (files::bin_file_interface::create(filePath, maxFileSize, &instance) == false)
                throw std::runtime_error("Failed to create BinFile");

            return BinFile(instance);
        }
    };

    // To wrap log_file API
    class LogFile : public FileBase
    {
    private:
        LogFile(files::log_file_interface* logFile) : FileBase(logFile)
        {
            // Nothing to do for now...
        }

		inline files::log_file_interface* core_log_file() const
		{
			if (Empty() == true)
				throw std::runtime_error("Empty LogFile");

			return static_cast<files::log_file_interface*>(static_cast<core::files::file_access_interface*>(m_core_object));
		}

    public:
        LogFile() {} // Empty object

        size_t Write(const char* buffer, size_t size)
        {  
            return core_log_file()->write(buffer, size);
        }

        size_t WriteLine(const char* buffer, size_t size)
        {       
            return core_log_file()->write_line(buffer, size);
        }

        size_t ReadFile(char* buffer, size_t size, bool skipHeader) const
        {
            return core_log_file()->read_file(buffer, size, skipHeader);
        }

        // Wrapper for factory method
        static LogFile Create(const char* filePath,
                              bool isCyclic,
                              size_t maxBuffSize,
                              size_t maxFileSize,
                              const char* headerData,
                              uint8_t linesToSkip)
        {
            utils::ref_count_ptr<files::log_file_interface> instance;
            if (files::log_file_interface::create(filePath,
                                                  isCyclic,
                                                  maxBuffSize,
                                                  maxFileSize, 
                                                  headerData,
                                                  linesToSkip,
                                                  &instance) == false)
            {
                throw std::runtime_error("Failed to create LogFile");
            }

            return LogFile(instance);
        }
    };

	// To wrap ini_file API
	class IniFile : public FileBase
	{

	public:
		struct Entry
		{
			std::string  m_section;
			std::string  m_key;
			std::string  m_value;

			Entry() :
				m_section({}),
				m_key({}),
				m_value({})
			{
			}

			Entry(std::string i_section, std::string i_key, std::string i_value):
				m_section(i_section),
				m_key(i_key),
				m_value(i_value)
			{
			}

			Entry(const Entry& other) :
				m_section(other.m_section),
			    m_key(other.m_key),
				m_value(other.m_value)
			{
			}

			bool operator==(const Entry &other) const
			{
				if (m_section == other.m_section && m_key == other.m_key && m_value == other.m_value)
					return true;
				return false;
			}

			// assume the object holds reusable storage, such as a heap-allocated buffer mArray
			Entry& operator=(const Entry& other) // copy assignment
			{
				if (this != &other) 
				{ 
					this->m_section = other.m_section;
					this->m_key = other.m_key;
					this->m_value = other.m_value;
				}
				return *this;
			}

			bool ParameterExist(const Entry &other) const
			{
				if (m_section == other.m_section && m_key == other.m_key)
					return true;
				return false;
			}

			bool ParameterExist(std::string i_section, std::string i_key) const
			{
				if (m_section == i_section && m_key == i_key)
					return true;
				return false;
			}

			bool Empty() const
			{
				if (m_section.empty() || m_key.empty() || m_value.empty())
					return true;
				return false;
			}

			bool Assign(const char* i_section, const char* i_key, const char* i_value)
			{
				m_section.assign(i_section);
				m_key.assign(i_key);
				m_value.assign(i_value);
				
				if (m_section.empty() || m_key.empty() || m_value.empty())
					return true;
				return false;
			}
		};

	private:
		IniFile(files::ini_file_interface* logFile) : FileBase(logFile)
		{
			// Nothing to do for now...
		}

		inline files::ini_file_interface* core_ini_file() const
		{
			if (Empty() == true)
				throw std::runtime_error("Empty IniFile");

			return static_cast<files::ini_file_interface*>(static_cast<core::files::file_access_interface*>(m_core_object));
		}

	public:
		IniFile() {} // Empty object

		bool WriteBool(const char* section, const char* key, bool value)
		{
			return core_ini_file()->write_bool(section, key, value);
		}

		bool WriteInt(const char* section, const char* key, int32_t value)
		{			
			return core_ini_file()->write_int(section, key, value);
		}

		bool WriteFloat(const char* section, const char* key, float value)
		{			
			return core_ini_file()->write_float(section, key, value);
		}

		bool WriteLong(const char* section, const char* key, int64_t value)
		{
			return core_ini_file()->write_long(section, key, value);
		}

		bool WriteDouble(const char* section, const char* key, double value)
		{
			return core_ini_file()->write_double(section, key, value);
		}

		bool WriteString(const char* section, const char* key, const char* value)
		{
			return core_ini_file()->write_string(section, key, value);
		}

		bool DeleteKey(const char* section, const char* key)
		{
			return core_ini_file()->delete_key(section, key);
		}

		bool ReadBool(const char* section, const char* key, bool& value, bool def_val)
		{
			return core_ini_file()->read_bool(section, key, value, def_val);
		}

		bool ReadInt(const char* section, const char* key, int32_t& value, int32_t def_val)
		{
			return core_ini_file()->read_int(section, key, value, def_val);
		}

		bool ReadFloat(const char* section, const char* key, float& value, float def_val)
		{
			return core_ini_file()->read_float(section, key, value, def_val);
		}

		bool ReadLong(const char* section, const char* key, int64_t& value, int64_t def_val)
		{
			return core_ini_file()->read_long(section, key, value, def_val);
		}

		bool ReadDouble(const char* section, const char* key, double& value, double def_val)
		{
			return core_ini_file()->read_double(section, key, value, def_val);
		}

		bool ReadString(const char* section, const char* key, char* value, size_t& size, const char* def_val)
		{
			return core_ini_file()->read_string(section, key, value, size, def_val);
		}


		bool ReadString(size_t index, char* section, char* key, char* value, size_t& max_buffer_size)
		{
			return core_ini_file()->read_string(index, section, key, value, max_buffer_size);
		}

		template <int SIZE>
		bool ReadAllAsString(std::list<Entry>& o_data)
		{
			int index = 0;

			while(true)
			{
				size_t size(1024);
				char o_section[SIZE] = {};
				char o_key[SIZE] = {};
				char o_value[SIZE] = {};

                if (this->ReadString(static_cast<size_t>(index), o_section, o_key, o_value, size))
				{
					Entry e(o_section, o_key, o_value);
					o_data.push_back(e);
				}
				else
				{
					break;
				}

				index++;
			} 

			return true;
		}


		// Wrapper for factory method
        static IniFile Create(const char* filePath)
        {
            utils::ref_count_ptr<files::ini_file_interface> instance;
            if (files::ini_file_interface::create(filePath, &instance) == false)
            {
                throw std::runtime_error("Failed to create IniFile");
            }

            return IniFile(instance);
        }
	};

	// To commonly wrap xml nodes API
	template <typename T>
	class XmlNodeBase : public Common::CoreObjectWrapper<T>
	{
	public:
		XmlNodeBase() {} // Empty node
		XmlNodeBase(T* node) : Common::CoreObjectWrapper<T>(node) {}

		virtual ~XmlNodeBase() = default;

		const char* Name() const
		{
            this->ThrowOnEmpty("XmlNodeBase");
            return this->m_core_object->name();
		}

		const char* Value() const
		{
            this->ThrowOnEmpty("XmlNodeBase");
            return this->m_core_object->value();
		}

		bool ValueAsBool(bool defVal) const
		{
            this->ThrowOnEmpty("XmlNodeBase");
            return this->m_core_object->value_as_bool(defVal);
		}

		int32_t ValueAsInt(int32_t defVal) const
		{
            this->ThrowOnEmpty("XmlNodeBase");
            return this->m_core_object->value_as_int(defVal);
		}

		uint32_t ValueAsUInt(uint32_t defVal) const
		{
			this->ThrowOnEmpty("XmlNodeBase");
			return this->m_core_object->value_as_uint(defVal);
		}

		int64_t ValueAsLong(int64_t defVal) const
		{
            this->ThrowOnEmpty("XmlNodeBase");
            return this->m_core_object->value_as_long(defVal);
		}

		uint64_t ValueAsULong(uint64_t defVal) const
		{
			this->ThrowOnEmpty("XmlNodeBase");
			return this->m_core_object->value_as_ulong(defVal);
		}

		float ValueAsFloat(float defVal) const
		{
            this->ThrowOnEmpty("XmlNodeBase");
            return this->m_core_object->value_as_float(defVal);
		}

		double ValueAsDouble(double defVal) const
		{
            this->ThrowOnEmpty("XmlNodeBase");
            return this->m_core_object->value_as_double(defVal);
		}

		void Value(const char* Val)
		{
            this->ThrowOnEmpty("XmlNodeBase");
            this->m_core_object->value(Val);
		}

		void Value(bool Val)
		{
            this->ThrowOnEmpty("XmlNodeBase");
            this->m_core_object->value(Val);
		}

		void Value(int32_t Val)
		{
            this->ThrowOnEmpty("XmlNodeBase");
            this->m_core_object->value(Val);
		}

		void Value(uint32_t Val)
		{
            this->ThrowOnEmpty("XmlNodeBase");
            this->m_core_object->value(Val);
		}

		void Value(int64_t Val)
		{
            this->ThrowOnEmpty("XmlNodeBase");
            this->m_core_object->value(Val);
		}

		void Value(uint64_t Val)
		{
            this->ThrowOnEmpty("XmlNodeBase");
            this->m_core_object->value(Val);
		}

		void Value(float Val)
		{
            this->ThrowOnEmpty("XmlNodeBase");
            this->m_core_object->value(Val);
		}

		void Value(double Val)
		{
            this->ThrowOnEmpty("XmlNodeBase");
            this->m_core_object->value(Val);
		}
	};

	// Concrete class for XML Attributes
	class XmlAttribute : public XmlNodeBase<files::xml_attribute_interface>
	{
	public:
		XmlAttribute(files::xml_attribute_interface* xmlAttr) : XmlNodeBase(xmlAttr)
		{
			// Nothing to do for now...
		}

		XmlAttribute() {}	// Empty object

		bool Equals(XmlAttribute attr)
		{
			if (Empty() && attr.Empty())
				// Both attributes are empty. This means that they are equal
				return true;

			// At least one attributes is not empty. If one of them is, then they are not equal
			if (Empty() || attr.Empty())
				// One attribute is empty and the other one is not
				return false;

			return m_core_object->equals(attr.m_core_object);
		}
	};

	// Common Iterator for XML Attributes or Elements
	template <typename T>
	class XmlIterator
	{
	private:
		std::function<T(size_t)> m_getAtFunc;
		size_t m_index;
		T m_value;

		void set_value()
		{
			m_value = m_getAtFunc(m_index);
		}

	public:
		XmlIterator(const std::function<T(size_t)>& getAtFunc, size_t index) 
			      : m_getAtFunc(getAtFunc),
  			        m_index(index)
		{
			set_value();
		}

		// prefix increment
		XmlIterator operator++()
		{
			XmlIterator retval = *this;
			m_index++;
			set_value();

			return retval;
		}

		// postfix increment
		XmlIterator operator++(int)
		{
			m_index++;
			set_value();

			return *this;
		}

		T& operator*()
		{
			return m_value;
		}

		T* operator->()
		{
			return &m_value;
		}

		bool operator==(const XmlIterator& rhs)
		{
			return m_value.Equals(rhs.m_value);
		}

		bool operator!=(const XmlIterator& rhs)
		{
			return !m_value.Equals(rhs.m_value);
		}
	};

	// Object range class to support modern for-each loop with XML Attributes/Elements Iterators
	template <typename T>
	class XmlObjectRange
	{
	private:
		std::function<T(size_t)> m_getAtFunc;

	public:
		XmlObjectRange()
		{
			//Empty constructor
		}

        XmlObjectRange(XmlObjectRange &&other):
            m_getAtFunc(std::move(other.m_getAtFunc))
        {

		}

		XmlObjectRange(const std::function<T(size_t)>& getAtFunc) 
			: m_getAtFunc(getAtFunc)
		{
		}

		XmlObjectRange& operator=(XmlObjectRange&& other)
		{
			m_getAtFunc = std::move(other.m_getAtFunc);
			return *this;
		}

		XmlIterator<T> begin()
		{
			if (m_getAtFunc == nullptr)
				throw std::runtime_error("Empty XmlObjectRange");

			return XmlIterator<T>(m_getAtFunc, 0);
		}

		XmlIterator<T> end()
		{
			if (m_getAtFunc == nullptr)
				throw std::runtime_error("Empty XmlObjectRange");

			return XmlIterator<T>(m_getAtFunc, (std::numeric_limits<size_t>::max)());
		}

		bool Empty()
		{
			if (begin() == end())
				return true;
			return false;
		}
	};

	// Concrete class for XML Elements
	class XmlElement : public XmlNodeBase<files::xml_element_interface>
	{	
	public:
		XmlElement(files::xml_element_interface* xmlElem) : XmlNodeBase(xmlElem)
		{
			// Nothing to do for now...
		}

		XmlElement() {}	// Empty object

		bool Equals(XmlElement elem)
		{
			if (Empty() && elem.Empty())
				// Both elements are empty. This means that they are equal
				return true;

			// At least one element is not empty. If one of them is, then they are not equal
			if (Empty() || elem.Empty())
				// One element is empty and the other one is not
				return false;

			return m_core_object->equals(elem.m_core_object);
		}

		XmlObjectRange<XmlElement> Children()
		{
			if (Empty() == true)
				throw std::runtime_error("Empty Element");

			XmlElement me = *this;
			return XmlObjectRange<XmlElement>([me](size_t index) mutable
			{
				return me.QueryChild(index);		// Implementation of 'getAtFunc' for element children
			});
		}

		XmlObjectRange<XmlElement> Children(const char* childName)
		{
			if (Empty() == true)
				throw std::runtime_error("Empty Element");

			if(childName == nullptr)
				throw std::invalid_argument("childName");

			XmlElement me = *this;
			std::string name(childName);
			return XmlObjectRange<XmlElement>([me,name](size_t index) mutable
			{
				// Implementation of 'getAtFunc' for element children
				utils::ref_count_ptr<files::xml_element_interface> element;
				
				if (me.m_core_object->query_child(index,name.c_str(), &element) == false)
				{
					//if not found return an empty element
					return XmlElement();
				}
				return XmlElement(element);
			});
		}

		XmlObjectRange<XmlAttribute> Attributes()
		{
			if (Empty() == true)
				throw std::runtime_error("Empty Element");

			XmlElement me = *this;
			return XmlObjectRange<XmlAttribute>([me](size_t index) mutable
			{
				return me.QueryAttribute(index);	// Implementation of 'getAtFunc' for element attributes
			});
		}

		XmlAttribute QueryAttribute(const char* name)
		{
			if (Empty() == true)
				throw std::runtime_error("Empty Element");

			utils::ref_count_ptr<files::xml_attribute_interface> attribute;
			if (m_core_object->query_attribute(name, &attribute) == false)
			{
				//return empty attribute
				return XmlAttribute();
			}
			return XmlAttribute(attribute);
		}

		XmlAttribute QueryAttribute(size_t index)
		{
			if (Empty() == true)
				throw std::runtime_error("Empty Element");

			utils::ref_count_ptr<files::xml_attribute_interface> attribute;
			if (m_core_object->query_attribute(index, &attribute) == false)
				// Attribute not found. Return empty attribute (instead of throwing exception)
				// so iterators and for-each loops will work
				return XmlAttribute();	
				
			return XmlAttribute(attribute);
		}

		XmlElement QueryChild(const char* name)
		{
			if (Empty() == true)
				throw std::runtime_error("Empty Element");

			utils::ref_count_ptr<files::xml_element_interface> child;
			if (m_core_object->query_child(name, &child) == false)
			{
				// Child not found. Return empty element (instead of throwing exception)
				// so iterators and for-each loops will work
				return XmlElement();
			}
			return XmlElement(child);
		}
				
		XmlElement QueryChild(size_t index)
		{
			if (Empty() == true)
				throw std::runtime_error("Empty Element");

			utils::ref_count_ptr<files::xml_element_interface> child;
			if (m_core_object->query_child(index, &child) == false)
				// Child not found. Return empty element (instead of throwing exception)
				// so iterators and for-each loops will work
				return XmlElement();

			return XmlElement(child);
		}
		
		bool IsComment()
		{
			if (Empty() == true)
				throw std::runtime_error("Empty Element");

			return m_core_object->is_comment();
		}

		bool AddAttribute(const char* name, const char* value)
		{
			if (Empty() == true)
				throw std::runtime_error("Empty Element");

			return m_core_object->add_attribute(name, value);
		}

		bool AddChild(const char* name, const char* value)
		{
			if (Empty() == true)
				throw std::runtime_error("Empty Element");

			return m_core_object->add_child(name, value);
		}

		bool RemoveAttribute(const char* name)
		{
			if (Empty() == true)
				throw std::runtime_error("Empty Element");

			return m_core_object->remove_attribute(name);
		}

		bool RemoveChild(const char* name)
		{
			if (Empty() == true)
				throw std::runtime_error("Empty Element");

			return m_core_object->remove_child(name);
		}
	};

	// To wrap xml_file API
	class XmlFile : public FileBase
	{
	private:
		XmlFile(files::xml_file_interface* xmlFile) : FileBase(xmlFile)
		{
			// Nothing to do for now...
		}

		inline files::xml_file_interface* core_xml_file() const
		{
			if (Empty() == true)
				throw std::runtime_error("Empty XmlFile");

			return static_cast<files::xml_file_interface*>(static_cast<core::files::file_access_interface*>(m_core_object));
		}

	public:
		XmlFile() {} // Empty object

		XmlElement QueryRoot()
		{
			utils::ref_count_ptr<files::xml_element_interface> root;
			if (core_xml_file()->query_root(&root) == false)
			{
				throw std::runtime_error("Failed to retrieve root element");
			}
			return XmlElement(root);
		}

		XmlElement QueryElement(const char* xpath)
		{
			utils::ref_count_ptr<files::xml_element_interface> element;
			if (core_xml_file()->query_element(xpath, &element) == false)
			{
				throw std::runtime_error("Failed to query element");
			}
			return XmlElement(element);
		}

		static XmlFile Create(const char* filePath)
		{
			utils::ref_count_ptr<files::xml_file_interface> instance;
			if (files::xml_file_interface::create(filePath, &instance) == false)
			{
				throw std::runtime_error("Failed to create XmlFile");
			}

			return XmlFile(instance);
		}
	};	

	// To wrap files handler API
	class FilesHandler : public Common::CoreObjectWrapper<core::files::files_handler_interface>
	{
	private:
		FilesHandler(core::files::files_handler_interface* handler) :
			Common::CoreObjectWrapper<core::files::files_handler_interface>(handler)
		{
		}

	public:
		static constexpr double DEFAULT_FLUSH_INTERVAL = 30000; // 30 Seconds

		FilesHandler()
		{
			// Empty FilesHandler
		}		

		virtual bool SubscribeFile(const Files::FileBase& file)
		{
			ThrowOnEmpty("FileHandler");

			if (file.Empty() == true)
				throw std::invalid_argument("file");

			utils::ref_count_ptr<core::files::file_access_interface> core_file;
			file.UnderlyingObject(&core_file);
			return m_core_object->subscribe_file(core_file);
		}

		virtual bool UnsubscribeFile(const Files::FileBase& file)
		{
			ThrowOnEmpty("FileHandler");

			if (file.Empty() == true)
				throw std::invalid_argument("file");

			utils::ref_count_ptr<core::files::file_access_interface> core_file;
			file.UnderlyingObject(&core_file);
			return m_core_object->unsubscribe_file(core_file);
		}

		virtual void Flush()
		{
			ThrowOnEmpty("FileHandler");
			m_core_object->flush();
		}

		virtual void Flush(const Files::FileBase& file)
		{			
			ThrowOnEmpty("FileHandler");

			if (file.Empty() == true)
				throw std::invalid_argument("file");

			utils::ref_count_ptr<core::files::file_access_interface> core_file;
			file.UnderlyingObject(&core_file);
			m_core_object->flush(core_file);
		}

		static FilesHandler Create(double flushInterval = DEFAULT_FLUSH_INTERVAL)
		{
			utils::ref_count_ptr<core::files::files_handler_interface> instance;
			if (files::files_handler::create(flushInterval, &instance) == false)
			{
				throw std::runtime_error("Failed to create FileHandler");
			}

			return FilesHandler(instance);
		}
	};

	class FilesSystem
	{
	public:
		static bool CopyFile(const char* src_file, const char* dest_file, bool overwrite_existing = false)
		{
			return files::filessystem::copy_file(src_file, dest_file, overwrite_existing);
		}
		static bool IsFileExist(const char* name)
		{
			return files::filessystem::is_file_exist(name);
		}
	};
}
