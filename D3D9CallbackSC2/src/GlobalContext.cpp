#include "Main.h"
#include "cachemap.h"
#include "md5.h"
#include <stdint.h>
#include <sstream>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string/predicate.hpp>
namespace fs = boost::filesystem;

#ifndef ULTRA_FAST
bool g_ReportingEvents = false;
#endif

typedef unsigned char uchar;

GlobalContext *g_Context;

// simple structure for storing pixel coordinates (x,y)
typedef struct coord
{
	int x;
	int y;

	bool operator<(const coord& rhs) const
	{
		return (rhs.y > y) || (rhs.y == y && rhs.x > x);
	}
} coord;

template <typename T>
T ToNumber(const std::string& Str)	//convert string to uint64_t
{
	T Number;
	std::stringstream S(Str);
	S >> Number;
	return Number;
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
unordered_set<uint64_t> nomatch_set;



//
// PATHS
//
fs::path TONBERRY_DIR("tonberry");
fs::path TEXTURES_DIR("textures");
fs::path DEBUG_DIR(TONBERRY_DIR / "debug");
fs::path HASHMAP_DIR(TONBERRY_DIR / "hashmap");
fs::path PREFS_TXT(TONBERRY_DIR / "prefs.txt");
fs::path ERROR_LOG(TONBERRY_DIR / "error.log");
fs::path DEBUG_LOG(DEBUG_DIR / "debug.log");
fs::path NOMATCH_LOG(DEBUG_DIR / "nomatch.log");
fs::path COLLISIONS_CSV(TONBERRY_DIR / "collisions.csv");
fs::path HASHMAP2_CSV(TONBERRY_DIR / "hash2map.csv");
fs::path OBJECTS_CSV(TONBERRY_DIR / "objmap.csv");



//
// FNV HASH CONSTANTS
//

const uint64_t FNV_HASH_LEN		= 64;						// length of FNV hash in bits
const uint64_t FNV_MODULO			= 1 << FNV_HASH_LEN;	// implicit: since uint64_t is 64-bits, overflow is equivalent to modulo
const uint64_t FNV_OFFSET_BASIS	= 14695981039346656037;		// starting value of FNV hash
const uint64_t FNV_OFFSET_PRIME	= 1099511628211;

// coordinates based on variance and frequently-colliding pixels 
// used with FNV-1a Hash Algorithm
const int FNV_COORDS_LEN = 121;
const coord FNV_COORDS[FNV_COORDS_LEN] = { coord{ 11, 9 }, coord{ 22, 7 }, coord{ 28, 7 }, coord{ 39, 9 }, coord{ 53, 9 }, coord{ 60, 7 }, coord{ 76, 11 }, coord{ 88, 8 }, coord{ 91, 11 }, coord{ 102, 7 }, coord{ 115, 9 }, coord{ 11, 15 }, coord{ 22, 19 }, coord{ 28, 20 }, coord{ 40, 22 }, coord{ 54, 16 }, coord{ 60, 17 }, coord{ 76, 17 }, coord{ 87, 20 }, coord{ 91, 20 }, coord{ 107, 20 }, coord{ 115, 13 }, coord{ 11, 24 }, coord{ 22, 29 }, coord{ 28, 30 }, coord{ 39, 29 }, coord{ 46, 30 }, coord{ 60, 30 }, coord{ 70, 30 }, coord{ 87, 25 }, coord{ 93, 25 }, coord{ 102, 27 }, coord{ 115, 33 }, coord{ 11, 38 }, coord{ 22, 39 }, coord{ 28, 44 }, coord{ 39, 41 }, coord{ 55, 35 }, coord{ 60, 38 }, coord{ 70, 40 }, coord{ 87, 37 }, coord{ 99, 44 }, coord{ 102, 43 }, coord{ 115, 35 }, coord{ 11, 50 }, coord{ 21, 46 }, coord{ 25, 51 }, coord{ 40, 48 }, coord{ 46, 46 }, coord{ 60, 46 }, coord{ 76, 47 }, coord{ 87, 47 }, coord{ 93, 49 }, coord{ 107, 53 }, coord{ 115, 49 }, coord{ 11, 61 }, coord{ 22, 59 }, coord{ 25, 59 }, coord{ 40, 59 }, coord{ 55, 57 }, coord{ 60, 61 }, coord{ 70, 58 }, coord{ 87, 59 }, coord{ 99, 58 }, coord{ 102, 59 }, coord{ 115, 57 }, coord{ 7, 77 }, coord{ 21, 77 }, coord{ 24, 71 }, coord{ 36, 77 }, coord{ 46, 76 }, coord{ 60, 77 }, coord{ 70, 70 }, coord{ 87, 71 }, coord{ 93, 77 }, coord{ 102, 69 }, coord{ 115, 77 }, coord{ 11, 84 }, coord{ 21, 80 }, coord{ 27, 85 }, coord{ 39, 79 }, coord{ 55, 85 }, coord{ 60, 81 }, coord{ 75, 86 }, coord{ 82, 84 }, coord{ 93, 84 }, coord{ 107, 79 }, coord{ 115, 79 }, coord{ 11, 92 }, coord{ 22, 97 }, coord{ 28, 90 }, coord{ 40, 93 }, coord{ 46, 92 }, coord{ 60, 91 }, coord{ 76, 97 }, coord{ 82, 98 }, coord{ 93, 90 }, coord{ 102, 99 }, coord{ 115, 99 }, coord{ 6, 107 }, coord{ 22, 101 }, coord{ 31, 102 }, coord{ 41, 108 }, coord{ 55, 107 }, coord{ 60, 107 }, coord{ 70, 104 }, coord{ 87, 101 }, coord{ 93, 105 }, coord{ 102, 101 }, coord{ 115, 109 }, coord{ 11, 112 }, coord{ 21, 112 }, coord{ 27, 113 }, coord{ 41, 112 }, coord{ 55, 113 }, coord{ 60, 120 }, coord{ 70, 116 }, coord{ 82, 112 }, coord{ 93, 113 }, coord{ 107, 117 }, coord{ 115, 113 } };

uint64_t uint64_tpow(uint64_t base, int exp)				// calculate base^exp modulo uint64_t_MAX
{
	uint64_t result = 1;
	for (int i = 0; i < exp; i++)
		result *= base;

	return result;
}

uint64_t FNV_NOLOWER_FACTOR		= uint64_tpow(FNV_OFFSET_PRIME, FNV_COORDS_LEN);			// multiplying an upper 128x128 object hash by this factor is equivalent to hashing a 128x256 object pair where the lower object is all (0,0,0)
uint64_t FNV_NOLOWER_RGB_FACTOR	= FNV_NOLOWER_FACTOR * uint64_tpow(FNV_OFFSET_PRIME, 3);	// same as above except includes RGB averages as well
uint64_t FNV_NOUPPER_BASIS		= FNV_OFFSET_BASIS * FNV_NOLOWER_FACTOR;					// starting a lower 128x128 object hash with this basis is equivalent to hashing a 128x256 object pair where the upper object is all (0,0,0)
uint64_t FNV_NOUPPER_RGB_BASIS	= FNV_OFFSET_BASIS * FNV_NOLOWER_RGB_FACTOR;				// same as above except includes RGB averages as well

//
// USER PREFERENCES
//

float RESIZE_FACTOR = 4.0;		// texture upscale factor
bool DEBUG = false;				// write debug information
unsigned CACHE_SIZE = 100;		// cache size in megabytes


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
				DEBUG = (boost::iequals(param, "yes"));		// ignore case
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
					while (getline(hashfile, line))											// Omzy's original code
					{
						// split line on ','
						std::deque<string> items;
						std::stringstream sstream(line);
						std::string item;
						while (std::getline(sstream, item, ',')) {
							items.push_back(item);
						}

						// format is "<field_name>,<hash_combined>{,<hash_upper>,<hash_lower>}"
						if (items.size() != 2 && items.size() != 4) {
							ofstream err;													//Error reporting
							err.open(ERROR_LOG.string(), ofstream::out | ofstream::app);
							err << "Error: bad hashmap. Format is \"<field_name>,<hash_combined>{,<hash_upper>,<hash_lower>}\": " << it->path().string() << endl;
							err.close();
							return;
						}

						// field names are stored only once
						string field = items[0];

						uint64_t hash_combined = ToNumber<uint64_t>(items[1]);
						fieldmap->insert(hash_combined, field);

						if (items.size() > 2) {
							uint64_t hash_upper = ToNumber<uint64_t>(items[2]);
							fieldmap->insert(hash_upper, field);

							uint64_t hash_lower = ToNumber<uint64_t>(items[3]);
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

	cache = new TextureCache(CACHE_SIZE);
	fieldmap = new FieldMap();

	load_fieldmaps();
	debug << "hashmap loaded." << endl << endl;

	debug << "fieldmap:" << endl;
	fieldmap->writeMap(debug);

	debug << endl;
	debug.close();
}

// fast 64-bit hash 
uint64_t FNV_Hash(BYTE* pData, UINT pitch, int width, int height, const coord* coords, const int len, bool use_RGB = true)
{
	uint64_t hash = FNV_OFFSET_BASIS;

	float red = 0, green = 0, blue = 0;
	size_t coord_count = 0;
	for (int i = 0; i < len; i++) {
		coord point = coords[i];
		unsigned char val = 0;
		if (point.x < width && point.y < height) { //respect texture sizes
			
			RGBColor* CurRow = (RGBColor*)(pData + (point.y) * pitch);
			RGBColor Color = CurRow[point.x];
			val = round((Color.r + Color.g + Color.b) / 3);

			// keep track of RGB sums of pixels
			if (use_RGB) {
				red		+= Color.r;
				green	+= Color.g;;
				blue	+= Color.b;
			}
		}

		hash ^= val;
		hash *= FNV_OFFSET_PRIME;
		coord_count++;
	}

	// factor RGB averages into hash
	if (use_RGB) {
		red /= coord_count;
		green /= coord_count;
		blue /= coord_count;

		hash ^= (uchar)round(red);
		hash *= FNV_OFFSET_PRIME;
		hash ^= (uchar)round(green);
		hash *= FNV_OFFSET_PRIME;
		hash ^= (uchar)round(blue);
		hash *= FNV_OFFSET_PRIME;
	}

	return hash;
}

uint64_t FNV_Hash_Full(BYTE* pData, UINT pitch, int width, int height)
{
	uint64_t hash = FNV_OFFSET_BASIS;

	float red = 0, green = 0, blue = 0;
	size_t coord_count = 0;
	for (int y = 0; y < height; y++) {
		RGBColor* CurRow = (RGBColor*)(pData + y * pitch);
		for (int x = 0; x < width; x++) {
			RGBColor Color = CurRow[x];
			hash ^= Color.r;
			hash *= FNV_OFFSET_PRIME;
			hash ^= Color.g;
			hash *= FNV_OFFSET_PRIME;
			hash ^= Color.b;
			hash *= FNV_OFFSET_PRIME;
		}
	}

	return hash;
}

// hash upper, lower, and combined separately 
uint64_t FNV_Hash_Combined(BYTE* pData, UINT pitch, int width, int height, uint64_t& hash_upper, uint64_t& hash_lower, const coord* coords, const int len, bool use_RGB = true)
{
	hash_lower = (height > VRAM_DIM / 2) ? (use_RGB) ? FNV_NOUPPER_RGB_BASIS : FNV_NOUPPER_BASIS : 0;

	uint64_t hash = FNV_OFFSET_BASIS;

	float red = 0, green = 0, blue = 0;
	size_t coord_count = 0;
	for (int i = 0; i < len; i++) {
		coord point = coords[i];
		unsigned char val = 0;
		if (point.x < width && point.y < height) { //respect texture sizes

			RGBColor* CurRow = (RGBColor*)(pData + (point.y) * pitch);
			RGBColor Color = CurRow[point.x];
			val = round((Color.r + Color.g + Color.b) / 3);

			// keep track of RGB sums of pixels
			if (use_RGB) {
				red += Color.r;
				green += Color.g;;
				blue += Color.b;
			}
		}

		hash ^= val;
		hash *= FNV_OFFSET_PRIME;
		coord_count++;
	}

	// factor RGB averages into hash
	if (use_RGB) {
		red /= coord_count;
		green /= coord_count;
		blue /= coord_count;

		hash ^= (uchar)round(red);
		hash *= FNV_OFFSET_PRIME;
		hash ^= (uchar)round(green);
		hash *= FNV_OFFSET_PRIME;
		hash ^= (uchar)round(blue);
		hash *= FNV_OFFSET_PRIME;
	}

	// set hash_upper equal to hash thus far
	hash_upper = hash;

	if (hash_lower) {														// make sure texture is big enough to hash lower
		// adjust hash_upper to include lower blank object
		hash_upper *= (use_RGB) ? FNV_NOLOWER_RGB_FACTOR : FNV_NOLOWER_FACTOR;

		// adjust img for lower hashing
		int half_dim = VRAM_DIM / 2;
		int last_obj_start = height - half_dim;
		pData += min(last_obj_start, half_dim) * pitch;						// point pData at last place where a full 128x128 object could be hashed but limit it to object directly under upper
		height = max(last_obj_start, half_dim);

		// hash lower and continue hashing combined
		red = 0, green = 0, blue = 0;
		coord_count = 0;
		for (int i = 0; i < len; i++) {
			coord point = coords[i];
			unsigned char val = 0;
			if (point.x < width && point.y < height) { //respect texture sizes

				RGBColor* CurRow = (RGBColor*)(pData + (point.y) * pitch);
				RGBColor Color = CurRow[point.x];
				val = round((Color.r + Color.g + Color.b) / 3);

				// keep track of RGB sums of pixels
				if (use_RGB) {
					red += Color.r;
					green += Color.g;;
					blue += Color.b;
				}
			}

			hash_lower ^= val;
			hash_lower *= FNV_OFFSET_PRIME;

			hash ^= val;
			hash *= FNV_OFFSET_PRIME;

			coord_count++;
		}

		// factor RGB averages into hash
		if (use_RGB) {
			red /= coord_count;
			green /= coord_count;
			blue /= coord_count;

			hash_lower ^= (uchar)round(red);
			hash_lower *= FNV_OFFSET_PRIME;
			hash_lower ^= (uchar)round(green);
			hash_lower *= FNV_OFFSET_PRIME;
			hash_lower ^= (uchar)round(blue);
			hash_lower *= FNV_OFFSET_PRIME;

			hash ^= (uchar)round(red);
			hash *= FNV_OFFSET_PRIME;
			hash ^= (uchar)round(green);
			hash *= FNV_OFFSET_PRIME;
			hash ^= (uchar)round(blue);
			hash *= FNV_OFFSET_PRIME;
		}
	}

	return hash;
}

bool get_fields(const uint64_t& hash_combined, const uint64_t& hash_upper, const uint64_t& hash_lower, string& field_combined, string& field_upper, string& field_lower)
{
	unordered_set<string> fields;
	int upper_matches = 0, lower_matches = 0;

	// search for hash_combined
	if (fieldmap->get_fields(hash_combined, fields)) {
		field_combined = *(fields.begin());									// a field matches whole texture: use this one
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
					RGBColor* CurRow = (RGBColor*)(replaced_pData + (replaced_height - 1 - y / 4) * replaced_pitch);
					Color = CurRow[(int)(x / RESIZE_FACTOR)];
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
					RGBColor* CurRow = (RGBColor*)(replaced_pData + (replaced_height - 1 - y / 4) * replaced_pitch);
					Color = CurRow[(int)(x / RESIZE_FACTOR)];
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

string md5(BYTE* pData, UINT width, UINT height, UINT pitch)
{
	MD5 md5;
	UINT hash_width = width;// min(width, VRAM_DIM / 2);
	uchar* buf = new uchar[hash_width * height * 3];
	int i = 0;
	for (int y = 0; y < height; y++) {
		for (int x = 0; x < hash_width; x++) {
			RGBColor* CurRow = (RGBColor*)(pData + y * pitch);
			RGBColor Color = CurRow[x];
			buf[i++] = Color.r;
			buf[i++] = Color.g;
			buf[i++] = Color.b;
		}
	}
	md5.update(buf, 128 * 256);
	md5.finalize();
	std::string hash = md5.hexdigest();
	delete[] buf;
	return hash;
}


//Then the unlockrect
void GlobalContext::UnlockRect(D3DSURFACE_DESC &Desc, Bitmap &BmpUseless, HANDLE Handle) //note BmpUseless
{
	IDirect3DTexture9* pTexture = (IDirect3DTexture9*)Handle;

	String debugtype = String("");

	ofstream debug(DEBUG_LOG.string(), ofstream::out | ofstream::app);

	bool handle_used = false;													// if false, Handle will be erased from the TextureCache
	if (pTexture && Desc.Width < 640 && Desc.Height < 480 && Desc.Format == D3DFORMAT::D3DFMT_A8R8G8B8 && Desc.Pool == D3DPOOL::D3DPOOL_MANAGED)    //640x480 are video
	{
		D3DLOCKED_RECT Rect;
		pTexture->LockRect(0, &Rect, NULL, 0);
		UINT pitch = (UINT)Rect.Pitch;
		BYTE* pData = (BYTE*)Rect.pBits;

		// get field matches using FNV hash
		uint64_t hash_combined = 0, hash_upper = 0, hash_lower = 0;
		string field_combined = "", field_upper = "", field_lower = "";

		// get hashes
		hash_combined = FNV_Hash_Combined(pData, pitch, Desc.Width, Desc.Height, hash_upper, hash_lower, FNV_COORDS, FNV_COORDS_LEN, true);

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
						if (Desc.Width > 0 && Desc.Height > 0) {
							uint64_t hash = FNV_Hash_Full(pData, pitch, Desc.Width, Desc.Height);
							if (nomatch_set.count(hash) == 0) {					// only write the image if it was not previously written
								pTexture->UnlockRect(0);
								ostringstream sstream;
								sstream << (DEBUG_DIR / "nomatch\\").string() << hash << ".bmp";
								if (D3DXSaveTextureToFile(sstream.str().c_str(), D3DXIFF_BMP, pTexture, NULL) == D3D_OK) {
									nomatch_set.insert(hash);
									ofstream nomatch(NOMATCH_LOG.string(), ofstream::out | ofstream::app);
									nomatch << hash << "," << hash_combined << "," << hash_upper << "," << hash_lower << endl;
									nomatch.close();
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