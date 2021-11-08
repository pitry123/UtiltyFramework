#pragma once
#include <utils/ref_count_base.hpp>
#include <rules/rules.h>
#include <map>
#include <iostream>
#include <vector>
namespace utils
{
	namespace rules
	{
		using rules_callback_func = std::function<double()>;
		class rule_func_callback : public utils::ref_count_base<core::rules::rule_func_callback_interface>
		{
		private:
			rules_callback_func m_func;
			std::string m_name;
		public:
			rule_func_callback(const char* name, rules_callback_func func) :
				m_func(func),
				m_name(name)
			{}
			bool execute(double& retval) override
			{
				if (m_func == nullptr)
					return false;

				retval = m_func();
				return true;
			}
			virtual const char* name() override
			{
				return m_name.c_str();
			}
		};

		template <typename T>
		class rules_data_and_types_base : public ref_count_base<T>
		{
		protected:
			std::map<std::string, utils::ref_count_ptr<core::rules::rule_func_callback_interface>> m_funcMap;
			std::map<std::string, int64_t> m_enumMap;
		
			bool add_function_map(const char* name, std::function<double()> func)
			{
				auto it = m_funcMap.emplace(name, utils::make_ref_count_ptr<rules::rule_func_callback>(name,func));
				return it.second;
			}

		public:
					
			virtual bool get_enumeration(const char* enum_string, int64_t &val) override
			{
				auto it = m_enumMap.find(std::string(enum_string));
				if(it != m_enumMap.end())
				{
					val = it->second;
					return true;
				}
				return false;
			}

			virtual bool get_enumeration(const char* enum_name_val, const char* enum_type_name, int64_t& val) override
			{
				return get_enumeration(enum_name_val, val);
			}
			virtual bool query_function(const char* func_string, core::rules::rule_func_callback_interface** callback) override
			{
				if (callback == nullptr)
					return false;

				if (m_funcMap.find(std::string(func_string)) == m_funcMap.end())
				{
					std::cout << __func__ << ": " << std::string(func_string) << " doesn't exist in the FuncTable\n";
					return false;
				}

				*callback = m_funcMap[std::string(func_string)];
				(*callback)->add_ref();
				return true;
			}

			virtual bool add_rule_enable_row(size_t rule_id, const char* row_name) override
			{
				return true;
			}

			virtual bool add_rule_existence_row(size_t rule_id, const char* row_name) override
			{
				return true;
			}

			std::function<double()> get_function(const char* func_string)
			{
				rules::rules_callback_func func = nullptr;
                utils::ref_count_ptr<core::rules::rule_func_callback_interface> callback;
				if (query_function(func_string, &callback))
				{
					func = rules::rules_callback_func([callback]() {
						double retval;
						if (false == callback->execute(retval))
							throw std::runtime_error("execute failed");

						return retval;
					});
				}
				else
					throw std::runtime_error("function does not exist");
				return func;
			}
			
		};

		inline bool get_node_and_offset_by_path(utils::ref_count_ptr<core::parsers::binary_parser_interface>& parser,
			const std::vector<std::string>& path_vector, core::parsers::binary_node_interface** node, size_t& offset)
		{
			size_t i = 0;
			if (path_vector.size() == 0)
				return false;

			utils::ref_count_ptr<core::parsers::binary_parser_interface> internal_parser;
			while (i < path_vector.size() - 1)
			{
				if (parser->read_complex(path_vector[i].c_str(), &internal_parser))
				{
					parser = nullptr;
					parser = internal_parser;
					internal_parser = nullptr;
				}
				else
				{
					return false;
				}
				i++;
			}

			utils::ref_count_ptr<const core::parsers::binary_metadata_interface> inetenal_metadata;

			if (parser->query_metadata(&inetenal_metadata))
			{
				inetenal_metadata->query_node(path_vector[path_vector.size() - 1].c_str(), node);
				offset = parser->offset() + (*node)->offset();
				return true;
			}

			return false;
		}
	}
}