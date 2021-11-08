#pragma once
#include <database/memory_database.h>
#include <utils/database.hpp>

#include <mutex>

namespace database
{
	class memory_row_impl :
		public utils::database::row_base<database::memory_row>
	{
	private:
		mutable std::mutex m_mutex;		
		size_t m_max_size;
		size_t m_current_size;		
		char* m_buffer;		
		bool m_unbounded_data_size;
		size_t m_unbounded_allocation_size;
		uint8_t m_write_priority;

		using row_callbacks_vector = std::vector<utils::ref_count_ptr<core::database::row_callback_interface>>;
		mutable utils::thread_safe_object<row_callbacks_vector> m_callbacks;				

	public:		
		memory_row_impl(const core::database::key& key, size_t size, char* buffer, const core::database::row_info &info, core::parsers::binary_metadata_interface* parser, core::database::table_interface* parent);
		memory_row_impl(const core::database::key& key, const core::database::row_info &info, core::parsers::binary_metadata_interface* parser, core::database::table_interface* parent);
		virtual ~memory_row_impl();
		
		virtual size_t data_size() const override;
		virtual uint8_t write_priority() const override;

		virtual bool read_bytes(void* buffer, size_t size) const override;
		virtual bool read_bytes(void* buff) const override;
		virtual bool write_bytes(const void* buffer, size_t size, bool force_report, uint8_t priority) override;
		virtual bool set_write_priority(uint8_t priority) override;
		// Overloading the new/delete operators is not actually reuried
		// We do so to because the factory method is allocating the memory
		// using 'data_row_impl::operator new' and delete will be called
		// whenever the object is destructed.
		// The following overloads allows us to make sure that allocation and deallocation is semetric.
		// Note that the implementation is anyway forwarding the calls to the default new/delete operators
		// so even if the memory was allocated by the default operator new, we're still on the safe side.

		// operator new: allocation function for single instance
		static void* operator new(std::size_t size);

		// operator new: placement function for single instance
		static void* operator new(std::size_t count, void* ptr);		

		// operator delete: deallocation function for single instance
		static void operator delete(void* ptr);				

		static void operator delete  (void* ptr, void* place);
	};
}