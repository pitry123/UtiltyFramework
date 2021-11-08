#include "enum_data_imp.h"
#include <utils/types.hpp>

parsers::enum_data_impl::enum_data_impl(const char* name) : 
	m_name(name == nullptr ? std::string() : std::string(name))
{
	if (name == nullptr)
		throw std::invalid_argument("enum_data_impl::name");
}

const char* 
parsers::enum_data_impl::name() const
{
	return m_name.c_str();
}

size_t
parsers::enum_data_impl::size() const
{
    return m_items.use<size_t>([&](enums_vector& items)
	{
		return items.size();
	});
}

bool
parsers::enum_data_impl::add_item(const char* name, int64_t val) const
{
	if (name == nullptr)
		return false;

    return m_items.use<bool>([&](enums_vector& items)
	{	
		core::parsers::enum_data_item item;

		STRCPY(item.name,sizeof(item.name), name);
		item.value = val;
		
		items.emplace_back(item);

		return true;
	});

}

bool 
parsers::enum_data_impl::item_by_index(size_t index, core::parsers::enum_data_item& item) const
{
    return m_items.use<bool>([&](enums_vector& items)
	{
		if (index > items.size())
			return false;
		
		item = items[index];
		return true;
	});
}

bool parsers::enum_data_impl::item_by_name(const char * name, core::parsers::enum_data_item& item) const
{
	if (name == nullptr)
		return false;

    return m_items.use<bool>([&](enums_vector& items)
	{
		for (auto it = items.begin(); it != items.end(); ++it)
		{
			if (std::strcmp(name, it->name) == 0)
			{
				item = *it;
				return true;
			}
		}
		return false;
	});
}

bool parsers::enum_data_impl::item_by_val(int64_t val, core::parsers::enum_data_item& item) const
{
    return m_items.use<bool>([&](enums_vector& items)
	{
		for (auto it = items.begin(); it != items.end(); ++it)
		{
			if (val == it->value)
			{
				item = *it;
				return true;
			}
		}
		return false;
	});
}

bool
parsers::enum_data_impl::val_by_name(const char* name, int64_t &val) const
{
	if (name == nullptr)
		return false;

    return m_items.use<bool>([&](enums_vector& items)
	{
		for (auto it = items.begin(); it != items.end(); ++it)
		{
			if (std::strcmp(name, it->name) == 0)
			{
				val = it->value;
				return true;
			}		
		}
		return false;
	});
}

const char* 
parsers::enum_data_impl::name_by_val(int64_t val) const
{
    return m_items.use<const char*>([&](enums_vector& items) -> const char*
	{
		for (auto it = items.begin(); it != items.end(); ++it)
		{
			if (it->value == val)
			{
                return it->name;
			}
		}

        return nullptr;
	});
}

const char* 
parsers::enum_data_impl::to_json(bool compact) const
{
	unordered_json json;
	int indent = -1;

	if (false == compact)
		indent = 4;

	if (enum_to_json(json))
	{
		m_json_str = json.dump(indent);

		return m_json_str.c_str();
	}

	return nullptr;
}

bool
parsers::enum_data_impl::from_json(const char* json_string) const
{
	unordered_json json;
	json = unordered_json::parse(json_string);

	std::string enum_name	  = json["name"].get<std::string>();
	unordered_json array_json = json["items"];

	if (enum_name.empty() || !array_json.is_array())
		return false;

	for (unordered_json::iterator it = array_json.begin(); it != array_json.end(); ++it)
	{
		unordered_json obj = *it;
		std::string name = obj["name"].get<std::string>();
		int val = obj["value"].get<int>();
		add_item(name.c_str(), val);
	}

    return m_items.use<bool>([&](enums_vector& items)
	{
		if (items.empty())
			return false;
		else
			return true;
	});
}

bool
parsers::enum_data_impl::enum_to_json(unordered_json& json) const
{
	json["name"] = name();

    return m_items.use<bool>([&](enums_vector& items)
	{
		int i = 0;

		for (auto it = items.begin(); it != items.end(); ++it)
		{
            json["items"][static_cast<size_t>(i)]["name"] = it->name;
            json["items"][static_cast<size_t>(i)]["value"] = it->value;
			i++;
		}

		return true;
	});
}

bool 
parsers::enum_data::create(const char* name, core::parsers::enum_data_interface** enum_data)
{
	if (enum_data == nullptr)
		return false;

	utils::ref_count_ptr<core::parsers::enum_data_interface> instance;
	try
	{
		instance = utils::make_ref_count_ptr<parsers::enum_data_impl>(name);
	}
	catch (...)
	{
		return false;
	}

	if (instance == nullptr)
		return false;

	*enum_data = instance;
	(*enum_data)->add_ref();
	return true;
}
