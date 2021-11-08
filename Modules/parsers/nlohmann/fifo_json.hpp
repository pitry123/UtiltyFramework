#pragma once 

#include "fifo_map.hpp"
#include "json.hpp"

template<class K, class V, class dummy_compare, class A>
using fifo_map = nlohmann::fifo_map<K, V, nlohmann::fifo_map_compare<K>, A>;
using unordered_json = nlohmann::basic_json<fifo_map>;
