#ifndef HASHCOORD_H
#define HASHCOORD_H

#include <string>
#include <sstream>

class HashCoord
{
public:
	friend std::hash < HashCoord > ;

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

	HashCoord(int _x, int _y, color _c) : x(_x), y(_y), c(_c) {}
	HashCoord(int _x = -1, int _y = -1, int _c = -1) : x(_x), y(_y)
	{
		if (_c < -1 || _c > 2) c = none;
		else c = (color)_c;
	}

	bool operator<(const HashCoord& rhs) const
	{
		return (rhs.y > y) || (rhs.y == y && rhs.x > x) || (rhs.y == y && rhs.x == x && rhs.c > c);
	}

	bool operator==(const HashCoord& rhs) const
	{
		return rhs.y == y && rhs.x == x && rhs.c == c;
	}

	std::string string()
	{
		std::stringstream ss;
		ss << "(" << x << ", " << y;
		switch (c) {
		case 0: ss << ", r"; break;
		case 1: ss << ", g"; break;
		case 2: ss << ", b"; break;
		}
		ss << ")";
		return ss.str();
	}
};

template <>
struct std::hash<HashCoord>
{
	std::size_t operator()(const HashCoord& key) const
	{
		// Compute individual hash values for x, y, and c and combine them using XOR and bit shifting:
		return ((hash<int>()(key.x) ^ (hash<int>()(key.y) << 1)) >> 1) ^ (hash<int>()(key.c) << 1);
	}
};

#endif // HASHCOORD_H

