#pragma once

#include <core/files.h>
#include <stdint.h>
#include <cstddef>

namespace files
{
	// forward declarations
	class json_object_interface;
	class json_array_interface;

	/// @class	json_value_interface
	/// 		
	/// @brief	JSON value API. 
	/// 		Represents an JSON value which can be either simple object (primitive type like number, 
	/// 		boolean, float) or a complex value like array or object.
	/// 		
	/// @date	21/05/2018
	class DLL_EXPORT json_value_interface : public core::ref_count_interface
	{
	public:
		/// @enum	json_type_enum
		///
		/// @brief	Represent JSON value types
		enum json_type_enum
		{
			j_null_type = 0,
			j_false_type = 1,
			j_true_type = 2,
			j_object_type = 3,
			j_array_type = 4,
			j_string_type = 5,
			j_number_type = 6
		};

		/// @brief	Default destructor
		virtual ~json_value_interface() = default;

		// Getters

		/// @brief	Internal value type
		///
		/// @return	Internal value type
		virtual json_type_enum value_type() = 0;

		/// @brief	Gets value as string
		///
		/// @return	Null if it fails, else the string.
		virtual const char* get_string() const = 0;

		/// @brief	Gets value as bool
		///
		/// @param [in,out]	value	Boolean value.
		/// @return	True if it succeeds, false if it fails.
		virtual bool get_bool(bool& value) const = 0;

		/// @brief	Gets value as 64 bit integer. If 32 bit is required value may be down-casted
		///
		/// @param [in,out]	value	Number value.
		/// @return	True if it succeeds, false if it fails.
		virtual bool get_long(int64_t& value) const = 0;

		/// @brief	Gets value as double. If float is required value may be down-casted
		///
		/// @param [in,out]	value	Number value.
		/// @return	True if it succeeds, false if it fails.
		virtual bool get_double(double& value) const = 0;

		/// @brief	Gets value as an object
		///
		/// @param [in,out]	value	If non-null, the value.
		/// @return	True if it succeeds, false if it fails.
		virtual bool get_object(json_object_interface** value) const = 0;

		/// @brief	Gets value as an array
		///
		/// @param [in,out]	value	If non-null, the value.
		/// @return	True if it succeeds, false if it fails.
		virtual bool get_array(json_array_interface** value) const = 0;

		// Setters
		
		/// @brief	Sets value as bool
		///
		/// @param	value	Value to set
		/// @return	True if it succeeds, false if it fails.
		virtual bool set_bool(bool value) = 0;

		/// @brief	Sets value as int
		///
		/// @param	value	Value to set
		/// @return	True if it succeeds, false if it fails.
		virtual bool set_int(int64_t value) = 0;

		/// @brief	Sets value as double
		/// 		
		/// @param	value	Value to set
		/// @return	True if it succeeds, false if it fails.
		virtual bool set_double(double value) = 0;

		/// @brief	Sets value as string
		///
		/// @param	value	Value to set
		/// @return	True if it succeeds, false if it fails.
		virtual bool set_string(const char* value) = 0;

		/// @brief	Sets value as an empty object. If given pointer is not null then the created
		/// 		object pointer is given back to enable setting it's members
		///
		/// @param [in,out]	object	If non-null, the created empty object.
		///
		/// @return	True if it succeeds, false if it fails.
		virtual bool set_object(json_object_interface** object) = 0;

		/// @brief	Sets value as an empty array. If given pointer is not null then the created
		/// 		array pointer is given back to enable setting it's values
		///
		/// @param [in,out]	array	If non-null, the created empty array.
		///
		/// @return	True if it succeeds, false if it fails.
		virtual bool set_array(json_array_interface** array) = 0;
	};

	/// @class	json_member_interface
	///
	/// @brief	JSON member API. 
	/// 		Defines API for JSON member, which are actually a pair of name (string) and JSON value
	///
	/// @date	21/05/2018
	class DLL_EXPORT json_member_interface : public files::json_value_interface
	{
	public:
		/// @brief	Default destructor
		virtual ~json_member_interface() = default;

		// Getters
		
		/// @brief	Gets the name of the member
		///
		/// @return	Null if it fails, else a pointer to a const char.
		virtual const char* name() const = 0;

		/// @brief	Sets the name of the member
		///
		/// @param	name	The name to set.
		/// @return	True if it succeeds, false if it fails.
		virtual bool name(const char* name) = 0;
	};

	/// @class	json_array_interface
	///
	/// @brief	JSON array API. 
	/// 		Defines API for JSON arrays, which are actually a collection of JSON values
	///
	/// @date	21/05/2018
	class DLL_EXPORT json_array_interface : public core::ref_count_interface
	{
	public:
		/// @brief	Default destructor
		virtual ~json_array_interface() = default;

		/// @brief	Queries the indexed value from array 
		///
		/// @param 		   	value_index	Zero-based index of the value.
		/// @param [in,out]	value	   	If non-null, the value.
		///
		/// @return	True if it succeeds, false if it fails.
		virtual bool query_value(size_t value_index, files::json_value_interface** value) const = 0;

		/// @brief	Push back an empty value to the array. If given pointer is not null then the created
		/// 		value pointer is given back to enable setting it's value
		///
		/// @param [in,out]	value	If non-null, the value.
		///
		/// @return	True if it succeeds, false if it fails.
		virtual bool add_value(files::json_value_interface** value) = 0;

		/// @brief	Removes the indexed value from the array
		///
		/// @param	value_index	Zero-based index of the value.
		/// @return	True if it succeeds, false if it fails.
		virtual bool remove_value(size_t value_index) = 0;
	};

	/// @class	json_object_interface
	///
	/// @brief	JSON object API. 
	/// 		Defines API for JSON objects, which are actually a collection of JSON members
	///
	/// @date	21/05/2018
	class DLL_EXPORT json_object_interface : public core::ref_count_interface
	{
	public:
		/// @brief	Default destructor
		virtual ~json_object_interface() = default;

		/// @brief	Queries a member from object by name
		///
		/// @param 		   	name  	Name of the member to query
		/// @param [in,out]	member	If non-null, the member.
		///
		/// @return	True if it succeeds, false if it fails.
		virtual bool query_member(const char* name, files::json_member_interface** member) const = 0;

		/// @brief	Queries a member from object by index
		///
		/// @param 		   	member_index	Index of the member to query.
		/// @param [in,out]	member			If non-null, the member.
		///
		/// @return	True if it succeeds, false if it fails.
		virtual bool query_member(size_t member_index, files::json_member_interface** member) const = 0;

		/// @brief	Adds an empty member to object with given name. If given pointer is not null then the
		/// 		created member pointer is given back to enable setting it's value
		///
		/// @param 		   	name  	The member's name.
		/// @param [in,out]	member	If non-null, the member.
		///
		/// @return	True if it succeeds, false if it fails.
		virtual bool add_member(const char* name, files::json_member_interface** member) = 0;

		/// @brief	Removes the member described by name from object
		///
		/// @param	name	The member's name.
		/// @return	True if it succeeds, false if it fails.
		virtual bool remove_member(const char* name) = 0;
	};

	/// @class	json_file_interface
	///
	/// @brief	JSON file API. Defines API for JSON files
	///
	/// @date	21/05/2018
	class DLL_EXPORT json_file_interface : public core::files::file_access_interface
	{
	public:
		/// @brief  Default destructor
		virtual ~json_file_interface() = default;

		// JSON file API:
		// --------------

		/// @brief	Queries the root object of the file
		///
		/// @param [in,out]	object	If non-null, the object.
		///
		/// @return	True if it succeeds, false if it fails.
		virtual bool query_root(files::json_object_interface** object) const = 0;
	};
}