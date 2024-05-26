#pragma once

#include "Utility.hpp"

#include <random>

namespace CSTM {

	template<typename H, typename T>
	concept Hasher = requires(T t)
	{
		{ H{}(t) } -> std::same_as<size_t>;
	};

	/*
	* SecureHash employs "secure" hash generation which incorporates a random seed (generated at runtime)
	* into the hash to ensure unpredictable hash values, which can help mitigate hash collision attacks as well as
	* hash flooding attacks.
	*/

	template<typename Key, Hasher<Key> Hash = std::hash<Key>>
	struct SecureHash
	{
		inline static uint32_t GlobalSeed = []
		{
			std::random_device rd;
			std::uniform_int_distribution<uint32_t> dist(0, std::numeric_limits<uint32_t>::max());
			return dist(rd);
		}();

		size_t operator()(const Key& key) const
		{
			return GlobalSeed ^ hasher(key);
		}

		CSTM_NoUniqueAddr Hash hasher;
	};

}
