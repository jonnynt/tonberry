#include "Main.h"
#include "cachemap.h"
#include "hashcoord.h"
#include "texturehash.h"
#include <stdint.h>
#include <sstream>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <chrono>
namespace fs = boost::filesystem;

#ifndef ULTRA_FAST
bool g_ReportingEvents = false;
#endif

typedef unsigned char uchar;

GlobalContext *g_Context;

template <typename T>
bool ToNumber(const std::string& str, T & val)							// convert string to uint64_t
{
	std::stringstream ss(str);
	return (ss >> val);
}

/**********************************
*
*	Global Variables
*
**********************************/

const int VRAM_DIM = 256;

int texture_count = 0;													// keep track of the number of textures processed

TextureCache* cache;
FieldMap* fieldmap;
unordered_set<uint64_t> nomatch_left;
unordered_set<uint64_t> nomatch_right;



//
// PATHS
//
fs::path TONBERRY_DIR("tonberry");
fs::path TEXTURES_DIR("textures");
fs::path DEBUG_DIR(TONBERRY_DIR / "debug");
fs::path HASHMAP_DIR(TONBERRY_DIR / "hashmap");
fs::path COORDS_CSV(TONBERRY_DIR / "coords.csv");
fs::path PREFS_TXT(TONBERRY_DIR / "prefs.txt");
fs::path ERROR_LOG(TONBERRY_DIR / "error.log");
fs::path DEBUG_LOG(DEBUG_DIR / "debug.log");
fs::path NOMATCH_LOG(DEBUG_DIR / "nomatch.log");
fs::path COLLISIONS_CSV(TONBERRY_DIR / "collisions.csv");
fs::path HASHMAP2_CSV(TONBERRY_DIR / "hash2map.csv");
fs::path OBJECTS_CSV(TONBERRY_DIR / "objmap.csv");

const size_t COORDS_LEN = 324;
const HashCoord COORDS[COORDS_LEN] = { HashCoord(6, 7), HashCoord(14, 7), HashCoord(20, 7), HashCoord(26, 6), HashCoord(30, 7), HashCoord(38, 7), HashCoord(49, 6), HashCoord(52, 7), HashCoord(58, 6), HashCoord(70, 7), HashCoord(74, 7), HashCoord(82, 7), HashCoord(86, 7), HashCoord(98, 7), HashCoord(100, 7), HashCoord(108, 7), HashCoord(114, 7), HashCoord(122, 7), HashCoord(7, 13), HashCoord(14, 14), HashCoord(18, 14), HashCoord(26, 14), HashCoord(34, 14), HashCoord(42, 14), HashCoord(46, 14), HashCoord(56, 14), HashCoord(58, 14), HashCoord(70, 13), HashCoord(74, 14), HashCoord(82, 12), HashCoord(90, 12), HashCoord(98, 14), HashCoord(102, 13), HashCoord(108, 12), HashCoord(114, 14), HashCoord(122, 12), HashCoord(6, 17), HashCoord(14, 19), HashCoord(18, 20), HashCoord(26, 18), HashCoord(34, 21), HashCoord(40, 20), HashCoord(44, 21), HashCoord(54, 21), HashCoord(58, 18), HashCoord(70, 17), HashCoord(74, 20), HashCoord(82, 17), HashCoord(90, 18), HashCoord(94, 21), HashCoord(104, 20), HashCoord(108, 21), HashCoord(114, 21), HashCoord(122, 20), HashCoord(7, 27), HashCoord(14, 27), HashCoord(20, 28), HashCoord(26, 26), HashCoord(34, 26), HashCoord(40, 28), HashCoord(44, 25), HashCoord(54, 24), HashCoord(58, 26), HashCoord(70, 27), HashCoord(76, 27), HashCoord(82, 28), HashCoord(88, 26), HashCoord(94, 28), HashCoord(102, 25), HashCoord(108, 25), HashCoord(114, 28), HashCoord(122, 28), HashCoord(6, 35), HashCoord(12, 35), HashCoord(18, 30), HashCoord(24, 34), HashCoord(34, 30), HashCoord(40, 32), HashCoord(44, 31), HashCoord(52, 30), HashCoord(58, 30), HashCoord(66, 30), HashCoord(76, 31), HashCoord(82, 35), HashCoord(88, 30), HashCoord(93, 31), HashCoord(104, 34), HashCoord(108, 33), HashCoord(114, 30), HashCoord(121, 35), HashCoord(6, 41), HashCoord(14, 39), HashCoord(20, 40), HashCoord(24, 42), HashCoord(30, 39), HashCoord(40, 40), HashCoord(44, 37), HashCoord(54, 42), HashCoord(58, 38), HashCoord(70, 41), HashCoord(72, 38), HashCoord(82, 38), HashCoord(88, 40), HashCoord(94, 41), HashCoord(102, 37), HashCoord(108, 42), HashCoord(116, 41), HashCoord(122, 42), HashCoord(6, 44), HashCoord(14, 47), HashCoord(18, 44), HashCoord(24, 44), HashCoord(34, 46), HashCoord(40, 44), HashCoord(48, 44), HashCoord(54, 45), HashCoord(58, 44), HashCoord(70, 45), HashCoord(74, 44), HashCoord(84, 45), HashCoord(90, 44), HashCoord(93, 45), HashCoord(102, 45), HashCoord(108, 44), HashCoord(114, 44), HashCoord(121, 44), HashCoord(6, 53), HashCoord(14, 51), HashCoord(18, 52), HashCoord(25, 51), HashCoord(34, 54), HashCoord(40, 52), HashCoord(48, 51), HashCoord(52, 51), HashCoord(58, 54), HashCoord(70, 53), HashCoord(74, 51), HashCoord(82, 52), HashCoord(90, 51), HashCoord(94, 51), HashCoord(100, 51), HashCoord(108, 51), HashCoord(114, 52), HashCoord(121, 54), HashCoord(6, 62), HashCoord(12, 59), HashCoord(18, 60), HashCoord(24, 60), HashCoord(30, 59), HashCoord(40, 58), HashCoord(44, 59), HashCoord(56, 58), HashCoord(58, 58), HashCoord(70, 58), HashCoord(75, 58), HashCoord(84, 58), HashCoord(88, 58), HashCoord(98, 58), HashCoord(102, 58), HashCoord(108, 58), HashCoord(114, 58), HashCoord(121, 62), HashCoord(7, 70), HashCoord(12, 69), HashCoord(18, 70), HashCoord(26, 70), HashCoord(34, 70), HashCoord(40, 68), HashCoord(44, 70), HashCoord(52, 70), HashCoord(60, 69), HashCoord(70, 69), HashCoord(74, 68), HashCoord(82, 70), HashCoord(86, 69), HashCoord(94, 67), HashCoord(104, 70), HashCoord(108, 69), HashCoord(116, 67), HashCoord(122, 66), HashCoord(7, 77), HashCoord(14, 77), HashCoord(18, 76), HashCoord(26, 74), HashCoord(30, 75), HashCoord(40, 76), HashCoord(46, 77), HashCoord(52, 75), HashCoord(58, 76), HashCoord(70, 77), HashCoord(76, 75), HashCoord(84, 77), HashCoord(86, 77), HashCoord(98, 76), HashCoord(104, 74), HashCoord(108, 75), HashCoord(114, 76), HashCoord(121, 74), HashCoord(7, 79), HashCoord(14, 79), HashCoord(20, 79), HashCoord(25, 79), HashCoord(34, 84), HashCoord(40, 84), HashCoord(44, 79), HashCoord(54, 79), HashCoord(58, 79), HashCoord(70, 79), HashCoord(74, 84), HashCoord(82, 84), HashCoord(88, 80), HashCoord(98, 82), HashCoord(104, 84), HashCoord(112, 83), HashCoord(114, 84), HashCoord(121, 84), HashCoord(7, 87), HashCoord(14, 87), HashCoord(18, 86), HashCoord(26, 86), HashCoord(34, 86), HashCoord(40, 86), HashCoord(44, 87), HashCoord(51, 86), HashCoord(58, 86), HashCoord(70, 87), HashCoord(76, 87), HashCoord(82, 87), HashCoord(86, 87), HashCoord(98, 86), HashCoord(104, 86), HashCoord(108, 87), HashCoord(114, 86), HashCoord(122, 86), HashCoord(6, 97), HashCoord(13, 97), HashCoord(16, 98), HashCoord(24, 98), HashCoord(32, 98), HashCoord(40, 96), HashCoord(48, 98), HashCoord(52, 98), HashCoord(58, 96), HashCoord(70, 97), HashCoord(76, 98), HashCoord(80, 98), HashCoord(86, 97), HashCoord(96, 98), HashCoord(105, 98), HashCoord(110, 97), HashCoord(114, 98), HashCoord(121, 98), HashCoord(7, 101), HashCoord(14, 101), HashCoord(16, 102), HashCoord(24, 102), HashCoord(30, 101), HashCoord(38, 101), HashCoord(46, 101), HashCoord(52, 102), HashCoord(62, 101), HashCoord(68, 102), HashCoord(76, 102), HashCoord(84, 102), HashCoord(91, 105), HashCoord(94, 101), HashCoord(102, 101), HashCoord(107, 101), HashCoord(114, 101), HashCoord(121, 102), HashCoord(7, 107), HashCoord(13, 107), HashCoord(21, 108), HashCoord(23, 107), HashCoord(31, 107), HashCoord(41, 108), HashCoord(45, 107), HashCoord(51, 107), HashCoord(58, 112), HashCoord(69, 108), HashCoord(77, 107), HashCoord(84, 111), HashCoord(91, 107), HashCoord(93, 108), HashCoord(103, 107), HashCoord(107, 107), HashCoord(116, 112), HashCoord(121, 108), HashCoord(6, 116), HashCoord(14, 115), HashCoord(20, 116), HashCoord(25, 114), HashCoord(33, 114), HashCoord(40, 116), HashCoord(44, 116), HashCoord(52, 114), HashCoord(58, 114), HashCoord(70, 117), HashCoord(74, 116), HashCoord(84, 114), HashCoord(86, 115), HashCoord(93, 114), HashCoord(102, 115), HashCoord(110, 115), HashCoord(114, 114), HashCoord(122, 115), HashCoord(7, 123), HashCoord(14, 121), HashCoord(21, 121), HashCoord(26, 121), HashCoord(34, 121), HashCoord(42, 121), HashCoord(44, 121), HashCoord(52, 121), HashCoord(58, 121), HashCoord(69, 121), HashCoord(74, 121), HashCoord(82, 121), HashCoord(87, 121), HashCoord(93, 121), HashCoord(100, 121), HashCoord(107, 121), HashCoord(114, 121), HashCoord(122, 121) };

//
// USER PREFERENCES
//

float RESIZE_FACTOR = 4.0;		// texture upscale factor
bool DEBUG = false;				// write debug information
unsigned CACHE_SIZE = 100;		// number of textures to hold in the cache size

void GraphicsInfo::Init()
{
	_Device = NULL;
	_Overlay = NULL;
}

void GraphicsInfo::SetDevice(LPDIRECT3DDEVICE9 Device)
{
	Assert(Device != NULL, "Device == NULL");
	D3D9Base::IDirect3DSwapChain9* pSwapChain;
	HRESULT hr = Device->GetSwapChain(0, &pSwapChain);
	Assert(SUCCEEDED(hr), "GetSwapChain failed");
	hr = pSwapChain->GetPresentParameters(&_PresentParameters);
	Assert(SUCCEEDED(hr), "GetPresentParameters failed");
	pSwapChain->Release();
	hr = Device->GetCreationParameters(&_CreationParameters);
	Assert(SUCCEEDED(hr), "GetCreationParameters failed");
	_Device = Device;
}

void load_prefs()
{
	ifstream prefsfile;
	prefsfile.open(PREFS_TXT.string(), ifstream::in);
	if (prefsfile.is_open()) {
		string line;
		while (getline(prefsfile, line)) {
			int eq_index = line.find("=");
			if (eq_index < 0) continue;

			string param = line.substr(0, eq_index);
			string value = line.substr(eq_index + 1);

			if (boost::iequals(param, "RESIZE_FACTOR"))		// ignore case
				RESIZE_FACTOR = ToNumber<float>(value);
			else if (boost::iequals(param, "debug_mode"))	// ignore case
				DEBUG = (boost::iequals(value, "yes"));		// ignore case
			else if (boost::iequals(param, "cache_size"))	// ignore case
				CACHE_SIZE = ToNumber<unsigned>(value);
		}
		prefsfile.close();
	} else {
		ofstream err;																		//Error reporting
		err.open(ERROR_LOG.string(), ofstream::out | ofstream::app);
		err << "Error: could not open prefs.txt" << endl;
		err.close();
	}
}

// Searches for _hm.csv files in \tonberry\hashmap and adds them to texname_map_{top,bottom}
void load_fieldmaps()
{
	if (!fs::exists(HASHMAP_DIR)) {
		ofstream err;																		// Error reporting file
		err.open(ERROR_LOG.string(), ofstream::out | ofstream::app);
		err << "Error: hashmap folder doesn't exist" << endl;
		err.close();
	} else {
		fs::directory_iterator end_it;														// get tonberry/hashmap folder iterator
		for (fs::directory_iterator it(HASHMAP_DIR); it != end_it; it++) {

			// boost::iequals ignores case in string match
			// so .CsV will work as well as .csv
			if (fs::is_regular_file(it->status()) && boost::iequals(it->path().extension().string(), ".csv")) {	// file is .csv
				ifstream hashfile;
				hashfile.open(it->path().string(), ifstream::in);							// open it and dump into the map
				if (hashfile.is_open()) {
					string line;
					while (getline(hashfile, line))
					{
						// split line on ','
						std::deque<string> items;
						std::stringstream sstream(line);
						std::string item;
						while (std::getline(sstream, item, ',')) {
							items.push_back(item);
						}

						// format is "<field_name>,<hash_combined>{,<hash_upper>,<hash_lower>}"
						if (!(items.size() == 2 || items.size() == 4)) {
							ofstream err;													//Error reporting
							err.open(ERROR_LOG.string(), ofstream::out | ofstream::app);
							err << "Error: bad hashmap. Format is \"<field_name>,<hash_combined>{,<hash_upper>,<hash_lower>}\": " << it->path.str() << endl;
							err.close();
							return;
						}

						// field names are stored only once
						string field = items[0];

						uint64_t hash_combined;
						if (!ToNumber<uint64_t>(items[1], hash_combined)) {
							ofstream err;													//Error reporting
							err.open(ERROR_LOG.string(), ofstream::out | ofstream::app);
							err << "Error: bad hashmap entry. Must be an integer: " << items[1] << endl;
							err.close();
						} else
							fieldmap->insert(hash_combined, field);

						if (items.size() > 2) {
							uint64_t hash_upper, hash_lower;
							if (!ToNumber<uint64_t>(items[2], hash_upper)) {
								ofstream err;												//Error reporting
								err.open(ERROR_LOG.string(), ofstream::out | ofstream::app);
								err << "Error: bad hashmap entry. Must be an integer: " << items[2] << endl;
								err.close();
							} else
								fieldmap->insert(hash_upper, field);

							if (!ToNumber<uint64_t>(items[3], hash_lower)) {
								ofstream err;												//Error reporting
								err.open(ERROR_LOG.string(), ofstream::out | ofstream::app);
								err << "Error: bad hashmap entry. Must be an integer: " << items[3] << endl;
								err.close();
							} else
								fieldmap->insert(hash_lower, field);
						}
					}
					hashfile.close();
				} else {
					ofstream err;															//Error reporting
					err.open(ERROR_LOG.string(), ofstream::out | ofstream::app);
					err << "Error: could not open " << it->path().string() << endl;
					err.close();
				}
			}
		}
	}
}

void GlobalContext::Init()
{	
	ofstream debug(DEBUG_LOG.string(), ofstream::out | ofstream::trunc);
	std::time_t time = std::time(nullptr);
	debug << "Initialized " << asctime(localtime(&time)) << endl;

	ofstream nomatch(NOMATCH_LOG.string(), ofstream::out | ofstream::trunc);
	nomatch << "Initialized " << asctime(localtime(&time)) << endl << endl;

	Graphics.Init();
	load_prefs();
	debug << "prefs.txt loaded." << endl;
	if (DEBUG) debug << "Debug mode enabled." << endl;

	cache = new TextureCache(CACHE_SIZE);
	fieldmap = new FieldMap();

	load_fieldmaps();
	debug << "hashmap loaded." << endl << endl;

	debug << "fieldmap:" << endl;
	fieldmap->writeMap(debug);

	debug << endl;
	debug.close();
}

uint64_t Murmur2_Hash(BYTE* pData, UINT pitch, int width, int height, const HashCoord* coords, const int len)
{
	int buflen = len * 3;
	char* buf = new char[buflen];
	int index = 0;

	const HashCoord* coord = coords;
	for (int i = 0; i < len; i++, coord++) {
		RGBColor color(0, 0, 0);
		if (coord->x < width && coord->y < height) {
			RGBColor* row = (RGBColor*)(pData + (coord->y) * pitch);
			color = row[coord->x];
		}
		buf[index++] = color.r;
		buf[index++] = color.g;
		buf[index++] = color.b;
	}

	uint64_t hash = TextureHash::Murmur2::MurmurHash64B(buf, buflen, TextureHash::Murmur2::MURMUR2_SEED);
	delete[] buf;
	return hash;
}

uint64_t Murmur2_Combined(BYTE* pData, UINT pitch, int width, int height, const HashCoord* coords, const int len, uint64_t hash_upper, uint64_t hash_lower)
{
	int buflen = len * 3 * 2;
	BYTE* buf = new BYTE[buflen];
	memset(buf, 0, sizeof(BYTE) * buflen);
	int index = 0;

	BYTE* data = pData;
	const HashCoord* coord = coords;
	for (int i = 0; i < len; i++, coord++) {
		RGBColor color(0, 0, 0);
		if (coord->x < width && coord->y < height)
			color = *((RGBColor*)(data + (coord->y * pitch) + coord->x));
		buf[index++] = color.r;
		buf[index++] = color.g;
		buf[index++] = color.b;
	}

	hash_upper = TextureHash::Murmur2::MurmurHash64B(buf, buflen, TextureHash::Murmur2::MURMUR2_SEED);

	if (height > (VRAM_DIM / 2)) {		// there is a lower half to hash
		data = pData + (VRAM_DIM / 2) * pitch;
		height -= (VRAM_DIM / 2);
		coord = coords;
		for (int i = 0; i < len; i++, coord++) {
			RGBColor color(0, 0, 0);
			if (coord->x < width && coord->y < height)
				color = *((RGBColor*)(data + (coord->y * pitch) + coord->x));
			buf[index++] = color.r;
			buf[index++] = color.g;
			buf[index++] = color.b;
		}
	}

	uint64_t hash_combined = TextureHash::Murmur2::MurmurHash64B(buf, buflen, TextureHash::Murmur2::MURMUR2_SEED);
	memset(buf, 0, sizeof(BYTE) * buflen / 2);
	hash_lower = TextureHash::Murmur2::MurmurHash64B(buf, buflen, TextureHash::Murmur2::MURMUR2_SEED);
	delete[] buf;
	return hash_combined;
}

bool get_fields(const uint64_t& hash_combined, const uint64_t& hash_upper, const uint64_t& hash_lower, string& field_combined, string& field_upper, string& field_lower)
{
	unordered_set<string> fields;
	int upper_matches = 0, lower_matches = 0;

	// search for hash_combined
	if (fieldmap->get_fields(hash_combined, fields)) {
		field_combined = *(fields.begin());										// a field matches whole texture: use this one
		return true;
	}

	// hash_upper and hash_lower should never match the first file, because hash_combined would already have matched it
	return (fieldmap->get_first_field(hash_upper, field_upper) || fieldmap->get_first_field(hash_lower, field_lower));
}

#define min(a, b) ((a <= b) ? a : b)

HANDLE create_newhandle(BYTE* replaced_pData, UINT replaced_width, UINT replaced_height, UINT replaced_pitch, const string* field_combined, const string* field_upper = NULL, const string* field_lower = NULL)
{
	ofstream debug((DEBUG_DIR / "create_newhandle.log").string(), ofstream::out | ofstream::trunc);
	bool use_combined, use_upper = false, use_lower = false;
	fs::path path_combined, path_upper, path_lower;
	ifstream ifile_combined, ifile_upper, ifile_lower;

	use_combined = (field_combined != NULL && !field_combined->empty());
	
	if (use_combined) {
		// get texture path from field name
		path_combined = ((((TEXTURES_DIR / field_combined->substr(0, 2))) / field_combined->substr(0, field_combined->rfind("_"))) / (*field_combined + ".png"));
		debug << "Loading combined from " << path_combined << "... ";

		// load file_combined
		ifile_combined.open(path_combined.string());
		if (ifile_combined.fail()) {
			debug << "failed." << endl;
			return NULL;													// file could not be opened, so no texture can be created
		}
		debug << "succeeded!" << endl;
	} else {
		use_upper = (field_upper != NULL && !field_upper->empty());
		use_lower = (field_lower != NULL && !field_lower->empty());
	
		if (use_upper) {
			// get texture path from field name
			path_upper = ((((TEXTURES_DIR / field_upper->substr(0, 2))) / field_upper->substr(0, field_upper->rfind("_"))) / (*field_upper + ".png"));
			debug << "Loading upper from " << path_upper << "... ";

			// load file_upper
			ifile_upper.open(path_upper.string());
			if (ifile_lower.fail()) {
				debug << "failed." << endl;
				use_upper = false;											// file could not be opened, so do not use upper half
			}
			debug << "succeeded!" << endl;
		}

		if (use_lower) {
			// get texture path from field name
			path_lower = ((((TEXTURES_DIR / field_lower->substr(0, 2))) / field_lower->substr(0, field_lower->rfind("_"))) / (*field_lower + ".png"));
			debug << "Loading lower from " << path_lower << "... ";

			// load file_lower
			ifile_lower.open(path_lower.string());
			if (ifile_lower.fail()) {
				debug << "failed." << endl;
				use_lower = false;											// file could not be opened, so do not use upper half
			}
			debug << "succeeded!" << endl;
		}

		if (!use_upper && !use_lower) return NULL;							// neither file could be loaded, so no texture can be created
	}

	LPDIRECT3DDEVICE9 Device = g_Context->Graphics.Device();
	IDirect3DTexture9* newtexture;
	Bitmap bmp_combined, bmp_upper, bmp_lower;

	// load replacement bitmaps
	if (use_combined) {
		bmp_combined.LoadPNG(String(path_combined.string().c_str()));
		debug << path_combined << ": " << bmp_combined.Width() << "x" << bmp_combined.Height() << endl;
	} else {
		if (use_upper) {
			bmp_upper.LoadPNG(String(path_upper.string().c_str()));
			debug << path_upper << ": " << bmp_upper.Width() << "x" << bmp_upper.Height() << endl;
		}
		if (use_lower) {
			bmp_lower.LoadPNG(String(path_lower.string().c_str()));
			debug << path_lower << ": " << bmp_lower.Width() << "x" << bmp_lower.Height() << endl;
		}
	}

	// initialize newtexture
	int replacement_width = int(RESIZE_FACTOR * (float)replaced_width);
	int replacement_height = int(RESIZE_FACTOR * (float)replaced_height);
	debug << "New Texture: " << replacement_width << "x" << replacement_height << endl;
	Device->CreateTexture(replacement_width, replacement_height, 0, D3DUSAGE_AUTOGENMIPMAP, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &newtexture, NULL);

	// load image data into newtexture
	D3DLOCKED_RECT newRect;
	newtexture->LockRect(0, &newRect, NULL, 0);
	BYTE* newData = (BYTE *)newRect.pBits;

	debug << "Copying Pixels:" << endl;
	for (UINT y = 0; y < replacement_height; y++) {
		//debug << "\t" << y << ":";
		RGBColor* CurRow = (RGBColor *)(newData + y * newRect.Pitch);
		RGBColor Color;
		if (use_combined) {
			for (UINT x = 0; x < replacement_width; x++) {
				//debug << " " << x;
				if (x < bmp_combined.Width() && y < bmp_combined.Height()) {				// respect texture sizes
					Color = bmp_combined[replacement_height - y - 1][x];					// must flip image
					CurRow[x] = RGBColor(Color.b, Color.g, Color.r, Color.a);
				}
			}
		} else if (y < replacement_height / 2) {											// set lower bits (because flipped)
			if (use_lower) {																// use pixels from bmp_lower
				for (UINT x = 0; x < replacement_width; x++) {
					//debug << " " << x;
					if (x < bmp_lower.Width() && y < bmp_lower.Height()) {					// respect texture sizes
						Color = bmp_lower[bmp_lower.Height() - 1 - y][x];
						CurRow[x] = RGBColor(Color.b, Color.g, Color.r, Color.a);
					}
				}
			} else {																		// use upscaled pixels from replaced pData
				for (UINT x = 0; x < replacement_width; x++) {
					//debug << " " << x;
					RGBColor* OldRow = (RGBColor*)(replaced_pData + (replaced_height - 1 - y / 4) * replaced_pitch);
					Color = OldRow[(int)(x / RESIZE_FACTOR)];
					CurRow[x] = RGBColor(Color.b, Color.g, Color.r, Color.a);
				}
			}
		} else {																			// set upper bits (because flipped)
			if (use_upper) {																// use pixels from bmp_upper
				for (UINT x = 0; x < replacement_width; x++) {
					//debug << " " << x;
					int upper_y = bmp_upper.Height() - 1 - y;
					if (upper_y < 0) upper_y += bmp_upper.Height();							// if bmp_upper is only half a full replacement texture
					if (x < bmp_upper.Width() && upper_y < bmp_upper.Height()) {			// respect texture sizes
						Color = bmp_upper[upper_y][x];
						CurRow[x] = RGBColor(Color.b, Color.g, Color.r, Color.a);
					}
				}
			} else {																		// use upscaled pixels from replaced pData
				for (UINT x = 0; x < replacement_width; x++) {
					//debug << " " << x;
					RGBColor* OldRow = (RGBColor*)(replaced_pData + (replaced_height - 1 - y / 4) * replaced_pitch);
					Color = OldRow[(int)(x / RESIZE_FACTOR)];
					CurRow[x] = RGBColor(Color.b, Color.g, Color.r, Color.a);
				}
			}
		}
		//debug << endl;
	}
	newtexture->UnlockRect(0);																// Texture loaded
	debug << "Texture loaded successfully." << endl;
	debug.close();
	return(HANDLE)newtexture;
}


void GlobalContext::UnlockRect(D3DSURFACE_DESC &Desc, Bitmap &BmpUseless, HANDLE Handle) // note BmpUseless
{
	IDirect3DTexture9* pTexture = (IDirect3DTexture9*)Handle;

	String debugtype = String("");

	ofstream debug(DEBUG_LOG.string(), ofstream::out | ofstream::app);

	bool handle_used = false;													// if false, Handle will be erased from the TextureCache
	if (pTexture && Desc.Width < 640 && Desc.Height < 480 && Desc.Format == D3DFORMAT::D3DFMT_A8R8G8B8 && Desc.Pool == D3DPOOL::D3DPOOL_MANAGED) {   //640x480 are video
		D3DLOCKED_RECT Rect;
		pTexture->LockRect(0, &Rect, NULL, 0);
		UINT pitch = (UINT)Rect.Pitch;
		BYTE* pData = (BYTE*)Rect.pBits;

		// get field matches using Murmur2 hash
		uint64_t hash_combined = 0, hash_upper = 0, hash_lower = 0;
		string field_combined = "", field_upper = "", field_lower = "";
		bool upper_exists = false, lower_exists = false;

		// get hashes
		hash_combined = Murmur2_Combined(pData, pitch, Desc.Width, Desc.Height, COORDS, COORDS_LEN, hash_upper, hash_lower);

		uint64_t hash_used;
		bool use_combined = cache->contains(hash_combined);

		if (use_combined) {														// there is an existing newhandle for hash_combined; use it!
			debug << "use_combined (" << hash_combined << ")" << endl;
			cache->insert(Handle, hash_combined);
			handle_used = true;
		} else {
			// look for matching fields
			get_fields(hash_combined, hash_upper, hash_lower, field_combined, field_upper, field_lower);
			bool create_combined = !field_combined.empty();

			if (create_combined) {												// there is a matching field for hash_combined; create it!
				debug << "create_combined (" << hash_combined << ") from " << field_combined << "... ";
				HANDLE newhandle = create_newhandle(pData, Desc.Width, Desc.Height, pitch, &field_combined, NULL, NULL);
				if (newhandle) {
					debug << "succeeded!" << endl;;
					cache->insert(Handle, hash_combined, newhandle);
					handle_used = true;
				} else
					debug << "failed..." << endl;;
			} else {
				bool use_upper = cache->contains(hash_upper);
				bool use_lower = cache->contains(hash_lower);

				if (use_upper && use_lower) {									// there are existing newhandles for hash_upper and hash_lower; combine them!
																				// TODO: implement
					debug << "use_upper && use_lower (not yet implemented)" << endl;
				} else {
					bool create_upper = !field_upper.empty();
					bool create_lower = !field_lower.empty();

					if (create_upper && create_lower) {							// there are matching fields for hash_upper and hash_lower; create a combination!
						debug << "create_upper (" << hash_upper << ") && create_lower (" << hash_lower << ") from " << field_upper << " and " << field_lower << "... ";
						HANDLE newhandle = create_newhandle(pData, Desc.Width, Desc.Height, pitch, NULL, &field_upper, &field_lower);
						if (newhandle) {
							debug << "succeeded!" << endl;;
							cache->insert(Handle, hash_combined, newhandle);
							handle_used = true;
						} else
							debug << "failed..." << endl;;
					} else if (use_upper) {										// there is an existing newhandle for hash_upper; use it!
						debug << "use_upper (" << hash_upper << ") only." << endl;
						cache->insert(Handle, hash_upper);						// TODO: this is wrong, need to create a new texture from existing newhandle upper half and Handle lower half
						handle_used = true;
					} else if (use_lower) {										// there is an existing newhandle for hash_lower; use it!
						debug << "use_lower (" << hash_lower << ") only." << endl;
						cache->insert(Handle, hash_lower);						// TODO: this is wrong, need to create a new texture from existing newhandle lower half and Handle upper half
						handle_used = true;
					} else if (create_upper) {									// there is a matching field for hash_upper; create it!
						debug << "create_upper (" << hash_upper << ") from " << field_upper << "... ";
						//HANDLE newhandle = create_newhandle(pData, Desc.Width, Desc.Height, pitch, NULL, &field_upper, NULL);
						HANDLE newhandle = create_newhandle(pData, Desc.Width, Desc.Height, pitch, &field_upper, NULL, NULL);
						if (newhandle) {
							//cache->insert(Handle, hash_combined, newhandle);	// TODO: this is wrong, need to store at hash_upper
							debug << "succeeded!" << endl;;
							cache->insert(Handle, hash_upper, newhandle);
							handle_used = true;
						} else
							debug << "failed..." << endl;;
					} else if (create_lower) {									// there is a matching field for hash_lower; create it!
						debug << "create_lower (" << hash_lower << ") from " << field_lower << "... ";
						HANDLE newhandle = create_newhandle(pData, Desc.Width, Desc.Height, pitch, NULL, NULL, &field_lower);
						if (newhandle) {
							debug << "succeeded!" << endl;;
							cache->insert(Handle, hash_combined, newhandle);	// TODO: this is wrong, need to store at hash_lower
							handle_used = true;
						} else
							debug << "failed..." << endl;;
					} else {													// NO MATCH
						if (DEBUG && Desc.Width > 0 && Desc.Height > 0) {

							long long time = chrono::duration_cast<chrono::milliseconds>(chrono::high_resolution_clock::now().time_since_epoch()).count();
							ostringstream sstream;
							if (Desc.Width <= VRAM_DIM / 2) {					// save the whole image
								uint64_t hash = FNV_Hash_Full(pData, pitch, Desc.Width, Desc.Height);
								if (nomatch_left.count(hash) == 0) {
									sstream << (DEBUG_DIR / "nomatch\\").string() << hash << ".bmp";
									pTexture->UnlockRect(0);
									if (D3DXSaveTextureToFile(sstream.str().c_str(), D3DXIFF_BMP, pTexture, NULL) == D3D_OK) {
										nomatch_left.insert(hash);
										ofstream nomatch(NOMATCH_LOG.string(), ofstream::out | ofstream::app);
										nomatch << time << "," << hash << "," << hash_combined << "," << hash_upper << "," << hash_lower << endl;
										nomatch.close();
									}
								} else {
									ofstream nomatch(NOMATCH_LOG.string(), ofstream::out | ofstream::app);
									nomatch << time << "," << "SKIPPED" << hash << endl;
									nomatch.close();
								}
							} else {
								uint16_t save_option = 0;						// 0: save nothing; 1: save left-half only; 2: save whole image
								uint64_t hash_left = FNV_Hash_Full(pData, pitch, VRAM_DIM / 2, Desc.Height);
								uint64_t hash_right = FNV_Hash_Full(pData, pitch, Desc.Width, Desc.Height, VRAM_DIM / 2);

								if (nomatch_left.count(hash_left) == 0)			// If we've never seen this left-half before:
									if (nomatch_left.count(hash_right) == 0)	//   If we've never seen this right-half on the left-half of an image: save the whole image
										save_option = 2;
									else										//   If we have seen this right-half on the left-half of an image: save only the left half
										save_option = 1;

								if (save_option < 2 &&
									nomatch_left.count(hash_right) == 0 &&		// If we've never seen this right-half on either side of the image: save the whole image
									nomatch_right.count(hash_right) == 0)
									save_option = 2;

								switch (save_option) {
								case 2:											// save the whole image
								{
									pTexture->UnlockRect(0);
									sstream << (DEBUG_DIR / "nomatch\\").string() << hash_left << "_" << hash_right << ".bmp";
									if (D3DXSaveTextureToFile(sstream.str().c_str(), D3DXIFF_BMP, pTexture, NULL) == D3D_OK) {
										nomatch_left.insert(hash_left);			// only insert if texture save was successful, so that \nomatch\ can be disabled by appending 0
										nomatch_right.insert(hash_right);
										ofstream nomatch(NOMATCH_LOG.string(), ofstream::out | ofstream::app);
										nomatch << time << "," << hash_left << "_" << hash_right << "," << hash_combined << "," << hash_upper << "," << hash_lower << endl;
										nomatch.close();
									}
									break;
								}
								case 1:											// save the left-half of the image
								{
									// create new texture
									LPDIRECT3DDEVICE9 Device = g_Context->Graphics.Device();
									IDirect3DTexture9* half_texture;
									Device->CreateTexture(Desc.Width / 2, Desc.Height, 0, D3DUSAGE_AUTOGENMIPMAP, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &half_texture, NULL);

									// copy left half to texture to half_texture
									D3DLOCKED_RECT newRect;
									half_texture->LockRect(0, &newRect, NULL, 0);
									BYTE* newData = (BYTE *)newRect.pBits;
									for (UINT y = 0; y < Desc.Height; y++) {
										RGBColor* CurRow = (RGBColor *)(newData + y * newRect.Pitch);
										for (UINT x = 0; x < Desc.Width / 2; x++) {
											RGBColor* OldRow = (RGBColor*)(pData + y * pitch);
											RGBColor Color = OldRow[x];
											CurRow[x] = RGBColor(Color.b, Color.g, Color.r, Color.a);
										}
									}

									// save half_texture
									half_texture->UnlockRect(0);
									sstream << (DEBUG_DIR / "nomatch\\").string() << hash_left << "_" << ".bmp";
									if (D3DXSaveTextureToFile(sstream.str().c_str(), D3DXIFF_BMP, half_texture, NULL) == D3D_OK) {
										nomatch_left.insert(hash_left);			// only insert if texture save was successful, so that \nomatch\ can be disabled by appending 0
										nomatch_right.insert(hash_right);
										ofstream nomatch(NOMATCH_LOG.string(), ofstream::out | ofstream::app);
										nomatch << time << "," << hash_left << "_" << "," << hash_combined << "," << hash_upper << "," << hash_lower << endl;
										nomatch.close();
									}

									// release half_texture
									half_texture->Release();
									break;
								}
								case 0:
								{
									ofstream nomatch(NOMATCH_LOG.string(), ofstream::out | ofstream::app);
									nomatch << time << "," << "SKIPPED" << hash_left << hash_right << endl;
									nomatch.close();
									break;
								}
								}
							}
						}
					}
				}
			}
		}
		pTexture->UnlockRect(0); //Finished reading pTextures bits
	} else { //Video textures/improper format
		//debug << "IMPROPER FORMAT";
	}

	if (!handle_used) cache->erase(Handle);

	debug.close();
	//if (debugtype == String("")) { debugtype = String("error"); }
	////Debug
	//String debugfile = String("tonberry\\debug\\") + debugtype + String("\\") + String::ZeroPad(String(m), 3) + String(".bmp");
	//D3DXSaveTextureToFile(debugfile.CString(), D3DXIFF_BMP, pTexture, NULL);
	texture_count++;
}

//and finally the settexture method

bool GlobalContext::SetTexture(DWORD Stage, HANDLE* SurfaceHandles, UINT SurfaceHandleCount)
{
	for (int j = 0; j < SurfaceHandleCount; j++) {
		IDirect3DTexture9* newtexture;
		if (SurfaceHandles[j] && (newtexture = (IDirect3DTexture9*)cache->at(SurfaceHandles[j]))) {
			g_Context->Graphics.Device()->SetTexture(Stage, newtexture);
			//((IDirect3DTexture9*)SurfaceHandles[j])->Release();
			return true;
		} // Texture replaced!
	}
	return false;
}
//Unused functions
void GlobalContext::UpdateSurface(D3DSURFACE_DESC &Desc, Bitmap &Bmp, HANDLE Handle) {}
void GlobalContext::Destroy(HANDLE Handle) {}
void GlobalContext::CreateTexture(D3DSURFACE_DESC &Desc, Bitmap &Bmp, HANDLE Handle, IDirect3DTexture9** ppTexture) {}
void GlobalContext::BeginScene() {}