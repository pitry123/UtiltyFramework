#include "xml_file_impl.h"

#include <utils/scope_guard.hpp>
#include <cstring>


//--------------------------------------------------------------------
// XML NODE
// -------------------------------------------------------------------

template <typename T>
files::xml_node<T>::xml_node(files::xml_file* file)
{
	m_file = file;
}

//--------------------------------------------------------------------
// XML ATTRIBUTE
// -------------------------------------------------------------------

files::xml_attribute::xml_attribute(files::xml_file* file, const pugi::xml_attribute& attr)
							: Super(file)
{
	m_attribute_dom = attr;
}

bool 
files::xml_attribute::equals(xml_attribute_interface* attr_intf)
{
	xml_attribute* attr = (xml_attribute *)attr_intf;

	if (attr->m_attribute_dom.empty() && m_attribute_dom.empty())
		// Both attributes are empty. This means that they are equal
		return true;

	// At least one attributes is not empty. If one of them is, then they are not equal
	if (attr->m_attribute_dom.empty() || m_attribute_dom.empty())
		// One attribute is empty and the other one is not
		return false;

	return (m_attribute_dom == attr->m_attribute_dom);
}

const char* 
files::xml_attribute::name() const
{
	return m_attribute_dom.name();
}

const char* 
files::xml_attribute::value() const
{
	return m_attribute_dom.value();
}

bool 
files::xml_attribute::value_as_bool(bool def_val) const
{
	return m_attribute_dom.as_bool(def_val);
}

int32_t 
files::xml_attribute::value_as_int(int32_t def_val) const
{
	return m_attribute_dom.as_int(def_val);
}

uint32_t 
files::xml_attribute::value_as_uint(uint32_t def_val) const
{
	return m_attribute_dom.as_uint(def_val);
}

int64_t 
files::xml_attribute::value_as_long(int64_t def_val) const
{
	return m_attribute_dom.as_llong(def_val);
}

uint64_t 
files::xml_attribute::value_as_ulong(uint64_t def_val) const
{
	return m_attribute_dom.as_ullong(def_val);
}

float 
files::xml_attribute::value_as_float(float def_val) const
{
	return m_attribute_dom.as_float(def_val);
}

double 
files::xml_attribute::value_as_double(double def_val) const
{
	return m_attribute_dom.as_double(def_val);
}

void 
files::xml_attribute::value(const char* value)
{
	// Get hold of data synchronization mutex (will be unlocked when goes out of scope)
	m_file->lock();
	utils::scope_guard unlocker([&]() { m_file->unlock(); });

	m_attribute_dom.set_value(value);
	m_file->need_update(true);
}

void
files::xml_attribute::value(bool value)
{
	// Get hold of data synchronization mutex (will be unlocked when goes out of scope)
	m_file->lock();
	utils::scope_guard unlocker([&]() { m_file->unlock(); });

	m_attribute_dom.set_value(value);
	m_file->need_update(true);
}

void 
files::xml_attribute::value(int32_t value)
{
	// Get hold of data synchronization mutex (will be unlocked when goes out of scope)
	m_file->lock();
	utils::scope_guard unlocker([&]() { m_file->unlock(); });

	m_attribute_dom.set_value(value);
	m_file->need_update(true);
}

void 
files::xml_attribute::value(uint32_t value)
{
	// Get hold of data synchronization mutex (will be unlocked when goes out of scope)
	m_file->lock();
	utils::scope_guard unlocker([&]() { m_file->unlock(); });

	m_attribute_dom.set_value(value);
	m_file->need_update(true);
}

void
files::xml_attribute::value(int64_t value)
{
	// Get hold of data synchronization mutex (will be unlocked when goes out of scope)
	m_file->lock();
	utils::scope_guard unlocker([&]() { m_file->unlock(); });

	m_attribute_dom.set_value(value);
	m_file->need_update(true);
}

void 
files::xml_attribute::value(uint64_t value)
{
	// Get hold of data synchronization mutex (will be unlocked when goes out of scope)
	m_file->lock();
	utils::scope_guard unlocker([&]() { m_file->unlock(); });

	m_attribute_dom.set_value(value);
	m_file->need_update(true);
}

void 
files::xml_attribute::value(float value)
{
	// Get hold of data synchronization mutex (will be unlocked when goes out of scope)
	m_file->lock();
	utils::scope_guard unlocker([&]() { m_file->unlock(); });

	m_attribute_dom.set_value(value);
	m_file->need_update(true);
}

void 
files::xml_attribute::value(double value)
{
	// Get hold of data synchronization mutex (will be unlocked when goes out of scope)
	m_file->lock();
	utils::scope_guard unlocker([&]() { m_file->unlock(); });

	m_attribute_dom.set_value(value);
	m_file->need_update(true);
}

//--------------------------------------------------------------------
// XML ELEMENT
// -------------------------------------------------------------------

files::xml_element::xml_element(files::xml_file* file, const pugi::xml_node& node)
						: Super(file)
{
	m_node_dom = node;
}

bool
files::xml_element::create_attribute(pugi::xml_attribute& pugi_attr, files::xml_attribute_interface** attribute)
{
	utils::ref_count_ptr<files::xml_attribute_interface> instance;
	try
	{
		instance = utils::make_ref_count_ptr<files::xml_attribute>(m_file, pugi_attr);
	}
	catch (...)
	{
		return false;
	}

	if (instance == nullptr)
		return false;

	instance->add_ref();
	*attribute = instance;
	return true;
}

bool 
files::xml_element::create_element(pugi::xml_node& pugi_node, files::xml_element_interface** element)
{
	utils::ref_count_ptr<files::xml_element_interface> instance;
	try
	{
		instance = utils::make_ref_count_ptr<files::xml_element>(m_file, pugi_node);
	}
	catch (...)
	{
		return false;
	}

	if (instance == nullptr)
		return false;

	instance->add_ref();
	*element = instance;
	return true;
}

bool 
files::xml_element::equals(xml_element_interface* elem_intf)
{
	xml_element* elem = (xml_element *)elem_intf;

	if (elem->m_node_dom.empty() && m_node_dom.empty())
		// Both attributes are empty. This means that they are equal
		return true;

	// At least one attributes is not empty. If one of them is, then they are not equal
	if (elem->m_node_dom.empty() || m_node_dom.empty())
		// One attribute is empty and the other one is not
		return false;

	return (m_node_dom == elem->m_node_dom);
}

bool 
files::xml_element::is_comment()
{
	return (m_node_dom.type() == pugi::node_comment);
}

bool 
files::xml_element::query_attribute(const char* name, files::xml_attribute_interface** attribute)
{
	if (name == nullptr)
		return false;

	// Get hold of data synchronization mutex (will be unlocked when goes out of scope)
	m_file->lock();
	utils::scope_guard unlocker([&]() { m_file->unlock(); });

	pugi::xml_attribute pugi_attr = m_node_dom.attribute(name);
	if (pugi_attr.empty())
		return false;
	return create_attribute(pugi_attr, attribute);
}

bool 
files::xml_element::query_attribute(size_t index, files::xml_attribute_interface** attribute)
{
	// Get hold of data synchronization mutex (will be unlocked when goes out of scope)
	m_file->lock();
	utils::scope_guard unlocker([&]() { m_file->unlock(); });

	int curr_indx = {};
	for (auto pugi_attr : m_node_dom.attributes())
	{
        if (static_cast<size_t>(curr_indx++) == index)
			return create_attribute(pugi_attr, attribute);
	}
	return false;	// Not found
}

bool 
files::xml_element::query_child(const char* name, files::xml_element_interface** element)
{
	if (name == nullptr)
		return false;

	// Get hold of data synchronization mutex (will be unlocked when goes out of scope)
	m_file->lock();
	utils::scope_guard unlocker([&]() { m_file->unlock(); });

	pugi::xml_node pugi_node = m_node_dom.child(name);

	if (pugi_node.empty())
		return false;

	return create_element(pugi_node, element);
}

bool files::xml_element::query_child(size_t index, const char* name, files::xml_element_interface** element)
{
	if (name == nullptr)
		return false;

	// Get hold of data synchronization mutex (will be unlocked when goes out of scope)
	m_file->lock();
	utils::scope_guard unlocker([&]() { m_file->unlock(); });
	int curr_indx = {};
	for (auto pugi_node : m_node_dom.children(name))
	{
		if (static_cast<size_t>(curr_indx++) == index)
			return create_element(pugi_node, element);
	}

	return false;
}

bool 
files::xml_element::query_child(size_t index, files::xml_element_interface** element)
{
	// Get hold of data synchronization mutex (will be unlocked when goes out of scope)
	m_file->lock();
	utils::scope_guard unlocker([&]() { m_file->unlock(); });

	int curr_indx = {};
	for (auto pugi_node : m_node_dom.children())
	{
        if (static_cast<size_t>(curr_indx++) == index)
			return create_element(pugi_node, element);
	}
	return false;	// Not found
}

bool 
files::xml_element::add_attribute(const char* name, const char* value)
{
	if (name == nullptr || value == nullptr)
		return false;

	// Get hold of data synchronization mutex (will be unlocked when goes out of scope)
	m_file->lock();
	utils::scope_guard unlocker([&]() { m_file->unlock(); });

	pugi::xml_attribute attr = m_node_dom.append_attribute(name);
	if (attr == nullptr)
		return false;

	attr.set_value(value);
	m_file->need_update(true);
	return true;
}

bool 
files::xml_element::add_child(const char* name, const char* value)
{
	if (name == nullptr)
		return false;

	// Get hold of data synchronization mutex (will be unlocked when goes out of scope)
	m_file->lock();
	utils::scope_guard unlocker([&]() { m_file->unlock(); });

	pugi::xml_node node = m_node_dom.append_child(name);
	if (node == nullptr)
		return false;

	if (value != nullptr)
		node.append_child(pugi::node_pcdata).set_value(value);

	m_file->need_update(true);
	return true;
}

bool 
files::xml_element::remove_attribute(const char* name)
{
	if (name == nullptr)
		return false;

	// Get hold of data synchronization mutex (will be unlocked when goes out of scope)
	m_file->lock();
	utils::scope_guard unlocker([&]() { m_file->unlock(); });

	if (m_node_dom.remove_attribute(name))
	{
		m_file->need_update(true);
		return true;
	}
	return false;
}

bool 
files::xml_element::remove_child(const char* name)
{
	if (name == nullptr)
		return false;

	// Get hold of data synchronization mutex (will be unlocked when goes out of scope)
	m_file->lock();
	utils::scope_guard unlocker([&]() { m_file->unlock(); });

	if (m_node_dom.remove_child(name))
	{
		m_file->need_update(true);
		return true;
	}
	return false;
}

const char*
files::xml_element::name() const
{
	return m_node_dom.name();
}

const char*
files::xml_element::value() const
{
	return m_node_dom.text().as_string();
}

bool
files::xml_element::value_as_bool(bool def_val) const
{
	return m_node_dom.text().as_bool(def_val);
}

int32_t
files::xml_element::value_as_int(int32_t def_val) const
{
	return m_node_dom.text().as_int(def_val);
}

uint32_t 
files::xml_element::value_as_uint(uint32_t def_val) const
{
	return m_node_dom.text().as_uint(def_val);
}

int64_t
files::xml_element::value_as_long(int64_t def_val) const
{
	return m_node_dom.text().as_llong(def_val);
}

uint64_t 
files::xml_element::value_as_ulong(uint64_t def_val) const
{
	return m_node_dom.text().as_ullong(def_val);
}

float
files::xml_element::value_as_float(float def_val) const
{
	return m_node_dom.text().as_float(def_val);
}

double
files::xml_element::value_as_double(double def_val) const
{
	return m_node_dom.text().as_double(def_val);
}

void
files::xml_element::value(const char* value)
{
	// Get hold of data synchronization mutex (will be unlocked when goes out of scope)
	m_file->lock();
	utils::scope_guard unlocker([&]() { m_file->unlock(); });

	m_node_dom.text().set(value);
	m_file->need_update(true);
}

void
files::xml_element::value(bool value)
{
	// Get hold of data synchronization mutex (will be unlocked when goes out of scope)
	m_file->lock();
	utils::scope_guard unlocker([&]() { m_file->unlock(); });

	m_node_dom.text().set(value);
	m_file->need_update(true);
}

void
files::xml_element::value(int32_t value)
{
	// Get hold of data synchronization mutex (will be unlocked when goes out of scope)
	m_file->lock();
	utils::scope_guard unlocker([&]() { m_file->unlock(); });

	m_node_dom.text().set(value);
	m_file->need_update(true);
}

void
files::xml_element::value(uint32_t value)
{
	// Get hold of data synchronization mutex (will be unlocked when goes out of scope)
	m_file->lock();
	utils::scope_guard unlocker([&]() { m_file->unlock(); });

	m_node_dom.text().set(value);
	m_file->need_update(true);
}

void
files::xml_element::value(int64_t value)
{
	// Get hold of data synchronization mutex (will be unlocked when goes out of scope)
	m_file->lock();
	utils::scope_guard unlocker([&]() { m_file->unlock(); });

	m_node_dom.text().set(value);
	m_file->need_update(true);
}

void
files::xml_element::value(uint64_t value)
{
	// Get hold of data synchronization mutex (will be unlocked when goes out of scope)
	m_file->lock();
	utils::scope_guard unlocker([&]() { m_file->unlock(); });

	m_node_dom.text().set(value);
	m_file->need_update(true);
}

void
files::xml_element::value(float value)
{
	// Get hold of data synchronization mutex (will be unlocked when goes out of scope)
	m_file->lock();
	utils::scope_guard unlocker([&]() { m_file->unlock(); });

	m_node_dom.text().set(value);
	m_file->need_update(true);
}

void
files::xml_element::value(double value)
{
	// Get hold of data synchronization mutex (will be unlocked when goes out of scope)
	m_file->lock();
	utils::scope_guard unlocker([&]() { m_file->unlock(); });

	m_node_dom.text().set(value);
	m_file->need_update(true);
}

//--------------------------------------------------------------------
// XML FILE
// -------------------------------------------------------------------

files::xml_file::xml_file(const char* file_path)
                  : Super(file_path),
                    m_need_update(false)
{
	bool res = m_file_dom.load_file(file_path, 
									pugi::parse_default | pugi::parse_comments,
									pugi::encoding_utf8);
    if(res == false)
        throw std::runtime_error("Failed loading XML file");
}

files::xml_file::~xml_file()
{
	// Flush before destruction
	flush();
}

bool
files::xml_file::create_element(const pugi::xml_node& pugi_node, files::xml_element_interface** element)
{
	utils::ref_count_ptr<files::xml_element_interface> instance;
	try
	{
		instance = utils::make_ref_count_ptr<files::xml_element>(this, pugi_node);
	}
	catch (...)
	{
		return false;
	}

	if (instance == nullptr)
		return false;

	instance->add_ref();
	*element = instance;
	return true;
}

bool
files::xml_file::flush()
{
	if (m_need_update == false)
		return true;    // Nothing to do. Exit gracefully

    // Get hold of file synchronization mutex (will be unlocked when goes out of scope)
    std::lock_guard<std::mutex> sync_file_guard(m_file_mutex);

    // Get hold of data synchronization mutex (will be unlocked when goes out of scope)
    std::lock_guard<std::mutex> sync_data_guard(m_data_mutex);

	if (m_file_dom.save_file(m_file_path.c_str()) == true)
	{
		m_need_update = false;
		return true;
	}
	return false;
}

bool 
files::xml_file::query_root(files::xml_element_interface** element)
{
	// Get hold of data synchronization mutex (will be unlocked when goes out of scope)
	std::lock_guard<std::mutex> sync_data_guard(m_data_mutex);

	// Actually, the pugi::xml_document represents the root of the file.
	// This method only creates xml_element that represents it
	pugi::xml_node pugi_node = m_file_dom;
	return create_element(pugi_node, element);
}

bool 
files::xml_file::query_element(const char* xpath, files::xml_element_interface** element)
{
	// Get hold of data synchronization mutex (will be unlocked when goes out of scope)
	std::lock_guard<std::mutex> sync_data_guard(m_data_mutex);

	// Query the element
	try
	{
		pugi::xpath_node xnode = m_file_dom.select_node(xpath);
		if (xnode == nullptr)
			return false;
		return create_element(xnode.node(), element); 
	}
	catch (...)
	{
		return false;
	}
}

bool
files::xml_file_interface::create(const char* file_path, files::xml_file_interface** file)
{
	if (file_path == nullptr || file == nullptr)
		return false;

	utils::ref_count_ptr<xml_file_interface> instance;
	try
	{
		instance = utils::make_ref_count_ptr<xml_file>(file_path);
	}
	catch (...)
	{
		return false;
	}

	if (instance == nullptr)
		return false;

	instance->add_ref();
	*file = instance;
	return true;
}
