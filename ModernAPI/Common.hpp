#pragma once
#include <utils/ref_count_ptr.hpp>
#include <string>

namespace Common
{
	/// A non constructible class
	///Class the derived from this class can not be constructed 
	/// This is helpful when designing class that only has static function/variables and we want to avoid creating and instance to it
	/// @date	05/06/2018
	class NonConstructible
	{
	private:
		NonConstructible() = delete;
	};

	/// A core object wrapper.
	///This Class is very helpful on wrapping core objects into a nice ModernAPI interface
	/// it is taking ownership on the core pointer (assuming it is derived from ref_count_interface)
	/// and allow all kind of operators.
	/// every class that is derived from CoreObjectWrapper holding through CoreObjectWrapper a protected member name m_core_object which is a ref_cout_ptr of T.
	/// this allows all pointer management automatically and simple access and usage of T. 
	/// @date	05/06/2018
	///
	/// @tparam	T	Generic type parameter.
	template <typename T>
	class CoreObjectWrapper
	{
	protected:
		using CoreObjectWrapperBase = Common::CoreObjectWrapper<T>; //simple syntax to use CoreObjectWrapper avoid write the re-template
		utils::ref_count_ptr<T> m_core_object;

		/// Throw on empty core pointer
		///
		/// @date	05/06/2018
		///
		/// @exception	std::runtime_error	Raised when a runtime error if the core pointer is null.
		///
		/// @param	object_name	(Optional) Name of the object.
		inline void ThrowOnEmpty(const char* object_name = nullptr) const
		{
            if (this->Empty() == true)
			{
				std::string what = (object_name != nullptr) ? std::string("Empty ") + object_name : "Empty Object";
				throw std::runtime_error(what);
			}
		}

	public:
		using UnderlyingObjectType = T;

		virtual ~CoreObjectWrapper() = default;

		/// Default constructor - Allow creation of empty core wrapper 
		/// Allows creating the object in-advance and get an empty created CoreObjectWrapper which can be initated later in the code
		///
		/// @date	05/06/2018
		CoreObjectWrapper()
		{
			// Empty
		}

		/// Constructor that gets the core object pointer on creation
		///
		/// @date	05/06/2018
		/// 	
		/// @param [in]	core_object	If non-null, the core object.
		CoreObjectWrapper(T* core_object) :
			m_core_object(core_object)
		{			
		}

		/// Copy Constructor
		///
		/// @date	05/06/2018
		///
		/// @param	other	The other.
		CoreObjectWrapper(const CoreObjectWrapper<T>& other) :
			m_core_object(other.m_core_object)
		{
		}

		///Move Constructor
		///
		/// @date	05/06/2018
		///
		/// @param [in,out]	other	The other.
		CoreObjectWrapper(CoreObjectWrapper<T>&& other) :
			m_core_object(std::move(other.m_core_object))
		{
		}

		/// Assignment operator
		///
		/// @date	05/06/2018
		///
		/// @param	other	The other.
		///
		/// @return	A shallow copy of this object.
		virtual CoreObjectWrapper<T>& operator=(const CoreObjectWrapper<T>& other)
		{
			m_core_object = other.m_core_object;
			return *this;
		}

		/// Move assignment operator
		///
		/// @date	05/06/2018
		///
		/// @param [in,out]	other	The other.
		///
		/// @return	A shallow copy of this object.
		virtual CoreObjectWrapper<T>& operator=(CoreObjectWrapper<T>&& other)
		{
			m_core_object = std::move(other.m_core_object);
			return *this;
		}

		virtual bool operator==(std::nullptr_t)
		{
			return Empty();
		}

		virtual bool operator!=(std::nullptr_t)
		{
			return !(*this == nullptr);
		}

		/// Equality operator
		///
		/// @date	05/06/2018
		///
		/// @param	other	The other.
		///
		/// @return	True if the parameters are considered equivalent.
		virtual bool operator==(const CoreObjectWrapper<T>& other)
		{
			return (m_core_object == other.m_core_object);
		}

		/// Inequality operator
		///
		/// @date	05/06/2018
		///
		/// @param	other	The other.
		///
		/// @return	True if the parameters are not considered equivalent.
		virtual bool operator !=(const CoreObjectWrapper<T>& other)
		{
			return !(*this == other);
		}

		/// Explicit t* casting operator - 
		/// allows static cast to the core object pointer and preventing implicit cast to the core object pointer
		///
		/// @date	05/06/2018
		///
		/// @return	The result of the operation.
        virtual explicit operator T*() const
		{
			return m_core_object;
		}

        template <typename CASTABLE>
		explicit operator CASTABLE() const
        {
            return static_cast<CASTABLE>(static_cast<T*>(m_core_object));
        }

		/// check id Empty - means core pointer is null
		///
		/// @date	05/06/2018
		///
		/// @return	True if it succeeds, false if it fails.
		virtual bool Empty() const
		{
			return (m_core_object == nullptr);
		}

		/// Underlying object - getting the core object and assuming the caller takes ownership on the core object
		///
		/// @date	05/06/2018
		///
		/// @exception	std::runtime_error   	thrown if Empty
		/// @exception	std::invalid_argument	Thrown if core_object is null.
		///
		/// @param [in,out]	core_object	If non-null, the core object.
        virtual void UnderlyingObject(T** core_object) const
		{
			if (Empty() == true)
				throw std::runtime_error("Empty Object");

			if (core_object == nullptr)
				throw std::invalid_argument("core_object");

			(*core_object) = m_core_object;
			(*core_object)->add_ref();
		}
	};	
}
