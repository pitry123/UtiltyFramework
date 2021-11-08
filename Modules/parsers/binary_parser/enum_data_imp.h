#include <core/parser.h>
#include <parsers/binary_parser.h>
#include <utils/ref_count_base.hpp>
#include <utils/thread_safe_object.hpp>

#include "../nlohmann/fifo_json.hpp"

namespace parsers
{
	class enum_data_impl : public utils::ref_count_base<core::parsers::enum_data_interface>
	{
	public:
		enum_data_impl(const char* name);

		const char* name() const override;

		bool add_item(const char* name, int64_t val) const override;
		bool item_by_index(size_t index, core::parsers::enum_data_item& item) const override;
		bool item_by_name(const char* name, core::parsers::enum_data_item& item) const override;
		bool item_by_val(int64_t val, core::parsers::enum_data_item& item) const override;
		bool val_by_name(const char* name, int64_t &val) const override;
		const char* name_by_val(int64_t val) const override;

		const char* to_json(bool compact) const override;
		bool from_json(const char* json_string) const override;

		size_t size() const override;
	private:
		
		// Helpers
		bool enum_to_json(unordered_json& json) const;

		std::string m_name;

		using enums_vector = std::vector<core::parsers::enum_data_item>;

		mutable utils::thread_safe_object<enums_vector> m_items;
		mutable std::string m_json_str;

	};
}