#include "texturehash.h"

namespace TextureHash
{
	namespace Omzy
	{
		uint64 Hash_Algorithm_1(const cv::Mat& img)
		{
			uint64 hash = 0;
			float first_val, val = 0, last_val = 0;

			cv::Vec3b pixel = img.at<cv::Vec3b>(hash1[0].y, hash1[0].x);
			first_val = last_val = (pixel[0] + pixel[1] + pixel[2]) / 3;

			const HashCoord* coord = hash1;
			for (int i = 0; i < 64; i++, coord++) {

				// get average pixel value
				if (coord->x < img.cols && coord->y < img.rows) {
					cv::Vec3b pixel = img.at<cv::Vec3b>(coord->y, coord->x);
					val = (pixel[0] + pixel[1] + pixel[2]) / 3;
				} else
					val = 0;

				// compare to last value for hash
				hash <<= 1;
				if (val >= last_val) hash |= 1;

				last_val = val;
			}

			// do wrap-around comparison
			if (first_val >= val) hash |= 0x8000000000000000;

			return hash;
		}


		uint64 Old_Hash_Algorithm_1(const cv::Mat& img)
		{
			int x, y;
			int pix = 0;
			float pixval[64] = { 0 };
			for (x = 0; x < 8; x++) //pixvals 0->31
			{
				for (y = 0; y < 4; y++) {
					if (x * BLOCKSIZE < img.cols && y * BLOCKSIZE < img.rows) {
						cv::Vec3b pixel = img.at<cv::Vec3b>(y * BLOCKSIZE, x * BLOCKSIZE);
						pixval[pix] = (pixel[0] + pixel[1] + pixel[2]) / 3;
					}
					pix++;
				}
			}

			for (x = 0; x < 2; x++) //pixvals 32->55
			{
				for (y = 4; y < 16; y++) {
					if (x * BLOCKSIZE < img.cols && y * BLOCKSIZE < img.rows) {
						cv::Vec3b pixel = img.at<cv::Vec3b>(y * BLOCKSIZE, x * BLOCKSIZE);
						pixval[pix] = (pixel[0] + pixel[1] + pixel[2]) / 3;
					}
					pix++;
				}
			}

			for (x = 3; x < 7; x += 3) //pixvals 56->63, note +=3
			{
				for (y = 5; y < 15; y += 3)	//note +=3
				{
					if (x * BLOCKSIZE < img.cols && y * BLOCKSIZE < img.rows) {
						cv::Vec3b pixel = img.at<cv::Vec3b>(y * BLOCKSIZE, x * BLOCKSIZE);
						pixval[pix] = (pixel[0] + pixel[1] + pixel[2]) / 3;
					}
					pix++;
				}
			}

			uint64 hashval = 0;
			float lastpixel = pixval[63];
			for (int i = 0; i < 64; i++) {
				hashval *= 2;
				if ((pixval[i] - lastpixel) >= 0) { hashval++; }
				lastpixel = pixval[i];
			}

			return hashval;
		}
	}

	namespace FNV
	{

		uint64 FNV_Full(const cv::Mat& img)
		{
			uint64 hash = FNV_OFFSET_BASIS_64;

			for (int y = 0; y < img.rows; y++) {
				cv::Mat row = img.row(y);
				for (int x = 0; x < row.cols; x++) {
					cv::Vec3b pixel = row.at<cv::Vec3b>(0, x);

					hash ^= pixel[0];
					hash *= FNV_OFFSET_PRIME_64;
					hash ^= pixel[1];
					hash *= FNV_OFFSET_PRIME_64;
					hash ^= pixel[2];
					hash *= FNV_OFFSET_PRIME_64;
				}
			}

			return hash;
		}

		uint64 FNV_Hash(const cv::Mat& img, const HashCoord* coords, const size_t len, bool use_RGB)
		{
			uint64 hash = FNV_OFFSET_BASIS_64;

			size_t coord_count = 0;
			const HashCoord* coord = coords;
			for (int i = 0; i < len; i++, coord++) {
				uchar val = 0, red = 0, green = 0, blue = 0;
				if (coord->x < img.cols && coord->y < img.rows) {
					cv::Vec3b pixel = img.at<cv::Vec3b>(coord->y, coord->x);
					if (use_RGB) {
						red = pixel[0];
						green = pixel[1];
						blue = pixel[2];
					} else {
						val = round((pixel[0] + pixel[1] + pixel[2]) / 3);
					}
				}

				if (use_RGB) {																// hash each R,G,B individually
					hash ^= (uchar)red;
					hash *= FNV_OFFSET_PRIME_64;
					hash ^= (uchar)green;
					hash *= FNV_OFFSET_PRIME_64;
					hash ^= (uchar)blue;
					hash *= FNV_OFFSET_PRIME_64;
				} else {																	// hash RGB average
					hash ^= val;
					hash *= FNV_OFFSET_PRIME_64;
				}
			}

			return hash;
		}


		uint64 FNV_Hash(const cv::Mat& img, std::deque<HashCoord> coords, bool use_RGB)
		{
			uint64 hash = FNV_OFFSET_BASIS_64;

			float red = 0, green = 0, blue = 0;
			size_t coord_count = 0;
			for (HashCoord coord : coords) {
				uchar val = 0;
				if (coord.x < img.cols && coord.y < img.rows) {
					cv::Vec3b pixel = img.at<cv::Vec3b>(coord.y, coord.x);
					if (use_RGB) {
						if (coord.c != coord.none) {
							val = pixel[coord.c];
						} else {
							red = pixel[0];
							green = pixel[1];
							blue = pixel[2];
						}
					} else {
						val = round((pixel[0] + pixel[1] + pixel[2]) / 3);
					}
				}

				if (use_RGB && coord.c == coord.none) {										// hash each R,G,B individually
					hash ^= (uchar)red;
					hash *= FNV_OFFSET_PRIME_64;
					hash ^= (uchar)green;
					hash *= FNV_OFFSET_PRIME_64;
					hash ^= (uchar)blue;
					hash *= FNV_OFFSET_PRIME_64;
				} else {																	// hash RGB average
					hash ^= val;
					hash *= FNV_OFFSET_PRIME_64;
				}
			}

			return hash;
		}

		uint64 FNV_Hash(const cv::Mat& img, bool use_RGB)
		{
			return FNV_Hash(img, COORDS, COORDS_LEN, use_RGB);
		}

		uint64 uint64pow(uint64 base, int exp)
		{
			uint64 result = 1;
			for (int i = 0; i < exp; i++)
				result *= base;

			return result;
		}

		// hash upper, lower, and combined separately 
		uint64 FNV_Hash_Combined_64(cv::Mat img, uint64& hash_upper, uint64& hash_lower, const HashCoord* coords, const int len, bool use_RGB)
		{
			hash_lower = (img.rows > VRAM_DIM / 2) ? (use_RGB) ? FNV_NOUPPER_RGB_BASIS_64 : FNV_NOUPPER_BASIS_64 : 0;

			uint64 hash = FNV_OFFSET_BASIS_64;

			float red = 0, green = 0, blue = 0;
			size_t coord_count = 0;
			for (int i = 0; i < len; i++) {
				HashCoord coord = coords[i];
				unsigned char val = 0;
				if (coord.x < img.cols && coord.y < img.rows) {
					cv::Vec3b pixel = img.at<cv::Vec3b>(coord.y, coord.x);
					val = round((pixel[0] + pixel[1] + pixel[2]) / 3);

					// keep track of RGB sums of pixels
					if (use_RGB) {
						red += pixel[0];
						green += pixel[1];
						blue += pixel[2];
					}
				}

				hash ^= val;
				hash *= FNV_OFFSET_PRIME_64;
				coord_count++;
			}

			// factor RGB averages into hash
			if (use_RGB) {
				red /= coord_count;
				green /= coord_count;
				blue /= coord_count;

				hash ^= (uchar)round(red);
				hash *= FNV_OFFSET_PRIME_64;
				hash ^= (uchar)round(green);
				hash *= FNV_OFFSET_PRIME_64;
				hash ^= (uchar)round(blue);
				hash *= FNV_OFFSET_PRIME_64;
			}

			// set hash_upper equal to hash thus far
			hash_upper = hash;

			if (hash_lower) {																// make sure texture is big enough to hash lower
				// adjust hash_upper to include lower blank object
				hash_upper *= (use_RGB) ? FNV_NOLOWER_RGB_FACTOR_64 : FNV_NOLOWER_FACTOR_64;

				// adjust img for lower hashing
				int half_dim = VRAM_DIM / 2;
				int last_obj_start = img.rows - half_dim;
				cv::Mat limg = img.rowRange(std::min(last_obj_start, half_dim), img.rows);	// coord limg at last place where a full 128x128 object could be hashed but limit it to object directly under upper

				// hash lower and continue hashing combined
				red = 0, green = 0, blue = 0;
				coord_count = 0;
				for (int i = 0; i < len; i++) {
					HashCoord coord = coords[i];
					unsigned char val = 0;
					if (coord.x < limg.cols && coord.y < limg.rows) {
						cv::Vec3b pixel = limg.at<cv::Vec3b>(coord.y, coord.x);
						val = round((pixel[0] + pixel[1] + pixel[2]) / 3);

						// keep track of RGB sums of pixels
						if (use_RGB) {
							red += pixel[0];
							green += pixel[1];
							blue += pixel[2];
						}
					}

					hash_lower ^= val;
					hash_lower *= FNV_OFFSET_PRIME_64;

					hash ^= val;
					hash *= FNV_OFFSET_PRIME_64;

					coord_count++;
				}

				// factor RGB averages into hash
				if (use_RGB) {
					red /= coord_count;
					green /= coord_count;
					blue /= coord_count;

					hash_lower ^= (uchar)round(red);
					hash_lower *= FNV_OFFSET_PRIME_64;
					hash_lower ^= (uchar)round(green);
					hash_lower *= FNV_OFFSET_PRIME_64;
					hash_lower ^= (uchar)round(blue);
					hash_lower *= FNV_OFFSET_PRIME_64;

					hash ^= (uchar)round(red);
					hash *= FNV_OFFSET_PRIME_64;
					hash ^= (uchar)round(green);
					hash *= FNV_OFFSET_PRIME_64;
					hash ^= (uchar)round(blue);
					hash *= FNV_OFFSET_PRIME_64;
				}
			}

			return hash;
		}

		uint64 uint32pow(uint32 base, int exp)
		{
			uint64 result = 1;
			for (int i = 0; i < exp; i++)
				result *= base;

			return result;
		}

		// hash upper, lower, and combined separately 
		uint32 FNV_Hash_Combined_32(cv::Mat img, uint32& hash_upper, uint32& hash_lower, const HashCoord* coords, const size_t len, bool use_RGB)
		{
			hash_lower = (img.rows > VRAM_DIM / 2) ? (use_RGB) ? FNV_NOUPPER_RGB_BASIS_32 : FNV_NOUPPER_BASIS_32 : 0;

			uint32 hash = FNV_OFFSET_BASIS_32;

			float red = 0, green = 0, blue = 0;
			size_t coord_count = 0;
			for (int i = 0; i < len; i++) {
				HashCoord coord = coords[i];
				unsigned char val = 0;
				if (coord.x < img.cols && coord.y < img.rows) {
					cv::Vec3b pixel = img.at<cv::Vec3b>(coord.y, coord.x);
					val = round((pixel[0] + pixel[1] + pixel[2]) / 3);

					// keep track of RGB sums of pixels
					if (use_RGB) {
						red += pixel[0];
						green += pixel[1];
						blue += pixel[2];
					}
				}

				hash ^= val;
				hash *= FNV_OFFSET_PRIME_32;
				coord_count++;
			}

			// factor RGB averages into hash
			if (use_RGB) {
				red /= coord_count;
				green /= coord_count;
				blue /= coord_count;

				hash ^= (uchar)round(red);
				hash *= FNV_OFFSET_PRIME_32;
				hash ^= (uchar)round(green);
				hash *= FNV_OFFSET_PRIME_32;
				hash ^= (uchar)round(blue);
				hash *= FNV_OFFSET_PRIME_32;
			}

			// set hash_upper equal to hash thus far
			hash_upper = hash;

			if (hash_lower) {																// make sure texture is big enough to hash lower
				// adjust hash_upper to include lower blank object
				hash_upper *= (use_RGB) ? FNV_NOLOWER_RGB_FACTOR_32 : FNV_NOLOWER_FACTOR_32;

				// adjust img for lower hashing
				int half_dim = VRAM_DIM / 2;
				int last_obj_start = img.rows - half_dim;
				cv::Mat limg = img.rowRange(std::min(last_obj_start, half_dim), img.rows);	// coord limg at last place where a full 128x128 object could be hashed but limit it to object directly under upper

				// hash lower and continue hashing combined
				red = 0, green = 0, blue = 0;
				coord_count = 0;
				for (int i = 0; i < len; i++) {
					HashCoord coord = coords[i];
					unsigned char val = 0;
					if (coord.x < limg.cols && coord.y < limg.rows) {
						cv::Vec3b pixel = limg.at<cv::Vec3b>(coord.y, coord.x);
						val = round((pixel[0] + pixel[1] + pixel[2]) / 3);

						// keep track of RGB sums of pixels
						if (use_RGB) {
							red += pixel[0];
							green += pixel[1];
							blue += pixel[2];
						}
					}

					hash_lower ^= val;
					hash_lower *= FNV_OFFSET_PRIME_32;

					hash ^= val;
					hash *= FNV_OFFSET_PRIME_32;

					coord_count++;
				}

				// factor RGB averages into hash
				if (use_RGB) {
					red /= coord_count;
					green /= coord_count;
					blue /= coord_count;

					hash_lower ^= (uchar)round(red);
					hash_lower *= FNV_OFFSET_PRIME_32;
					hash_lower ^= (uchar)round(green);
					hash_lower *= FNV_OFFSET_PRIME_32;
					hash_lower ^= (uchar)round(blue);
					hash_lower *= FNV_OFFSET_PRIME_32;

					hash ^= (uchar)round(red);
					hash *= FNV_OFFSET_PRIME_32;
					hash ^= (uchar)round(green);
					hash *= FNV_OFFSET_PRIME_32;
					hash ^= (uchar)round(blue);
					hash *= FNV_OFFSET_PRIME_32;
				}
			}

			return hash;
		}
	}

	namespace Murmur2
	{

		uint64 MurmurHash64B(const void * key, int len, uint64 seed)
		{
			const uint32 m = 0x5bd1e995;
			const int r = 24;

			uint32 h1 = uint32(seed) ^ len;
			uint32 h2 = uint32(seed >> 32);
			//std::cout << "Rem = " << len << "; h1 = " << h1 << "; h2 = " << h2 << std::endl;

			const uint32 * data = (const uint32 *)key;

			while (len >= 8) {
				uint32 k1 = *data++;
				k1 *= m; k1 ^= k1 >> r; k1 *= m;
				h1 *= m; h1 ^= k1;
				len -= 4;

				uint32 k2 = *data++;
				k2 *= m; k2 ^= k2 >> r; k2 *= m;
				h2 *= m; h2 ^= k2;
				len -= 4;

				//std::cout << "k1 = " << k1 << "; k2 = " << k2 << std::endl;
				//std::cout << "Rem = " << len << "; h1 = " << h1 << "; h2 = " << h2 << std::endl;
			}

			if (len >= 4) {
				uint32 k1 = *data++;
				k1 *= m; k1 ^= k1 >> r; k1 *= m;
				h1 *= m; h1 ^= k1;

				len -= 4; std::cout << "k1 = " << k1 << ";" << std::endl;
				//std::cout << "Rem = " << len << "; h1 = " << h1 << "; h2 = " << h2 << std::endl;
			}

			switch (len) {
			case 3: h2 ^= ((unsigned char*)data)[2] << 16;
			case 2: h2 ^= ((unsigned char*)data)[1] << 8;
			case 1: h2 ^= ((unsigned char*)data)[0];
				h2 *= m;
			};

			h1 ^= h2 >> 18; h1 *= m;
			h2 ^= h1 >> 22; h2 *= m;
			h1 ^= h2 >> 17; h1 *= m;
			h2 ^= h1 >> 19; h2 *= m;

			uint64 h = h1;

			h = (h << 32) | h2;

			return h;
		}

		uint64 Murmur2_Full(const cv::Mat& img)
		{
			int buflen = img.rows * img.cols * 3;
			uchar* buf = new uchar[buflen];
			int index = 0;

			for (int y = 0; y < img.rows; y++) {
				cv::Mat row = img.row(y);
				for (int x = 0; x < row.cols; x++) {
					cv::Vec3b pixel = row.at<cv::Vec3b>(0, x);
					buf[index++] = pixel[0];
					buf[index++] = pixel[1];
					buf[index++] = pixel[2];
				}
			}

			uint64 hash = MurmurHash64B(buf, buflen, MURMUR2_SEED);
			delete[] buf;
			return hash;
		}

		uint64 Murmur2_Hash(const cv::Mat& img, const HashCoord* coords, const size_t len)
		{
			int buflen = len * 3;
			uchar* buf = new uchar[buflen];
			int index = 0;

			const HashCoord* coord = coords;
			for (int i = 0; i < len; i++, coord++) {
				uchar val = 0, red = 0, green = 0, blue = 0;
				if (coord->x < img.cols && coord->y < img.rows) {
					cv::Vec3b pixel = img.at<cv::Vec3b>(coord->y, coord->x);
					red = pixel[0];
					green = pixel[1];
					blue = pixel[2];
				}
				buf[index++] = red;
				buf[index++] = green;
				buf[index++] = blue;
			}

			uint64 hash = MurmurHash64B(buf, buflen, MURMUR2_SEED);
			delete[] buf;
			return hash;
		}


		uint64 Murmur2_Hash(const cv::Mat& img, std::deque<HashCoord> coords)
		{
			int buflen = coords.size() * 3;
			uchar* buf = new uchar[buflen];
			int index = 0;

			for (HashCoord coord : coords) {
				uchar val = 0, red = 0, green = 0, blue = 0;
				if (coord.x < img.cols && coord.y < img.rows) {
					cv::Vec3b pixel = img.at<cv::Vec3b>(coord.y, coord.x);
					red = pixel[0];
					green = pixel[1];
					blue = pixel[2];
				}
				buf[index++] = red;
				buf[index++] = green;
				buf[index++] = blue;
			}

			uint64 hash = MurmurHash64B(buf, buflen, MURMUR2_SEED);
			delete[] buf;
			return hash;
		}

		uint64 Murmur2_Hash(const cv::Mat& img)
		{
			return Murmur2_Hash(img, COORDS, COORDS_LEN);
		}

		uint64 Murmur2_Hash_Combined_Naive(cv::Mat& img, uint64& hash_upper, uint64& hash_lower, const HashCoord* coords, const size_t len)
		{
			int buflen = len * 3;
			uchar* buf_upper = new uchar[buflen * 2];
			uchar* buf_lower = new uchar[buflen * 2];
			uchar* buf_combined = new uchar[buflen * 2];
			int index = 0;

			const HashCoord* coord = coords;
			for (int i = 0; i < len; i++, coord++) {
				uchar red = 0, green = 0, blue = 0;
				if (coord->x < img.cols && coord->y < img.rows) {
					cv::Vec3b pixel = img.at<cv::Vec3b>(coord->y, coord->x);
					red = pixel[0];
					green = pixel[1];
					blue = pixel[2];
				}
				buf_combined[index] = buf_upper[index] = red;
				buf_lower[index++] = 0;
				buf_combined[index] = buf_upper[index] = green;
				buf_lower[index++] = 0;
				buf_combined[index] = buf_upper[index] = blue;
				buf_lower[index++] = 0;
			}

			if (img.rows > (VRAM_DIM / 2)) {		// there is a lower half to hash
				coord = coords;
				for (int i = 0; i < len; i++, coord++) {
					uchar red = 0, green = 0, blue = 0;
					if (coord->x < img.cols && (coord->y + (VRAM_DIM / 2)) < img.rows) {
						cv::Vec3b pixel = img.at<cv::Vec3b>(coord->y + (VRAM_DIM / 2), coord->x);
						red = pixel[0];
						green = pixel[1];
						blue = pixel[2];
					}
					buf_upper[index] = 0;
					buf_combined[index] = buf_lower[index++] = red;
					buf_upper[index] = 0;
					buf_combined[index] = buf_lower[index++] = green;
					buf_upper[index] = 0;
					buf_combined[index] = buf_lower[index++] = blue;
				}
			}

			hash_upper = TextureHash::Murmur2::MurmurHash64B(buf_upper, buflen * 2, TextureHash::Murmur2::MURMUR2_SEED);
			hash_lower = TextureHash::Murmur2::MurmurHash64B(buf_lower, buflen * 2, TextureHash::Murmur2::MURMUR2_SEED);
			return TextureHash::Murmur2::MurmurHash64B(buf_combined, buflen * 2, TextureHash::Murmur2::MURMUR2_SEED);
		}

		inline cv::Vec3b get_correct_pixel(int rem, cv::Mat & img, const HashCoord* coords, const size_t len, bool use_mod = true)
		{
			int mod = (VRAM_DIM / 2);
			const HashCoord* coord;
			if (rem > len) {
				coord = coords + len * 2 - rem;
				return img.at<cv::Vec3b>(coord->y, coord->x);
			}
			coord = coords + len - rem;
			return (use_mod) ? img.at<cv::Vec3b>(coord->y + mod, coord->x) : img.at<cv::Vec3b>(coord->y, coord->x);
		}

		inline void murmur2_rem4(uint32& h1, uint32& h2, uint32& rem, cv::Mat& img, const HashCoord* coords, const size_t len, bool use_mod = true)
		{
			cv::Vec3b pixel1, pixel2;
			uint32 k;

			// k1, pixel 1rgb + 2r; reverse endianness: 2r | 1b | 1g | 1r
			k = 0;

			pixel1 = get_correct_pixel(rem--, img, coords, len, use_mod);
			pixel2 = get_correct_pixel(rem--, img, coords, len, use_mod);

			k |= pixel2[0] << 24 | pixel1[2] << 16 | pixel1[1] << 8 | pixel1[0];

			k *= m; k ^= k >> r; k *= m;
			h1 *= m; h1 ^= k;

			//std::cout << "k1 = " << k << "; ";

			// k2, pixel 2gb + 3rg; reverse endianness: 3g | 3r | 2b | 2g
			k = 0;

			pixel1 = get_correct_pixel(rem--, img, coords, len, use_mod);

			k |= pixel1[1] << 24 | pixel1[0] << 16 | pixel2[2] << 8 | pixel2[1];

			k *= m; k ^= k >> r; k *= m;
			h2 *= m; h2 ^= k;

			//std::cout << "k2 = " << k << std::endl;

			//std::cout << "Rem = " << rem * 3 + 2 << "; h1 = " << h1 << "; h2 = " << h2 << std::endl;

			// k1, pixel 3b + 4rgb; reverse endianness: 4b | 4g | 4r | 3b
			k = 0;

			pixel2 = get_correct_pixel(rem--, img, coords, len, use_mod);

			k |= pixel2[2] << 24 | pixel2[1] << 16 | pixel2[0] << 8 | pixel1[2];

			k *= m; k ^= k >> r; k *= m;
			h1 *= m; h1 ^= k;

			//std::cout << "k1 = " << k << ";";
		}

		inline void murmur2_rem32(cv::Vec3b& pixel1, cv::Vec3b& pixel2, bool swap, uint32& h1, uint32& h2, uint32 rem, cv::Mat& img, const HashCoord* coords, const size_t len, bool use_mod = true)
		{
			switch (rem) {
			case 3:
				uint32 k;

				// k1, pixel 1rgb + 2r; reverse endianness: 2r | 1b | 1g | 1r
				k = 0;

				pixel1 = get_correct_pixel(rem--, img, coords, len, use_mod);
				pixel2 = get_correct_pixel(rem--, img, coords, len, use_mod);

				k |= pixel2[0] << 24 | pixel1[2] << 16 | pixel1[1] << 8 | pixel1[0];

				k *= m; k ^= k >> r; k *= m;

				if (swap) {
					h2 *= m; h2 ^= m;
					//std::cout << "k2 = " << k << std::endl;
				} else {
					h1 *= m; h1 ^= k;
					//std::cout << "k1 = " << k << "; ";
				}


				// k2, pixel 2gb + 3rg; reverse endianness: 3g | 3r | 2b | 2g
				k = 0;

				pixel1 = get_correct_pixel(rem--, img, coords, len, use_mod);

				k |= pixel1[1] << 24 | pixel1[0] << 16 | pixel2[2] << 8 | pixel2[1];

				k *= m; k ^= k >> r; k *= m;

				if (swap) {
					h1 *= m; h1 ^= k;
					//std::cout << "k1 = " << k << ";" << std::endl;
				} else {
					h2 *= m; h2 ^= m;
					//std::cout << "k2 = " << k << std::endl;
				}
				break;
			case 2:
				// k1, pixel 1rgb + 2r; reverse endianness: 2r | 1b | 1g | 1r
				k = 0;

				pixel1 = get_correct_pixel(rem--, img, coords, len, use_mod);
				pixel2 = get_correct_pixel(rem--, img, coords, len, use_mod);

				k |= pixel2[0] << 24 | pixel1[2] << 16 | pixel1[1] << 8 | pixel1[0];

				k *= m; k ^= k >> r; k *= m;

				if (swap) {
					h2 *= m; h2 ^= m;
					//std::cout << "k2 = " << k << std::endl;
				} else {
					h1 *= m; h1 ^= k;
					//std::cout << "k1 = " << k << ";" << std::endl;
				}

				// k2, only pixel 2gb remaining
				h2 ^= pixel2[1] << 8;
				h2 ^= pixel2[2];
				h2 *= m;
				break;
			}
		}

		inline void murmur2_finish(uint32& h1, uint32& h2, uint32 rem, cv::Mat& img, const HashCoord* coords, const size_t len)
		{
			cv::Vec3b pixel1, pixel2;

			bool swap = false;
			if (rem >= 4) {
				swap = true;
				murmur2_rem4(h1, h2, rem, img, coords, len);
			}

			if (rem > 1) murmur2_rem32(pixel1, pixel2, swap, h1, h2, rem, img, coords, len);

			switch (rem) {
			case 3:
				// k2, only pixel 3b remaining
				rem -= 3;
				h2 ^= pixel1[2];
				h2 *= m;
				break;
			case 2:
				// k2, only pixel 2gb remaining
				rem -= 2;
				h2 ^= pixel2[1] << 8;
				h2 ^= pixel2[2];
				h2 *= m;
				break;
			case 1:
				// k2, only pixel 1rgb remaining
				pixel1 = get_correct_pixel(rem--, img, coords, len);

				h2 ^= pixel1[0] << 16;
				h2 ^= pixel1[1] << 8;
				h2 ^= pixel1[2];
			}

			h1 ^= h2 >> 18; h1 *= m;
			h2 ^= h1 >> 22; h2 *= m;
			h1 ^= h2 >> 17; h1 *= m;
			h2 ^= h1 >> 19; h2 *= m;
		}

		uint64 Murmur2_Hash_Combined(cv::Mat& img, uint64& hash_upper, uint64& hash_lower, const HashCoord* coords, const size_t len)
		{
			uint32 h1_u;
			uint32 h2_u;

			uint32 h1_l = uint32(TextureHash::Murmur2::MURMUR2_SEED) ^ (len * 3 * 2);
			uint32 h2_l = uint32(TextureHash::Murmur2::MURMUR2_SEED >> 32);

			uint32 h1_c = uint32(TextureHash::Murmur2::MURMUR2_SEED) ^ (len * 3 * 2);
			uint32 h2_c = uint32(TextureHash::Murmur2::MURMUR2_SEED >> 32);

			const HashCoord* coord = coords;
			uint32 rem = len * 2;
			//std::cout << "Rem = " << rem * 3 << "; h1 = " << h1_u << "; h2 = " << h2_u << std::endl;

			cv::Vec3b pixel1, pixel2;
			while (rem >= 8) {
				// save hash_upper progress
				if ((rem - len) < 8) (h1_u = h1_c) && (h2_u = h2_c);
				uint32 k;

				// k1, pixel 1rgb + 2r; reverse endianness: 2r | 1b | 1g | 1r
				k = 0;

				pixel1 = get_correct_pixel(rem--, img, coords, len);
				pixel2 = get_correct_pixel(rem--, img, coords, len);

				k |= pixel2[0] << 24 | pixel1[2] << 16 | pixel1[1] << 8 | pixel1[0];

				k *= m; k ^= k >> r; k *= m;
				h1_c *= m; h1_c ^= k;

				//std::cout << "k1 = " << k << "; ";

				// k2, pixel 2gb + 3rg; reverse endianness: 3g | 3r | 2b | 2g
				k = 0;

				pixel1 = get_correct_pixel(rem--, img, coords, len);

				k |= pixel1[1] << 24 | pixel1[0] << 16 | pixel2[2] << 8 | pixel2[1];

				k *= m; k ^= k >> r; k *= m;
				h2_c *= m; h2_c ^= k;

				//std::cout << "k2 = " << k << std::endl;

				//std::cout << "Rem = " << rem * 3 + 2 << "; h1 = " << h1_c << "; h2 = " << h2_c << std::endl;

				// k1, pixel 3b + 4rgb; reverse endianness: 4b | 4g | 4r | 3b
				k = 0;

				pixel2 = get_correct_pixel(rem--, img, coords, len);

				k |= pixel2[2] << 24 | pixel2[1] << 16 | pixel2[0] << 8 | pixel1[2];

				k *= m; k ^= k >> r; k *= m;
				h1_c *= m; h1_c ^= k;

				//std::cout << "k1 = " << k << "; ";

				// k2, pixel 5rgb + 6r; reverse endianness: 6r | 5b | 5g | 5r
				k = 0;

				pixel1 = get_correct_pixel(rem--, img, coords, len);
				pixel2 = get_correct_pixel(rem--, img, coords, len);

				k |= pixel2[0] << 24 | pixel1[2] << 16 | pixel1[1] << 8 | pixel1[0];

				k *= m; k ^= k >> r; k *= m;
				h2_c *= m; h2_c ^= k;

				//std::cout << "k2 = " << k << std::endl;

				//std::cout << "Rem = " << rem * 3 << "; h1 = " << h1_c << "; h2 = " << h2_c << std::endl;

				// k1, pixel 6gb + 7rg; reverse endianness: 7g | 7r | 6b | 6g
				k = 0;

				pixel1 = get_correct_pixel(rem--, img, coords, len);

				k |= pixel1[1] << 24 | pixel1[0] << 16 | pixel2[2] << 8 | pixel2[1];

				k *= m; k ^= k >> r; k *= m;
				h1_c *= m; h1_c ^= k;

				//std::cout << "k1 = " << k << "; ";

				// k2, pixel 7b + 8rgb; reverse endianness: 8b | 8g | 8r | 7b
				k = 0;

				pixel2 = get_correct_pixel(rem--, img, coords, len);

				k |= pixel2[2] << 24 | pixel2[1] << 16 | pixel2[0] << 8 | pixel1[2];

				k *= m; k ^= k >> r; k *= m;
				h2_c *= m; h2_c ^= k;

				//std::cout << "k2 = " << k << std::endl;

				//std::cout << "Rem = " << rem * 3 << "; h1 = " << h1_c << "; h2 = " << h2_c << std::endl;

				//std::cout << "After " << (len - rem) << " - h1 = " << h1_c << "; h2 = " << h2_c << std::endl;
			}

			// finish hash_combined
			murmur2_finish(h1_c, h2_c, rem, img, coords, len);

			// finish hash_upper
			rem = len % 8;
			bool swap = false;
			if (rem >= 4) {
				swap = true;
				murmur2_rem4(h1_u, h2_u, rem, img, coords, len, false);
			}
			if (rem > 1) murmur2_rem32(pixel1, pixel2, swap, h1_u, h2_u, rem, img, coords, len, false);
			switch (rem) {
			case 3:
				uint32 k;
				// k1, only pixel 3b remaining; reverse endianness: 0 | 0 | 0 | 3b
				k = pixel1[2];

				k *= m; k ^= k >> r; k *= m;

				if (swap) {
					h2_u *= m; h2_u ^= m;
					//std::cout << "k2 = " << k << std::endl;
				} else {
					h1_u *= m; h1_u ^= k;
					//std::cout << "k1 = " << k << "; ";
				}

				break;
			case 2:
				// k2, only pixel 2gb remaining; reverse enianness: 0 | 0 | 2b | 2g
				k = pixel2[2] << 8 | pixel2[1];
				
				k *= m; k ^= k >> r; k *= m;

				if (swap) {
					h1_u *= m; h1_u ^= k;
					//std::cout << "k1 = " << k << "; ";
				} else {
					h2_u *= m; h2_u ^= m;
					//std::cout << "k2 = " << k << std::endl;
				}

				break;
			case 1:
				// only pixel 1rgb remaining; reverse endianness: 0 | 1b | 1g | 1r
				pixel1 = get_correct_pixel(rem--, img, coords, len);

				k = pixel1[2] << 24 | pixel1[1] << 16 | pixel1[0];

				if (swap) {
					h2_u *= m; h2_u ^= m;
					//std::cout << "k2 = " << k << std::endl;
				} else {
					h1_u *= m; h1_u ^= k;
					//std::cout << "k1 = " << k << "; ";
				}

				break;
			case 0:
				// need to do all |len| pixels worth of 0s
				for (int i = 0; i < len * 3 - 8; i += 8) {
					h1_u *= m;
					h2_u *= m;
				}

				if (swap) {				// means rem = 4
					h2_u *= m;
				}
			}

			h1_u ^= h2_u >> 18; h1_u *= m;
			h2_u ^= h1_u >> 22; h2_u *= m;
			h1_u ^= h2_u >> 17; h1_u *= m;
			h2_u ^= h1_u >> 19; h2_u *= m;

			hash_upper = uint64(h1_u) << 32 | h2_u;

			return uint64(h1_c) << 32 | h2_c;
			//if (height > VRAM_DIM / 2) rem += len;		// there is a lower portion to hash
		}

		//void Murmur2_Combined(char* pData, UINT pitch, int width, int height, const HashCoord* coords, const int len, uint64_t & hash_combined, uint64_t hash_upper, uint64_t hash_lower)
		//{
		//	const uint32 m = 0x5bd1e995;
		//	const int r = 24;

		//	uint32 h1_u = uint32(TextureHash::Murmur2::MURMUR2_SEED) ^ len;
		//	uint32 h2_u = uint32(TextureHash::Murmur2::MURMUR2_SEED >> 32);

		//	uint32 h1_l = uint32(TextureHash::Murmur2::MURMUR2_SEED) ^ len;
		//	uint32 h2_l = uint32(TextureHash::Murmur2::MURMUR2_SEED >> 32);

		//	BYTE* data = pData;
		//	const HashCoord* coord = coords;
		//	int rem = len;

		//	RGBColor pixel;
		//	while (rem >= 8) {
		//		uint32 k;

		//		// k1, pixel 1rgb + 2r
		//		k = 0;

		//		pixel = *((RGBColor*)(data + (coord->y) * pitch + coord->x));
		//		k |= pixel.r << 24 | pixel.g << 16 | pixel.b << 8;
		//		coord++;

		//		pixel = *((RGBColor*)(data + (coord->y) * pitch + coord->x));
		//		k |= pixel.r;
		//		coord++;

		//		k *= m; k ^= k >> r; k *= m;
		//		h1_u *= m; h1_u ^= k;

		//		// k2, pixel 2gb + 3rg
		//		k = 0;

		//		k |= pixel.g << 24 | pixel.b << 16;

		//		pixel = *((RGBColor*)(data + (coord->y) * pitch + coord->x));
		//		k |= pixel.r << 8 | pixel.g;
		//		coord++;

		//		k *= m; k ^= k >> r; k *= m;
		//		h2_u *= m; h2_u ^= k;

		//		// k1, pixel 3b + 4rgb
		//		k = 0;

		//		k |= pixel.b << 24;

		//		pixel = *((RGBColor*)(data + (coord->y) * pitch + coord->x));
		//		k |= pixel.r << 16 | pixel.g << 8 | pixel.b;
		//		coord++;

		//		k *= m; k ^= k >> r; k *= m;
		//		h1_u *= m; h1_u ^= k;

		//		// k2, pixel 5rgb + 6r
		//		k = 0;

		//		pixel = *((RGBColor*)(data + (coord->y) * pitch + coord->x));
		//		k |= pixel.r << 24 | pixel.g << 16 | pixel.b << 8;
		//		coord++;

		//		pixel = *((RGBColor*)(data + (coord->y) * pitch + coord->x));
		//		k |= pixel.r;
		//		coord++;

		//		k *= m; k ^= k >> r; k *= m;
		//		h2_u *= m; h2_u ^= k;

		//		// k1, pixel 6gb + 7rg
		//		k = 0;

		//		k |= pixel.g << 24 | pixel.b << 16;

		//		pixel = *((RGBColor*)(data + (coord->y) * pitch + coord->x));
		//		k |= pixel.r << 8 | pixel.g;
		//		coord++;

		//		k *= m; k ^= k >> r; k *= m;
		//		h1_u *= m; h1_u ^= k;

		//		// k2, pixel 7b + 8rgb
		//		k = 0;

		//		k |= pixel.b << 24;

		//		pixel = *((RGBColor*)(data + (coord->y) * pitch + coord->x));
		//		k |= pixel.r << 16 | pixel.g << 8 | pixel.b;
		//		coord++;

		//		k *= m; k ^= k >> r; k *= m;
		//		h2_u *= m; h2_u ^= k;

		//		rem -= 8;
		//	}

		//	// save current hash progress for hash_combined

		//	if (rem >= 4) {
		//		uint32 k;

		//		// k1, pixel 1rgb + 2r
		//		k = 0;

		//		pixel = *((RGBColor*)(data + (coord->y) * pitch + coord->x));
		//		k |= pixel.r << 24 | pixel.g << 16 | pixel.b << 8;
		//		coord++;

		//		pixel = *((RGBColor*)(data + (coord->y) * pitch + coord->x));
		//		k |= pixel.r;
		//		coord++;

		//		k *= m; k ^= k >> r; k *= m;
		//		h1_u *= m; h1_u ^= k;

		//		// k2, pixel 2gb + 3rg
		//		k = 0;

		//		k |= pixel.g << 24 | pixel.b << 16;

		//		pixel = *((RGBColor*)(data + (coord->y) * pitch + coord->x));
		//		k |= pixel.r << 8 | pixel.g;
		//		coord++;

		//		k *= m; k ^= k >> r; k *= m;
		//		h2_u *= m; h2_u ^= k;

		//		// k1, pixel 3b + 4rgb
		//		k = 0;

		//		k |= pixel.b << 24;

		//		pixel = *((RGBColor*)(data + (coord->y) * pitch + coord->x));
		//		k |= pixel.r << 16 | pixel.g << 8 | pixel.b;
		//		coord++;

		//		k *= m; k ^= k >> r; k *= m;
		//		h1_u *= m; h1_u ^= k;
		//	}

		//	switch (rem) {
		//	case 3:
		//		uint32 k;

		//		// k1, pixel 1rgb + 2r
		//		k = 0;

		//		pixel = *((RGBColor*)(data + (coord->y) * pitch + coord->x));
		//		k |= pixel.r << 24 | pixel.g << 16 | pixel.b << 8;
		//		coord++;

		//		pixel = *((RGBColor*)(data + (coord->y) * pitch + coord->x));
		//		k |= pixel.r;
		//		coord++;

		//		k *= m; k ^= k >> r; k *= m;
		//		h1_u *= m; h1_u ^= k;

		//		// k2, pixel 2gb + 3rg
		//		k = 0;

		//		k |= pixel.g << 24 | pixel.b << 16;

		//		pixel = *((RGBColor*)(data + (coord->y) * pitch + coord->x));
		//		k |= pixel.r << 8 | pixel.g;
		//		coord++;

		//		k *= m; k ^= k >> r; k *= m;
		//		h2_u *= m; h2_u ^= k;

		//		// k2, only pixel 3b remaining
		//		h2_u ^= pixel.b;
		//		h2_u *= m;
		//		break;
		//	case 2:
		//		uint32 k;

		//		// k1, pixel 1rgb + 2r
		//		k = 0;

		//		pixel = *((RGBColor*)(data + (coord->y) * pitch + coord->x));
		//		k |= pixel.r << 24 | pixel.g << 16 | pixel.b << 8;
		//		coord++;

		//		pixel = *((RGBColor*)(data + (coord->y) * pitch + coord->x));
		//		k |= pixel.r;
		//		coord++;

		//		k *= m; k ^= k >> r; k *= m;
		//		h1_u *= m; h1_u ^= k;

		//		// k2, only pixel 2gb remaining
		//		h2_u ^= pixel.g << 8;
		//		h2_u ^= pixel.b;
		//		h2_u *= m;
		//		break;
		//	case 1:
		//		// k2, only pixel 1rgb remaining
		//		h2_u ^= pixel.r << 16;
		//		h2_u ^= pixel.g << 8;
		//		h2_u ^= pixel.b;
		//	}

		//	h1_u ^= h2_u >> 18; h1_u *= m;
		//	h2_u ^= h1_u >> 22; h2_u *= m;
		//	h1_u ^= h2_u >> 17; h1_u *= m;
		//	h2_u ^= h1_u >> 19; h2_u *= m;

		//	hash_upper = (h1_u << 32) | h2_u;

		//	if (height > VRAM_DIM / 2) rem += len;		// there is a lower portion to hash
		//}
	}
}