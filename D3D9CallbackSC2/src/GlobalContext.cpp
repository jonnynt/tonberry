#include "Main.h"
//#include "cachemap.h"
#include <stdint.h>
#include <sstream>
#include <unordered_set>
#include <unordered_map>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string/predicate.hpp>
namespace fs = boost::filesystem;

#ifndef ULTRA_FAST
bool g_ReportingEvents = false;
#endif

typedef unsigned long long uint64;
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
T ToNumber(const std::string& Str)	//convert string to unsigned long long -> uint64
{
	T Number;
	std::stringstream S(Str);
	S >> Number;
	return Number;
}

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
fs::path COLLISIONS_CSV(TONBERRY_DIR / "collisions.csv");
fs::path HASHMAP2_CSV(TONBERRY_DIR / "hash2map.csv");
fs::path OBJECTS_CSV(TONBERRY_DIR / "objmap.csv");
// coordinates based on variance and frequently-colliding pixels 
// used with FNV-1a Hash Algorithm


/**********************************
*
*	Global Variables
*
**********************************/

const int VRAM_DIM = 256;

int texture_count = 0;													// keep track of the number of textures processed

//
// FIELD NAME SET AND HASHMAPS
//
typedef unordered_set<string> fieldset_t;								// holds field names
typedef fieldset_t::const_iterator fieldset_iter;						// points to field names

fieldset_t fieldset;

struct fieldset_iter_hasher
{
	size_t operator()(fieldset_iter iter) const {						// since fieldset entries are unique, use field to hash fieldset_iter
		return std::hash<string>()(*iter);
	}
};

struct fieldset_iter_compare
{
	bool operator()(const fieldset_iter& lhs, const fieldset_iter& rhs)
	{
		return *lhs == *rhs;											// since fieldset entries are unique, use fields to compare fieldset_iters
	}
};

typedef unordered_set<fieldset_iter, fieldset_iter_hasher> fieldset_iter_set_t;	// holds sets of fieldset_iter
typedef fieldset_iter_set_t::iterator fieldset_iter_set_iter;					// iterates sets of fieldset_iter

typedef unordered_map<uint64, fieldset_iter_set_t> fieldmap_t;			// maps hashes to a set of matching field names; unordered for O(1) access/insertion, set for collisions
typedef fieldmap_t::iterator fieldmap_iter;

fieldmap_t fieldmap;													// maps original texture hash to replacement texture name

//
// HANDLE CACHES
//
typedef pair<uint64, HANDLE>						nhcache_item_t;			// associates hashes with newhandles
typedef list<nhcache_item_t>						nhcache_list_t;			// holds hashes and their associated newhandle in least-recently-accessed order
typedef nhcache_list_t::iterator					nhcache_list_iter;
typedef unordered_map<uint64, nhcache_list_iter>	nhcache_map_t;			// maps hashes to an entry in the newhandle list
typedef nhcache_map_t::iterator						nhcache_map_iter;
typedef unordered_map<HANDLE, nhcache_map_iter>		handlecache_t;			// maps handles to an entry in the nhcache map
typedef handlecache_t::iterator						handlecache_iter;

typedef struct nhcache_map_iter_hasher
{
	size_t operator()(nhcache_map_iter iter) const{
		return iter->first;													// use nhcache entry hash to index reverse_handlecache
	}
};

typedef unordered_map<nhcache_map_iter, HANDLE, nhcache_map_iter_hasher> reverse_handlecache_t;	// reverse indexing of handlecache; needed for when values in handlecache are removed from the nhcache
typedef reverse_handlecache_t::iterator				reverse_handlecache_iter;

// together these make nhcache:
nhcache_list_t nh_list;
nhcache_map_t nh_map;

// handlecache:
handlecache_t handlecache;
reverse_handlecache_t reverse_handlecache;

//
// FNV HASH CONSTANTS
//
const uint64 FNV_HASH_LEN		= 64;						// length of FNV hash in bits
const uint64 FNV_MODULO			= 1 << FNV_HASH_LEN;		// implicit: since uint64 is 64-bits, overflow is equivalent to modulo
const uint64 FNV_OFFSET_BASIS	= 14695981039346656037;		// starting value of FNV hash
const uint64 FNV_OFFSET_PRIME	= 1099511628211;

const int FNV_COORDS_LEN = 121;
const coord FNV_COORDS[FNV_COORDS_LEN] = { coord{ 11, 9 }, coord{ 22, 7 }, coord{ 28, 7 }, coord{ 39, 9 }, coord{ 53, 9 }, coord{ 60, 7 }, coord{ 76, 11 }, coord{ 88, 8 }, coord{ 91, 11 }, coord{ 102, 7 }, coord{ 115, 9 }, coord{ 11, 15 }, coord{ 22, 19 }, coord{ 28, 20 }, coord{ 40, 22 }, coord{ 54, 16 }, coord{ 60, 17 }, coord{ 76, 17 }, coord{ 87, 20 }, coord{ 91, 20 }, coord{ 107, 20 }, coord{ 115, 13 }, coord{ 11, 24 }, coord{ 22, 29 }, coord{ 28, 30 }, coord{ 39, 29 }, coord{ 46, 30 }, coord{ 60, 30 }, coord{ 70, 30 }, coord{ 87, 25 }, coord{ 93, 25 }, coord{ 102, 27 }, coord{ 115, 33 }, coord{ 11, 38 }, coord{ 22, 39 }, coord{ 28, 44 }, coord{ 39, 41 }, coord{ 55, 35 }, coord{ 60, 38 }, coord{ 70, 40 }, coord{ 87, 37 }, coord{ 99, 44 }, coord{ 102, 43 }, coord{ 115, 35 }, coord{ 11, 50 }, coord{ 21, 46 }, coord{ 25, 51 }, coord{ 40, 48 }, coord{ 46, 46 }, coord{ 60, 46 }, coord{ 76, 47 }, coord{ 87, 47 }, coord{ 93, 49 }, coord{ 107, 53 }, coord{ 115, 49 }, coord{ 11, 61 }, coord{ 22, 59 }, coord{ 25, 59 }, coord{ 40, 59 }, coord{ 55, 57 }, coord{ 60, 61 }, coord{ 70, 58 }, coord{ 87, 59 }, coord{ 99, 58 }, coord{ 102, 59 }, coord{ 115, 57 }, coord{ 7, 77 }, coord{ 21, 77 }, coord{ 24, 71 }, coord{ 36, 77 }, coord{ 46, 76 }, coord{ 60, 77 }, coord{ 70, 70 }, coord{ 87, 71 }, coord{ 93, 77 }, coord{ 102, 69 }, coord{ 115, 77 }, coord{ 11, 84 }, coord{ 21, 80 }, coord{ 27, 85 }, coord{ 39, 79 }, coord{ 55, 85 }, coord{ 60, 81 }, coord{ 75, 86 }, coord{ 82, 84 }, coord{ 93, 84 }, coord{ 107, 79 }, coord{ 115, 79 }, coord{ 11, 92 }, coord{ 22, 97 }, coord{ 28, 90 }, coord{ 40, 93 }, coord{ 46, 92 }, coord{ 60, 91 }, coord{ 76, 97 }, coord{ 82, 98 }, coord{ 93, 90 }, coord{ 102, 99 }, coord{ 115, 99 }, coord{ 6, 107 }, coord{ 22, 101 }, coord{ 31, 102 }, coord{ 41, 108 }, coord{ 55, 107 }, coord{ 60, 107 }, coord{ 70, 104 }, coord{ 87, 101 }, coord{ 93, 105 }, coord{ 102, 101 }, coord{ 115, 109 }, coord{ 11, 112 }, coord{ 21, 112 }, coord{ 27, 113 }, coord{ 41, 112 }, coord{ 55, 113 }, coord{ 60, 120 }, coord{ 70, 116 }, coord{ 82, 112 }, coord{ 93, 113 }, coord{ 107, 117 }, coord{ 115, 113 } };

uint64 uint64pow(uint64 base, int exp)						// calculate base^exp modulo UINT64_MAX
{
	uint64 result = 1;
	for (int i = 0; i < exp; i++)
		result *= base;

	return result;
}

uint64 FNV_NOLOWER_FACTOR		= uint64pow(FNV_OFFSET_PRIME, FNV_COORDS_LEN);			// multiplying an upper 128x128 object hash by this factor is equivalent to hashing a 128x256 object pair where the lower object is all (0,0,0)
uint64 FNV_NOLOWER_RGB_FACTOR	= FNV_NOLOWER_FACTOR * uint64pow(FNV_OFFSET_PRIME, 3);	// same as above except includes RGB averages as well
uint64 FNV_NOUPPER_BASIS		= FNV_OFFSET_BASIS * FNV_NOLOWER_FACTOR;				// starting a lower 128x128 object hash with this basis is equivalent to hashing a 128x256 object pair where the upper object is all (0,0,0)
uint64 FNV_NOUPPER_RGB_BASIS	= FNV_OFFSET_BASIS * FNV_NOLOWER_RGB_FACTOR;			// same as above except includes RGB averages as well

//
// USER PREFERENCES
//
float RESIZE_FACTOR = 4.0;		// texture upscale factor
bool DEBUG = false;				// write debug information
unsigned CACHE_SIZE = 100;		// cache size in megabytes

//TextureCache
//unsigned cache_size = 1000;
//TextureCache texcache(cache_size);


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
							ofstream err;															//Error reporting
							err.open(ERROR_LOG.string(), ofstream::out | ofstream::app);
							err << "Error: bad hashmap. Format is \"<field_name>,<hash_combined>{,<hash_upper>,<hash_lower>}\": " << it->path().string() << endl;
							err.close();
							return;
						}

						// field names are stored only once
						string field = items[0];
						fieldset_iter ptr_to_field_name = fieldset.insert(field).first;

						uint64 hash_combined = ToNumber<uint64>(items[1]);
						fieldmap[hash_combined].insert(ptr_to_field_name);

						if (items.size() > 2) {
							uint64 hash_upper = ToNumber<uint64>(items[2]);
							fieldmap[hash_upper].insert(ptr_to_field_name);

							uint64 hash_lower = ToNumber<uint64>(items[3]);
							fieldmap[hash_lower].insert(ptr_to_field_name);
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
	ofstream debug;
	debug.open("tonberry\\debug\\debug.log", ofstream::out);
	std::time_t time = std::time(nullptr);
	debug << "Initialized " << std::asctime(std::localtime(&time));

	Graphics.Init();
	load_prefs();
	debug << "prefs.txt loaded." << endl;
	load_fieldmaps();
	debug << "hashmap loaded." << endl;

	debug << "fieldmap:" << endl;
	fieldmap_iter map_iter;
	for (map_iter = fieldmap.begin(); map_iter != fieldmap.end(); map_iter++) {
		debug << map_iter->first << ":";
		fieldset_iter_set_iter field_iter;
		for (field_iter = map_iter->second.begin(); field_iter != map_iter->second.end(); field_iter++) {
			debug << " " << **field_iter << ";";
		}
		debug << endl;
	}

	debug.close();
}

// fast 64-bit hash 
uint64 FNV_Hash(BYTE* pData, UINT pitch, int width, int height, const coord* coords, const int len, bool use_RGB = true)
{
	uint64 hash = FNV_OFFSET_BASIS;

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

// hash upper, lower, and combined separately 
uint64 FNV_Hash_Combined(BYTE* pData, UINT pitch, int width, int height, uint64& hash_upper, uint64& hash_lower, const coord* coords, const int len, bool use_RGB = true)
{
	hash_lower = (height > VRAM_DIM / 2) ? (use_RGB) ? FNV_NOUPPER_RGB_BASIS : FNV_NOUPPER_BASIS : 0;

	uint64 hash = FNV_OFFSET_BASIS;

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

	if (hash_lower) {																// make sure texture is big enough to hash lower
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

bool get_fields(const uint64& hash_combined, const uint64& hash_upper, const uint64& hash_lower, fieldset_iter& field_combined, fieldset_iter& field_upper, fieldset_iter& field_lower)
{
	fieldmap_iter iter, iter_upper, iter_lower;
	int upper_matches = 0, lower_matches = 0;
	fieldset_iter_set_t intersection;

	// search for hash_combined
	if ((iter = fieldmap.find(hash_combined)) != fieldmap.end()) {
		field_combined = *(iter->second.begin());							// one field matches whole texture: use this one
		return true;
	}

	// search for hash_upper
	if ((iter_upper = fieldmap.find(hash_upper)) != fieldmap.end())
		upper_matches = iter_upper->second.size();

	// if texture was large enough, search for hash_lower as well
	if (hash_lower > 0 && ((iter_lower = fieldmap.find(hash_lower)) != fieldmap.end())) {
			lower_matches = iter_lower->second.size();
	}

	if (upper_matches & lower_matches)										// both upper and lower have at least 1 match
		set_intersection(iter_upper->second.begin(), iter_upper->second.end(), iter_lower->second.begin(), iter_lower->second.end(), inserter(intersection, intersection.end()), fieldset_iter_compare());

	if (intersection.size() > 0) {											// both upper and lower share at least 1 match
		field_upper = field_lower = *(intersection.begin());				// so set both upper and lower to the first one (based on hash)
		return true;
	}
	
	bool match_found = false;
	if (upper_matches) {													// upper has at least 1 match
		field_upper = *(iter_upper->second.begin());						// so set upper to the first one (based on hash)
		match_found = true;
	}

	if (lower_matches) {													// lower has at least 1 match
		field_lower = *(iter_lower->second.begin());						// so set lower to the first one (based on hash)
		match_found = true;
	}

	return match_found;
}


//Then the unlockrect
void GlobalContext::UnlockRect(D3DSURFACE_DESC &Desc, Bitmap &BmpUseless, HANDLE Handle) //note BmpUseless
{
	IDirect3DTexture9* pTexture = (IDirect3DTexture9*)Handle;

	String debugtype = String("");

	ofstream debug;
	debug.open(DEBUG_LOG.string(), ofstream::out | ofstream::app);

	bool cache_update = false;																			// if false, then Handle will need to be erased from handlecache
	if (pTexture && Desc.Width < 640 && Desc.Height < 480 && Desc.Format == D3DFORMAT::D3DFMT_A8R8G8B8 && Desc.Pool == D3DPOOL::D3DPOOL_MANAGED)    //640x480 are video
	{
		D3DLOCKED_RECT Rect;
		pTexture->LockRect(0, &Rect, NULL, 0);
		UINT pitch = (UINT)Rect.Pitch;
		BYTE* pData = (BYTE*)Rect.pBits;

		// get field matches using FNV hash
		uint64 hash_combined = 0, hash_upper = 0, hash_lower = 0;

		// get hashes
		hash_combined = FNV_Hash_Combined(pData, pitch, Desc.Width, Desc.Height, hash_upper, hash_lower, FNV_COORDS, FNV_COORDS_LEN, true);

		// look for matching fields
		fieldset_iter field_combined = fieldset.end();
		fieldset_iter field_upper = fieldset.end();
		fieldset_iter field_lower = fieldset.end();

		bool field_found = get_fields(hash_combined, hash_upper, hash_lower, field_combined, field_upper, field_lower);

		if (field_found) {
			uint64 hash_used;
			nhcache_map_iter ptr_to_replacement;
			string field;

			// figure out the game plan
			if ((ptr_to_replacement = nh_map.find(hash_combined)) != nh_map.end()) {					// there is an existing newhandle for hash_combined; use it!
				hash_used = hash_combined;
			} else if (field_combined != fieldset.end()) {												// there is a matching field for hash_combined; create it!
				field = *field_combined;
				hash_used = hash_combined;
			} else if ((ptr_to_replacement = nh_map.find(hash_upper)) != nh_map.end()) {				// there is an existing newhandle for hash_upper; use it!
				hash_used = hash_upper;
			} else if (field_upper != fieldset.end()) {													// there is a matching field for hash_upper; create it!
				field = *field_upper;
				hash_used = hash_upper;
			} else if ((ptr_to_replacement = nh_map.find(hash_lower)) != nh_map.end()) {				// there is an existing newhandle for hash_lower; use it!
				hash_used = hash_lower;
			} else if (field_lower != fieldset.end()) {													// there is a matching field for hash_lower; create it!
				field = *field_lower;
				hash_used = hash_lower;
			}
			
			if (ptr_to_replacement != nh_map.end()) {													// use an existing newhandle
				/* UPDATE NH CACHE ACCESS ORDER */
				nhcache_list_iter ptr_to_nh_list = ptr_to_replacement->second;

				debug << "\tFOUND: (" << ptr_to_nh_list->first << ", " << ptr_to_nh_list->second << ") ";

				// move (most-recently-accessed) list item to front of nh_list
				nh_list.erase(ptr_to_nh_list);
				debug << "Moving (" << ptr_to_nh_list->first << ", " << ptr_to_nh_list->second << ") to front of list: ";
				nh_list.push_front(nhcache_item_t(ptr_to_nh_list->first, ptr_to_nh_list->second));

				/* END UPDATE NH CACHE */

				// erase now-invalidate handles from handlecache
				reverse_handlecache_iter checkpointer = reverse_handlecache.find(ptr_to_replacement);
				if (checkpointer != reverse_handlecache.end()) {										// when we moved the nh_list_item, we invalidated an entry in handlecache
					handlecache.erase(checkpointer->second);
					reverse_handlecache.erase(checkpointer);
				}

				cache_update = true;
			} else {																					// create a new newhandle
				string filename = "textures\\" + field.substr(0, 2) + "\\" + field.substr(0, field.rfind("_")) + "\\" + field + ".png";

				ifstream ifile(filename);
				if (ifile.fail()) {
					debug << "Failed to load " << filename << endl;
					debugtype = String("noreplace"); //No file, allow normal SetTexture
				} else {																			// Load texture into cache
					LPDIRECT3DDEVICE9 Device = g_Context->Graphics.Device();
					IDirect3DTexture9* newtexture;
					Bitmap Bmp;
					Bmp.LoadPNG(String(filename.c_str()));
					DWORD Usage = D3DUSAGE_AUTOGENMIPMAP;
					D3DPOOL Pool = D3DPOOL_MANAGED;
					D3DFORMAT Format = D3DFMT_A8R8G8B8;
					Device->CreateTexture(int(RESIZE_FACTOR*(float)Desc.Width), int(RESIZE_FACTOR*(float)Desc.Height), 0, Usage, Format, Pool, &newtexture, NULL);
					D3DLOCKED_RECT newRect;
					newtexture->LockRect(0, &newRect, NULL, 0);
					BYTE* newData = (BYTE *)newRect.pBits;
					for (UINT y = 0; y < Bmp.Height(); y++) {
						RGBColor* CurRow = (RGBColor *)(newData + y * newRect.Pitch);
						for (UINT x = 0; x < Bmp.Width(); x++)									// works for textures of any size (e.g. 4-bit indexed)
						{
							RGBColor Color = Bmp[Bmp.Height() - y - 1][x];						// must flip image
							CurRow[x] = RGBColor(Color.b, Color.g, Color.r, Color.a);
						}
					}
					newtexture->UnlockRect(0); //Texture loaded
					HANDLE tempnewhandle = (HANDLE)newtexture;

					debug << "\tCREATED: (" << hash_used << ", " << tempnewhandle << ") ";

					// we know hash is not in the nhcache, so add hash->tempnewhandle to nhcache
					nh_list.push_front(nhcache_item_t(hash_used, tempnewhandle));
					cache_update = true;

					/* MAKE SURE NHCACHE IS THE CORRECT SIZE */
					while (nh_list.size() > CACHE_SIZE) {										// "while" for completeness but this should only ever loop once
						// get pointer to last (least recent) list item
						nhcache_list_iter last_elem = nh_list.end();
						--last_elem;

						debug << "Removing (" << last_elem->first << ", " << last_elem->second << ") from back of list: ";

						// dispose of texture
						((IDirect3DTexture9*)last_elem->second)->Release();
						last_elem->second = NULL;

						// if we're going to delete an nh_map entry, we need to remove it from handlecache as well
						nhcache_map_iter to_delete = nh_map.find(last_elem->first);
						reverse_handlecache_iter backpointer = reverse_handlecache.find(to_delete);

						if (backpointer != reverse_handlecache.end()) {							// is it even possible for handlecache to not contain to_delete?
							handlecache.erase(backpointer->second);								// remove from both maps
							reverse_handlecache.erase(backpointer);
						}

						// remove from map (this is why the nh_list stores pair<hash, handle>)
						nh_map.erase(to_delete);
						nhcache_map_iter temp_iter = nh_map.find(to_delete->first);
						if (temp_iter == nh_map.end())
							debug << "nh_map[" << last_elem->first << "] = NULL" << endl;
						else
							debug << "!!! nh_map[" << last_elem->first << "] = (" << nh_map[last_elem->first]->first << ", " << nh_map[last_elem->first]->second << ")" << endl;
						// pop from list
						nh_list.pop_back();

						debug << "nh_list.size() = " << nh_list.size() << endl;
					}
					/* END MAKE SURE NHCACHE IS THE CORRECT SIZE */
				}
			}

			if (cache_update) {

				debug << "Adding (" << hash_used << ", " << nh_list.begin()->second << ") to front of list: ";

				// update nh_map with new list item pointer
				std::pair<nhcache_map_iter, bool> insertion = nh_map.insert(std::pair<uint64_t, nhcache_list_iter>(hash_used, nh_list.begin()));    // returns iterator to nh_map[hash_used] and boolean success
				if (!insertion.second)															// if nh_map already contained hash, 
					insertion.first->second = nh_list.begin();									// change nh_map[hash] to nh_list.begin()

				debug << "nh_map[" << hash_used << "] = (" << nh_map[hash_used]->first << ", " << nh_map[hash_used]->second << ")" << endl;

				handlecache[Handle] = insertion.first;											// insert in both maps
				reverse_handlecache[insertion.first] = Handle;
			}

			debug << endl << endl;
		} else {
			//debug << "NO MATCH";
		}
		pTexture->UnlockRect(0); //Finished reading pTextures bits
	} else { //Video textures/improper format
		//debug << "IMPROPER FORMAT";
	}

	if (!cache_update) {
		handlecache_iter iter;
		if (handlecache.size() > 0 && ((iter = handlecache.find(Handle)) != handlecache.end())) {
			reverse_handlecache.erase(iter->second);            // Handle has been recycled, so any old cache entries should be removed from both maps
			handlecache.erase(iter);
		}
	}

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
		if (SurfaceHandles[j]) {
			handlecache_iter ptr_to_replacement;
			if (handlecache.size() > 0 && (ptr_to_replacement = handlecache.find(SurfaceHandles[j])) != handlecache.end())   //SurfaceHandles[j] found on handlecache
			{
				IDirect3DTexture9* newtexture = NULL;
				newtexture = (IDirect3DTexture9*)ptr_to_replacement->second->second->second;	//ptr_to_replacement->second =                 pointer to nh_map
																								//ptr_to_replacement->second->second =         pointer to nh_list
																								//ptr_to_replacement->second->second->second = pointer to replacement HANDLE
				if (newtexture) {
					g_Context->Graphics.Device()->SetTexture(Stage, newtexture);
					//((IDirect3DTexture9*)SurfaceHandles[j])->Release();
					return true;
				}   //Texture replaced!
			}
		}
	}
	return false;
}
//Unused functions
void GlobalContext::UpdateSurface(D3DSURFACE_DESC &Desc, Bitmap &Bmp, HANDLE Handle) {}
void GlobalContext::Destroy(HANDLE Handle) {}
void GlobalContext::CreateTexture(D3DSURFACE_DESC &Desc, Bitmap &Bmp, HANDLE Handle, IDirect3DTexture9** ppTexture) {}
void GlobalContext::BeginScene() {}