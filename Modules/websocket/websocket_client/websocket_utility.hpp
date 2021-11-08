#pragma once
#include <websocket/websocket_client.h>
#include <utils/ref_count_base.hpp>
#include <utils/ref_count_ptr.hpp>
#include <utils/disposable_ptr.hpp>
#include <random>

#include <boost/asio.hpp>

#include <boost/asio/steady_timer.hpp>
#include <atomic>
#include <chrono>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>

namespace websocket
{
	inline bool case_insensitive_equal(const std::string &str1, const std::string &str2) noexcept
	{
		return str1.size() == str2.size() &&
			std::equal(str1.begin(), str1.end(), str2.begin(), [](char a, char b)
		{
			return tolower(a) == tolower(b);
		});
	}

	class case_insensitive_equals
	{
	public:
		bool operator()(const std::string &str1, const std::string &str2) const noexcept
		{
			return case_insensitive_equal(str1, str2);
		}
	};

	// Based on https://stackoverflow.com/questions/2590677/how-do-i-combine-hash-values-in-c0x/2595226#2595226
	class case_insensitive_hash
	{
	public:
		std::size_t operator()(const std::string &str) const noexcept {
			std::size_t h = 0;
			std::hash<int> hash;
			for (auto c : str)
				h ^= hash(tolower(c)) + 0x9e3779b9 + (h << 6) + (h >> 2);
			return h;
		}
	};

	// case_insensitive_multimap CaseInsensitiveHash 
	using case_insensitive_multimap = std::unordered_multimap<std::string, std::string, case_insensitive_hash, case_insensitive_equals>;

	/// The buffer is consumed during send operations.
	//OutMessage 
	class out_message_data : public std::iostream
	{
	public:
		boost::asio::streambuf streambuf;

		out_message_data() noexcept : std::iostream(&streambuf) {}
		out_message_data(std::size_t capacity) noexcept : std::iostream(&streambuf)
		{
			streambuf.prepare(capacity);
		}

		/// Returns the size of the buffer
		std::size_t size() const noexcept
		{
			return streambuf.size();
		}
	};

	class in_message : public std::istream
	{
	public:
		unsigned char fin_rsv_opcode;
		std::size_t length;
		boost::asio::streambuf streambuf;

		std::size_t size() noexcept
		{
			return length;
		}

		/// Convenience function to return std::string. The stream buffer is consumed.
		std::string string() noexcept
		{
			try
			{
				std::string str;
				auto size = streambuf.size();
				str.resize(size);
				read(&str[0], static_cast<std::streamsize>(size));
				return str;
			}
			catch (...)
			{
				return std::string();
			}
		}

		in_message() noexcept :
			std::istream(&streambuf), length(0)
		{
		}

		in_message(unsigned char fin_rsv_opcode, std::size_t length) noexcept :
			std::istream(&streambuf), fin_rsv_opcode(fin_rsv_opcode), length(length)
		{
		}
	};

	class config_communication
	{
	public:
		/// Timeout on request handling. Defaults to no timeout.
		long timeout_request = 0;
		/// Idle timeout. Defaults to no timeout.
		long timeout_idle = 0;
		/// Maximum size of incoming messages. Defaults to architecture maximum.
		/// Exceeding this limit will result in a message_size error code and the connection will be closed.
		std::size_t max_message_size;
		/// Additional header fields to send when performing WebSocket upgrade.
		/// Use this variable to for instance set Sec-WebSocket-Protocol.
		case_insensitive_multimap header;
		/// Set proxy server (server:port)
		std::string proxy_server;

		config_communication() noexcept :
			max_message_size((std::numeric_limits<size_t>::max)())
		{
		}
	};

	/// Percent encoding and decoding
	class percent
	{
	public:
		/// Returns percent-encoded string
		static std::string encode(const std::string& value) noexcept
		{
			static auto hex_chars = "0123456789ABCDEF";

			std::string result;
			result.reserve(value.size()); // Minimum size of result

			for (auto &chr : value)
			{
				if (!((chr >= '0' && chr <= '9') || (chr >= 'A' && chr <= 'Z') || (chr >= 'a' && chr <= 'z') || chr == '-' || chr == '.' || chr == '_' || chr == '~'))
					result += std::string("%") + hex_chars[static_cast<unsigned char>(chr) >> 4] + hex_chars[static_cast<unsigned char>(chr) & 15];
				else
					result += chr;
			}

			return result;
		}

		/// Returns percent-decoded string
		static std::string decode(const std::string& value) noexcept
		{
			std::string result;
			result.reserve(value.size() / 3 + (value.size() % 3)); // Minimum size of result

			for (std::size_t i = 0; i < value.size(); ++i)
			{
				auto &chr = value[i];
				if (chr == '%' && i + 2 < value.size())
				{
					auto hex = value.substr(i + 1, 2);
					auto decoded_chr = static_cast<char>(std::strtol(hex.c_str(), nullptr, 16));
					result += decoded_chr;
					i += 2;
				}
				else if (chr == '+')
				{
					result += ' ';
				}
				else
				{
					result += chr;
				}
			}

			return result;
		}
	};

	class http_header
	{
	public:
		/// Parse header fields from stream
		static case_insensitive_multimap parse(std::istream &stream) noexcept
		{
			case_insensitive_multimap result;
			std::string line;
			std::size_t param_end;
			while (getline(stream, line) && (param_end = line.find(':')) != std::string::npos)
			{
				std::size_t value_start = param_end + 1;
				while (value_start + 1 < line.size() && line[value_start] == ' ')
					++value_start;
				if (value_start < line.size())
					result.emplace(line.substr(0, param_end), line.substr(value_start, line.size() - value_start - (line.back() == '\r' ? 1 : 0)));
			}

			return result;
		}

		class field_value
		{
		public:
			class semicolon_separated_attributes
			{
			public:
				/// Parse Set-Cookie or Content-Disposition from given header field value.
				/// Attribute values are percent-decoded.
				static case_insensitive_multimap parse(const std::string &value)
				{
					case_insensitive_multimap result;

					std::size_t name_start_pos = std::string::npos;
					std::size_t name_end_pos = std::string::npos;
					std::size_t value_start_pos = std::string::npos;
					for (std::size_t c = 0; c < value.size(); ++c)
					{
						if (name_start_pos == std::string::npos)
						{
							if (value[c] != ' ' && value[c] != ';')
								name_start_pos = c;
						}
						else
						{
							if (name_end_pos == std::string::npos)
							{
								if (value[c] == ';')
								{
									result.emplace(value.substr(name_start_pos, c - name_start_pos), std::string());
									name_start_pos = std::string::npos;
								}
								else if (value[c] == '=')
								{
									name_end_pos = c;
								}
							}
							else
							{
								if (value_start_pos == std::string::npos)
								{
									if (value[c] == '"' && c + 1 < value.size())
										value_start_pos = c + 1;
									else
										value_start_pos = c;
								}
								else if (value[c] == '"' || value[c] == ';')
								{
									result.emplace(value.substr(name_start_pos, name_end_pos - name_start_pos), percent::decode(value.substr(value_start_pos, c - value_start_pos)));
									name_start_pos = std::string::npos;
									name_end_pos = std::string::npos;
									value_start_pos = std::string::npos;
								}
							}
						}
					}

					if (name_start_pos != std::string::npos)
					{
						if (name_end_pos == std::string::npos)
						{
							result.emplace(value.substr(name_start_pos), std::string());
						}
						else if (value_start_pos != std::string::npos)
						{
							if (value.back() == '"')
								result.emplace(value.substr(name_start_pos, name_end_pos - name_start_pos), percent::decode(value.substr(value_start_pos, value.size() - 1)));
							else
								result.emplace(value.substr(name_start_pos, name_end_pos - name_start_pos), percent::decode(value.substr(value_start_pos)));
						}
					}

					return result;
				}
			};
		};
	};

	class request_message
	{
	public:
		/** Parse request line and header fields from a request stream.
		 *
		 * @param[in]  stream       Stream to parse.
		 * @param[out] method       HTTP method.
		 * @param[out] path         Path from request URI.
		 * @param[out] query_string Query string from request URI.
		 * @param[out] version      HTTP version.
		 * @param[out] header       Header fields.
		 *
		 * @return True if stream is parsed successfully, false if not.
		 */
		static bool parse(std::istream &stream, std::string &method, std::string &path, std::string &query_string, std::string &version, case_insensitive_multimap &header) noexcept
		{
			std::string line;
			std::size_t method_end;

			if (getline(stream, line) && (method_end = line.find(' ')) != std::string::npos)
			{
				method = line.substr(0, method_end);

				std::size_t query_start = std::string::npos;
				std::size_t path_and_query_string_end = std::string::npos;
				for (std::size_t i = method_end + 1; i < line.size(); ++i)
				{
					if (line[i] == '?' && (i + 1) < line.size())
					{
						query_start = i + 1;
					}
					else if (line[i] == ' ')
					{
						path_and_query_string_end = i;
						break;
					}
				}
				if (path_and_query_string_end != std::string::npos)
				{
					if (query_start != std::string::npos)
					{
						path = line.substr(method_end + 1, query_start - method_end - 2);
						query_string = line.substr(query_start, path_and_query_string_end - query_start);
					}
					else
					{
						path = line.substr(method_end + 1, path_and_query_string_end - method_end - 1);
					}

					std::size_t protocol_end;
					if ((protocol_end = line.find('/', path_and_query_string_end + 1)) != std::string::npos)
					{
						if (line.compare(path_and_query_string_end + 1, protocol_end - path_and_query_string_end - 1, "HTTP") != 0)
							return false;

						version = line.substr(protocol_end + 1, line.size() - protocol_end - 2);
					}
					else
					{
						return false;
					}

					header = http_header::parse(stream);
				}
				else
				{
					return false;
				}
			}
			else
			{
				return false;
			}

			return true;
		}
	};

	class response_message
	{
	public:
		/** Parse status line and header fields from a response stream.
		 *
		 * @param[in]  stream      Stream to parse.
		 * @param[out] version     HTTP version.
		 * @param[out] status_code HTTP status code.
		 * @param[out] header      Header fields.
		 *
		 * @return True if stream is parsed successfully, false if not.
		 */
		static bool parse(std::istream &stream, std::string &version, std::string &status_code, case_insensitive_multimap &header) noexcept
		{
			std::string line;
			std::size_t version_end;

			if (getline(stream, line) && (version_end = line.find(' ')) != std::string::npos)
			{
				if (5 < line.size())
					version = line.substr(5, version_end - 5);
				else
					return false;

				if ((version_end + 1) < line.size())
					status_code = line.substr(version_end + 1, line.size() - (version_end + 1) - (line.back() == '\r' ? 1 : 0));
				else
					return false;

				header = http_header::parse(stream);
			}
			else
			{
				return false;
			}

			return true;
		}
	};

#ifdef __SSE2__
#include <emmintrin.h>
	inline void spin_loop_pause() noexcept { _mm_pause(); }
#elif defined(_MSC_VER) && _MSC_VER >= 1800 && (defined(_M_X64) || defined(_M_IX86))
#include <intrin.h>
	inline void spin_loop_pause() noexcept { _mm_pause(); }
#else
	inline void spin_loop_pause() noexcept {}
#endif

	//ScopeRunner 
	class scope_runner
	{
		/// Scope count that is set to -1 if scopes are to be canceled.
		std::atomic<long> count;

	public:
		class shared_lock
		{
			friend class scope_runner;
			std::atomic<long>& count;

			shared_lock(std::atomic<long> &count) noexcept : count(count)
			{
			}

			shared_lock &operator=(const shared_lock &) = delete;
			shared_lock(const shared_lock &) = delete;

		public:
			~shared_lock() noexcept
			{
				count.fetch_sub(1);
			}
		};

		scope_runner() noexcept :
			count(0)
		{
		}

		/// Returns nullptr if scope should be exited, or a shared lock otherwise.
		/// The shared lock ensures that a potential destructor call is delayed until all locks are released.
		std::unique_ptr<shared_lock> continue_lock() noexcept
		{
			long expected = count;
			while (expected >= 0 && !count.compare_exchange_weak(expected, expected + 1))
				spin_loop_pause();

			if (expected < 0)
				return nullptr;
			else
				return std::unique_ptr<shared_lock>(new shared_lock(count));
		}

		/// Blocks until all shared locks are released, then prevents future shared locks.
		void stop() noexcept
		{
			long expected = 0;
			while (!count.compare_exchange_weak(expected, -1))
			{
				if (expected < 0)
					return;

				expected = 0;
				spin_loop_pause();
			}
		}
	};
}

namespace crypto
{
	namespace base64
	{
		static const std::string base64_chars =
			"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
			"abcdefghijklmnopqrstuvwxyz"
			"0123456789+/";

		static inline bool is_base64(unsigned char c)
		{
			return (isalnum(c) || (c == '+') || (c == '/'));
		}

		std::string base64_encode(unsigned char const* bytes_to_encode, unsigned int in_len) 
		{
			std::string ret;
			int i = 0;
			int j = 0;
			unsigned char char_array_3[3];
			unsigned char char_array_4[4];

			while (in_len--) 
			{
				char_array_3[i++] = *(bytes_to_encode++);
				if (i == 3) 
				{
					char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
					char_array_4[1] = static_cast<unsigned char>(((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4));
					char_array_4[2] = static_cast<unsigned char>(((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6));
					char_array_4[3] = char_array_3[2] & 0x3f;

					for (i = 0; (i < 4); i++)
						ret += base64_chars[char_array_4[i]];
					i = 0;
				}
			}

			if (i)
			{
				for (j = i; j < 3; j++)
					char_array_3[j] = '\0';

				char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
				char_array_4[1] = static_cast<unsigned char>(((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4));
				char_array_4[2] = static_cast<unsigned char>(((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6));

				for (j = 0; (j < i + 1); j++)
					ret += base64_chars[char_array_4[j]];

				while ((i++ < 3))
					ret += '=';

			}

			return ret;
		}

		std::string base64_decode(std::string const& encoded_string) 
		{
			size_t in_len = encoded_string.size();
			int i = 0;
			int j = 0;
			int in_ = 0;
            unsigned char char_array_4[4], char_array_3[3];
			std::string ret;

            while (in_len-- && (encoded_string[static_cast<size_t>(in_)] != '=') && is_base64((unsigned char)(encoded_string[static_cast<size_t>(in_)])))
			{
                char_array_4[i++] = (unsigned char)(encoded_string[static_cast<size_t>(in_)]); in_++;
				if (i == 4) {
					for (i = 0; i < 4; i++)
                        char_array_4[i] = (unsigned char)(base64_chars.find((char)char_array_4[i]) & 0xff);

                    char_array_3[0] = (unsigned char)((char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4));
                    char_array_3[1] = (unsigned char)(((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2));
                    char_array_3[2] = (unsigned char)(((char_array_4[2] & 0x3) << 6) + char_array_4[3]);

					for (i = 0; (i < 3); i++)
                        ret += (char)(char_array_3[static_cast<size_t>(i)]);
					i = 0;
				}
			}

			if (i) 
			{
				for (j = 0; j < i; j++)
                    char_array_4[j] = (unsigned char)base64_chars.find((char)(char_array_4[j])) & 0xff;

				char_array_3[0] = static_cast<unsigned char>((char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4));
				char_array_3[1] = static_cast<unsigned char>(((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2));

				for (j = 0; (j < i - 1); j++)
                    ret += (char)(char_array_3[j]);
			}

			return ret;
		}
	}

	namespace sha1 
	{
		// Rotate an integer value to left.
		inline unsigned int rol(unsigned int value, unsigned int steps) 
		{
			return ((value << steps) | (value >> (32 - steps)));
		}

		// Sets the first 16 integers in the buffert to zero.
		// Used for clearing the W buffert.
		inline void clearWBuffert(unsigned int * buffert)
		{
			for (int pos = 16; --pos >= 0;)
			{
				buffert[pos] = 0;
			}
		}

		inline void innerHash(unsigned int * result, unsigned int * w)
		{
			unsigned int a = result[0];
			unsigned int b = result[1];
			unsigned int c = result[2];
			unsigned int d = result[3];
			unsigned int e = result[4];

			int round = 0;

#define sha1macro(func,val){const unsigned int t = rol(a, 5) + (func)+e + val + w[round];e = d;d = c;c = rol(b, 30);b = a;a = t;}

			while (round < 16)
			{
				sha1macro((b & c) | (~b & d), 0x5a827999)
					++round;
			}

			while (round < 20)
			{
				w[round] = rol((w[round - 3] ^ w[round - 8] ^ w[round - 14] ^ w[round - 16]), 1);
				sha1macro((b & c) | (~b & d), 0x5a827999)
					++round;
			}

			while (round < 40)
			{
				w[round] = rol((w[round - 3] ^ w[round - 8] ^ w[round - 14] ^ w[round - 16]), 1);
				sha1macro(b ^ c ^ d, 0x6ed9eba1)
					++round;
			}

			while (round < 60)
			{
				w[round] = rol((w[round - 3] ^ w[round - 8] ^ w[round - 14] ^ w[round - 16]), 1);
				sha1macro((b & c) | (b & d) | (c & d), 0x8f1bbcdc)
					++round;
			}

			while (round < 80)
			{
				w[round] = rol((w[round - 3] ^ w[round - 8] ^ w[round - 14] ^ w[round - 16]), 1);
				sha1macro(b ^ c ^ d, 0xca62c1d6)
					++round;
			}

#undef sha1macro

			result[0] += a;
			result[1] += b;
			result[2] += c;
			result[3] += d;
			result[4] += e;
		}


		/// Calculate a SHA1 hash
		/**
		 * @param src points to any kind of data to be hashed.
		 * @param bytelength the number of bytes to hash from the src pointer.
		 * @param hash should point to a buffer of at least 20 bytes of size for storing
		 * the sha1 result in.
		 */
		inline char * calc(void const * src, size_t bytelength, char * hash) 
		{
			// Init the result array.
			//char hash[21];
			unsigned int result[5] = { 0x67452301, 0xefcdab89, 0x98badcfe,
									   0x10325476, 0xc3d2e1f0 };

			// Cast the void src pointer to be the byte array we can work with.
			unsigned char const * sarray = (unsigned char const *)src;

			// The reusable round buffer
			unsigned int w[80];

			// Loop through all complete 64byte blocks.

			size_t endCurrentBlock;
			size_t currentBlock = 0;

			if (bytelength >= 64) 
			{
				size_t const endOfFullBlocks = bytelength - 64;

				while (currentBlock <= endOfFullBlocks) 
				{
					endCurrentBlock = currentBlock + 64;

					// Init the round buffer with the 64 byte block data.
					for (int roundPos = 0; currentBlock < endCurrentBlock; currentBlock += 4)
					{
						// This line will swap endian on big endian and keep endian on
						// little endian.
						w[roundPos++] = (unsigned int)sarray[currentBlock + 3]
							| (((unsigned int)sarray[currentBlock + 2]) << 8)
							| (((unsigned int)sarray[currentBlock + 1]) << 16)
							| (((unsigned int)sarray[currentBlock]) << 24);
					}

					innerHash(result, w);
				}
			}

			// Handle the last and not full 64 byte block if existing.
			endCurrentBlock = bytelength - currentBlock;
			clearWBuffert(w);
			size_t lastBlockBytes = 0;
			for (;lastBlockBytes < endCurrentBlock; ++lastBlockBytes) 
			{
				w[lastBlockBytes >> 2] |= (unsigned int)sarray[lastBlockBytes + currentBlock] << ((3 - (lastBlockBytes & 3)) << 3);
			}

            w[static_cast<size_t>(lastBlockBytes >> 2)] |= static_cast<unsigned int>(0x80 << ((3 - (lastBlockBytes & 3)) << 3));
			if (endCurrentBlock >= 56) 
			{
				innerHash(result, w);
				clearWBuffert(w);
			}

			w[15] = static_cast<unsigned int>(bytelength) << 3;
			innerHash(result, w);

			// Store hash in result pointer, and make sure we get in in the correct
			// order on both endian models.
			hash[20] = 0;
			for (int hashByte = 20; --hashByte >= 0;) 
			{
				hash[hashByte] = static_cast<char>((result[hashByte >> 2] >> (((3 - hashByte) & 0x3) << 3)) & 0xff);
			}
			return hash;
		}
	}// namespace sha1
}

