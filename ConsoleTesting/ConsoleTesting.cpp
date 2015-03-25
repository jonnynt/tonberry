// ConsoleTesting.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"
#include "MurmurHash2.h"
#include "md5.h"
#include <iostream>
#include <ctime>
#include <array>
#include <unordered_set>
#include <opencv2/opencv.hpp>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <sstream>
namespace fs = boost::filesystem;
using std::cout;
using std::cin;
using std::endl;
using std::string;
using std::hash;
using std::array;
using std::deque;
using std::set;
using std::unordered_set;
using std::map;
using std::pair;
using std::min;
using std::max_element;
using std::ofstream;
using std::greater;
using std::list;
using std::stringstream;

const static fs::path FF8_ROOT("C:\\Program Files (x86)\\Steam\\steamapps\\common\\FINAL FANTASY VIII");
const static int BLOCKSIZE = 16;

typedef unsigned int uint32;

// Return path when appended to a_From will resolve to same as a_To
fs::path make_relative(fs::path a_From, fs::path a_To)
{
	a_From = fs::absolute(a_From); a_To = fs::absolute(a_To);
	fs::path ret;
	fs::path::const_iterator itrFrom(a_From.begin()), itrTo(a_To.begin());
	// Find common base
	for (fs::path::const_iterator toEnd(a_To.end()), fromEnd(a_From.end()); itrFrom != fromEnd && itrTo != toEnd && *itrFrom == *itrTo; ++itrFrom, ++itrTo);
	// Navigate backwards in directory to reach previously found base
	for (fs::path::const_iterator fromEnd(a_From.end()); itrFrom != fromEnd; ++itrFrom) {
		if ((*itrFrom) != ".")
			ret /= "..";
	}
	// Now navigate down the directory branch
	for (; itrTo != a_To.end(); ++itrTo)
		ret /= *itrTo;
	return ret;
}

typedef struct coord {
	enum color
	{
		none = -1,
		r = 0,
		g = 1,
		b = 2
	};

	int x;
	int y;
	color c;

	coord(int _x, int _y, color _c) : x(_x), y(_y), c(_c) {}
	coord(int _x = -1, int _y = -1, int _c = -1) : x(_x), y(_y)
	{
		if (_c < -1 || _c > 2) c = none;
		else c = (color)_c;
	}

	bool operator<(const coord& rhs) const {
		return (rhs.y > y) || (rhs.y == y && rhs.x > x) || (rhs.y == y && rhs.x == x && rhs.c > c);
	}

	bool operator==(const coord& rhs) const {
		return rhs.y == y && rhs.x == x && rhs.c == c;
	}

	string string() {
		stringstream ss;
		ss << "(" << x << ", " << y;
		switch (c) {
		case 0: {
			ss << ", r";
			break;
		}
		case 1: {
			ss << ", g";
			break;
		}
		case 2: {
			ss << ", b";
			break;
		}
		}
		ss << ")";
		return ss.str();
	}
} coord;

template <>
struct hash<coord>
{
	std::size_t operator()(const coord& key) const {
		// Compute individual hash values for x, y, and c and combine them using XOR and bit shifting:
		return ((hash<int>()(key.x) ^ (hash<int>()(key.y) << 1)) >> 1) ^ (hash<int>()(key.c) << 1);
	}
};

//int hash1_x[64] = { 0, 16, 32, 48, 64, 80, 96, 112, 0, 16, 32, 48, 64, 80, 96, 112, 0, 16, 32, 48, 64, 80, 96, 112, 0, 16, 32, 48, 64, 80, 96, 112, 0, 16, 0, 16, 48, 96, 0, 16, 0, 16, 0, 16, 48, 96, 0, 16, 0, 16, 0, 16, 48, 96, 0, 16, 0, 16, 0, 16, 48, 96, 0, 16 };
//int hash1_y[64] = { 0, 0, 0, 0, 0, 0, 0, 0, 16, 16, 16, 16, 16, 16, 16, 16, 32, 32, 32, 32, 32, 32, 32, 32, 48, 48, 48, 48, 48, 48, 48, 48, 64, 64, 80, 80, 80, 80, 96, 96, 112, 112, 128, 128, 128, 128, 144, 144, 160, 160, 176, 176, 176, 176, 192, 192, 208, 208, 224, 224, 224, 224, 240, 240 };
coord hash1[64] = { coord{ 0, 0 }, coord{ 16, 0 }, coord{ 32, 0 }, coord{ 48, 0 }, coord{ 64, 0 }, coord{ 80, 0 }, coord{ 96, 0 }, coord{ 112, 0 }, coord{ 0, 16 }, coord{ 16, 16 }, coord{ 32, 16 }, coord{ 48, 16 }, coord{ 64, 16 }, coord{ 80, 16 }, coord{ 96, 16 }, coord{ 112, 16 }, coord{ 0, 32 }, coord{ 16, 32 }, coord{ 32, 32 }, coord{ 48, 32 }, coord{ 64, 32 }, coord{ 80, 32 }, coord{ 96, 32 }, coord{ 112, 32 }, coord{ 0, 48 }, coord{ 16, 48 }, coord{ 32, 48 }, coord{ 48, 48 }, coord{ 64, 48 }, coord{ 80, 48 }, coord{ 96, 48 }, coord{ 112, 48 }, coord{ 0, 64 }, coord{ 16, 64 }, coord{ 0, 80 }, coord{ 16, 80 }, coord{ 48, 80 }, coord{ 96, 80 }, coord{ 0, 96 }, coord{ 16, 96 }, coord{ 0, 112 }, coord{ 16, 112 }, coord{ 0, 128 }, coord{ 16, 128 }, coord{ 48, 128 }, coord{ 96, 128 }, coord{ 0, 144 }, coord{ 16, 144 }, coord{ 0, 160 }, coord{ 16, 160 }, coord{ 0, 176 }, coord{ 16, 176 }, coord{ 48, 176 }, coord{ 96, 176 }, coord{ 0, 192 }, coord{ 16, 192 }, coord{ 0, 208 }, coord{ 16, 208 }, coord{ 0, 224 }, coord{ 16, 224 }, coord{ 48, 224 }, coord{ 96, 224 }, coord{ 0, 240 }, coord{ 16, 240 } };

//coord high_var64[64] = { coord(62, 0), coord(58, 0), coord(60, 0), coord(63, 0), coord(48, 0), coord(62, 1), coord(60, 1), coord(59, 1), coord(56, 1), coord(58, 1), coord(55, 1), coord(54, 1), coord(56, 2), coord(54, 2), coord(62, 2), coord(50, 2), coord(57, 2), coord(60, 2), coord(55, 2), coord(52, 2), coord(58, 2), coord(58, 3), coord(60, 3), coord(58, 4), coord(59, 4), coord(56, 4), coord(2, 6), coord(4, 7), coord(2, 7), coord(1, 7), coord(0, 8), coord(2, 8), coord(1, 8), coord(4, 8), coord(3, 9), coord(4, 9), coord(2, 9), coord(0, 10), coord(3, 10), coord(2, 10), coord(1, 13), coord(1, 14), coord(3, 15), coord(3, 16), coord(1, 16), coord(2, 17), coord(56, 30), coord(62, 51), coord(61, 51), coord(62, 52), coord(61, 52), coord(62, 53), coord(63, 53), coord(61, 53), coord(60, 54), coord(63, 54), coord(61, 55), coord(60, 56), coord(63, 57), coord(62, 59), coord(63, 59), coord(63, 60), coord(62, 60), coord(60, 62) };
//coord high_var128[64] = { coord(62, 0), coord(62, 1), coord(56, 2), coord(4, 7), coord(62, 59), coord(5, 135), coord(100, 135), coord(12, 149), coord(0, 149), coord(126, 150), coord(28, 163), coord(36, 163), coord(44, 165), coord(34, 165), coord(30, 165), coord(40, 166), coord(42, 166), coord(54, 166), coord(36, 166), coord(15, 179), coord(9, 179), coord(5, 180), coord(68, 180), coord(9, 180), coord(15, 180), coord(5, 181), coord(73, 181), coord(11, 181), coord(7, 181), coord(9, 181), coord(37, 181), coord(9, 182), coord(5, 182), coord(11, 182), coord(3, 182), coord(19, 182), coord(25, 182), coord(73, 182), coord(7, 182), coord(23, 183), coord(22, 183), coord(127, 183), coord(11, 183), coord(21, 183), coord(9, 183), coord(20, 183), coord(55, 183), coord(15, 183), coord(37, 183), coord(1, 183), coord(61, 183), coord(13, 183), coord(127, 184), coord(127, 185), coord(71, 185), coord(127, 186), coord(123, 189), coord(124, 189), coord(123, 191), coord(10, 236), coord(26, 240), coord(12, 241), coord(34, 251), coord(14, 255) };
//coord high_var_block128_66[66] = { coord(4, 7), coord(24, 10), coord(56, 2), coord(80, 17), coord(99, 17), coord(126, 8), coord(3, 40), coord(38, 35), coord(56, 30), coord(72, 43), coord(94, 30), coord(112, 31), coord(10, 55), coord(24, 57), coord(62, 59), coord(65, 60), coord(101, 59), coord(107, 57), coord(9, 85), coord(28, 74), coord(63, 72), coord(65, 72), coord(87, 80), coord(125, 84), coord(2, 97), coord(25, 104), coord(50, 114), coord(71, 111), coord(97, 98), coord(121, 98), coord(5, 135), coord(39, 133), coord(49, 131), coord(66, 128), coord(100, 135), coord(126, 128), coord(12, 149), coord(28, 147), coord(54, 153), coord(83, 143), coord(98, 154), coord(126, 150), coord(5, 180), coord(23, 183), coord(44, 165), coord(73, 181), coord(95, 183), coord(125, 184), coord(16, 186), coord(25, 186), coord(51, 186), coord(73, 186), coord(96, 190), coord(123, 189), coord(16, 218), coord(26, 214), coord(44, 226), coord(66, 229), coord(90, 213), coord(110, 211), coord(10, 236), coord(26, 240), coord(54, 244), coord(78, 248), coord(86, 240), coord(122, 238) };
//array<coord, 121> high_var_block64_121 = { coord{ 4, 7 }, coord{ 14, 9 }, coord{ 24, 10 }, coord{ 44, 2 }, coord{ 54, 2 }, coord{ 62, 2 }, coord{ 68, 2 }, coord{ 81, 3 }, coord{ 90, 9 }, coord{ 101, 11 }, coord{ 121, 8 }, coord{ 3, 15 }, coord{ 13, 22 }, coord{ 27, 20 }, coord{ 38, 21 }, coord{ 54, 18 }, coord{ 63, 19 }, coord{ 72, 21 }, coord{ 80, 17 }, coord{ 99, 17 }, coord{ 103, 17 }, coord{ 119, 21 }, coord{ 8, 28 }, coord{ 13, 24 }, coord{ 30, 32 }, coord{ 40, 24 }, coord{ 54, 27 }, coord{ 57, 30 }, coord{ 73, 32 }, coord{ 84, 32 }, coord{ 94, 30 }, coord{ 110, 31 }, coord{ 112, 31 }, coord{ 3, 40 }, coord{ 20, 41 }, coord{ 28, 36 }, coord{ 38, 35 }, coord{ 52, 44 }, coord{ 57, 44 }, coord{ 72, 43 }, coord{ 83, 35 }, coord{ 98, 35 }, coord{ 102, 35 }, coord{ 113, 35 }, coord{ 10, 55 }, coord{ 22, 54 }, coord{ 24, 55 }, coord{ 44, 48 }, coord{ 54, 51 }, coord{ 62, 52 }, coord{ 73, 54 }, coord{ 82, 54 }, coord{ 98, 54 }, coord{ 109, 55 }, coord{ 115, 50 }, coord{ 10, 62 }, coord{ 22, 57 }, coord{ 24, 57 }, coord{ 41, 64 }, coord{ 55, 61 }, coord{ 62, 59 }, coord{ 69, 61 }, coord{ 84, 63 }, coord{ 91, 63 }, coord{ 101, 59 }, coord{ 117, 66 }, coord{ 11, 71 }, coord{ 22, 73 }, coord{ 28, 74 }, coord{ 42, 74 }, coord{ 49, 69 }, coord{ 63, 72 }, coord{ 68, 73 }, coord{ 87, 75 }, coord{ 94, 77 }, coord{ 101, 74 }, coord{ 118, 71 }, coord{ 9, 85 }, coord{ 19, 88 }, coord{ 31, 88 }, coord{ 38, 87 }, coord{ 47, 79 }, coord{ 63, 80 }, coord{ 77, 85 }, coord{ 87, 80 }, coord{ 97, 84 }, coord{ 104, 79 }, coord{ 121, 82 }, coord{ 2, 97 }, coord{ 22, 92 }, coord{ 32, 99 }, coord{ 36, 99 }, coord{ 54, 99 }, coord{ 62, 91 }, coord{ 71, 99 }, coord{ 79, 90 }, coord{ 97, 98 }, coord{ 108, 99 }, coord{ 121, 98 }, coord{ 4, 109 }, coord{ 20, 101 }, coord{ 25, 104 }, coord{ 36, 105 }, coord{ 55, 104 }, coord{ 61, 106 }, coord{ 71, 108 }, coord{ 84, 109 }, coord{ 93, 102 }, coord{ 110, 102 }, coord{ 112, 106 }, coord{ 3, 120 }, coord{ 20, 119 }, coord{ 26, 114 }, coord{ 35, 112 }, coord{ 50, 114 }, coord{ 63, 117 }, coord{ 76, 118 }, coord{ 79, 116 }, coord{ 99, 119 }, coord{ 105, 112 }, coord{ 120, 113 } };
//array<coord, 253> high_var_block256_253 = { coord{ 4, 7 }, coord{ 14, 9 }, coord{ 24, 10 }, coord{ 44, 2 }, coord{ 54, 2 }, coord{ 62, 2 }, coord{ 68, 2 }, coord{ 81, 3 }, coord{ 90, 9 }, coord{ 101, 11 }, coord{ 121, 8 }, coord{ 3, 15 }, coord{ 13, 22 }, coord{ 27, 20 }, coord{ 38, 21 }, coord{ 54, 18 }, coord{ 63, 19 }, coord{ 72, 21 }, coord{ 80, 17 }, coord{ 99, 17 }, coord{ 103, 17 }, coord{ 119, 21 }, coord{ 8, 28 }, coord{ 13, 24 }, coord{ 30, 32 }, coord{ 40, 24 }, coord{ 54, 27 }, coord{ 57, 30 }, coord{ 73, 32 }, coord{ 84, 32 }, coord{ 94, 30 }, coord{ 110, 31 }, coord{ 112, 31 }, coord{ 3, 40 }, coord{ 20, 41 }, coord{ 28, 36 }, coord{ 38, 35 }, coord{ 52, 44 }, coord{ 57, 44 }, coord{ 72, 43 }, coord{ 83, 35 }, coord{ 98, 35 }, coord{ 102, 35 }, coord{ 113, 35 }, coord{ 10, 55 }, coord{ 22, 54 }, coord{ 24, 55 }, coord{ 44, 48 }, coord{ 54, 51 }, coord{ 62, 52 }, coord{ 73, 54 }, coord{ 82, 54 }, coord{ 98, 54 }, coord{ 109, 55 }, coord{ 115, 50 }, coord{ 10, 62 }, coord{ 22, 57 }, coord{ 24, 57 }, coord{ 41, 64 }, coord{ 55, 61 }, coord{ 62, 59 }, coord{ 69, 61 }, coord{ 84, 63 }, coord{ 91, 63 }, coord{ 101, 59 }, coord{ 117, 66 }, coord{ 11, 71 }, coord{ 22, 73 }, coord{ 28, 74 }, coord{ 42, 74 }, coord{ 49, 69 }, coord{ 63, 72 }, coord{ 68, 73 }, coord{ 87, 75 }, coord{ 94, 77 }, coord{ 101, 74 }, coord{ 118, 71 }, coord{ 9, 85 }, coord{ 19, 88 }, coord{ 31, 88 }, coord{ 38, 87 }, coord{ 47, 79 }, coord{ 63, 80 }, coord{ 77, 85 }, coord{ 87, 80 }, coord{ 97, 84 }, coord{ 104, 79 }, coord{ 121, 82 }, coord{ 2, 97 }, coord{ 22, 92 }, coord{ 32, 99 }, coord{ 36, 99 }, coord{ 54, 99 }, coord{ 62, 91 }, coord{ 71, 99 }, coord{ 79, 90 }, coord{ 97, 98 }, coord{ 108, 99 }, coord{ 121, 98 }, coord{ 4, 109 }, coord{ 20, 101 }, coord{ 25, 104 }, coord{ 36, 105 }, coord{ 55, 104 }, coord{ 61, 106 }, coord{ 71, 108 }, coord{ 84, 109 }, coord{ 93, 102 }, coord{ 110, 102 }, coord{ 112, 106 }, coord{ 3, 120 }, coord{ 20, 119 }, coord{ 26, 114 }, coord{ 35, 112 }, coord{ 50, 114 }, coord{ 63, 117 }, coord{ 76, 118 }, coord{ 79, 116 }, coord{ 99, 119 }, coord{ 105, 112 }, coord{ 120, 113 }, coord{ 3, 132 }, coord{ 15, 127 }, coord{ 31, 132 }, coord{ 44, 130 }, coord{ 49, 131 }, coord{ 64, 128 }, coord{ 69, 128 }, coord{ 79, 128 }, coord{ 94, 128 }, coord{ 104, 131 }, coord{ 120, 129 }, coord{ 5, 135 }, coord{ 16, 142 }, coord{ 27, 134 }, coord{ 39, 134 }, coord{ 53, 138 }, coord{ 57, 136 }, coord{ 69, 136 }, coord{ 83, 143 }, coord{ 99, 135 }, coord{ 104, 136 }, coord{ 119, 137 }, coord{ 4, 153 }, coord{ 22, 146 }, coord{ 28, 147 }, coord{ 44, 145 }, coord{ 54, 153 }, coord{ 64, 152 }, coord{ 74, 153 }, coord{ 88, 154 }, coord{ 98, 154 }, coord{ 110, 154 }, coord{ 118, 153 }, coord{ 10, 164 }, coord{ 22, 161 }, coord{ 28, 163 }, coord{ 44, 165 }, coord{ 48, 165 }, coord{ 58, 161 }, coord{ 68, 159 }, coord{ 82, 156 }, coord{ 98, 157 }, coord{ 110, 157 }, coord{ 118, 159 }, coord{ 8, 176 }, coord{ 15, 176 }, coord{ 25, 175 }, coord{ 44, 168 }, coord{ 52, 167 }, coord{ 62, 167 }, coord{ 68, 168 }, coord{ 79, 170 }, coord{ 98, 172 }, coord{ 104, 174 }, coord{ 114, 174 }, coord{ 5, 180 }, coord{ 22, 183 }, coord{ 25, 182 }, coord{ 37, 183 }, coord{ 55, 183 }, coord{ 61, 183 }, coord{ 73, 181 }, coord{ 79, 183 }, coord{ 95, 183 }, coord{ 107, 186 }, coord{ 119, 187 }, coord{ 8, 193 }, coord{ 16, 191 }, coord{ 31, 191 }, coord{ 44, 191 }, coord{ 48, 195 }, coord{ 61, 193 }, coord{ 70, 189 }, coord{ 80, 190 }, coord{ 96, 190 }, coord{ 106, 191 }, coord{ 115, 190 }, coord{ 10, 202 }, coord{ 16, 209 }, coord{ 28, 204 }, coord{ 44, 202 }, coord{ 50, 207 }, coord{ 66, 203 }, coord{ 76, 200 }, coord{ 86, 203 }, coord{ 90, 207 }, coord{ 106, 200 }, coord{ 118, 209 }, coord{ 8, 218 }, coord{ 16, 218 }, coord{ 26, 214 }, coord{ 36, 216 }, coord{ 48, 218 }, coord{ 62, 220 }, coord{ 74, 215 }, coord{ 86, 218 }, coord{ 90, 213 }, coord{ 110, 211 }, coord{ 118, 216 }, coord{ 2, 226 }, coord{ 22, 223 }, coord{ 30, 228 }, coord{ 44, 226 }, coord{ 50, 230 }, coord{ 66, 229 }, coord{ 72, 227 }, coord{ 80, 226 }, coord{ 98, 224 }, coord{ 110, 224 }, coord{ 114, 224 }, coord{ 10, 236 }, coord{ 20, 235 }, coord{ 26, 240 }, coord{ 38, 238 }, coord{ 48, 234 }, coord{ 66, 240 }, coord{ 74, 239 }, coord{ 86, 240 }, coord{ 98, 241 }, coord{ 104, 239 }, coord{ 120, 234 }, coord{ 8, 244 }, coord{ 20, 244 }, coord{ 28, 251 }, coord{ 38, 244 }, coord{ 54, 244 }, coord{ 60, 245 }, coord{ 70, 247 }, coord{ 84, 248 }, coord{ 92, 250 }, coord{ 108, 251 }, coord{ 120, 252 } };
//coord high_var_block256_collision44[297] = { coord{ 4, 7 }, coord{ 14, 9 }, coord{ 24, 10 }, coord{ 44, 2 }, coord{ 54, 2 }, coord{ 62, 2 }, coord{ 68, 2 }, coord{ 81, 3 }, coord{ 90, 9 }, coord{ 101, 11 }, coord{ 121, 8 }, coord{ 3, 15 }, coord{ 13, 22 }, coord{ 27, 20 }, coord{ 38, 21 }, coord{ 54, 18 }, coord{ 63, 19 }, coord{ 72, 21 }, coord{ 80, 17 }, coord{ 99, 17 }, coord{ 103, 17 }, coord{ 119, 21 }, coord{ 8, 28 }, coord{ 13, 24 }, coord{ 30, 32 }, coord{ 40, 24 }, coord{ 54, 27 }, coord{ 57, 30 }, coord{ 73, 32 }, coord{ 84, 32 }, coord{ 94, 30 }, coord{ 110, 31 }, coord{ 112, 31 }, coord{ 3, 40 }, coord{ 20, 41 }, coord{ 28, 36 }, coord{ 38, 35 }, coord{ 52, 44 }, coord{ 57, 44 }, coord{ 72, 43 }, coord{ 83, 35 }, coord{ 98, 35 }, coord{ 102, 35 }, coord{ 113, 35 }, coord{ 10, 55 }, coord{ 22, 54 }, coord{ 24, 55 }, coord{ 44, 48 }, coord{ 54, 51 }, coord{ 62, 52 }, coord{ 73, 54 }, coord{ 82, 54 }, coord{ 98, 54 }, coord{ 109, 55 }, coord{ 115, 50 }, coord{ 10, 62 }, coord{ 22, 57 }, coord{ 24, 57 }, coord{ 41, 64 }, coord{ 55, 61 }, coord{ 62, 59 }, coord{ 69, 61 }, coord{ 84, 63 }, coord{ 91, 63 }, coord{ 101, 59 }, coord{ 117, 66 }, coord{ 11, 71 }, coord{ 22, 73 }, coord{ 28, 74 }, coord{ 42, 74 }, coord{ 49, 69 }, coord{ 63, 72 }, coord{ 68, 73 }, coord{ 87, 75 }, coord{ 94, 77 }, coord{ 101, 74 }, coord{ 118, 71 }, coord{ 9, 85 }, coord{ 19, 88 }, coord{ 31, 88 }, coord{ 38, 87 }, coord{ 47, 79 }, coord{ 63, 80 }, coord{ 77, 85 }, coord{ 87, 80 }, coord{ 97, 84 }, coord{ 104, 79 }, coord{ 121, 82 }, coord{ 2, 97 }, coord{ 22, 92 }, coord{ 32, 99 }, coord{ 36, 99 }, coord{ 54, 99 }, coord{ 62, 91 }, coord{ 71, 99 }, coord{ 79, 90 }, coord{ 97, 98 }, coord{ 108, 99 }, coord{ 121, 98 }, coord{ 4, 109 }, coord{ 20, 101 }, coord{ 25, 104 }, coord{ 36, 105 }, coord{ 55, 104 }, coord{ 61, 106 }, coord{ 71, 108 }, coord{ 84, 109 }, coord{ 93, 102 }, coord{ 110, 102 }, coord{ 112, 106 }, coord{ 3, 120 }, coord{ 20, 119 }, coord{ 26, 114 }, coord{ 35, 112 }, coord{ 50, 114 }, coord{ 63, 117 }, coord{ 76, 118 }, coord{ 79, 116 }, coord{ 99, 119 }, coord{ 105, 112 }, coord{ 120, 113 }, coord{ 3, 132 }, coord{ 15, 127 }, coord{ 31, 132 }, coord{ 44, 130 }, coord{ 49, 131 }, coord{ 64, 128 }, coord{ 69, 128 }, coord{ 79, 128 }, coord{ 94, 128 }, coord{ 104, 131 }, coord{ 120, 129 }, coord{ 5, 135 }, coord{ 16, 142 }, coord{ 27, 134 }, coord{ 39, 134 }, coord{ 53, 138 }, coord{ 57, 136 }, coord{ 69, 136 }, coord{ 83, 143 }, coord{ 99, 135 }, coord{ 104, 136 }, coord{ 119, 137 }, coord{ 4, 153 }, coord{ 22, 146 }, coord{ 28, 147 }, coord{ 44, 145 }, coord{ 54, 153 }, coord{ 64, 152 }, coord{ 74, 153 }, coord{ 88, 154 }, coord{ 98, 154 }, coord{ 110, 154 }, coord{ 118, 153 }, coord{ 10, 164 }, coord{ 22, 161 }, coord{ 28, 163 }, coord{ 44, 165 }, coord{ 48, 165 }, coord{ 58, 161 }, coord{ 68, 159 }, coord{ 82, 156 }, coord{ 98, 157 }, coord{ 110, 157 }, coord{ 118, 159 }, coord{ 8, 176 }, coord{ 15, 176 }, coord{ 25, 175 }, coord{ 44, 168 }, coord{ 52, 167 }, coord{ 62, 167 }, coord{ 68, 168 }, coord{ 79, 170 }, coord{ 98, 172 }, coord{ 104, 174 }, coord{ 114, 174 }, coord{ 5, 180 }, coord{ 22, 183 }, coord{ 25, 182 }, coord{ 37, 183 }, coord{ 55, 183 }, coord{ 61, 183 }, coord{ 73, 181 }, coord{ 79, 183 }, coord{ 95, 183 }, coord{ 107, 186 }, coord{ 119, 187 }, coord{ 8, 193 }, coord{ 16, 191 }, coord{ 31, 191 }, coord{ 44, 191 }, coord{ 48, 195 }, coord{ 61, 193 }, coord{ 70, 189 }, coord{ 80, 190 }, coord{ 96, 190 }, coord{ 106, 191 }, coord{ 115, 190 }, coord{ 10, 202 }, coord{ 16, 209 }, coord{ 28, 204 }, coord{ 44, 202 }, coord{ 50, 207 }, coord{ 66, 203 }, coord{ 76, 200 }, coord{ 86, 203 }, coord{ 90, 207 }, coord{ 106, 200 }, coord{ 118, 209 }, coord{ 8, 218 }, coord{ 16, 218 }, coord{ 26, 214 }, coord{ 36, 216 }, coord{ 48, 218 }, coord{ 62, 220 }, coord{ 74, 215 }, coord{ 86, 218 }, coord{ 90, 213 }, coord{ 110, 211 }, coord{ 118, 216 }, coord{ 2, 226 }, coord{ 22, 223 }, coord{ 30, 228 }, coord{ 44, 226 }, coord{ 50, 230 }, coord{ 66, 229 }, coord{ 72, 227 }, coord{ 80, 226 }, coord{ 98, 224 }, coord{ 110, 224 }, coord{ 114, 224 }, coord{ 10, 236 }, coord{ 20, 235 }, coord{ 26, 240 }, coord{ 38, 238 }, coord{ 48, 234 }, coord{ 66, 240 }, coord{ 74, 239 }, coord{ 86, 240 }, coord{ 98, 241 }, coord{ 104, 239 }, coord{ 120, 234 }, coord{ 8, 244 }, coord{ 20, 244 }, coord{ 28, 251 }, coord{ 38, 244 }, coord{ 54, 244 }, coord{ 60, 245 }, coord{ 70, 247 }, coord{ 84, 248 }, coord{ 92, 250 }, coord{ 108, 251 }, coord{ 120, 252 }, coord{ 97, 198 }, coord{ 104, 198 }, coord{ 106, 198 }, coord{ 107, 198 }, coord{ 96, 198 }, coord{ 98, 198 }, coord{ 99, 198 }, coord{ 100, 198 }, coord{ 101, 198 }, coord{ 102, 198 }, coord{ 103, 198 }, coord{ 105, 198 }, coord{ 124, 193 }, coord{ 125, 193 }, coord{ 126, 193 }, coord{ 127, 193 }, coord{ 120, 193 }, coord{ 121, 193 }, coord{ 122, 193 }, coord{ 123, 193 }, coord{ 117, 193 }, coord{ 12, 225 }, coord{ 113, 193 }, coord{ 119, 193 }, coord{ 112, 193 }, coord{ 118, 193 }, coord{ 114, 193 }, coord{ 116, 193 }, coord{ 115, 193 }, coord{ 116, 197 }, coord{ 117, 197 }, coord{ 120, 196 }, coord{ 121, 196 }, coord{ 124, 196 }, coord{ 125, 196 }, coord{ 126, 196 }, coord{ 127, 196 }, coord{ 112, 197 }, coord{ 113, 197 }, coord{ 118, 197 }, coord{ 119, 197 }, coord{ 120, 197 }, coord{ 121, 197 }, coord{ 197, 124 } };

//coord high_var_block128_collision44[165] = { coord{ 4, 7 }, coord{ 15, 8 }, coord{ 24, 8 }, coord{ 39, 6 }, coord{ 55, 2 }, coord{ 57, 2 }, coord{ 72, 8 }, coord{ 84, 3 }, coord{ 90, 9 }, coord{ 103, 5 }, coord{ 121, 2 }, coord{ 3, 15 }, coord{ 13, 22 }, coord{ 28, 14 }, coord{ 44, 16 }, coord{ 54, 21 }, coord{ 58, 22 }, coord{ 72, 13 }, coord{ 82, 13 }, coord{ 98, 18 }, coord{ 103, 17 }, coord{ 117, 18 }, coord{ 4, 28 }, coord{ 18, 24 }, coord{ 31, 31 }, coord{ 35, 28 }, coord{ 55, 31 }, coord{ 61, 26 }, coord{ 73, 32 }, coord{ 87, 33 }, coord{ 94, 30 }, coord{ 108, 24 }, coord{ 112, 31 }, coord{ 7, 40 }, coord{ 14, 39 }, coord{ 31, 35 }, coord{ 35, 35 }, coord{ 49, 44 }, coord{ 61, 36 }, coord{ 77, 35 }, coord{ 83, 35 }, coord{ 95, 38 }, coord{ 102, 35 }, coord{ 113, 35 }, coord{ 10, 55 }, coord{ 15, 49 }, coord{ 25, 55 }, coord{ 44, 48 }, coord{ 47, 49 }, coord{ 61, 55 }, coord{ 77, 49 }, coord{ 81, 48 }, coord{ 90, 48 }, coord{ 104, 55 }, coord{ 114, 46 }, coord{ 10, 61 }, coord{ 16, 57 }, coord{ 31, 59 }, coord{ 41, 64 }, coord{ 47, 64 }, coord{ 62, 59 }, coord{ 69, 61 }, coord{ 85, 63 }, coord{ 95, 63 }, coord{ 101, 59 }, coord{ 117, 66 }, coord{ 7, 74 }, coord{ 21, 73 }, coord{ 28, 74 }, coord{ 42, 74 }, coord{ 49, 69 }, coord{ 62, 69 }, coord{ 72, 68 }, coord{ 83, 74 }, coord{ 94, 77 }, coord{ 103, 77 }, coord{ 118, 77 }, coord{ 2, 85 }, coord{ 20, 85 }, coord{ 29, 81 }, coord{ 38, 87 }, coord{ 47, 79 }, coord{ 63, 79 }, coord{ 77, 87 }, coord{ 88, 83 }, coord{ 97, 84 }, coord{ 105, 79 }, coord{ 113, 88 }, coord{ 7, 94 }, coord{ 18, 98 }, coord{ 32, 99 }, coord{ 36, 99 }, coord{ 54, 99 }, coord{ 62, 98 }, coord{ 71, 99 }, coord{ 80, 98 }, coord{ 97, 97 }, coord{ 108, 99 }, coord{ 113, 95 }, coord{ 4, 109 }, coord{ 20, 101 }, coord{ 25, 101 }, coord{ 35, 102 }, coord{ 51, 104 }, coord{ 61, 106 }, coord{ 71, 108 }, coord{ 84, 109 }, coord{ 95, 101 }, coord{ 108, 101 }, coord{ 112, 104 }, coord{ 11, 116 }, coord{ 13, 118 }, coord{ 27, 120 }, coord{ 37, 121 }, coord{ 52, 113 }, coord{ 59, 121 }, coord{ 77, 115 }, coord{ 85, 112 }, coord{ 93, 112 }, coord{ 104, 112 }, coord{ 115, 112 }, coord{ 127, 127 }, coord{ 127, 126 }, coord{ 16, 45 }, coord{ 18, 44 }, coord{ 118, 85 }, coord{ 18, 42 }, coord{ 20, 45 }, coord{ 36, 45 }, coord{ 124, 80 }, coord{ 119, 84 }, coord{ 124, 84 }, coord{ 119, 85 }, coord{ 119, 86 }, coord{ 120, 86 }, coord{ 119, 89 }, coord{ 8, 33 }, coord{ 18, 43 }, coord{ 19, 43 }, coord{ 17, 44 }, coord{ 19, 44 }, coord{ 20, 44 }, coord{ 37, 44 }, coord{ 9, 45 }, coord{ 21, 45 }, coord{ 26, 45 }, coord{ 27, 45 }, coord{ 28, 45 }, coord{ 29, 45 }, coord{ 30, 45 }, coord{ 31, 45 }, coord{ 32, 45 }, coord{ 33, 45 }, coord{ 34, 45 }, coord{ 35, 45 }, coord{ 38, 45 }, coord{ 120, 89 }, coord{ 121, 89 }, coord{ 106, 90 }, coord{ 99, 93 }, coord{ 100, 93 }, coord{ 68, 110 }, coord{ 61, 117 }, coord{ 77, 117 }, coord{ 93, 5 } };

// low_mode_block128
//const int FNV_COORDS_LEN = 121;
//coord FNV_COORDS[FNV_COORDS_LEN] = { coord{ 11, 9 }, coord{ 22, 7 }, coord{ 28, 7 }, coord{ 39, 9 }, coord{ 53, 9 }, coord{ 60, 7 }, coord{ 76, 11 }, coord{ 88, 8 }, coord{ 91, 11 }, coord{ 102, 7 }, coord{ 115, 9 }, coord{ 11, 15 }, coord{ 22, 19 }, coord{ 28, 20 }, coord{ 40, 22 }, coord{ 54, 16 }, coord{ 60, 17 }, coord{ 76, 17 }, coord{ 87, 20 }, coord{ 91, 20 }, coord{ 107, 20 }, coord{ 115, 13 }, coord{ 11, 24 }, coord{ 22, 29 }, coord{ 28, 30 }, coord{ 39, 29 }, coord{ 46, 30 }, coord{ 60, 30 }, coord{ 70, 30 }, coord{ 87, 25 }, coord{ 93, 25 }, coord{ 102, 27 }, coord{ 115, 33 }, coord{ 11, 38 }, coord{ 22, 39 }, coord{ 28, 44 }, coord{ 39, 41 }, coord{ 55, 35 }, coord{ 60, 38 }, coord{ 70, 40 }, coord{ 87, 37 }, coord{ 99, 44 }, coord{ 102, 43 }, coord{ 115, 35 }, coord{ 11, 50 }, coord{ 21, 46 }, coord{ 25, 51 }, coord{ 40, 48 }, coord{ 46, 46 }, coord{ 60, 46 }, coord{ 76, 47 }, coord{ 87, 47 }, coord{ 93, 49 }, coord{ 107, 53 }, coord{ 115, 49 }, coord{ 11, 61 }, coord{ 22, 59 }, coord{ 25, 59 }, coord{ 40, 59 }, coord{ 55, 57 }, coord{ 60, 61 }, coord{ 70, 58 }, coord{ 87, 59 }, coord{ 99, 58 }, coord{ 102, 59 }, coord{ 115, 57 }, coord{ 7, 77 }, coord{ 21, 77 }, coord{ 24, 71 }, coord{ 36, 77 }, coord{ 46, 76 }, coord{ 60, 77 }, coord{ 70, 70 }, coord{ 87, 71 }, coord{ 93, 77 }, coord{ 102, 69 }, coord{ 115, 77 }, coord{ 11, 84 }, coord{ 21, 80 }, coord{ 27, 85 }, coord{ 39, 79 }, coord{ 55, 85 }, coord{ 60, 81 }, coord{ 75, 86 }, coord{ 82, 84 }, coord{ 93, 84 }, coord{ 107, 79 }, coord{ 115, 79 }, coord{ 11, 92 }, coord{ 22, 97 }, coord{ 28, 90 }, coord{ 40, 93 }, coord{ 46, 92 }, coord{ 60, 91 }, coord{ 76, 97 }, coord{ 82, 98 }, coord{ 93, 90 }, coord{ 102, 99 }, coord{ 115, 99 }, coord{ 6, 107 }, coord{ 22, 101 }, coord{ 31, 102 }, coord{ 41, 108 }, coord{ 55, 107 }, coord{ 60, 107 }, coord{ 70, 104 }, coord{ 87, 101 }, coord{ 93, 105 }, coord{ 102, 101 }, coord{ 115, 109 }, coord{ 11, 112 }, coord{ 21, 112 }, coord{ 27, 113 }, coord{ 41, 112 }, coord{ 55, 113 }, coord{ 60, 120 }, coord{ 70, 116 }, coord{ 82, 112 }, coord{ 93, 113 }, coord{ 107, 117 }, coord{ 115, 113 } };

// low_mode_prog_collisions
//const int FNV_COORDS_LEN = 500;
//coord FNV_COORDS[FNV_COORDS_LEN] = { coord{ 38, 36, -1 }, coord{ 36, 35, -1 }, coord{ 41, 37, -1 }, coord{ 28, 12, -1 }, coord{ 18, 8, -1 }, coord{ 12, 10, -1 }, coord{ 19, 8, -1 }, coord{ 33, 102, -1 }, coord{ 20, 18, -1 }, coord{ 4, 27, -1 }, coord{ 17, 8, -1 }, coord{ 10, 105, -1 }, coord{ 12, 39, -1 }, coord{ 17, 5, -1 }, coord{ 19, 7, -1 }, coord{ 16, 8, -1 }, coord{ 17, 9, -1 }, coord{ 16, 9, -1 }, coord{ 39, 7, -1 }, coord{ 18, 10, -1 }, coord{ 12, 37, -1 }, coord{ 19, 9, -1 }, coord{ 3, 15, -1 }, coord{ 18, 9, -1 }, coord{ 5, 18, -1 }, coord{ 19, 6, -1 }, coord{ 42, 36, -1 }, coord{ 4, 16, -1 }, coord{ 20, 27, -1 }, coord{ 18, 5, -1 }, coord{ 3, 16, -1 }, coord{ 2, 7, -1 }, coord{ 0, 22, -1 }, coord{ 23, 9, -1 }, coord{ 19, 15, -1 }, coord{ 4, 49, -1 }, coord{ 9, 54, -1 }, coord{ 33, 56, -1 }, coord{ 13, 59, -1 }, coord{ 33, 58, -1 }, coord{ 18, 7, -1 }, coord{ 13, 102, -1 }, coord{ 10, 104, -1 }, coord{ 11, 7, -1 }, coord{ 16, 11, -1 }, coord{ 18, 4, -1 }, coord{ 12, 8, -1 }, coord{ 10, 103, -1 }, coord{ 11, 39, -1 }, coord{ 16, 6, -1 }, coord{ 13, 9, -1 }, coord{ 17, 10, -1 }, coord{ 35, 36, -1 }, coord{ 41, 51, -1 }, coord{ 20, 8, -1 }, coord{ 13, 17, -1 }, coord{ 39, 56, -1 }, coord{ 22, 10, -1 }, coord{ 41, 57, -1 }, coord{ 15, 9, -1 }, coord{ 17, 6, -1 }, coord{ 40, 9, -1 }, coord{ 38, 39, -1 }, coord{ 83, 5, -1 }, coord{ 39, 9, -1 }, coord{ 20, 9, -1 }, coord{ 21, 9, -1 }, coord{ 23, 10, -1 }, coord{ 11, 16, -1 }, coord{ 2, 3, -1 }, coord{ 41, 40, -1 }, coord{ 16, 49, -1 }, coord{ 23, 7, -1 }, coord{ 17, 4, -1 }, coord{ 19, 53, -1 }, coord{ 34, 58, -1 }, coord{ 19, 16, -1 }, coord{ 35, 49, -1 }, coord{ 16, 72, -1 }, coord{ 10, 108, -1 }, coord{ 10, 109, -1 }, coord{ 12, 104, -1 }, coord{ 12, 100, -1 }, coord{ 11, 105, -1 }, coord{ 2, 83, -1 }, coord{ 11, 104, -1 }, coord{ 11, 103, -1 }, coord{ 11, 108, -1 }, coord{ 36, 100, -1 }, coord{ 2, 49, -1 }, coord{ 10, 107, -1 }, coord{ 40, 48, -1 }, coord{ 37, 54, -1 }, coord{ 37, 55, -1 }, coord{ 39, 57, -1 }, coord{ 82, 74, -1 }, coord{ 11, 110, -1 }, coord{ 4, 40, -1 }, coord{ 2, 4, -1 }, coord{ 4, 2, -1 }, coord{ 2, 6, -1 }, coord{ 12, 107, -1 }, coord{ 16, 5, -1 }, coord{ 35, 35, -1 }, coord{ 2, 1, -1 }, coord{ 12, 9, -1 }, coord{ 33, 57, -1 }, coord{ 38, 101, -1 }, coord{ 4, 5, -1 }, coord{ 3, 19, -1 }, coord{ 4, 19, -1 }, coord{ 4, 34, -1 }, coord{ 6, 41, -1 }, coord{ 41, 36, -1 }, coord{ 41, 56, -1 }, coord{ 32, 57, -1 }, coord{ 11, 102, -1 }, coord{ 35, 102, -1 }, coord{ 4, 105, -1 }, coord{ 11, 109, -1 }, coord{ 35, 48, -1 }, coord{ 18, 105, -1 }, coord{ 4, 0, -1 }, coord{ 39, 38, -1 }, coord{ 42, 38, -1 }, coord{ 38, 46, -1 }, coord{ 34, 59, -1 }, coord{ 4, 107, -1 }, coord{ 16, 108, -1 }, coord{ 36, 45, -1 }, coord{ 1, 7, -1 }, coord{ 7, 37, -1 }, coord{ 2, 5, -1 }, coord{ 3, 17, -1 }, coord{ 6, 39, -1 }, coord{ 36, 0, -1 }, coord{ 3, 6, -1 }, coord{ 14, 10, -1 }, coord{ 5, 16, -1 }, coord{ 4, 17, -1 }, coord{ 6, 33, -1 }, coord{ 10, 50, -1 }, coord{ 5, 17, -1 }, coord{ 34, 37, -1 }, coord{ 17, 69, -1 }, coord{ 36, 34, -1 }, coord{ 40, 49, -1 }, coord{ 4, 48, -1 }, coord{ 41, 49, -1 }, coord{ 2, 24, -1 }, coord{ 10, 35, -1 }, coord{ 41, 50, -1 }, coord{ 35, 58, -1 }, coord{ 12, 103, -1 }, coord{ 36, 103, -1 }, coord{ 13, 5, -1 }, coord{ 13, 10, -1 }, coord{ 4, 13, -1 }, coord{ 8, 18, -1 }, coord{ 36, 49, -1 }, coord{ 37, 51, -1 }, coord{ 35, 37, -1 }, coord{ 43, 37, -1 }, coord{ 4, 50, -1 }, coord{ 39, 50, -1 }, coord{ 33, 6, -1 }, coord{ 41, 39, -1 }, coord{ 43, 40, -1 }, coord{ 42, 44, -1 }, coord{ 39, 49, -1 }, coord{ 38, 50, -1 }, coord{ 17, 51, -1 }, coord{ 40, 41, -1 }, coord{ 35, 59, -1 }, coord{ 35, 103, -1 }, coord{ 9, 7, -1 }, coord{ 14, 18, -1 }, coord{ 43, 36, -1 }, coord{ 42, 45, -1 }, coord{ 10, 51, -1 }, coord{ 19, 51, -1 }, coord{ 35, 51, -1 }, coord{ 39, 51, -1 }, coord{ 41, 55, -1 }, coord{ 12, 66, -1 }, coord{ 11, 40, -1 }, coord{ 12, 71, -1 }, coord{ 11, 107, -1 }, coord{ 16, 112, -1 }, coord{ 43, 56, -1 }, coord{ 36, 7, -1 }, coord{ 11, 53, -1 }, coord{ 38, 56, -1 }, coord{ 80, 74, -1 }, coord{ 82, 75, -1 }, coord{ 35, 13, -1 }, coord{ 13, 67, -1 }, coord{ 10, 113, -1 }, coord{ 38, 48, -1 }, coord{ 43, 52, -1 }, coord{ 35, 55, -1 }, coord{ 36, 59, -1 }, coord{ 18, 67, -1 }, coord{ 70, 1, -1 }, coord{ 20, 4, -1 }, coord{ 15, 5, -1 }, coord{ 13, 7, -1 }, coord{ 36, 13, -1 }, coord{ 34, 14, -1 }, coord{ 40, 37, -1 }, coord{ 27, 49, -1 }, coord{ 17, 54, -1 }, coord{ 12, 108, -1 }, coord{ 12, 110, -1 }, coord{ 13, 110, -1 }, coord{ 17, 50, -1 }, coord{ 13, 108, -1 }, coord{ 12, 113, -1 }, coord{ 39, 5, -1 }, coord{ 38, 34, -1 }, coord{ 40, 38, -1 }, coord{ 41, 38, -1 }, coord{ 39, 40, -1 }, coord{ 4, 52, -1 }, coord{ 11, 56, -1 }, coord{ 12, 70, -1 }, coord{ 4, 42, -1 }, coord{ 42, 51, -1 }, coord{ 18, 103, -1 }, coord{ 17, 7, -1 }, coord{ 43, 39, -1 }, coord{ 16, 102, -1 }, coord{ 4, 14, -1 }, coord{ 14, 16, -1 }, coord{ 4, 21, -1 }, coord{ 18, 41, -1 }, coord{ 43, 41, -1 }, coord{ 39, 53, -1 }, coord{ 39, 54, -1 }, coord{ 41, 54, -1 }, coord{ 43, 54, -1 }, coord{ 39, 55, -1 }, coord{ 12, 57, -1 }, coord{ 43, 57, -1 }, coord{ 39, 58, -1 }, coord{ 18, 70, -1 }, coord{ 81, 75, -1 }, coord{ 13, 105, -1 }, coord{ 14, 105, -1 }, coord{ 10, 106, -1 }, coord{ 10, 110, -1 }, coord{ 14, 111, -1 }, coord{ 21, 5, -1 }, coord{ 18, 6, -1 }, coord{ 30, 9, -1 }, coord{ 85, 10, -1 }, coord{ 8, 26, -1 }, coord{ 80, 75, -1 }, coord{ 98, 4, -1 }, coord{ 14, 81, -1 }, coord{ 17, 56, -1 }, coord{ 13, 107, -1 }, coord{ 12, 111, -1 }, coord{ 40, 8, -1 }, coord{ 38, 45, -1 }, coord{ 25, 48, -1 }, coord{ 10, 49, -1 }, coord{ 17, 49, -1 }, coord{ 19, 49, -1 }, coord{ 40, 50, -1 }, coord{ 38, 51, -1 }, coord{ 10, 52, -1 }, coord{ 41, 53, -1 }, coord{ 35, 56, -1 }, coord{ 69, 56, -1 }, coord{ 10, 111, -1 }, coord{ 41, 112, -1 }, coord{ 16, 114, -1 }, coord{ 43, 38, -1 }, coord{ 41, 48, -1 }, coord{ 14, 17, -1 }, coord{ 9, 18, -1 }, coord{ 40, 36, -1 }, coord{ 39, 39, -1 }, coord{ 41, 41, -1 }, coord{ 39, 42, -1 }, coord{ 27, 50, -1 }, coord{ 27, 51, -1 }, coord{ 42, 57, -1 }, coord{ 99, 8, -1 }, coord{ 18, 107, -1 }, coord{ 83, 11, -1 }, coord{ 27, 15, -1 }, coord{ 26, 20, -1 }, coord{ 20, 35, -1 }, coord{ 42, 39, -1 }, coord{ 106, 40, -1 }, coord{ 16, 41, -1 }, coord{ 38, 43, -1 }, coord{ 38, 44, -1 }, coord{ 99, 2, -1 }, coord{ 51, 9, -1 }, coord{ 22, 18, -1 }, coord{ 38, 33, -1 }, coord{ 41, 42, -1 }, coord{ 17, 100, -1 }, coord{ 27, 16, -1 }, coord{ 7, 17, -1 }, coord{ 28, 18, -1 }, coord{ 19, 52, -1 }, coord{ 38, 55, -1 }, coord{ 65, 56, -1 }, coord{ 41, 58, -1 }, coord{ 99, 10, -1 }, coord{ 39, 11, -1 }, coord{ 17, 48, -1 }, coord{ 12, 109, -1 }, coord{ 21, 6, -1 }, coord{ 41, 9, -1 }, coord{ 25, 16, -1 }, coord{ 27, 18, -1 }, coord{ 40, 46, -1 }, coord{ 19, 50, -1 }, coord{ 21, 51, -1 }, coord{ 17, 52, -1 }, coord{ 12, 59, -1 }, coord{ 11, 111, -1 }, coord{ 40, 112, -1 }, coord{ 11, 8, -1 }, coord{ 40, 10, -1 }, coord{ 51, 19, -1 }, coord{ 38, 42, -1 }, coord{ 36, 46, -1 }, coord{ 47, 51, -1 }, coord{ 43, 53, -1 }, coord{ 40, 56, -1 }, coord{ 40, 57, -1 }, coord{ 82, 67, -1 }, coord{ 18, 104, -1 }, coord{ 15, 7, -1 }, coord{ 20, 7, -1 }, coord{ 17, 11, -1 }, coord{ 21, 11, -1 }, coord{ 18, 13, -1 }, coord{ 40, 39, -1 }, coord{ 19, 48, -1 }, coord{ 42, 49, -1 }, coord{ 65, 55, -1 }, coord{ 65, 57, -1 }, coord{ 22, 69, -1 }, coord{ 17, 102, -1 }, coord{ 13, 103, -1 }, coord{ 13, 111, -1 }, coord{ 11, 112, -1 }, coord{ 41, 5, -1 }, coord{ 84, 5, -1 }, coord{ 13, 16, -1 }, coord{ 51, 20, -1 }, coord{ 20, 23, -1 }, coord{ 43, 7, -1 }, coord{ 30, 10, -1 }, coord{ 39, 10, -1 }, coord{ 13, 11, -1 }, coord{ 36, 15, -1 }, coord{ 15, 17, -1 }, coord{ 21, 17, -1 }, coord{ 8, 22, -1 }, coord{ 0, 24, -1 }, coord{ 39, 41, -1 }, coord{ 16, 47, -1 }, coord{ 19, 54, -1 }, coord{ 38, 54, -1 }, coord{ 65, 54, -1 }, coord{ 43, 58, -1 }, coord{ 12, 99, -1 }, coord{ 13, 109, -1 }, coord{ 21, 7, -1 }, coord{ 53, 9, -1 }, coord{ 87, 10, -1 }, coord{ 20, 11, -1 }, coord{ 21, 13, -1 }, coord{ 38, 14, -1 }, coord{ 10, 26, -1 }, coord{ 10, 27, -1 }, coord{ 4, 28, -1 }, coord{ 40, 33, -1 }, coord{ 16, 42, -1 }, coord{ 35, 45, -1 }, coord{ 36, 47, -1 }, coord{ 18, 48, -1 }, coord{ 18, 51, -1 }, coord{ 40, 52, -1 }, coord{ 42, 54, -1 }, coord{ 20, 56, -1 }, coord{ 23, 56, -1 }, coord{ 12, 58, -1 }, coord{ 38, 102, -1 }, coord{ 17, 104, -1 }, coord{ 18, 106, -1 }, coord{ 14, 109, -1 }, coord{ 18, 112, -1 }, coord{ 10, 114, -1 }, coord{ 98, 2, -1 }, coord{ 31, 6, -1 }, coord{ 40, 6, -1 }, coord{ 41, 7, -1 }, coord{ 9, 9, -1 }, coord{ 21, 10, -1 }, coord{ 23, 12, -1 }, coord{ 18, 16, -1 }, coord{ 31, 19, -1 }, coord{ 6, 23, -1 }, coord{ 107, 27, -1 }, coord{ 98, 11, -1 }, coord{ 36, 33, -1 }, coord{ 40, 40, -1 }, coord{ 36, 43, -1 }, coord{ 39, 44, -1 }, coord{ 51, 53, -1 }, coord{ 106, 54, -1 }, coord{ 34, 57, -1 }, coord{ 35, 104, -1 }, coord{ 36, 105, -1 }, coord{ 14, 7, -1 }, coord{ 31, 10, -1 }, coord{ 17, 13, -1 }, coord{ 22, 13, -1 }, coord{ 17, 16, -1 }, coord{ 12, 26, -1 }, coord{ 38, 41, -1 }, coord{ 18, 43, -1 }, coord{ 40, 53, -1 }, coord{ 40, 54, -1 }, coord{ 55, 54, -1 }, coord{ 22, 56, -1 }, coord{ 21, 57, -1 }, coord{ 52, 110, -1 }, coord{ 25, 50, -1 }, coord{ 18, 61, -1 }, coord{ 12, 112, -1 }, coord{ 41, 8, -1 }, coord{ 16, 27, -1 }, coord{ 102, 33, -1 }, coord{ 35, 34, -1 }, coord{ 40, 35, -1 }, coord{ 106, 41, -1 }, coord{ 40, 42, -1 }, coord{ 49, 48, -1 }, coord{ 46, 50, -1 }, coord{ 42, 55, -1 }, coord{ 42, 59, -1 }, coord{ 82, 68, -1 }, coord{ 102, 71, -1 }, coord{ 22, 12, -1 }, coord{ 17, 108, -1 }, coord{ 36, 14, -1 }, coord{ 21, 56, -1 }, coord{ 34, 81, -1 }, coord{ 53, 109, -1 }, coord{ 6, 112, -1 }, coord{ 14, 5, -1 }, coord{ 82, 26, -1 }, coord{ 38, 32, -1 }, coord{ 40, 58, -1 }, coord{ 37, 104, -1 }, coord{ 12, 19, -1 }, coord{ 7, 21, -1 }, coord{ 40, 34, -1 }, coord{ 39, 43, -1 }, coord{ 41, 43, -1 }, coord{ 43, 44, -1 }, coord{ 39, 45, -1 }, coord{ 46, 110, -1 }, coord{ 18, 111, -1 }, coord{ 19, 112, -1 }, coord{ 20, 112, -1 }, coord{ 37, 112, -1 }, coord{ 16, 2, -1 }, coord{ 6, 11, -1 }, coord{ 17, 15, -1 }, coord{ 25, 17, -1 }, coord{ 28, 17, -1 }, coord{ 24, 24, -1 }, coord{ 24, 27, -1 }, coord{ 21, 50, -1 }, coord{ 21, 52, -1 }, coord{ 21, 53, -1 }, coord{ 47, 57, -1 }, coord{ 100, 83, -1 }, coord{ 11, 113, -1 }, coord{ 18, 102, -1 }, coord{ 20, 102, -1 }, coord{ 31, 102, -1 }, coord{ 22, 11, -1 }, coord{ 13, 19, -1 }, coord{ 28, 19, -1 }, coord{ 12, 21, -1 }, coord{ 55, 22, -1 }, coord{ 9, 23, -1 }, coord{ 64, 24, -1 } };

// low_mode_block128 8x8
const int FNV_COORDS_LEN = 196;
coord FNV_COORDS[FNV_COORDS_LEN] = { coord{ 2, 9 }, coord{ 18, 8 }, coord{ 24, 8 }, coord{ 36, 9 }, coord{ 40, 9 }, coord{ 50, 8 }, coord{ 60, 9 }, coord{ 66, 9 }, coord{ 74, 9 }, coord{ 84, 9 }, coord{ 92, 3 }, coord{ 102, 9 }, coord{ 110, 9 }, coord{ 124, 8 }, coord{ 8, 15 }, coord{ 18, 14 }, coord{ 22, 15 }, coord{ 34, 15 }, coord{ 42, 15 }, coord{ 48, 14 }, coord{ 60, 11 }, coord{ 66, 12 }, coord{ 80, 14 }, coord{ 90, 11 }, coord{ 98, 12 }, coord{ 102, 13 }, coord{ 114, 14 }, coord{ 122, 18 }, coord{ 4, 27 }, coord{ 18, 26 }, coord{ 26, 27 }, coord{ 36, 26 }, coord{ 40, 27 }, coord{ 52, 24 }, coord{ 62, 27 }, coord{ 66, 26 }, coord{ 78, 26 }, coord{ 88, 26 }, coord{ 98, 26 }, coord{ 102, 26 }, coord{ 114, 22 }, coord{ 124, 27 }, coord{ 2, 36 }, coord{ 18, 36 }, coord{ 22, 35 }, coord{ 32, 30 }, coord{ 40, 29 }, coord{ 54, 29 }, coord{ 56, 30 }, coord{ 66, 30 }, coord{ 74, 30 }, coord{ 86, 30 }, coord{ 92, 31 }, coord{ 102, 33 }, coord{ 110, 33 }, coord{ 124, 30 }, coord{ 2, 44 }, coord{ 18, 42 }, coord{ 22, 44 }, coord{ 30, 40 }, coord{ 38, 38 }, coord{ 54, 45 }, coord{ 60, 45 }, coord{ 66, 38 }, coord{ 80, 44 }, coord{ 88, 38 }, coord{ 94, 45 }, coord{ 102, 38 }, coord{ 110, 45 }, coord{ 124, 43 }, coord{ 8, 52 }, coord{ 16, 47 }, coord{ 22, 47 }, coord{ 36, 47 }, coord{ 38, 47 }, coord{ 52, 51 }, coord{ 56, 54 }, coord{ 70, 49 }, coord{ 74, 52 }, coord{ 90, 50 }, coord{ 92, 47 }, coord{ 108, 47 }, coord{ 110, 47 }, coord{ 124, 47 }, coord{ 8, 60 }, coord{ 12, 60 }, coord{ 26, 60 }, coord{ 36, 56 }, coord{ 40, 58 }, coord{ 50, 56 }, coord{ 56, 56 }, coord{ 72, 56 }, coord{ 74, 56 }, coord{ 88, 58 }, coord{ 98, 56 }, coord{ 102, 56 }, coord{ 114, 62 }, coord{ 122, 62 }, coord{ 8, 72 }, coord{ 12, 67 }, coord{ 22, 72 }, coord{ 34, 72 }, coord{ 40, 65 }, coord{ 48, 67 }, coord{ 56, 70 }, coord{ 66, 72 }, coord{ 74, 68 }, coord{ 88, 65 }, coord{ 98, 67 }, coord{ 104, 67 }, coord{ 114, 67 }, coord{ 120, 67 }, coord{ 2, 74 }, coord{ 18, 78 }, coord{ 24, 78 }, coord{ 30, 75 }, coord{ 38, 80 }, coord{ 50, 74 }, coord{ 56, 76 }, coord{ 66, 79 }, coord{ 76, 75 }, coord{ 86, 81 }, coord{ 98, 76 }, coord{ 104, 74 }, coord{ 112, 80 }, coord{ 126, 75 }, coord{ 8, 84 }, coord{ 12, 83 }, coord{ 23, 83 }, coord{ 36, 85 }, coord{ 38, 83 }, coord{ 48, 87 }, coord{ 56, 86 }, coord{ 70, 90 }, coord{ 74, 83 }, coord{ 86, 90 }, coord{ 92, 83 }, coord{ 108, 83 }, coord{ 112, 83 }, coord{ 120, 90 }, coord{ 8, 92 }, coord{ 18, 94 }, coord{ 26, 92 }, coord{ 34, 92 }, coord{ 38, 92 }, coord{ 50, 92 }, coord{ 56, 92 }, coord{ 70, 92 }, coord{ 80, 92 }, coord{ 86, 95 }, coord{ 94, 93 }, coord{ 102, 99 }, coord{ 114, 92 }, coord{ 124, 93 }, coord{ 9, 105 }, coord{ 16, 102 }, coord{ 24, 102 }, coord{ 31, 103 }, coord{ 41, 108 }, coord{ 51, 104 }, coord{ 56, 102 }, coord{ 67, 108 }, coord{ 81, 108 }, coord{ 90, 108 }, coord{ 92, 102 }, coord{ 106, 102 }, coord{ 110, 101 }, coord{ 119, 103 }, coord{ 2, 114 }, coord{ 18, 112 }, coord{ 24, 112 }, coord{ 36, 115 }, coord{ 40, 116 }, coord{ 48, 111 }, coord{ 56, 112 }, coord{ 72, 111 }, coord{ 78, 111 }, coord{ 90, 111 }, coord{ 92, 111 }, coord{ 102, 111 }, coord{ 110, 111 }, coord{ 124, 111 }, coord{ 2, 120 }, coord{ 18, 124 }, coord{ 24, 126 }, coord{ 36, 119 }, coord{ 38, 119 }, coord{ 54, 124 }, coord{ 60, 121 }, coord{ 66, 122 }, coord{ 74, 122 }, coord{ 86, 119 }, coord{ 98, 124 }, coord{ 104, 120 }, coord{ 114, 124 }, coord{ 124, 119 } };

uint64 Hash_Algorithm_1(const cv::Mat& img)
{
	uint64 hash = 0;
	float first_val, val = 0, last_val = 0;

	cv::Vec3b pixel = img.at<cv::Vec3b>(hash1[0].y, hash1[0].x);
	first_val = last_val = (pixel[0] + pixel[1] + pixel[2]) / 3;

	coord* point = hash1;
	for (int i = 0; i < 64; i++, point++) {

		// get average pixel value
		if (point->x < img.cols && point->y < img.rows) {
			cv::Vec3b pixel = img.at<cv::Vec3b>(point->y, point->x);
			val = (pixel[0] + pixel[1] + pixel[2]) / 3;
		}
		else
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
		for (y = 0; y < 4; y++)
		{
			if (x * BLOCKSIZE < img.cols && y * BLOCKSIZE < img.rows) {
				cv::Vec3b pixel = img.at<cv::Vec3b>(y * BLOCKSIZE, x * BLOCKSIZE);
				pixval[pix] = (pixel[0] + pixel[1] + pixel[2]) / 3;
			}
			pix++;
		}
	}

	for (x = 0; x < 2; x++) //pixvals 32->55
	{
		for (y = 4; y < 16; y++)
		{
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
	for (int i = 0; i < 64; i++)
	{
		hashval *= 2;
		if ((pixval[i] - lastpixel) >= 0) { hashval++; }
		lastpixel = pixval[i];
	}

	return hashval;
}


uint64 FNV_HASH_LEN_64 = 64;
uint64 FNV_MODULO_64 = 1 << FNV_HASH_LEN_64;
uint64 FNV_OFFSET_BASIS_64 = 14695981039346656037;
uint64 FNV_OFFSET_PRIME_64 = 1099511628211;


uint32 FNV_HASH_LEN_32 = 32;
uint32 FNV_MODULO_32 = 1 << FNV_HASH_LEN_32;
uint32 FNV_OFFSET_BASIS_32 = 2166136261;
uint32 FNV_OFFSET_PRIME_32 = 16777619;

//template<typename container>
//typename boost::enable_if<boost::is_same<typename container::value_type, coord>, uint64>::type
//FNV_Hash(const cv::Mat& img, container& coords, bool use_RGB = true)
uint64 FNV_Hash(const cv::Mat& img, coord* coords, int len, bool use_RGB = true)
{
	uint64 hash = FNV_OFFSET_BASIS_64;

	size_t coord_count = 0;
	coord* point = coords;
	for (int i = 0; i < len; i++, point++) {
		uchar val = 0, red = 0, green = 0, blue = 0;
		if (point->x < img.cols && point->y < img.rows) {
			cv::Vec3b pixel = img.at<cv::Vec3b>(point->y, point->x);
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


uint64 FNV_Hash(const cv::Mat& img, deque<coord> coords, bool use_RGB = true)
{
	uint64 hash = FNV_OFFSET_BASIS_64;

	float red = 0, green = 0, blue = 0;
	size_t coord_count = 0;
	for (coord point : coords) {
		uchar val = 0;
		if (point.x < img.cols && point.y < img.rows) {
			cv::Vec3b pixel = img.at<cv::Vec3b>(point.y, point.x);
			if (use_RGB) {
				if (point.c != point.none) {
					val = pixel[point.c];
				} else {
					red = pixel[0];
					green = pixel[1];
					blue = pixel[2];
				}
			} else {
				val = round((pixel[0] + pixel[1] + pixel[2]) / 3);
			}
		}

		if (use_RGB && point.c == point.none) {										// hash each R,G,B individually
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

uint64 MURMUR2_SEED = 0x6d6176697269636b;

uint64 Murmur2_Full(const cv::Mat& img)
{
	int buflen = img.rows * img.cols * 3;
	char* buf = new char[buflen];
	int index = 0;

	for (int y = 0; y < img.rows; y++) {
		cv::Mat row = img.row(y);
		for (int x = 0; x < row.cols; x++) {
			cv::Vec3b pixel = row.at<cv::Vec3b>(0, x);
			for (int c = 0; c < 3; c++)
				buf[index++] = pixel[c];
		}
	}

	uint64 hash = MurmurHash64B(buf, buflen, MURMUR2_SEED);
	delete[] buf;
	return hash;
}

uint64 Murmur2_Hash(const cv::Mat& img, coord* coords, int len, bool use_RGB = true)
{
	int buflen = (use_RGB) ? len * 3 : len;
	char* buf = new char[buflen];
	int index = 0;

	coord* point = coords;
	for (int i = 0; i < len; i++, point++) {
		uchar val = 0, red = 0, green = 0, blue = 0;
		if (point->x < img.cols && point->y < img.rows) {
			cv::Vec3b pixel = img.at<cv::Vec3b>(point->y, point->x);
			if (use_RGB) {
				red = pixel[0];
				green = pixel[1];
				blue = pixel[2];
			} else if (point->c != coord::none) {
				val = pixel[point->c];
			} else {
				val = round((pixel[0] + pixel[1] + pixel[2]) / 3);
			}
		}
		if (use_RGB) {
			buf[index++] = red;
			buf[index++] = green;
			buf[index++] = blue;
		} else
			buf[index++] = val;
	}

	uint64 hash = MurmurHash64B(buf, buflen, MURMUR2_SEED);
	delete[] buf;
	return hash;
}


uint64 Murmur2_Hash(const cv::Mat& img, deque<coord> coords, bool use_RGB = true)
{
	int buflen = (use_RGB) ? coords.size() * 3 : coords.size();
	char* buf = new char[buflen];
	int index = 0;

	for (coord point : coords) {
		uchar val = 0, red = 0, green = 0, blue = 0;
		if (point.x < img.cols && point.y < img.rows) {
			cv::Vec3b pixel = img.at<cv::Vec3b>(point.y, point.x);
			if(use_RGB) {
				red = pixel[0];
				green = pixel[1];
				blue = pixel[2];
			} else if (point.c != coord::none) {
				val = pixel[point.c];
			} else {
				val = round((pixel[0] + pixel[1] + pixel[2]) / 3);
			}
		}
		if (use_RGB) {
			buf[index++] = red;
			buf[index++] = green;
			buf[index++] = blue;
		} else
			buf[index++] = val;
	}

	uint64 hash = MurmurHash64B(buf, buflen, MURMUR2_SEED);
	delete[] buf;
	return hash;
}


void Copy_Unique_Left_Half(fs::path debug, fs::path dest)										// TODO: STOP USING MD5!
{
	unordered_set<string> md5s;

	try {
		fs::directory_iterator iter_end;
		for (fs::directory_iterator debug_outer_dir(debug); debug_outer_dir != iter_end; debug_outer_dir++) {
			// [FFVIII]\tonberry\debug\*
			string dirname = debug_outer_dir->path().filename().string();
			// skip files and directories that end in 0
			if (!fs::is_directory(debug_outer_dir->path()) || dirname.substr(dirname.length() - 1, 1) == "0") continue;
			for (fs::directory_iterator texture(debug_outer_dir->path()); texture != iter_end; texture++) {
				// [FFVIII]\tonberry\debug\debug_outer_dir\*
				string tex_name = texture->path().string();
				cv::Mat img = cv::imread(tex_name, CV_LOAD_IMAGE_COLOR);
				if (img.cols < 128 || img.rows < 256) continue;
				
				// md5 left half
				MD5 md5;
				uchar buf[128 * 256 * 3];
				int i = 0;
				for (int x = 0; x < 128; x++) {
					for (int y = 0; y < 256; y++) {
						cv::Vec3b pixel = img.at<cv::Vec3b>(y, x);
						buf[i++] = pixel[0];
						buf[i++] = pixel[1];
						buf[i++] = pixel[2];
					}
				}
				md5.update(buf, 128 * 256);
				md5.finalize();
				string hash = md5.hexdigest();

				// check for collision
				if (md5s.count(hash) > 0) continue;

				// add to set
				md5s.insert(hash);

				// get left half
				cv::Mat left_half = img(cv::Rect(0, 0, 128, 256));

				// write left half to dest
				cv::imwrite((dest / texture->path().filename()).string(), left_half);
			}
		}
	}
	catch (fs::filesystem_error e) {
		cout << e.what() << endl;
	}
}

void Copy_Unique_Left_Objects(fs::path analysis, fs::path dest)										// TODO: STOP USING MD5!
{
	unordered_set<string> md5s;

	try {
		fs::directory_iterator iter_end;
		for (fs::directory_iterator texture(analysis); texture != iter_end; texture++) {
			// [FFVIII]\tonberry\debug\analysis\*.bmp
			string tex_name = texture->path().string();
			cv::Mat img = cv::imread(tex_name, CV_LOAD_IMAGE_COLOR);
			if (img.cols < 128 || img.rows < 256) continue;

			// md5 top corner
			MD5 top_md5;
			uchar buf[128 * 128 * 3];
			int i = 0;
			for (int y = 0; y < 128; y++) {
				for (int x = 0; x < 128; x++) {
					cv::Vec3b pixel = img.at<cv::Vec3b>(y, x);
					buf[i++] = pixel[0];
					buf[i++] = pixel[1];
					buf[i++] = pixel[2];
				}
			}
			top_md5.update(buf, 128 * 128);
			top_md5.finalize();
			string hash = top_md5.hexdigest();

			// check for collision
			if (md5s.count(hash) == 0) {

				// add to set
				md5s.insert(hash);

				// get top corner
				cv::Mat corner = img(cv::Rect(0, 0, 128, 128));

				// write corner to dest
				cv::imwrite((dest / texture->path().filename()).string(), corner);
			}

			// md5 top corner
			MD5 bottom_md5;
			i = 0;
			for (int y = 128; y < 128; y++) {
				for (int x = 0; x < 128; x++) {
					cv::Vec3b pixel = img.at<cv::Vec3b>(y, x);
					buf[i++] = pixel[0];
					buf[i++] = pixel[1];
					buf[i++] = pixel[2];
				}
			}
			bottom_md5.update(buf, 128 * 128);
			bottom_md5.finalize();
			hash = bottom_md5.hexdigest();

			// check for collision
			if (md5s.count(hash) == 0) {

				// add to set
				md5s.insert(hash);

				// get top corner
				cv::Mat corner = img(cv::Rect(0, 128, 128, 128));

				// write corner to dest
				cv::imwrite((dest / texture->path().filename()).string(), corner);
			}
		}
	}
	catch (fs::filesystem_error e) {
		cout << e.what() << endl;
	}
}

// hold image paths and data
struct image
{
	fs::path path;
	cv::Rect rect;

	image(fs::path p, cv::Rect r) : path(p), rect(r)
	{
		//mat = cv::imread(path.string(), CV_LOAD_IMAGE_COLOR);
	}

	cv::Mat mat()
	{
		return cv::imread(path.string())(rect);
	}
};

const int DIM_X = 128;
const int DIM_Y = 128;

void get_images(fs::path analysis, deque<image*>& images, unordered_set<uint64>& image_hashes)
{
	try {
		fs::directory_iterator iter(analysis), end;
		for (; iter != end; iter++) {
			fs::path path = iter->path();
			if (fs::is_directory(path)) {/*
				if (analysis.stem().string() == "mapdata2")
					cout << "Skipping " << path.stem().string() << endl;
				else if (analysis.parent_path().stem().string() == "mapdata2" && !isdigit(path.string().at(path.string().length() - 1)))
					cout << "Skipping " << path.stem().string() << endl;
				else*/
					get_images(path, images, image_hashes);						// recursive
			}
			else if (fs::is_regular_file(path) &&
					(boost::iequals(path.extension().string(), ".bmp") ||
					boost::iequals(path.extension().string(), ".png"))) {
				cv::Mat img = cv::imread(path.string());

				// store 1-4 128x128 objects
				// 		|_a_|_b_|
				//		|_c_|_d_|

				// a
				cv::Rect rect = cv::Rect(0, 0, min(img.cols, DIM_X), min(img.rows, DIM_Y));
				uint64 hash = Murmur2_Full(img(rect));													// hash the object
				if (image_hashes.insert(hash).second) images.push_back(new image(path, rect));			// only insert unique objects

				// b
				if (img.cols > DIM_X) {
					rect = cv::Rect(DIM_X, 0, min(img.cols - DIM_X, DIM_X), min(img.rows, DIM_Y));
					uint64 hash = Murmur2_Full(img(rect));
					if (image_hashes.insert(hash).second) images.push_back(new image(path, rect));
				}

				// c
				if (img.rows > DIM_Y) {
					rect = cv::Rect(0, DIM_Y, min(img.cols, DIM_X), min(img.rows - DIM_Y, DIM_Y));
					uint64 hash = Murmur2_Full(img(rect));
					if (image_hashes.insert(hash).second) images.push_back(new image(path, rect));
				}

				// d
				if (img.cols > DIM_X && img.rows > DIM_Y) {
					rect = cv::Rect(DIM_X, DIM_Y, min(img.cols - DIM_X, DIM_X), min(img.rows - DIM_Y, DIM_Y));
					uint64 hash = Murmur2_Full(img(rect));
					if (image_hashes.insert(hash).second) images.push_back(new image(path, rect));
				}
			}
		}
	} catch (fs::filesystem_error e) {
		cout << e.what() << endl;
	}
}

typedef long long_dim[DIM_X];
typedef double double_dim[DIM_X];
typedef long long_count[255];

void Analyze_Pixels(fs::path analysis, fs::path dest)
{
	bool DO_OLD_ALGO = false;
	bool DO_HIGH_VAR = false;
	bool DO_LOW_MODE = true;
	bool DO_COLANLYZ = false;

	int COLLIDE_COUNT	= 100;

	cout << "Running Image Analysis..." << endl;
	cout << "Reading images...";

	deque<image*> images;
	unordered_set<uint64> image_hashes;
	get_images(analysis, images, image_hashes);
	
	cout << " found " << images.size() << " images." << endl;

	map<uint64, set<int>> hashmap;
	clock_t start_time, end_time, total_time = 0;
	double avg_time;
	int collisions = 0;

	if (DO_OLD_ALGO) {
		// hash images using old algorithm
		hashmap.clear();
		total_time = 0;
		for (int i = 0; i < images.size(); i++) {
			cv::Mat img = images[i]->mat();
			uint64 hash;

			start_time = clock();
			hash = Old_Hash_Algorithm_1(img);
			end_time = clock();
			total_time += end_time - start_time;
			hashmap[hash].insert(i);
		}
		avg_time = ((double)total_time) / images.size();

		// count collisions
		collisions = 0;
		for (pair<uint64, set<int>> hashset : hashmap) {
			collisions += hashset.second.size() - 1;
		}

		cout << "Omzy:      " << images.size() << " images; " << collisions << " collisions; ~" << avg_time << " ms per image" << endl;
	}

	deque<coord> coords;
	
	if (DO_HIGH_VAR) {
		// get sum, mean, and variance of each pixel value in image set
		ofstream out;
		out.open(dest.string());
		out << "y,x,sum,mean,var,mode_count" << endl;


		long_dim* sum = new long_dim[DIM_Y];
		double_dim* mean = new double_dim[DIM_Y];
		double_dim* var = new double_dim[DIM_Y];
		deque<uchar> pixvals[DIM_Y][DIM_X];

		// init to 0
		for (int y = 0; y < DIM_Y; y++) {
			for (int x = 0; x < DIM_X; x++) {
				sum[y][x] = 0;
			}
		}


		// get pixvals and sums
		for (image* img_ptr : images) {
			cv::Mat img = img_ptr->mat();
			for (int y = 0; y < DIM_Y; y++) {
				for (int x = 0; x < DIM_X; x++) {
					cv::Vec3b pixel = (y < img.rows && x < img.cols) ? img.at<cv::Vec3b>(y, x) : cv::Vec3b(0, 0, 0);
					uchar pixval = round((pixel[0] + pixel[1] + pixel[2]) / 3);
					sum[y][x] += pixval;
					pixvals[y][x].push_back(pixval);
				}
			}
		}

		for (int y = 0; y < DIM_Y; y++) {
			for (int x = 0; x < DIM_X; x++) {
				// mean
				mean[y][x] = ((long double)sum[y][x]) / images.size();

				// variance
				var[y][x] = 0;
				for (uchar pixval : pixvals[y][x]) {
					var[y][x] += pow(pixval - mean[y][x], 2);
				}
				var[y][x] /= images.size();

				// max_with_same_pixel

				out << y << "," << x << "," << sum[y][x] << "," << mean[y][x] << "," << var[y][x] << endl;
			}
		}
		out.close();

		// find highest variance in 20x22 blocks
		//int block_width = 20;
		//int block_height = 22;
		// find highest variance in 10x10 blocks
		int block_width = 10;
		int block_height = 10;

		coords.clear();
		for (int y = 2; y < DIM_Y - block_height; y += block_height + 1) {
			for (int x = 2; x < DIM_X - block_width; x += block_width + 1) {
				coord best;
				double high_var = 0;
				for (int r = y; r < y + block_height; r++) {
					for (int c = x; c < x + block_width; c++) {
						if (var[r][c] > high_var) {
							best.x = c;
							best.y = r;
							high_var = var[r][c];
						}
					}
				}
				coords.push_back(best);
				//cout << "(" << best.x << ", " << best.y << "): " << high_var << endl;
			}
		}

		// write high-variance coordinates
		out.open((dest.parent_path() / "analysis_coordinates.csv").string());
		out << endl << "High-Variance Coordinates" << endl;
		out << "y,x" << endl;

		for (coord point : coords)
			out << point.y << "," << point.x << endl;

		out.close();

		// hash images using high-variance coordinates and store collisions
		hashmap.clear();
		total_time = 0;
		for (int i = 0; i < images.size(); i++) {
			cv::Mat img = images[i]->mat();
			uint64 hash;

			start_time = clock();
			hash = FNV_Hash(img, coords);
			end_time = clock();
			total_time += end_time - start_time;
			hashmap[hash].insert(i);
		}
		avg_time = ((double)total_time) / images.size();

		// count collisions
		collisions = 0;
		for (pair<uint64, set<int>> hashset : hashmap) {
			collisions += hashset.second.size() - 1;
		}

		cout << "High-Var:  " << images.size() << " images; " << collisions << " collisions; ~" << avg_time << " ms per image" << endl;

		out.close();
		delete[] sum;
		delete[] mean;
		delete[] var;
	}

	if (DO_LOW_MODE) {
		// find lowest mode_count in all collision images for LOW_MODE_COUNT iterations
		coords.clear();
		hashmap.clear();
		deque<int> low_modes;

		// get pixcounts for each possible x,y,RGB
		unsigned long* pixcounts = new unsigned long[DIM_Y * DIM_X * 3 * 256];
		for (int y = 0; y < DIM_Y; y++)
			for (int x = 0; x < DIM_X; x++)
				for (int c = 0; c < 3; c++)
					for (int p = 0; p < 256; p++)
						pixcounts[(y * DIM_X * 3 * 256) + (x * 3 * 256) + (c * 256) + p] = 0;

		for (image* img_ptr : images) {
			cv::Mat img = img_ptr->mat();
			for (int y = 0; y < DIM_Y; y++) {
				for (int x = 0; x < DIM_X; x++) {
					cv::Vec3b pixel = (y < img.rows && x < img.cols) ? img.at<cv::Vec3b>(y, x) : cv::Vec3b(0, 0, 0);
					for (int c = 0; c < 3; c++)
						pixcounts[(y * DIM_X * 3 * 256) + (x * 3 * 256) + (c * 256) + pixel[c]]++;
				}
			}
		}

		// find highest variance in 8x8 blocks - total of (128/9)^2 = 14^2 = 196 coordinates
		int block_width = 8;
		int block_height = 8;

		for (int y = 2; y < DIM_Y - block_height; y += block_height + 1) {
			for (int x = 2; x < DIM_X - block_width; x += block_width + 1) {
				coord best;
				unsigned long low_mode = ULONG_MAX;
				//long double low_mode = LDBL_MAX;
				for (int r = y; r < y + block_height; r++) {
					for (int c = x; c < x + block_width; c++) {
						// use overall mode
						unsigned long mode = *max_element(pixcounts + (r * DIM_X * 3 * 256) + (c * 3 * 256),
														  pixcounts + (r * DIM_X * 3 * 256) + (c * 3 * 256) + (3 * 256));
						// use RGB mode average
						//unsigned long mode_red = *max_element(pixcounts + (r * DIM_X * 3 * 256) + (c * 3 * 256),
						//									  pixcounts + (r * DIM_X * 3 * 256) + (c * 3 * 256) + 256);
						//unsigned long mode_green = *max_element(pixcounts + (r * DIM_X * 3 * 256) + (c * 3 * 256) + 256,
						//										pixcounts + (r * DIM_X * 3 * 256) + (c * 3 * 256) + 512);
						//unsigned long mode_blue	= *max_element(pixcounts + (r * DIM_X * 3 * 256) + (c * 3 * 256) + 512,
						//									   pixcounts + (r * DIM_X * 3 * 256) + (c * 3 * 256) + 768);
						//long double mode_avg = ((long double)(mode_red + mode_green + mode_blue)) / 3;
						if (mode < low_mode) {
						//if (mode_avg < low_mode) {
							best.x = c;
							best.y = r;
							low_mode = mode;
							//low_mode = mode_avg;
						}
					}
				}
				coords.push_back(best);
				low_modes.push_back(low_mode);
				//cout << "(" << best.x << ", " << best.y << "): " << high_var << endl;
			}
		}

		// write low mode count coordinates
		ofstream out;
		out.open((dest.parent_path() / "analysis_coordinates.csv").string());
		out << endl << coords.size() << " Lowest |Mode| Coordinates" << endl;
		out << "x,y,mode" << endl;

		for (int i = 0; i < coords.size(); i++)
			out << coords[i].x << "," << coords[i].y << "," << low_modes[i] << endl;

		hashmap.clear();
		total_time = 0;
		for (int i = 0; i < images.size(); i++) {
			cv::Mat img = images[i]->mat();
			uint64 hash;

			start_time = clock();
			hash = Murmur2_Hash(img, coords);
			end_time = clock();
			total_time += end_time - start_time;
			hashmap[hash].insert(i);
		}
		avg_time = ((double)total_time) / images.size();

		// find the differing coordinates in images with hash collisions
		ofstream collout;
		collout.open((dest.parent_path() / "analysis_collisions.csv").string());
		collisions = 0;
		map<coord, int> collision_coords;
		for (pair<uint64, set<int>> hashset : hashmap) {
			if (hashset.second.size() < 2) continue;
			collisions += hashset.second.size() - 1;

			// write collisions to collout
			collout << hashset.first;
			for (int i : hashset.second) {
				collout << "," << images[i]->path.stem().string() << " (" << Murmur2_Full(images[i]->mat()) << ")";
			}
			collout << endl;
		}
		cout << "Low-Mode (" << coords.size() << "): " << images.size() << " images; " << collisions << " collisions; ~" << avg_time << " ms per image" << endl;

		out.close();
		collout.close();
		delete[] pixcounts;
	}

	if (DO_COLANLYZ) {
		// find the differing coordinates in images with hash collisions
		ofstream collout;
		collout.open((dest.parent_path() / "analysis_collisions.csv").string());
		collisions = 0;
		map<coord, int> collision_coords;
		for (pair<uint64, set<int>> hashset : hashmap) {
			if (hashset.second.size() < 2) continue;

			set<int>::iterator iter = hashset.second.begin();
			while (iter != hashset.second.end()) {
				for (int j : hashset.second) {				// compare each image in the collision set to every other image in the collision set
					if (*iter == j) continue;				// (excluding itself)
					cv::Mat img1 = images[*iter]->mat();
					cv::Mat img2 = images[j]->mat();

					for (int y = 0; y < min(img1.rows, img2.rows); y++) {
						cv::Mat row1 = img1.row(y);
						cv::Mat row2 = img2.row(y);
						for (int x = 0; x < min(row1.cols, row2.cols); x++) {
							cv::Vec3b pixel1 = row1.at<cv::Vec3b>(0, x);
							cv::Vec3b pixel2 = row2.at<cv::Vec3b>(0, x);
							uchar val1 = round((pixel1[0] + pixel1[1] + pixel1[2]) / 3);
							uchar val2 = round((pixel2[0] + pixel2[1] + pixel2[2]) / 3);
							if (val1 != val2)
								collision_coords[{x, y}]++;		// increase collision count for differing pixels
						}
					}
				}
				hashset.second.erase(iter);
				iter = hashset.second.begin();
			}
		}

		// find most common collision coordinates
		// reverse collision_coords to sort coordinates by frequency
		map<int, set<coord>, greater<int>> common_collisions;
		for (pair<coord, int> collision : collision_coords) {
			common_collisions[collision.second].insert(collision.first);
		}

		ofstream out;
		out.open((dest.parent_path() / "analysis_coordinates.csv").string());
		out << endl << "Frequently-Colliding Coordinates" << endl;
		out << "y,x,count" << endl;

		// add 44 most frequent collision_coords to coords
		int count = 0;
		map<int, set<coord>, greater<int>>::iterator collision_iter = common_collisions.begin();
		while (collision_iter != common_collisions.end() && count < 88) {
			set<coord>::iterator coord_iter = collision_iter->second.begin();
			while (coord_iter != collision_iter->second.end() && count < 44) {
				coords.push_back(*coord_iter);
				count++;

				out << coord_iter->y << "," << coord_iter->x << "," << collision_iter->first << endl;

				coord_iter++;
			}
			collision_iter++;
		}

		// hash images using frequently-colliding coords
		hashmap.clear();
		total_time = 0;
		for (int i = 0; i < images.size(); i++) {
			cv::Mat img = images[i]->mat();
			uint64 hash;

			start_time = clock();
			hash = FNV_Hash(img, coords);
			end_time = clock();
			total_time += end_time - start_time;
			hashmap[hash].insert(i);
		}
		avg_time = ((double)total_time) / images.size();

		// count collisions
		collisions = 0;
		for (pair<uint64, set<int>> hashset : hashmap) {
			collisions += hashset.second.size() - 1;
		}

		cout << "Freq-Coll (" << coords.size() << ") : " << images.size() << " images; " << collisions << " collisions; ~" << avg_time << " ms per image" << endl;
		out.close();
		collout.close();
	}

}


void Analyze_Collisions(fs::path analysis, fs::path dest)
{
	cout << "Running Collision Analysis..." << endl;
	cout << "Reading images...";

	deque<image*> images;
	unordered_set<uint64> image_hashes;
	get_images(analysis, images, image_hashes);

	cout << " found " << images.size() << " images." << endl;

	map<uint64, set<int>> hashmap;
	clock_t start_time, end_time, total_time;
	double avg_time;
	long collisions = 0;

	total_time = 0;
	for (int i = 0; i < images.size(); i++) {
		cv::Mat img = images[i]->mat();
		uint64 hash;

		start_time = clock();
		hash = Murmur2_Hash(img, FNV_COORDS, FNV_COORDS_LEN);
		end_time = clock();
		total_time += end_time - start_time;
		hashmap[hash].insert(i);
	}
	avg_time = ((double)total_time) / images.size();

	// find the differing coordinates in images with hash collisions
	ofstream collout;
	collout.open(dest.string());
	collisions = 0;
	map<coord, int> collision_coords;
	long coll_pairs = 0;
	long total_differing = 0;
	long total_difference = 0;
	for (pair<uint64, set<int>> hashset : hashmap) {
		if (hashset.second.size() < 2) continue;
		collisions += hashset.second.size() - 1;

		// write collisions to collout
		collout << hashset.first;
		for (int i : hashset.second) {
			collout << "," << i << " - " << make_relative(analysis, images[i]->path.string()).string() << " (rect " << images[i]->rect.x << ", " << images[i]->rect.y  << " )";
		}
		collout << endl;

		set<int>::iterator iter2 = hashset.second.begin();
		set<int>::iterator iter = hashset.second.begin();
		set<int>::iterator end = hashset.second.end();
		end--;
		while (iter != end) {
			iter2++;
			cv::Mat img = images[*iter]->mat();
			while (iter2 != hashset.second.end()) {
				long differing = 0;
				long difference = 0;

				cv::Mat img2 = images[*iter2]->mat();
				for (int y = 0; y < min(img.rows, img2.rows); y++) {
					for (int x = 0; x < min(img.cols, img2.cols); x++) {
						cv::Vec3b pixel = img.at<cv::Vec3b>(y, x);
						cv::Vec3b pixel2 = img2.at<cv::Vec3b>(y, x);
						for (int c = 0; c < 3; c++) {
							if (pixel[c] != pixel2[c]) {
								differing++;
								difference += abs(pixel[c] - pixel2[c]);
							}
						}
					}
				}

				if (differing > 0) {
					coll_pairs++;
				}

				stringstream ss;
				ss << *iter << " vs " << *iter2 << ": " << differing <<
					" differing pixels for a total of " << difference <<
					" and an average difference of " << ((differing == 0) ? 0 : ((double)difference) / differing);
				cout << ss.str() << endl;
				collout << ss.str() << endl;
				total_differing += differing;
				total_difference += difference;
				iter2++;
			}
			collout << endl;
			iter2 = ++iter;
		}
	}
	cout << "Low-Mode (" << FNV_COORDS_LEN << "):  " << images.size() << " images; " << collisions << " collisions in " << coll_pairs << " pairs; ~" << avg_time << " ms per image" << endl;
	cout << "Differing Pixels: " << total_differing << endl;
	cout << "Total Difference: " << total_difference << endl;
	cout << "Average Difference: " << ((total_differing == 0) ? 0 : (((double)total_difference) / total_differing)) << endl;
	cout << "Differing/CollPair: " << ((coll_pairs == 0) ? 0 : (((double)total_differing) / coll_pairs)) << endl;
	cout << "Difference/CollPair: " << ((coll_pairs == 0) ? 0 : (((double)total_difference) / coll_pairs)) << endl;
}


string md5_Hash(const cv::Mat& img, coord* coords = nullptr, int len = 64)
{
	MD5 md5;

	if (coords == nullptr) {
		for (int y = 0; y < DIM_Y; y++) {
			char buf[DIM_X];
			for (int x = 0; x < DIM_X; x++) {
				uchar val = 0;
				if (x < img.cols && y < img.rows) {
					cv::Vec3b pixel = img.at<cv::Vec3b>(y, x);
					val = round((pixel[0] + pixel[1] + pixel[2]) / 3);
				}

				buf[x] = val;
			}
			md5.update(buf, DIM_X);
		}
		md5.finalize();
		return md5.hexdigest();
	}

	coord* coord_ptr = coords;
	char* buf = new char[len];
	for (int i = 0; i < len; i++, coord_ptr++) {
		uchar val = 0;
		if ((*coord_ptr).x < img.cols && (*coord_ptr).y < img.rows) {
			cv::Vec3b pixel = img.at<cv::Vec3b>((*coord_ptr).y, (*coord_ptr).x);
			val = round((pixel[0] + pixel[1] + pixel[2]) / 3);
		}

		buf[i] = val;
	}

	md5.update(buf, 64);
	md5.finalize();
	delete buf;
	return md5.hexdigest();
}

int VRAM_DIM = 256;

uint64 uint64pow(uint64 base, int exp)
{
	uint64 result = 1;
	for (int i = 0; i < exp; i++)
		result *= base;

	return result;
}

uint64 FNV_NOLOWER_FACTOR_64		= uint64pow(FNV_OFFSET_PRIME_64, FNV_COORDS_LEN);
uint64 FNV_NOLOWER_RGB_FACTOR_64	= FNV_NOLOWER_FACTOR_64 * uint64pow(FNV_OFFSET_PRIME_64, 3);
uint64 FNV_NOUPPER_BASIS_64			= FNV_OFFSET_BASIS_64 * FNV_NOLOWER_FACTOR_64;
uint64 FNV_NOUPPER_RGB_BASIS_64		= FNV_OFFSET_BASIS_64 * FNV_NOLOWER_RGB_FACTOR_64;

// hash upper, lower, and combined separately 
uint64 FNV_Hash_Combined_64(cv::Mat img, uint64& hash_upper, uint64& hash_lower, const coord* coords, const int len, bool use_RGB = true)
{
	hash_lower = (img.rows > VRAM_DIM / 2) ? (use_RGB) ? FNV_NOUPPER_RGB_BASIS_64 : FNV_NOUPPER_BASIS_64 : 0;
	
	uint64 hash = FNV_OFFSET_BASIS_64;

	float red = 0, green = 0, blue = 0;
	size_t coord_count = 0;
	for (int i = 0; i < len; i++) {
		coord point = coords[i];
		unsigned char val = 0;
		if (point.x < img.cols && point.y < img.rows) {
			cv::Vec3b pixel = img.at<cv::Vec3b>(point.y, point.x);
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
		cv::Mat limg = img.rowRange(min(last_obj_start, half_dim), img.rows);	// point limg at last place where a full 128x128 object could be hashed but limit it to object directly under upper

		// hash lower and continue hashing combined
		red = 0, green = 0, blue = 0;
		coord_count = 0;
		for (int i = 0; i < len; i++) {
			coord point = coords[i];
			unsigned char val = 0;
			if (point.x < limg.cols && point.y < limg.rows) {
				cv::Vec3b pixel = limg.at<cv::Vec3b>(point.y, point.x);
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

uint32 FNV_NOLOWER_FACTOR_32 = uint32pow(FNV_OFFSET_PRIME_32, FNV_COORDS_LEN);
uint32 FNV_NOLOWER_RGB_FACTOR_32 = FNV_NOLOWER_FACTOR_32 * uint32pow(FNV_OFFSET_PRIME_32, 3);
uint32 FNV_NOUPPER_BASIS_32 = FNV_OFFSET_BASIS_32 * FNV_NOLOWER_FACTOR_32;
uint32 FNV_NOUPPER_RGB_BASIS_32 = FNV_OFFSET_BASIS_32 * FNV_NOLOWER_RGB_FACTOR_32;

// hash upper, lower, and combined separately 
uint32 FNV_Hash_Combined_32(cv::Mat img, uint32& hash_upper, uint32& hash_lower, const coord* coords, const int len, bool use_RGB = true)
{
	hash_lower = (img.rows > VRAM_DIM / 2) ? (use_RGB) ? FNV_NOUPPER_RGB_BASIS_32 : FNV_NOUPPER_BASIS_32 : 0;

	uint32 hash = FNV_OFFSET_BASIS_32;

	float red = 0, green = 0, blue = 0;
	size_t coord_count = 0;
	for (int i = 0; i < len; i++) {
		coord point = coords[i];
		unsigned char val = 0;
		if (point.x < img.cols && point.y < img.rows) {
			cv::Vec3b pixel = img.at<cv::Vec3b>(point.y, point.x);
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
		cv::Mat limg = img.rowRange(min(last_obj_start, half_dim), img.rows);	// point limg at last place where a full 128x128 object could be hashed but limit it to object directly under upper

		// hash lower and continue hashing combined
		red = 0, green = 0, blue = 0;
		coord_count = 0;
		for (int i = 0; i < len; i++) {
			coord point = coords[i];
			unsigned char val = 0;
			if (point.x < limg.cols && point.y < limg.rows) {
				cv::Vec3b pixel = limg.at<cv::Vec3b>(point.y, point.x);
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

void Create_Hashmap(fs::path texture_dir, fs::path output_dir, bool append = false)
{
	fs::path hashmap_csv(output_dir / (texture_dir.filename().string() + "_hm.csv"));
	//fs::path collisions_dir(output_dir / (texture_dir.filename().string() + "_coll.csv"));

	ofstream out;
	unsigned int open_mode = ofstream::out;
	if (append) open_mode |= ofstream::app;
	out.open(hashmap_csv.string(), open_mode);

	fs::directory_iterator end;
	for (fs::directory_iterator iter(texture_dir); iter != end; iter++) {
		fs::path path = iter->path();
		if (fs::is_directory(path)) {
			// recursive
			Create_Hashmap(path, output_dir, true);
		} else if (fs::is_regular_file(path) && boost::iequals(path.extension().string(), ".bmp")) {
			cv::Mat img = cv::imread(path.string(), CV_LOAD_IMAGE_COLOR);
			uint64 hash_combined, hash_upper, hash_lower;
			hash_combined = FNV_Hash_Combined_64(img, hash_upper, hash_lower, FNV_COORDS, FNV_COORDS_LEN, true);
			out << path.stem().string() << "," << hash_combined << "," << hash_upper << "," << hash_lower << endl;
		}
	}
}

void Get_Blank_Hashes()
{
	cv::Mat sel_13 = cv::imread("H:\\Game Saves\\Final Fantasy VIII\\Mods\\Berrymapper - Hash Textures\\BerryMapper\\INPUT\\sql_78.bmp", CV_LOAD_IMAGE_COLOR);
	cv::Mat normal = sel_13(cv::Rect(0, 0, 128, 256));
	cv::Mat nolower = normal.clone();
	cv::Mat noupper = normal.clone();

	for (int y = 0; y < 256; y++) {
		for (int x = 0; x < 128; x++) {
			if (y < 128) {
				noupper.data[noupper.step[0] * y + noupper.step[1] * x + 0] = 0;
				noupper.data[noupper.step[0] * y + noupper.step[1] * x + 1] = 0;
				noupper.data[noupper.step[0] * y + noupper.step[1] * x + 2] = 0;
			} else {
				nolower.data[nolower.step[0] * y + nolower.step[1] * x + 0] = 0;
				nolower.data[nolower.step[0] * y + nolower.step[1] * x + 1] = 0;
				nolower.data[nolower.step[0] * y + nolower.step[1] * x + 2] = 0;
			}
		}
	}

	uint64 upper, lower, combined;
	uint64 upper_rgb, lower_rgb, combined_rgb;

	cout << "NORMAL:" << endl;
	combined = FNV_Hash_Combined_64(normal, upper, lower, FNV_COORDS, FNV_COORDS_LEN, false);
	combined_rgb = FNV_Hash_Combined_64(normal, upper_rgb, lower_rgb, FNV_COORDS, FNV_COORDS_LEN, true);

	cout << "  upper:        " << upper << endl;
	cout << "  upper_rgb:    " << upper_rgb  << endl << endl;

	cout << "  lower:        " << lower << endl;
	cout << "  lower_rgb:    " << lower_rgb << endl << endl;

	cout << "  combined:     " << combined << endl;
	cout << "  combined_rgb: " << combined_rgb << endl << endl;

	cout << "NO LOWER:" << endl;
	combined = FNV_Hash_Combined_64(nolower, upper, lower, FNV_COORDS, FNV_COORDS_LEN, false);
	combined_rgb = FNV_Hash_Combined_64(nolower, upper_rgb, lower_rgb, FNV_COORDS, FNV_COORDS_LEN, true);

	cout << "  combined:     " << combined << endl;
	cout << "  combined_rgb: " << combined_rgb << endl << endl;

	cout << "NO UPPER:" << endl;
	combined = FNV_Hash_Combined_64(noupper, upper, lower, FNV_COORDS, FNV_COORDS_LEN, false);
	combined_rgb = FNV_Hash_Combined_64(noupper, upper_rgb, lower_rgb, FNV_COORDS, FNV_COORDS_LEN, true);

	cout << "  combined:     " << combined << endl;
	cout << "  combined_rgb: " << combined_rgb << endl << endl;
}

void Test_Texture_Replacement()
{
	cv::Mat sel_13 = cv::imread("H:\\Game Saves\\Final Fantasy VIII\\Mods\\Berrymapper - Hash Textures\\BerryMapper\\INPUT\\sel_13.bmp", CV_LOAD_IMAGE_COLOR);
	cv::Mat wm_ocean = cv::imread("C:\\Program Files (x86)\\Steam\\steamapps\\common\\FINAL FANTASY VIII\\tonberry\\MCINDUS_Ocean2\\wm\\wm_oceancstlt\\wm_oceancstlt_13.png", CV_LOAD_IMAGE_COLOR);
	cv::Mat wm_ocean_upper = wm_ocean.rowRange(0, wm_ocean.rows / 2);
	cv::Mat wm_ocean_lower = wm_ocean.rowRange(wm_ocean.rows / 2, wm_ocean.rows);

	cv::Mat test(1024, 1024, CV_8UC3);
	// UPSCALE ORIGINAL TEXTURE
	for (int y = 0; y < 1024; y++) {
		for (int x = 0; x < 1024; x++) {
			cv::Vec3b pixel = sel_13.at<cv::Vec3b>(y / 4, x / 4);
			test.data[test.step[0] * y + test.step[1] * x + 0] = pixel[0];
			test.data[test.step[0] * y + test.step[1] * x + 1] = pixel[1];
			test.data[test.step[0] * y + test.step[1] * x + 2] = pixel[2];
		}
	}

	//cv::namedWindow("image", cv::WINDOW_AUTOSIZE);
	//cv::imshow("image", test);
	//cv::waitKey(0);
	cv::imwrite("img_testing\\upscaled.bmp", test);

	// UPSCALE ORIGINAL FLIPPED
	for (int y = 0; y < 1024; y++) {
		for (int x = 0; x < 1024; x++) {
			cv::Vec3b pixel = sel_13.at<cv::Vec3b>(sel_13.rows - 1 - y / 4, x / 4);
			test.data[test.step[0] * y + test.step[1] * x + 0] = pixel[0];
			test.data[test.step[0] * y + test.step[1] * x + 1] = pixel[1];
			test.data[test.step[0] * y + test.step[1] * x + 2] = pixel[2];
		}
	}

	//cv::namedWindow("image", cv::WINDOW_AUTOSIZE);
	//cv::imshow("image", test);
	//cv::waitKey(0);
	cv::imwrite("img_testing\\upscaled_flipped.bmp", test);

	// UPSCALE OCEAN UPPER
	for (int y = 0; y < 1024; y++) {
		for (int x = 0; x < 1024; x++) {
			cv::Vec3b pixel;
			if (y < 512)
				pixel = wm_ocean.at<cv::Vec3b>(y, x);
			else
				pixel = sel_13.at<cv::Vec3b>(y / 4, x / 4);
			test.data[test.step[0] * y + test.step[1] * x + 0] = pixel[0];
			test.data[test.step[0] * y + test.step[1] * x + 1] = pixel[1];
			test.data[test.step[0] * y + test.step[1] * x + 2] = pixel[2];
		}
	}

	//cv::namedWindow("image", cv::WINDOW_AUTOSIZE);
	//cv::imshow("image", test);
	//cv::waitKey(0);
	cv::imwrite("img_testing\\upper.bmp", test);

	// UPSCALE OCEAN UPPER FLIPPED
	cv::Mat* to_use;
	to_use = &wm_ocean;
	for (int y = 0; y < 1024; y++) {
		for (int x = 0; x < 1024; x++) {
			cv::Vec3b pixel;
			if (y < 512)
				pixel = sel_13.at<cv::Vec3b>(sel_13.rows - 1 - y / 4, x / 4);
			else {
				int repl_y = to_use->rows - 1 - y;
				if (repl_y < 0) repl_y += 512;
				pixel = to_use->at<cv::Vec3b>(repl_y, x);
			}
			test.data[test.step[0] * y + test.step[1] * x + 0] = pixel[0];
			test.data[test.step[0] * y + test.step[1] * x + 1] = pixel[1];
			test.data[test.step[0] * y + test.step[1] * x + 2] = pixel[2];
		}
	}

	cv::namedWindow("image", cv::WINDOW_AUTOSIZE);
	cv::imshow("image", test);
	cv::waitKey(0);
	cv::imwrite("img_testing\\full_upper_flipped.bmp", test);

	// UPSCALE OCEAN_UPPER FLIPPED
	to_use = &wm_ocean_upper;
	for (int y = 0; y < 1024; y++) {
		for (int x = 0; x < 1024; x++) {
			cv::Vec3b pixel;
			if (y < 512)
				pixel = sel_13.at<cv::Vec3b>(sel_13.rows - 1 - y / 4, x / 4);
			else {
				int repl_y = to_use->rows - 1 - y;
				if (repl_y < 0) repl_y += 512;
				pixel = to_use->at<cv::Vec3b>(repl_y, x);
			}
			test.data[test.step[0] * y + test.step[1] * x + 0] = pixel[0];
			test.data[test.step[0] * y + test.step[1] * x + 1] = pixel[1];
			test.data[test.step[0] * y + test.step[1] * x + 2] = pixel[2];
		}
	}

	cv::namedWindow("image", cv::WINDOW_AUTOSIZE);
	cv::imshow("image", test);
	cv::waitKey(0);
	cv::imwrite("img_testing\\half_upper_flipped.bmp", test);

	// UPSCALE OCEAN LOWER
	for (int y = 0; y < 1024; y++) {
		for (int x = 0; x < 1024; x++) {
			cv::Vec3b pixel;
			if (y < 512)
				pixel = sel_13.at<cv::Vec3b>(y / 4, x / 4);
			else
				pixel = wm_ocean.at<cv::Vec3b>(y, x);
			test.data[test.step[0] * y + test.step[1] * x + 0] = pixel[0];
			test.data[test.step[0] * y + test.step[1] * x + 1] = pixel[1];
			test.data[test.step[0] * y + test.step[1] * x + 2] = pixel[2];
		}
	}

	cv::namedWindow("image", cv::WINDOW_AUTOSIZE);
	cv::imshow("image", test);
	cv::waitKey(0);
	cv::imwrite("img_testing\\lower.bmp", test);

	// UPSCALE OCEAN LOWER FLIPPED
	to_use = &wm_ocean;
	for (int y = 0; y < 1024; y++) {
		for (int x = 0; x < 1024; x++) {
			cv::Vec3b pixel;
			if (y < 512) {
				int repl_y = to_use->rows - 1 - y;
				//if (repl_y < 0) repl_y += 512;
				pixel = to_use->at<cv::Vec3b>(repl_y, x);
			}
			else
				pixel = sel_13.at<cv::Vec3b>(sel_13.rows - 1 - y / 4, x / 4);
			test.data[test.step[0] * y + test.step[1] * x + 0] = pixel[0];
			test.data[test.step[0] * y + test.step[1] * x + 1] = pixel[1];
			test.data[test.step[0] * y + test.step[1] * x + 2] = pixel[2];
		}
	}

	cv::namedWindow("image", cv::WINDOW_AUTOSIZE);
	cv::imshow("image", test);
	cv::waitKey(0);
	cv::imwrite("img_testing\\full_lower_flipped.bmp", test);

	// UPSCALE OCEAN_LOWER FLIPPED
	to_use = &wm_ocean_lower;
	for (int y = 0; y < 1024; y++) {
		for (int x = 0; x < 1024; x++) {
			cv::Vec3b pixel;
			if (y < 512) {
				int repl_y = to_use->rows - 1 - y;
				//if (repl_y < 0) repl_y += 512;
				pixel = to_use->at<cv::Vec3b>(repl_y, x);
			} else
				pixel = sel_13.at<cv::Vec3b>(sel_13.rows - 1 - y / 4, x / 4);
			test.data[test.step[0] * y + test.step[1] * x + 0] = pixel[0];
			test.data[test.step[0] * y + test.step[1] * x + 1] = pixel[1];
			test.data[test.step[0] * y + test.step[1] * x + 2] = pixel[2];
		}
	}

	cv::namedWindow("image", cv::WINDOW_AUTOSIZE);
	cv::imshow("image", test);
	cv::waitKey(0);
	cv::imwrite("img_testing\\half_lower_flipped.bmp", test);
}

void Delete_Non_Unique(fs::path dir)
{

	unordered_set<string> md5s;
	list<string> duplicates;

	try {
		fs::directory_iterator iter_end;
		for (fs::directory_iterator texture(dir); texture != iter_end; texture++) {
			string tex_name = texture->path().string();
			cv::Mat img = cv::imread(tex_name, CV_LOAD_IMAGE_COLOR);
			if (img.cols < 128 || img.rows < 256) continue;

			// md5 left half
			MD5 md5;
			uchar buf[128 * 256 * 3];
			int i = 0;
			for (int x = 0; x < 128; x++) {
				for (int y = 0; y < 256; y++) {
					cv::Vec3b pixel = img.at<cv::Vec3b>(y, x);
					buf[i++] = pixel[0];
					buf[i++] = pixel[1];
					buf[i++] = pixel[2];
				}
			}
			md5.update(buf, 128 * 256);
			md5.finalize();
			string hash = md5.hexdigest();

			// try to insert; delete duplicates
			if (!md5s.insert(hash).second)
				duplicates.push_back(tex_name);
		}

	} catch (fs::filesystem_error e) {
		cout << e.what() << endl;
	}

	// delete duplicates
	for (string duplicate : duplicates)
		remove(duplicate.c_str());
}

int _tmain(int argc, _TCHAR* argv[])
{
	fs::path debug(FF8_ROOT / "tonberry\\debug");
	fs::path analysis(debug / "analysis0");
	fs::path textures(FF8_ROOT / "textures");

	//Delete_Non_Unique(debug / "unique");
	//Copy_Unique_Left_Half(debug, debug / "analysis0");
	//Copy_Unique_Left_Objects(analysis, analysis / "objects");
	//return 0;

	//Analyze_Pixels(analysis, debug / "object_analysis.csv");
	//getchar();
	//return 0;

	Analyze_Collisions(analysis, debug / "collision_analysis.csv");
	getchar();
	return 0;

	//fs::path ocean2(FF8_ROOT + "tonberry\\MCINDUS_Ocean2");
	//Create_Hashmap(ocean2 / "original bitmaps", ocean2);
	//return 0;

	//Get_Blank_Hashes();
	//return 0;

	Test_Texture_Replacement();
	return 0;
	
	deque<cv::Mat> images;
	deque<string> image_names;
	try {
		fs::directory_iterator iter_end;
		for (fs::directory_iterator debug_outer_dir(debug); debug_outer_dir != iter_end; debug_outer_dir++) {
			// [FFVIII]\tonberry\debug\*
			string dirname = debug_outer_dir->path().filename().string();
			// skip files and directories that end in 0
			if (!fs::is_directory(debug_outer_dir->path()) || dirname.substr(dirname.length() -1, 1) == "0") continue;
			for (fs::directory_iterator texture(debug_outer_dir->path()); texture != iter_end; texture++) {
				// [FFVIII]\tonberry\debug_outer_dir\*
				string tex_name = texture->path().string();
				//cout << tex_name << endl;
				cv::Mat img = cv::imread(tex_name, CV_LOAD_IMAGE_COLOR);
				if (img.cols < 1 || img.rows < 1) {
					//cout << "Skipping " << tex_name.substr(debug.string().length()) << " (" << img.cols << "x" << img.rows << ")" << endl;
					continue;
				}
				images.push_back(img);
				image_names.push_back(tex_name.substr(debug.string().length()));
			}
		}
	} catch (fs::filesystem_error e) {
		cout << e.what() << endl;
	}


	ofstream hashout, collisionout;
	hashout.open((debug / "hashes.csv").string());
	collisionout.open((debug / "collisions.csv").string());

	// hash images for collisions.csv output
	map<uint64, set<string>> hashmap;
	//map<string, set<string>> hashmap;
	for (int i = 0; i < images.size(); i++) {
		cv::Mat img = images[i];
		//uint64 hash = Old_Hash_Algorithm_1(img);
		uint64 hash = FNV_Hash(img, FNV_COORDS, FNV_COORDS_LEN);
		hashmap[hash].insert(image_names[i]);
	}

	// count collisions for collisions.csv output
	int collisions = 0;
	for (pair<uint64, set<string>> hashset : hashmap) {
		if (hashset.second.size() < 2) continue;
		collisions += hashset.second.size() - 1;
		collisionout << hashset.first;
		for (string tex : hashset.second) {
			collisionout << "," << tex;
		}
		collisionout << endl;
	}

	//
	// Algorithm Comparisons
	//

	// hash images using fast hash1 algorithm
	clock_t start_time, end_time, total_time = 0;
	double avg_time;
	hashmap.clear();
	total_time = 0;
	for (int i = 0; i < images.size(); i++) {
		cv::Mat img = images[i];
		start_time = clock();
		uint64 hash = Old_Hash_Algorithm_1(img);
		end_time = clock();
		total_time += end_time - start_time;
		hashmap[hash].insert(image_names[i]);
	}
	avg_time = ((double)total_time) / images.size();

	// count collisions
	collisions = 0;
	for (pair<uint64, set<string>> hashset : hashmap) {
		collisions += hashset.second.size() - 1;
	}

	cout << images.size() << " images; " << collisions << " collisions; ~" << avg_time << " ms per image" << endl;

	// hash images using fast hash1 algorithm
	hashmap.clear();
	total_time = 0;
	for (int i = 0; i < images.size(); i++) {
		cv::Mat img = images[i];
		start_time = clock();
		uint64 hash = Hash_Algorithm_1(img);
		end_time = clock();
		total_time += end_time - start_time;
		hashmap[hash].insert(image_names[i]);
	}
	avg_time = ((double)total_time) / images.size();

	// count collisions
	collisions = 0;
	for (pair<uint64, set<string>> hashset : hashmap) {
		collisions += hashset.second.size() - 1;
	}

	cout << images.size() << " images; " << collisions << " collisions; ~" << avg_time << " ms per image" << endl;

	// hash images using FNV algorithm with hash1 coords
	hashmap.clear();
	total_time = 0;
	for (int i = 0; i < images.size(); i++) {
		cv::Mat img = images[i];
		start_time = clock();
		uint64 hash = FNV_Hash(img, hash1, 64, false);
		end_time = clock();
		total_time += end_time - start_time;
		hashmap[hash].insert(image_names[i]);
	}
	avg_time = ((double)total_time) / images.size();

	// count collisions
	collisions = 0;
	for (pair<uint64, set<string>> hashset : hashmap) {
		collisions += hashset.second.size() - 1;
	}

	cout << images.size() << " images; " << collisions << " collisions; ~" << avg_time << " ms per image" << endl;

	// hash images using FNV algorithm with high-variance coords
	hashmap.clear();
	total_time = 0;
	for (int i = 0; i < images.size(); i++) {
		cv::Mat img = images[i];
		start_time = clock();
		uint64 hash = FNV_Hash(img, FNV_COORDS, FNV_COORDS_LEN, false);
		end_time = clock();
		total_time += end_time - start_time;
		hashmap[hash].insert(image_names[i]);
	}
	avg_time = ((double)total_time) / images.size();

	// count collisions
	collisions = 0;
	for (pair<uint64, set<string>> hashset : hashmap) {
		collisions += hashset.second.size() - 1;
	}

	cout << images.size() << " images; " << collisions << " collisions; ~" << avg_time << " ms per image" << endl;

	// hash images using FNV algorithm with high-variance coords AND frequently-colliding coords AND RGB averages
	hashmap.clear();
	total_time = 0;
	for (int i = 0; i < images.size(); i++) {
		cv::Mat img = images[i];
		start_time = clock();
		uint64 hash = FNV_Hash(img, FNV_COORDS, FNV_COORDS_LEN);
		end_time = clock();
		total_time += end_time - start_time;
		hashmap[hash].insert(image_names[i]);
	}
	avg_time = ((double)total_time) / images.size();

	// count collisions
	collisions = 0;
	for (pair<uint64, set<string>> hashset : hashmap) {
		collisions += hashset.second.size() - 1;
	}

	cout << images.size() << " images; " << collisions << " collisions; ~" << avg_time << " ms per image" << endl;

	//cv::Mat sel_13, sql_78, zel_13;
	//uint64 hash_sel_13, hash_sql_78, hash_zel_13;
	//sel_13 = cv::imread("H:\\Game Saves\\Final Fantasy VIII\\Mods\\Berrymapper - Hash Textures\\BerryMapper\\INPUT\\sel_13.bmp", CV_LOAD_IMAGE_COLOR);
	//sql_78 = cv::imread("H:\\Game Saves\\Final Fantasy VIII\\Mods\\Berrymapper - Hash Textures\\BerryMapper\\INPUT\\sql_78.bmp", CV_LOAD_IMAGE_COLOR);
	//zel_13 = cv::imread("H:\\Game Saves\\Final Fantasy VIII\\Mods\\Berrymapper - Hash Textures\\BerryMapper\\INPUT\\zel_13.bmp", CV_LOAD_IMAGE_COLOR);
	//hash_sel_13 = Hash_Algorithm_1(sel_13);
	//hash_sql_78 = Hash_Algorithm_1(sql_78);
	//hash_zel_13 = Hash_Algorithm_1(zel_13);
	//cout << hash_sel_13 << endl;
	//cout << hash_sql_78 << endl;
	//cout << hash_zel_13 << endl;

	//try {
	//	fs::directory_iterator iter_end;
	//	for (fs::directory_iterator tex_outer_dir(textures); tex_outer_dir != iter_end; tex_outer_dir++) {
	//		// [FFVIII]\textures\*
	//		if (!fs::is_directory(tex_outer_dir->path())) continue;
	//		for (fs::directory_iterator tex_inner_dir(tex_outer_dir->path()); tex_inner_dir != iter_end; tex_inner_dir++) {
	//			// [FFVIII]\textures\tex_inner_dir\*
	//			if (!fs::is_directory(tex_inner_dir->path())) continue;
	//			for (fs::directory_iterator files(tex_inner_dir->path()); files != iter_end; files++) {
	//				// [FFVIII]\textures\tex_outer_dir\tex_inner_dir\*.bmp
	//				string pathstring = files->path().string();
	//				cout << pathstring << endl;
	//			}
	//		}
	//	}
	//}
	//catch (fs::fs_error e) {
	//	cout << e.what() << endl;
	//}
	return 0;
}

//
// Old Algorithms
//

uint64_t Hash_Algorithm_slow(const cv::Mat& img)	//hash algorithm that preferences top and left sides
{
	uint64 hash = 0;
	float first_val, val = 0, last_val = 0;
	for (int i = 0; i < 64; i++) {
		int x, y;
		if (i < 32) {
			x = i / 4;
			y = i % 4;
		}
		else if (i < 56) {
			x = (i - 32) / 12;
			y = (i - 32) % 12 + 4;
		}
		else {
			x = (i - 56) / 4 * 3 + 3;
			y = (i - 56) % 4 * 3 + 5;
		}

		x *= BLOCKSIZE;
		y *= BLOCKSIZE;

		// get average pixel value
		if (x < img.cols && y < img.rows) {
			cv::Vec3b pixel = img.at<cv::Vec3b>(y, x);
			val = (pixel[0] + pixel[1] + pixel[2]) / 3;
		}
		else
			val = 0;

		// compare to last value for hash
		if (i > 0) {
			hash <<= 1;
			if (val >= last_val)
				hash |= 1;
		}
		else
			first_val = val;

		// do wrap-around comparison
		if (i == 63 && first_val >= val) {
			hash |= 0x8000000000000000;
		}

		last_val = val;
	}

	return hash;
}

uint64 Hash_Algorithm_1_lessslow(const cv::Mat& img)
{
	int x = 0, y = 0;
	float first_val, val = 0, last_val = 0;
	uint64 hash = 0;
	cv::Vec3b pixel;

	// first_val
	pixel = img.at<cv::Vec3b>(y, x);
	first_val = last_val = (pixel[0] + pixel[1] + pixel[2]) / 3;

	//int i = 0;
	//printf("%d:(%d,%d)\n", i++, x >> 4, y >> 4);

	for (y = 1; y < 4; y++) {
		//for (y = BLOCKSIZE; y < BLOCKSIZE << 2; y += BLOCKSIZE) { // pixvals 1->3

		if (y < img.rows) {
			pixel = img.at<cv::Vec3b>(y, x);
			val = (pixel[0] + pixel[1] + pixel[2]) / 3;
		}
		else
			val = 0;

		//printf("%d:(%d,%d)\n", i++, x >> 4, y >> 4);

		hash <<= 1;
		if (val >= last_val) hash++;
		last_val = val;
	}

	//for (x = 1; x < 8; x++)
	for (x = BLOCKSIZE; x < BLOCKSIZE << 3; x += BLOCKSIZE) // pixvals 4->31
	{
		//for (y = 0; y < 4; y++)
		for (y = 0; y < BLOCKSIZE << 2; y += BLOCKSIZE)
		{
			if (x < img.cols && y < img.rows) {
				pixel = img.at<cv::Vec3b>(y, x);
				val = (pixel[0] + pixel[1] + pixel[2]) / 3;
			}
			else
				val = 0;

			//printf("%d:(%d,%d)\n", i++, x >> 4, y >> 4);

			hash <<= 1;
			if (val >= last_val) hash++;
			last_val = val;
		}
	}

	//for (x = 0; x < 2; x++)
	for (x = 0; x < BLOCKSIZE << 1; x += BLOCKSIZE) // pixvals 32->55
	{
		// for (y = 4; y < 16; y++)
		for (y = BLOCKSIZE << 2; y < BLOCKSIZE << 4; y += BLOCKSIZE)
		{
			if (x < img.cols && y < img.rows) {
				pixel = img.at<cv::Vec3b>(y, x);
				val = (pixel[0] + pixel[1] + pixel[2]) / 3;
			}
			else
				val = 0;

			//printf("%d:(%d,%d)\n", i++, x >> 4, y >> 4);

			hash <<= 1;
			if (val >= last_val) hash++;
			last_val = val;
		}
	}

	//for (x = 3; x <= 6; x += 3)
	for (x = (BLOCKSIZE << 1) + BLOCKSIZE; x <= (BLOCKSIZE << 1) + (BLOCKSIZE << 2); x += (BLOCKSIZE << 1) + BLOCKSIZE) // pixvals 56->63, note +=3
	{
		//for (y = 5; y < 15; y += 3)
		for (y = (BLOCKSIZE << 2) + BLOCKSIZE; y <= (BLOCKSIZE << 1) + (BLOCKSIZE << 2) + (BLOCKSIZE << 3); y += (BLOCKSIZE << 1) + BLOCKSIZE) // note +=3
		{
			if (x < img.cols && y < img.rows) {
				pixel = img.at<cv::Vec3b>(y, x);
				val = (pixel[0] + pixel[1] + pixel[2]) / 3;
			}
			else
				val = 0;

			//printf("%d:(%d,%d)\n", i++, x >> 4, y >> 4);

			hash <<= 1;
			if (val >= last_val) hash++;
			last_val = val;
		}
	}

	if (first_val >= last_val) hash |= 0x8000000000000000;

	return hash;
}