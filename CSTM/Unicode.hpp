#pragma once

#include "Utility.hpp"

#include <array>
#include <bitset>
#include <cstdint>

namespace CSTM {

	constexpr std::array<byte, 4> utf32_to_utf8(uint32_t codePoint, uint32_t& outCodePointByteCount)
	{
		std::array<byte, 4> result{};

		if (codePoint <= 0x7F)
		{
			result[0] = codePoint;
			outCodePointByteCount = 1;
		}
		else if (codePoint <= 0x7FF)
		{
			result[0] = 0xC0 | (codePoint >> 6);
			result[1] = 0x80 | (codePoint & 0x3F);
			outCodePointByteCount = 2;
		}
		else if (codePoint <= 0xFFFF)
		{
			result[0] = 0xE0 | (codePoint >> 12);
			result[1] = 0x80 | ((codePoint >> 6) & 0x3F);
			result[2] = 0x80 | (codePoint & 0x3F);
			outCodePointByteCount = 3;
		}
		else if (codePoint <= 0x10FFFF)
		{
			result[0] = 0xF0 | (codePoint >> 18);
			result[1] = 0x80 | ((codePoint >> 12) & 0x3F);
			result[2] = 0x80 | ((codePoint >> 6) & 0x3F);
			result[3] = 0x80 | (codePoint & 0x3F);
			outCodePointByteCount = 4;
		}

		return result;
	}

	constexpr Optional<uint8_t> get_trailing_byte_count(const byte b)
	{
		// NOTE(Peter): bitset has the first bit at index 7 and goes in reverse
		const std::bitset<8> bits(b);

		// If the first bit is not set this is most likely ASCII
		if (!bits[7])
		{
			return 0;
		}

		// If the second bit is NOT set at this point this means *current* isn't a leading byte
		// so just return NullOpt
		if (!bits[6])
		{
			return NullOpt;
		}

		// Check the number of bits after the second bit to figure out how many
		// trailing bytes we actually have in this code point.
		// We already know we have at least one from bit 2 being
		// set, so we can simply skip checking that one
		uint8_t trailingByteCount = 1;

		for (size_t i = 5; i >= 4; i--)
		{
			if (!bits[i])
			{
				break;
			}

			trailingByteCount++;
		}

		return trailingByteCount;
	}

	constexpr Optional<uint32_t> utf8_to_utf32(std::array<byte, 4> bytes, uint32_t& outCodePointByteCount)
	{
		auto as_trailing_byte = [](byte b)
		{
			// Make sure the byte starts with 0b10xx_xxxx
			TheiaAssert(b >> 6 == 0b10);

			// Only return the data, ignoring the continuation indicator (0b10)
			return b & 0b0011'1111;
		};

		auto trailingByteCount = get_trailing_byte_count(bytes[0]);

		if (!trailingByteCount.has_value())
		{
			// First byte isn't a leading byte
			return NullOpt;
		}

		std::array<byte, 4> codePointBytes{};

		switch (trailingByteCount.value())
		{
		case 0:
		{
			codePointBytes[0] = bytes[0];
			break;
		}
		case 1:
		{
			const byte b0 = bytes[0] & 0b0001'1111;
			const byte b1 = as_trailing_byte(bytes[1]);

			codePointBytes[0] = b1 | b0 << 6;
			codePointBytes[1] = b1 >> 2;
			break;
		}
		case 2:
		{
			const byte b0 = bytes[0] & 0b0000'1111;
			const byte b1 = as_trailing_byte(bytes[1]);
			const byte b2 = as_trailing_byte(bytes[2]);

			codePointBytes[0] = b2 | b1 << 6;
			codePointBytes[1] = b1 >> 2 | b0 << 4;
			break;
		}
		case 3:
		{
			const byte b0 = bytes[0] & 0b0000'0111;
			const byte b1 = as_trailing_byte(bytes[1]);
			const byte b2 = as_trailing_byte(bytes[2]);
			const byte b3 = as_trailing_byte(bytes[3]);

			codePointBytes[0] = b3 | b2 << 6;
			codePointBytes[1] = b2 >> 2 | b1 << 4;
			codePointBytes[2] = b0 << 2 | b1 >> 4;
			break;
		}
		default:
			TheiaAssert(false);
			break;
		}

		outCodePointByteCount = trailingByteCount.value() + 1;

		return from_little_endian(codePointBytes);
	}

	constexpr bool is_leading_byte(const byte b)
	{
		// NOTE(Peter): bitset has the first bit at index 7 and goes in reverse
		const std::bitset<8> bits(b);

		if (bits[7] && !bits[6])
		{
			return false;
		}

		return true;
	}

	constexpr bool is_ascii_upper_alpha_code_point(uint32_t codePoint)
	{
		return codePoint >= 0x0041 && codePoint <= 0x005A;
	}

	constexpr bool is_ascii_lower_alpha_code_point(uint32_t codePoint)
	{
		return codePoint >= 0x0061 && codePoint <= 0x007A;
	}

	constexpr bool is_ascii_digit(uint32_t codePoint)
	{
		return codePoint >= 0x0030 && codePoint <= 0x0039;
	}

	constexpr bool is_ascii_alpha_code_point(uint32_t codePoint)
	{
		return is_ascii_upper_alpha_code_point(codePoint) || is_ascii_lower_alpha_code_point(codePoint);
	}

	constexpr bool is_ascii_alphanumeric_code_point(uint32_t codePoint)
	{
		return is_ascii_alpha_code_point(codePoint) || is_ascii_digit(codePoint);
	}

	constexpr uint32_t ascii_to_lower_code_point(uint32_t codePoint)
	{
		if (!is_ascii_alpha_code_point(codePoint) || is_ascii_lower_alpha_code_point(codePoint))
		{
			return codePoint;
		}

		return codePoint + 0x0020;
	}

}
