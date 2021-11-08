#include <core/guid.h>
#include <utils/ref_count_ptr.hpp>
#include <utils/ref_count_base.hpp>

#include <mutex>

#ifdef __GNUC__
#   pragma GCC diagnostic ignored "-Wconversion-null"
#endif

#include <boost/uuid/random_generator.hpp>


class guid_generator_impl : public utils::ref_count_base<core::guid_generator>
{
private:
	boost::uuids::random_generator m_generator;

public:
	core::guid generate() override
	{
		boost::uuids::uuid uuid = m_generator();
		core::guid retval = {};

#ifndef BOOST_NO_CXX11_CONSTEXPR
		static_assert(boost::uuids::uuid::static_size() == sizeof(core::guid), "Cannot convert boost::uuids::uuid to core::guid. Different size...");
#endif

		std::memcpy(&retval, &uuid, sizeof(core::guid));
		return retval;
	}
};

static utils::ref_count_ptr<core::guid_generator> guid_instance;

bool core::guid_generator::instance(core::guid_generator** instance)
{
	if (instance == nullptr)
		return false;

	static std::once_flag guid_flag;

	if (guid_instance == nullptr)
	{
		try
		{
			std::call_once(guid_flag, [&]()
			{
				guid_instance = utils::make_ref_count_ptr<guid_generator_impl>();
			});
		}
		catch (...)
		{
			return false;
		}
	}

	if (guid_instance == nullptr)
		return false;

	guid_instance->add_ref();
	*instance = guid_instance;
	return true;
}
