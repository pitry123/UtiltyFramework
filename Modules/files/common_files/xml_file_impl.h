#pragma once

#include "pugixml/pugixml.hpp"

#include <files/xml_file_interface.h>
#include <utils/files.hpp>
#include <utils/ref_count_ptr.hpp>


namespace files
{
	// Forward declarations
	class xml_file;

	/// @class	xml_node
	/// @brief	Implementation of an XML node which can be either XML 
	/// 		element or XML attribute.
	///
	/// @date	11/03/2018
	template <typename T>
	class xml_node : public utils::ref_count_base<T>
	{
	private:
		/// @brief  Base class type definition
		using Super = utils::ref_count_base<T>;

	protected:
		/// @brief	The XML file (that holds the xml DOM object). This will
		/// 		also prevent from file object to be freed while nodes
		/// 		are using its' data
		utils::ref_count_ptr<files::xml_file> m_file;

	public:

		/// @brief	Constructor
		///
		/// @param [in,out]	file	xml file object (that contains xml DOM)
		xml_node(files::xml_file* file);

		/// @brief	Default destructor
		virtual ~xml_node() = default;
	};

	/// @class  xml_attribute
	/// @brief	Implementation of XML Attribute
	///
	/// @date	15/03/2018
	class xml_attribute : public files::xml_node<xml_attribute_interface>
	{
	private:
		/// @brief	The attribute DOM data
		pugi::xml_attribute m_attribute_dom = {};

		/// @brief	Base class type definition
		using Super = files::xml_node<xml_attribute_interface>;

	public:

		/// @brief	Constructor
		///
		/// @param [in,out]	file	XML file (contains DOM object)
		/// @param 		   	attr	Attribute DOM object
		xml_attribute(files::xml_file* file, const pugi::xml_attribute& attr);

		/// @brief	Default destructor
		virtual ~xml_attribute() = default;

		/// @brief	Checks equality with given attribute
		///
		/// @param [in,out]	attr_intf	Attribute to compare to.
		///
		/// @return	True if equals, false otherwise.
		virtual bool equals(xml_attribute_interface* attr_intf) override;

		// xml_node_interface API implementation for attribute:
		
		// Getters
		
		/// @brief	XML attribute name getter
		///
		/// @return	XML attribute name. Name cannot be NULL
		virtual const char* name() const override;

		/// @brief	XML node value getter
		///
		/// @return	XML node value. Value can be empty but not NULL
		virtual const char* value() const override;

		/// @brief	Various value getters (boolean, integer, float etc.)
		///
		/// @param	def_val	default value to return in case value is empty
		/// 				or attribute is empty
		///
		/// @return	XML node value.
		
		virtual bool value_as_bool(bool def_val) const override;
		virtual int32_t value_as_int(int32_t def_val) const override;
		virtual uint32_t value_as_uint(uint32_t def_val) const override;
		virtual int64_t value_as_long(int64_t def_val) const override;
		virtual uint64_t value_as_ulong(uint64_t def_val) const override;
		virtual float value_as_float(float def_val) const override;
		virtual double value_as_double(double def_val) const override;

		// Setters

		/// @brief	XML attribute value setter
		///
		/// @param	value	New value to set.
		virtual void value(const char* value) override;
	
		/// @brief	Various value setters (boolean, integer, float etc.)
		///
		/// @param	value	New value to set
		
		virtual void value(bool value) override;
		virtual void value(int32_t value) override;
		virtual void value(uint32_t value) override;
		virtual void value(int64_t value) override;
		virtual void value(uint64_t value) override;
		virtual void value(float value) override;
		virtual void value(double value) override;
	};

	/// class	xml_element
	/// @brief	Implementation of XML element
	///
	/// @date	15/03/2018
	class xml_element : public xml_node<xml_element_interface>
	{
	private:
		/// @brief	The element DOM data
		pugi::xml_node m_node_dom = {};

		/// @brief	Base class type definition
		using Super = xml_node<xml_element_interface>;

		/// @brief	Creates an attribute as should ref_count_ptr object be 
		/// 		constructed
		///
		/// @param [in,out]	pugi_attr	The pugi attribute.
		/// @param [in,out]	attribute	Attribute to construct
		///
		/// @return	True if it succeeds, false if it fails.
		bool create_attribute(pugi::xml_attribute& pugi_attr, files::xml_attribute_interface** attribute);

		/// @brief	Creates an element as should ref_count_ptr object be
		/// 		constructed
		///
		/// @param [in,out]	pugi_node	The pugi node.
		/// @param [in,out]	element  	Element to construct
		///
		/// @return	True if it succeeds, false if it fails.
		bool create_element(pugi::xml_node& pugi_node, files::xml_element_interface** element);

	public:
		/// @brief	Constructor
		///
		/// @param [in,out]	file	XML file (contains DOM object)
		/// @param 		   	node	Node DOM object
		xml_element(files::xml_file* file, const pugi::xml_node& node);

		/// @brief	Default destructor
		virtual ~xml_element() = default;

		/// @brief	Checks equality with given element
		///
		/// @param [in,out]	elem_intf	Element to compare to.
		///
		/// @return	True if equals, false otherwise.
		virtual bool equals(xml_element_interface* elem_intf) override;


		/// @brief	Determines if element is of comment type
		///
		/// @return	True if comment, false if not.
		virtual bool is_comment() override;

		/// @brief	Queries an attribute by name
		///
		/// @param 		   	name	 	Attribute name.
		/// @param [in,out]	attribute	If non-null, the attribute.
		///
		/// @return	True if it succeeds, false if it fails.
		virtual bool query_attribute(const char* name, files::xml_attribute_interface** attribute) override;

		/// @brief	Queries an attribute by index
		///
		/// @param 		   	index	 	Zero-based index of the attribute
		/// @param [in,out]	attribute	If non-null, the attribute.
		///
		/// @return	True if it succeeds, false if it fails.
		virtual bool query_attribute(size_t index, files::xml_attribute_interface** attribute) override;

		/// @brief	Queries a child element by name
		///
		/// @param 		   	name   	Child element name.
		/// @param [in,out]	element	If non-null, the element.
		///
		/// @return	True if it succeeds, false if it fails.
		virtual bool query_child(const char* name, files::xml_element_interface** element) override;

		/// Queries child iterator iterate over the xml child with the name
		/// specified
		/// @date	16/10/2018
		/// @param 			index  	Zero-based index of the.
		/// @param 			name   	The name.
		/// @param [out]	element	If non-null, the element.
		/// @return	True if it succeeds, false if it fails.
		virtual bool query_child(size_t index, const char* name, files::xml_element_interface** element) override;

		/// @brief	Queries a child element by index
		///
		/// @param 		   	index  	Zero-based index of the child element
		/// @param [in,out]	element	If non-null, the element.
		///
		/// @return	True if it succeeds, false if it fails.
		virtual bool query_child(size_t index, files::xml_element_interface** element) override;

		/// @brief	Adds an attribute (push back) to the element
		///
		/// @param	name 	Attribute name (NULL is invalid)
		/// @param	value	Attribute value (NULL is invalid)
		///
		/// @return	True if it succeeds, false if it fails.
		virtual bool add_attribute(const char* name, const char* value) override;

		/// @brief	Adds a child element (push back) to the element
		///
		/// @param	name 	Attribute name (NULL is invalid)
		/// @param	value	Attribute value (NULL is invalid)
		///
		/// @return	True if it succeeds, false if it fails.
		virtual bool add_child(const char* name, const char* value) override;

		/// @brief	Removes an attribute described by name from element
		///
		/// @param	name	Attribute name (NULL is invalid)
		///
		/// @return	True if it succeeds, false if it fails.
		virtual bool remove_attribute(const char* name) override;

		/// @brief	Removes a child described by name from element
		///
		/// @param	name	Child name (NULL is invalid)
		///
		/// @return	True if it succeeds, false if it fails.
		virtual bool remove_child(const char* name) override;

		// xml_node_interface API implementation for attribute:

		// Getters

		/// @brief	XML attribute name getter
		///
		/// @return	XML attribute name. Name cannot be NULL
		virtual const char* name() const override;

		/// @brief	XML node value getter
		///
		/// @return	XML node value. Value can be empty but not NULL
		virtual const char* value() const override;

		/// @brief	Various value getters (boolean, integer, float etc.)
		///
		/// @param	def_val	default value to return in case value is empty
		/// 				or attribute is empty
		///
		/// @return	XML node value.
		
		virtual bool value_as_bool(bool def_val) const override;
		virtual int32_t value_as_int(int32_t def_val) const override;
		virtual uint32_t value_as_uint(uint32_t def_val) const override;
		virtual int64_t value_as_long(int64_t def_val) const override;
		virtual uint64_t value_as_ulong(uint64_t def_val) const override;
		virtual float value_as_float(float def_val) const override;
		virtual double value_as_double(double def_val) const override;

		// Setters

		/// @brief	XML attribute value setter
		///
		/// @param	value	New value to set.
		virtual void value(const char* value) override;
	
		/// @brief	Various value setters (boolean, integer, float etc.)
		///
		/// @param	value	New value to set
		
		virtual void value(bool value) override;
		virtual void value(int32_t value) override;
		virtual void value(uint32_t value) override;
		virtual void value(int64_t value) override;
		virtual void value(uint64_t value) override;
		virtual void value(float value) override;
		virtual void value(double value) override;
	};

    /// @class	xml_file
    /// @brief	Implementation of XML file.
    ///
    /// @date	15/03/2018
    class xml_file : public utils::files::file_access_base<xml_file_interface>
    {
    private:
        /// @brief  The XML file DOM data (also root element)
        pugi::xml_document m_file_dom = {};

        /// @brief  Mutex for internal data protection
        mutable std::mutex m_data_mutex;

        /// @brief  True if file needs to be flushed
        bool m_need_update;

        /// @brief  Base class type definition
        using Super = utils::files::file_access_base<xml_file_interface>;

		/// @brief	Creates an element as should ref_count_ptr object be
		/// 		constructed
		///
		/// @param [in,out]	pugi_node	The pugi node.
		/// @param [in,out]	element  	Element to construct
		///
		/// @return	True if it succeeds, false if it fails.
        bool create_element(const pugi::xml_node& pugi_node, files::xml_element_interface** element);

    public:
        /// @brief  Constructor
        ///
        /// @param  file_path   Full pathname of the file.
        xml_file(const char* file_path);

        /// @brief  Destructor
        virtual ~xml_file();

		/// @brief	Getter for need_update property
		///
		/// @return	True if file needs update, false otherwise
		bool need_update() const { return m_need_update;  }

		/// @brief	Setter for need_update property
		///
		/// @param	need_update	True to set that file needs update
		void need_update(bool need_update) { m_need_update = need_update;  }

        // Inherited from file_access_interface:

        /// @brief  File flush implementation for file writing logic and format
        ///
        /// @return True if it succeeds, false if it fails.
        virtual bool flush() override;

        // Inherited from xml_file_interface:

		/// @brief	Queries the root element of the file
		///
		/// @param [in,out]	element	If non-null, the element.
		///
		/// @return	True if it succeeds, false if it fails.
		virtual bool query_root(files::xml_element_interface** element) override;

		/// @brief	Queries an element using xpath API
		///
		/// @param 		   	xpath  	The xpath to search
		/// @param [in,out]	element	If non-null, the element.
		///
		/// @return	True if it succeeds, false if it fails.
		virtual bool query_element(const char* xpath, files::xml_element_interface** element) override;

		/// @brief	API to lock data for write/change - Multi-threading
		/// 		support
		void lock()	{ m_data_mutex.lock(); }

		/// @brief	API to unlock data for write/change - Multi-threading
		/// 		support
		void unlock() {	m_data_mutex.unlock(); }
	};
}