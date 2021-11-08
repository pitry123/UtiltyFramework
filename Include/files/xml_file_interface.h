/// @file	files/xml_file_interface.h.
/// @brief	Declares the XML file class
#pragma once

#include <core/files.h>
#include <stdint.h>
#include <cstddef>

namespace files
{
	/// @class	xml_node_interface
	/// 		
	/// @brief	Represents an XML node which can be either XML element or 
	/// 		XML attribute. This class defined common API for both.
	/// 		
	/// @date	11/03/2018
	class DLL_EXPORT xml_node_interface : public core::ref_count_interface
	{
	public:

		/// @brief	Default destructor
		virtual ~xml_node_interface() = default;

		// Getters
		
		/// @brief	XML node name getter
		/// 		
		/// @return	XML node name. Name cannot be NULL		
		virtual const char* name() const = 0;

		/// @brief	XML node value getter
		/// 		
		/// @return	XML node value. Value can be empty but not NULL
		virtual const char* value() const = 0;

		/// @brief	Various value getters (boolean, integer, float etc.)
		/// 		
		/// @param	def_val	default value to return in case value is empty
		/// 				or attribute is empty
		/// 				
		/// @return	XML node value.
		virtual bool value_as_bool(bool def_val) const = 0;
		virtual int32_t value_as_int(int32_t def_val) const = 0;
		virtual uint32_t value_as_uint(uint32_t def_val) const = 0;
		virtual int64_t value_as_long(int64_t def_val) const = 0;
		virtual uint64_t value_as_ulong(uint64_t def_val) const = 0;
		virtual float value_as_float(float def_val) const = 0;
		virtual double value_as_double(double def_val) const = 0;

		// Setters
		
		/// @brief	XML node value setter
		/// 		
		/// @param	value	New value to set.		
		virtual void value(const char* value) = 0;

		/// @brief	Various value setters (boolean, integer, float etc.)
		/// 		
		/// @param	value	New value to set
		virtual void value(bool value) = 0;
		virtual void value(int32_t value) = 0;
		virtual void value(uint32_t value) = 0;
		virtual void value(int64_t value) = 0;
		virtual void value(uint64_t value) = 0;
		virtual void value(float value) = 0;
		virtual void value(double value) = 0;
	};

	/// @class	xml_attribute_interface
	/// 		
	/// @brief	XML attribute API
	/// 		
	/// @date	11/03/2018
	class DLL_EXPORT xml_attribute_interface : public files::xml_node_interface
	{
	public:
		/// @brief	Default destructor
		virtual ~xml_attribute_interface() = default;

		/// @brief	Checks equality with given attribute
		/// 		
		/// @param [in]		attr_intf	Attribute to compare to
		/// 				
		/// @return	True if equals, false otherwise
		virtual bool equals(xml_attribute_interface* attr_intf) = 0;
	};

	/// @class	xml_element_interface
	/// 		
	/// @brief	XML element API
	/// 		
	/// @date	11/03/2018
	class DLL_EXPORT xml_element_interface : public files::xml_node_interface
	{
	public:
		/// @brief	Default destructor
		virtual ~xml_element_interface() = default;

		/// @brief	Checks equality with given element
		/// 		
		/// @param [in,out]	elem_intf	Element to compare to.
		/// 				
		/// @return	True if equals, false otherwise.
		virtual bool equals(xml_element_interface* elem_intf) = 0;

		/// @brief	Determines if element is of comment type
		/// 		
		/// @return	True if comment, false if not.
		virtual bool is_comment() = 0;

		/// @brief	Queries an attribute by name
		/// 		
		/// @param 		   	name	 	Attribute name.
		/// @param [out]	attribute	An address of a pointer to files::xml_attribute_interface
		/// 				
		/// @return	True if it succeeds, false if it fails.
		virtual bool query_attribute(const char* name, files::xml_attribute_interface** attribute) = 0;

		/// @brief	Queries an attribute by index
		/// 		
		/// @param 		   	index	 	Zero-based index of the attribute
		/// @param [out]	attribute	An address of a pointer to files::xml_attribute_interface
		/// 				
		/// @return	True if it succeeds, false if it fails.
		virtual bool query_attribute(size_t index, files::xml_attribute_interface** attribute) = 0;

		/// @brief	Queries a child element by name
		/// 		
		/// @param 		   	name   	Child element name.
		/// @param [out]	element An address of a pointer to files::xml_element_interface
		/// 				
		/// @return	True if it succeeds, false if it fails.
		virtual bool query_child(const char* name, files::xml_element_interface** element) = 0;

		/// Queries child iterator 
		/// iterate over the xml child with the name specified
		/// @date	16/10/2018
		/// @param 		   	index  	Zero-based index of the.
		/// @param 		   	name   	The name.
		/// @param [out]	element	If non-null, the element.
		/// @return	True if it succeeds, false if it fails.
		virtual bool query_child(size_t index, const char* name, files::xml_element_interface** element) = 0;

		/// @brief	Queries a child element by index
		/// 		
		/// @param 		   	index  	Zero-based index of the child element
		/// @param [out]	element	An address of a pointer to files::xml_element_interface
		/// 				
		/// @return	True if it succeeds, false if it fails.
		virtual bool query_child(size_t index, files::xml_element_interface** element) = 0;

		/// @brief	Adds an attribute (push back) to the element
		/// 		
		/// @param	name 	Attribute name (NULL is invalid)
		/// @param	value	Attribute value (NULL is invalid)
		/// 				
		/// @return	True if it succeeds, false if it fails.
		virtual bool add_attribute(const char* name, const char* value) = 0;

		/// @brief	Adds a child element (push back) to the element
		/// 		
		/// @param	name 	Child name (NULL is invalid)
		/// @param	value	Child value (NULL is valid)
		/// 				
		/// @return	True if it succeeds, false if it fails.
		virtual bool add_child(const char* name, const char* value) = 0;

		/// @brief	Removes an attribute described by name from element
		/// 		
		/// @param	name	Attribute name (NULL is invalid)
		/// 				
		/// @return	True if it succeeds, false if it fails.
		virtual bool remove_attribute(const char* name) = 0;

		/// @brief	Removes a child described by name from element
		/// 		
		/// @param	name	Child name (NULL is invalid)
		/// 				
		/// @return	True if it succeeds, false if it fails.
		virtual bool remove_child(const char* name) = 0;
	};

    /// @class  xml_file_interface
    /// 		
    /// @brief  XML file API. Defines API for XML files
    /// 		
    /// @date   11/03/2018
    class DLL_EXPORT xml_file_interface : public core::files::file_access_interface
    {
    public:

        /// @brief  Default destructor
        virtual ~xml_file_interface() = default;

		// XML file API:
		// -------------

		/// @brief	Queries the root element of the file
		/// 		
		/// @param [out]	element	An address of a pointer to files::xml_element_interface
		/// 				
		/// @return	True if it succeeds, false if it fails.
		virtual bool query_root(files::xml_element_interface** element) = 0;

		/// @brief	Queries an element using xpath API
		/// 		
		/// @param 		   	xpath  	The xpath to search
		/// @param [out]	element	An address of a pointer to files::xml_element_interface
		/// 				
		/// @return	True if it succeeds, false if it fails.
		virtual bool query_element(const char* xpath, files::xml_element_interface** element) = 0;

		/// @brief  Static factory: Creates new XML file instance
		/// 		
		/// @param          file_path   Full pathname of the file.
		/// @param [out]	file        An address of a pointer to files::xml_file_interface
		/// 				
		/// @return True if it succeeds, false if it fails.
		static bool create(const char* file_path, files::xml_file_interface** file);
    };
}