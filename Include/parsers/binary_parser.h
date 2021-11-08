#pragma once
#include <core/parser.h>

namespace parsers
{
	class DLL_EXPORT binary_metadata : public core::parsers::binary_metadata_builder_interface
	{
	public:
		virtual ~binary_metadata() = default;

		static bool create(bool big_endian, core::parsers::binary_metadata_store_interface* store, core::parsers::binary_parser_creator_interface* creator, core::parsers::binary_metadata_builder_interface** metadata);
		static bool create(const char* metadata_json, core::parsers::binary_metadata_store_interface* store, core::parsers::binary_parser_creator_interface* creator, core::parsers::binary_metadata_builder_interface** metadata);
		
	};

	class DLL_EXPORT binary_parser:  public core::parsers::binary_parser_interface
	{
	public:
		virtual ~binary_parser() = default;

		static bool create(const core::parsers::binary_metadata_interface* metadata,bool reset, core::parsers::binary_parser_interface** parser);
	};

	class DLL_EXPORT enum_data : public core::parsers::enum_data_interface
	{
	public:
		virtual ~enum_data() = default;

		static bool create(const char* name, core::parsers::enum_data_interface** enum_data);
	};

	class DLL_EXPORT binary_metadata_store : public core::parsers::binary_metadata_store_interface
	{
	public:
		virtual ~binary_metadata_store() = default;

		/// Instances of the given metadata store (default)
		/// @date	25/07/2019
		/// @param [in,out]	metadata_store	If non-null, the metadata store.
		/// @return	True if it succeeds, false if it fails.
		static bool instance(core::parsers::binary_metadata_store_interface** metadata_store);

		/// Creates a new store
		/// @date	25/07/2019
		/// @param [out]	metadata_store	If non-null, the metadata store.
		/// @return	True if it succeeds, false if it fails.
		static bool create(core::parsers::binary_metadata_store_interface** metadata_store);
	};
}