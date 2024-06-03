#pragma once

#include "Hash.hpp"
#include "Result.hpp"

#include <vector>
#include <list>
#include <memory>
#include <initializer_list>

namespace CSTM {

	/*
	 * Hash map implementation meant to solve some of the design flaws of std::unordered_map:
	 *	1. The subscript operator of this hash map will NEVER modify the map itself, e.g it will not insert a key
	 *		if it doesn't already exist, instead it will throw
	 *
	 *	2. Insertions are ALWAYS an explicit operation, e.g the only way a KV pair can be
	 *		inserted is via an explicit call to the Insert function
	 *
	 *	3. We utilize "secure" hash generation which incorporates a random seed (generated at runtime) into the key hash
	 *		to ensure unpredictable hash values, which can help mitigate hash collision attacks as well as
	 *		hash flooding attacks. Incorporating a random seed *can* also result in a more uniform distribution of hash
	 *		values across the map.
	 *		This decision was primarily inspired by Swifts' Hashable implementation.
	 */

	template<typename Key, typename Value, Hasher<Key> Hash>
	class BasicHashMap
	{
		using BucketType = std::list<std::pair<Key, Value>>;
		using BucketList = std::unique_ptr<BucketType[]>;

	public:
		static constexpr size_t InitialBucketCount = 10;
		static constexpr double MaxLoadFactor = 1.0;

		BasicHashMap()
			: BasicHashMap(InitialBucketCount)
		{}

		explicit BasicHashMap(size_t bucketCount)
			: m_buckets(new BucketType[bucketCount]), m_element_count(0), m_bucket_count(bucketCount) {}

		BasicHashMap(const BasicHashMap& other) noexcept
		{
			copy_construct(other);
		}

		BasicHashMap(BasicHashMap&& other) noexcept
		{
			move_construct(std::forward<BasicHashMap>(other));
		}

		// FIXME(Peter): This constructor causes strange behavior, probably due to being in a weird state
		BasicHashMap(std::initializer_list<std::pair<Key, Value>> elements)
			: BasicHashMap()
		{
			for (const auto& kv : elements)
			{
				insert(kv.first, kv.second);
			}

			force_rehash(m_bucket_count);
		}

		BasicHashMap& operator=(const BasicHashMap& other) noexcept
		{
			copy_construct(other);
			return *this;
		}

		BasicHashMap& operator=(BasicHashMap&& other) noexcept
		{
			move_construct(std::forward<BasicHashMap>(other));
			return *this;
		}

		void insert(const Key& key, const Value& value)
		{
			find_key_bucket(key)
				.template throw_on_value<std::runtime_error>("Key already present in map!");

			try_rehash();

			size_t bucketIndex = get_bucket_index(key);
			m_buckets[bucketIndex].emplace_back(std::make_pair(key, value));
			m_element_count++;
		}

		void remove(const Key& key)
		{
			auto it = find_key_bucket(key)
				.template throw_on_error<std::runtime_error>("Key not present in map!")
				.value();

			it.first.erase(it.second);

			m_element_count--;
		}

		[[nodiscard]]
		bool contains(const Key& key) const noexcept
		{
			return find_key_bucket(key).has_value();
		}

		[[nodiscard]]
		size_t element_count() const noexcept { return m_element_count; }

		[[nodiscard]]
		size_t bucket_count() const noexcept { return m_bucket_count; }

		[[nodiscard]]
		bool is_empty() const noexcept { return m_element_count == 0; }

		void clear()
		{
			for (size_t i = 0; i < m_bucket_count; i++)
			{
				m_buckets[i].clear();
			}

			m_element_count = 0;
		}

		[[nodiscard]]
		decltype(auto) operator[](this auto&& self, const Key& key)
		{
			using Self = decltype(self);
			return std::forward<Self>(self).at(key);
		}

		[[nodiscard]]
		decltype(auto) at(this auto&& self, const Key& key)
		{
			using Self = decltype(self);

			auto it = std::forward<Self>(self)
				.find_key_bucket(key)
				.template throw_on_error<std::runtime_error>("Key not found!")
				.value();

			return std::forward_like<Self>(it.second->second);
		}

	private:
		decltype(auto) find_key_bucket(this auto&& self, const Key& key) noexcept
		{
			using Self = decltype(self);

			size_t bucketIndex = std::forward<Self>(self).get_bucket_index(key);
			auto&& bucket = std::forward<Self>(self).m_buckets[bucketIndex];

			using BucketType = decltype(bucket);

			auto it = std::ranges::find_if(std::forward<BucketType>(bucket), [&key](const auto& kv)
			{
				return kv.first == key;
			});

			using BucketIt = std::pair<BucketType&, decltype(it)>;

			if (it == bucket.end())
			{
				return Result<BucketIt, NullType>{ Null };
			}

			return Result<BucketIt, NullType>{ BucketIt{ bucket, it } };
		}

		void try_rehash()
		{
			if (static_cast<double>(m_element_count) / static_cast<double>(m_bucket_count) <= MaxLoadFactor)
			{
				return;
			}

			// TODO(Peter): Implement more optimal rules for bucket growth
			//				MSVC will try to grow by x8 initally, unsure what libstdc++ does
			force_rehash(m_bucket_count * 2 + 1);
		}

		void force_rehash(size_t bucketCount)
		{
			BucketList buckets(new BucketType[bucketCount]);

			for (size_t i = 0; i < m_bucket_count; i++)
			{
				for (const auto& pair : m_buckets[i])
				{
					size_t bucketIndex = compute_bucket_index(bucketCount, pair.first);
					buckets[bucketIndex].emplace_back(pair);
				}
			}

			m_buckets = std::move(buckets);
			m_bucket_count = bucketCount;
		}

		size_t get_bucket_index(const Key& key) const noexcept
		{
			return compute_bucket_index(m_bucket_count, key);
		}

		size_t compute_bucket_index(size_t bucketCount, const Key& key) const noexcept
		{
			return m_hasher(key) % bucketCount;
		}

		void copy_construct(const BasicHashMap& other) noexcept
		{
			m_buckets.reset(new BucketType[other.m_bucket_count]);
			m_hasher = other.m_hasher;
			m_element_count = other.m_element_count;
			m_bucket_count = other.m_bucket_count;

			for (size_t i = 0; i < m_bucket_count; i++)
			{
				const auto& srcBucket = other.m_buckets[i];
				auto& dstBucket = m_buckets[i];
				dstBucket.resize(srcBucket.size());
				std::ranges::copy(srcBucket.begin(), srcBucket.end(), std::back_inserter(dstBucket));
			}
		}

		void move_construct(BasicHashMap&& other) noexcept
		{
			m_buckets = std::move(other.m_buckets);
			m_hasher = std::move(other.m_hasher);
			m_element_count = other.m_element_count;
			m_bucket_count = other.m_bucket_count;

			other.m_element_count = 0;
			other.m_bucket_count = 0;
		}

	private:
		BucketList m_buckets;
		CSTM_NoUniqueAddr Hash m_hasher;

		size_t m_element_count;
		size_t m_bucket_count;
	};

	template<typename Key, typename Value, Hasher<Key> Hash = std::hash<Key>>
	using HashMap = BasicHashMap<Key, Value, SecureHash<Key, Hash>>;

	// NOTE(Peter): Hash map that doesn't use SecureHash to ensure unpredictable hash generation.
	//				I don't recommend utilizing this for anything other than tests where deterministic hash values
	//				are required.
	template<typename Key, typename Value, Hasher<Key> Hash = std::hash<Key>>
	using DeterministicHashMap = BasicHashMap<Key, Value, Hash>;
}
