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

namespace Murmur2 {

	uint64 MurmurHash64B(const void * key, int len, uint64 seed)
	{
		const uint32 m = 0x5bd1e995;
		const int r = 24;

		uint32 h1 = uint32(seed) ^ len;
		uint32 h2 = uint32(seed >> 32);

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
		}

		if (len >= 4) {
			uint32 k1 = *data++;
			k1 *= m; k1 ^= k1 >> r; k1 *= m;
			h1 *= m; h1 ^= k1;
			len -= 4;
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
		char* buf = new char[buflen];
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
		char* buf = new char[buflen];
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
		char* buf = new char[buflen];
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

	uint64 Murmur_Hash_Combined(cv::Mat img, uint64& hash_upper, uint64& hash_lower, const HashCoord* coords, const size_t len)
	{
		return 0;
		//hash_lower = (img.rows > VRAM_DIM / 2) ? (use_RGB) ? FNV_NOUPPER_RGB_BASIS : FNV_NOUPPER_BASIS : 0;

		//int buflen = (hash_lower) ? len * 3 * 2 : len * 3;
		//char* buf = new char[buflen];
		//int index = 0;

		//const coord* point = coords;
		//for (int i = 0; i < len; i++, point++) {
		//	uchar red = 0, green = 0, blue = 0;
		//	if (point->x < img.cols && point->y < img.rows) {
		//		cv::Vec3b pixel = img.at<cv::Vec3b>(point->y, point->x);
		//		buf[index++] = pixel[0];
		//		buf[index++] = pixel[1];
		//		buf[index++] = pixel[2];
		//	}
		//}

		//uint64 hash = MurmurHash64B(buf, buflen, MURMUR2_SEED);

		//// set hash_upper equal to hash thus far
		//hash_upper = hash;

		//if (hash_lower) {																// make sure texture is big enough to hash lower
		//	// adjust hash_upper to include lower blank object
		//	hash_upper *= (use_RGB) ? FNV_NOLOWER_RGB_FACTOR : FNV_NOLOWER_FACTOR;

		//	// adjust img for lower hashing
		//	int half_dim = VRAM_DIM / 2;
		//	int last_obj_start = img.rows - half_dim;
		//	cv::Mat limg = img.rowRange(std::min(last_obj_start, half_dim), img.rows);	// point limg at last place where a full 128x128 object could be hashed but limit it to object directly under upper

		//	// hash lower and continue hashing combined
		//	red = 0, green = 0, blue = 0;
		//	coord_count = 0;
		//	for (int i = 0; i < len; i++) {
		//		coord point = coords[i];
		//		unsigned char val = 0;
		//		if (point.x < limg.cols && point.y < limg.rows) {
		//			cv::Vec3b pixel = limg.at<cv::Vec3b>(point.y, point.x);
		//			val = round((pixel[0] + pixel[1] + pixel[2]) / 3);

		//			// keep track of RGB sums of pixels
		//			if (use_RGB) {
		//				red += pixel[0];
		//				green += pixel[1];
		//				blue += pixel[2];
		//			}
		//		}

		//		hash_lower ^= val;
		//		hash_lower *= FNV_OFFSET_PRIME;

		//		hash ^= val;
		//		hash *= FNV_OFFSET_PRIME;

		//		coord_count++;
		//	}

		//	// factor RGB averages into hash
		//	if (use_RGB) {
		//		red /= coord_count;
		//		green /= coord_count;
		//		blue /= coord_count;

		//		hash_lower ^= (uchar)round(red);
		//		hash_lower *= FNV_OFFSET_PRIME;
		//		hash_lower ^= (uchar)round(green);
		//		hash_lower *= FNV_OFFSET_PRIME;
		//		hash_lower ^= (uchar)round(blue);
		//		hash_lower *= FNV_OFFSET_PRIME;

		//		hash ^= (uchar)round(red);
		//		hash *= FNV_OFFSET_PRIME;
		//		hash ^= (uchar)round(green);
		//		hash *= FNV_OFFSET_PRIME;
		//		hash ^= (uchar)round(blue);
		//		hash *= FNV_OFFSET_PRIME;
		//	}
		//}

		//return hash;
	}
}
}