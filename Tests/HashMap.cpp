#include "Test.hpp"

#include <HashMap.hpp>

using namespace CSTM;

DeclTest(hash_map, insert_access_remove)
{
	HashMap<size_t, std::string> map;
	map.insert(0, "Hello, World!");
	const auto& v = map[0];
	map.remove(0);
}
