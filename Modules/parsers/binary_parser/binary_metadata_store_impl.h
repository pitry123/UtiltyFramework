#pragma once
#include <core/parser.h>
#include <parsers/binary_parser.h>
#include <utils/ref_count_base.hpp>
#include <utils/ref_count_ptr.hpp>

namespace parsers
{
	class binary_metadata_store_impl :
		public utils::ref_count_base<parsers::binary_metadata_store>
	{
	private:
		using metadata_parser_map =
			std::unordered_map<std::string, utils::ref_count_ptr<core::parsers::binary_metadata_interface>>;

		using metadata_enum_map =
			std::unordered_map<std::string, utils::ref_count_ptr<core::parsers::enum_data_interface>>;

		utils::thread_safe_object<metadata_parser_map> m_parser_metadata;
		utils::thread_safe_object<metadata_enum_map> m_enums_metadata;

	public:
		binary_metadata_store_impl();
		bool add_parser_metadata(const char* name, core::parsers::binary_metadata_interface *parser_metadata) override;
		bool query_parser_metadata(const char* name, core::parsers::binary_metadata_interface **parser_metadata) override;
		bool add_enum(const char* name, core::parsers::enum_data_interface* enum_instance) override;
		bool query_enum(const char* name, core::parsers::enum_data_interface** enum_instance) override;
	};
}