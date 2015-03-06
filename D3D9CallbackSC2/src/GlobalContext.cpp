#include "Main.h"
#include "cachemap.h"
#include <stdint.h>
#include <sstream>
#include <unordered_map>
#include <boost\filesystem.hpp>

//-------------
/*#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>*/
//-------------

#ifndef ULTRA_FAST
bool g_ReportingEvents = false;
#endif

namespace fs = boost::filesystem;

GlobalContext *g_Context;

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

template <typename T>
T ToNumber(const std::string& Str)	//convert string to unsigned long long -> uint64_t
{
	T Number;
	std::stringstream S(Str);
	S >> Number;
	return Number;
}

typedef struct coord
{
	int x;
	int y;

	bool operator<(const coord& rhs) const
	{
		return (rhs.y > y) || (rhs.y == y && rhs.x > x);
	}
} coord;

// coordinates used in Omzy's hash algorithm
coord hash1[64] = { coord{ 0, 0 }, coord{ 16, 0 }, coord{ 32, 0 }, coord{ 48, 0 }, coord{ 64, 0 }, coord{ 80, 0 }, coord{ 96, 0 }, coord{ 112, 0 }, coord{ 0, 16 }, coord{ 16, 16 }, coord{ 32, 16 }, coord{ 48, 16 }, coord{ 64, 16 }, coord{ 80, 16 }, coord{ 96, 16 }, coord{ 112, 16 }, coord{ 0, 32 }, coord{ 16, 32 }, coord{ 32, 32 }, coord{ 48, 32 }, coord{ 64, 32 }, coord{ 80, 32 }, coord{ 96, 32 }, coord{ 112, 32 }, coord{ 0, 48 }, coord{ 16, 48 }, coord{ 32, 48 }, coord{ 48, 48 }, coord{ 64, 48 }, coord{ 80, 48 }, coord{ 96, 48 }, coord{ 112, 48 }, coord{ 0, 64 }, coord{ 16, 64 }, coord{ 0, 80 }, coord{ 16, 80 }, coord{ 48, 80 }, coord{ 96, 80 }, coord{ 0, 96 }, coord{ 16, 96 }, coord{ 0, 112 }, coord{ 16, 112 }, coord{ 0, 128 }, coord{ 16, 128 }, coord{ 48, 128 }, coord{ 96, 128 }, coord{ 0, 144 }, coord{ 16, 144 }, coord{ 0, 160 }, coord{ 16, 160 }, coord{ 0, 176 }, coord{ 16, 176 }, coord{ 48, 176 }, coord{ 96, 176 }, coord{ 0, 192 }, coord{ 16, 192 }, coord{ 0, 208 }, coord{ 16, 208 }, coord{ 0, 224 }, coord{ 16, 224 }, coord{ 48, 224 }, coord{ 96, 224 }, coord{ 0, 240 }, coord{ 16, 240 } };

//Global Variables
int pixval[64];
int pixval2[98];
int objval[64];
map<uint64_t, string> hashmap;	//Hashmap of unique hashvals 
map<uint64_t, string> collmap;	//Hashmap of collision duplicates
map<string, string> coll2map;	//Hashmap of collision hashvals after algorithm 2
map<uint64_t, string> objmap;
map<uint64_t, string>::iterator it;
map<string, string>::iterator it2;
uint64_t hashval; //current hashval of left half of memory
uint64_t objtop; //object in top left corner of memory
uint64_t objbot; //object in bottom left corner of memory
BigInteger hashval2; //hashval for algo 2

//
// CONSTANT VARIABLES
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

//
// USER PREFERENCES
//
float RESIZE_FACTOR = 4.0;		// texture upscale factor
bool DEBUG = false;				// write debug information
unsigned CACHE_SIZE = 100;		// cache size in megabytes

//CACHE added Jay
//hashmap cache
HashCache hashcache(CACHE_SIZE);
HashCache::iterator hash_it;
pair<uint64_t, string> hash_element;

//TextureCache
//unsigned cache_size = 1000;
//TextureCache texcache(cache_size);

void loadPrefs()
{
	RESIZE_FACTOR = 4.0;
	ifstream prefsfile;
	prefsfile.open(PREFS_TXT.string(), ifstream::in);
	if (prefsfile.is_open()) {
		string line;
		while (getline(prefsfile, line)) {
			int eq_index = line.find("=");
			if (eq_index < 0) continue;

			string param = line.substr(0, eq_index);
			string value = line.substr(eq_index + 1);

			if (param == "RESIZE_FACTOR")
				RESIZE_FACTOR = ToNumber<float>(value);
			else if (param == "debug_mode")
				DEBUG = (value == "yes");
			//else if (param == "cache_size")
			//	CACHE_SIZE = ToNumber<unsigned>(value);
		}
		prefsfile.close();
	} else {
		ofstream err;																		//Error reporting
		err.open(ERROR_LOG.string(), ofstream::out | ofstream::app);
		err << "Error: could not open prefs.txt" << endl;
		err.close();
	}
}

// Mod Jay
void loadHashmap()																			// Expects hash1map folder to be in ff8/tonberry directory
{
	if (!fs::exists(HASHMAP_DIR)) {
		ofstream err;																		// Error reporting file
		err.open(ERROR_LOG.string(), ofstream::out | ofstream::app);
		err << "Error: hashmap folder doesn't exist" << endl;
		err.close();
	} else {
		fs::directory_iterator end_it;														// get tonberry/hashmap folder iterator
		for (fs::directory_iterator it(HASHMAP_DIR); it != end_it; it++) {
			if (fs::is_regular_file(it->status()) && it->path().extension() == ".csv") {	// file is .csv
				ifstream hashfile;
				hashfile.open(it->path().string(), ifstream::in);							// open it and dump into the map
				if (hashfile.is_open()) {
					string line;
					while (getline(hashfile, line))											// Omzy's original code
					{
						int comma = line.find(",");
						string field = line.substr(0, comma);
						string valuestr = line.substr(comma + 1);
						uint64_t value = ToNumber<uint64_t>(valuestr);
						hashmap.insert(pair<uint64_t, string>(value, field));				// key, value for unique names, value, key for unique hashvals
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

void loadCollisions()																		//Expects collisions.csv to be in ff8/tonberry directory
{
	ifstream collfile;
	collfile.open(COLLISIONS_CSV.string(), ifstream::in);
	if (collfile.is_open()) {
		string line;
		while (getline(collfile, line))														//~10000 total number of 128x256 texture blocks in ff8
		{
			int comma = line.find(",");
			string field = line.substr(0, comma);
			string valuestr = line.substr(comma + 1);
			uint64_t value = ToNumber<uint64_t>(valuestr);
			collmap.insert(pair<uint64_t, string>(value, field));							//key, value for unique names, value, key for unique hashvals
		}
		collfile.close();
	} else {
		ofstream err;																		//Error reporting
		err.open(ERROR_LOG.string(), ofstream::out | ofstream::app);
		err << "Error: could not open collisions.csv" << endl;
		err.close();
	}
}

void loadHashmap2() 																		//Expects hash2map.csv to be in ff8/tonberry directory
{
	ifstream coll2file;
	coll2file.open(HASHMAP2_CSV.string(), ifstream::in);
	string line;
	if (coll2file.is_open()) {
		while (getline(coll2file, line))													//~10000 total number of 128x256 texture blocks in ff8
		{
			int comma = line.find(",");
			string field = line.substr(0, comma);
			string valuestr = line.substr(comma + 1);
			coll2map.insert(pair<string, string>(valuestr, field));							//key, value for unique names, value, key for unique hashvals
		}
		coll2file.close();
	} else {
		ofstream err;																		//Error reporting
		err.open(ERROR_LOG.string(), ofstream::out | ofstream::app);
		err << "Error: could not open hash2map.csv" << endl;
		err.close();
	}
}

// Mod Jay
void loadObjects()																			//Expects objmap.csv to be in ff8/tonberry directory
{
	ifstream objfile;
	objfile.open(OBJECTS_CSV.string(), ifstream::in);
	if (objfile.is_open()) {
		string line;
		while (getline(objfile, line))														//~10000 total number of 128x256 texture blocks in ff8
		{
			int comma = line.find(",");
			string field = line.substr(0, comma);
			string valuestr = line.substr(comma + 1);
			uint64_t value = ToNumber<uint64_t>(valuestr);
			collmap.insert(pair<uint64_t, string>(value, field));							//key, value for unique names, value, key for unique hashvals
		}
		objfile.close();
	} else {
		ofstream err;																		//Error reporting
		err.open(ERROR_LOG.string(), ofstream::out | ofstream::app);
		err << "Error: could not open objmap.csv" << endl;
		err.close();
	}
}

void GlobalContext::Init()
{
	ofstream debug;
	debug.open("tonberry\\debug\\debug.log", ofstream::out | ofstream::app);
	std::time_t time = std::time(nullptr);
	debug << "Initialized " << std::asctime(std::localtime(&time));

	Graphics.Init();
	loadPrefs();
	debug << "prefs.txt loaded." << endl;
	loadHashmap();
	debug << "hashmap loaded." << endl;
	loadCollisions();
	debug << "collisions loaded." << endl;
	loadHashmap2();
	debug << "hash2map loaded." << endl;
	loadObjects();
	debug << "objmap loaded." << endl;

	debug.close();
}

// Omzy's original hash algorithm
void Hash_Algorithm_1(BYTE* pData, UINT pitch, int width, int height)	//hash algorithm that preferences top and left sides
{
	int blocksize = 16;
	UINT x, y;
	int pix = 0;
	int toppix = 0;
	int botpix = 0;
	for (x = 0; x < 8; x++) //pixvals 0->31
	{
		for (y = 0; y < 4; y++) {
			if (x*blocksize < width && y*blocksize < height) //respect texture sizes
			{
				RGBColor* CurRow = (RGBColor*)(pData + (y*blocksize) * pitch);
				RGBColor Color = CurRow[x*blocksize];
				pixval[pix] = (Color.r + Color.g + Color.b) / 3;
				if (y*blocksize < 128) { if (toppix < 32) { objval[toppix] = pixval[pix]; toppix++; } } else if (botpix < 32) { objval[botpix + 32] = pixval[pix]; botpix++; }
			} else { pixval[pix] = 0; } //out of bounds
			pix++;
		}
	}
	for (x = 0; x < 2; x++) //pixvals 32->55
	{
		for (y = 4; y < 16; y++) {
			if (x*blocksize < width && y*blocksize < height) //respect texture sizes
			{
				RGBColor* CurRow = (RGBColor*)(pData + (y*blocksize) * pitch);
				RGBColor Color = CurRow[x*blocksize];
				pixval[pix] = (Color.r + Color.g + Color.b) / 3;
				if (y*blocksize < 128) { if (toppix < 32) { objval[toppix] = pixval[pix]; toppix++; } } else if (botpix < 32) { objval[botpix + 32] = pixval[pix]; botpix++; }
			} else { pixval[pix] = 0; } //out of bounds
			pix++;
		}
	}
	for (x = 3; x < 7; x += 3) //pixvals 56->63, note +=3
	{
		for (y = 5; y < 15; y += 3)	//note +=3
		{
			if (x*blocksize < width && y*blocksize < height) //respect texture sizes
			{
				RGBColor* CurRow = (RGBColor*)(pData + (y*blocksize) * pitch);
				RGBColor Color = CurRow[x*blocksize];
				pixval[pix] = (Color.r + Color.g + Color.b) / 3;
				if (y*blocksize < 128) { if (toppix < 32) { objval[toppix] = pixval[pix]; toppix++; } } else if (botpix < 32) { objval[botpix + 32] = pixval[pix]; botpix++; }
			} else { pixval[pix] = 0; } //out of bounds
			pix++;
		}
	}
}

void Hash_Algorithm_2(BYTE* pData, UINT pitch, int width, int height)	//hash algorithm that chooses unique pixels selected by hand
{
	int pix = 0;
	UINT x0, y0;
	UINT x[76] = { 44, 0, 17, 25, 15, 111, 35, 25, 3, 46, 112, 34, 21, 1, 72, 80, 25, 32, 15, 4, 123, 16, 47, 14, 110, 78, 3, 66, 0, 86, 58, 27, 39, 4, 6, 49, 7, 71, 121, 17, 22, 16, 84, 115, 118, 119, 126, 59, 96, 88, 64, 1, 21, 31, 107, 92, 73, 116, 118, 58, 47, 18, 93, 78, 97, 106, 107, 77, 99, 13, 100, 125, 12, 33, 53, 61 };
	UINT y[76] = { 243, 0, 2, 19, 35, 24, 0, 12, 23, 7, 5, 0, 4, 0, 2, 218, 30, 2, 20, 23, 4, 4, 2, 8, 7, 7, 25, 0, 1, 0, 11, 15, 2, 0, 0, 1, 15, 15, 16, 7, 7, 0, 244, 245, 245, 245, 253, 203, 135, 184, 9, 15, 80, 81, 244, 245, 249, 255, 238, 237, 216, 218, 240, 216, 116, 164, 244, 247, 236, 245, 21, 59, 25, 8, 16, 108 };
	for (int i = 0; i < 76; i++) //pixvals 0->75
	{
		if (x[i] < width && y[i] < height) //respect texture sizes
		{
			RGBColor* CurRow = (RGBColor*)(pData + (y[i]/*blocksize*/) * pitch); //blocksize already included
			RGBColor Color = CurRow[x[i]/*blocksize*/];
			pixval2[pix] = (Color.r + Color.g + Color.b) / 3;
		} else { pixval2[pix] = 0; } //out of bounds
		pix++;
	}

	for (x0 = 0; x0 < 44; x0 += 4) //pixvals 76->97, note +=4
	{
		for (y0 = 7; y0 < 16; y0 += 8) //note +=8
		{
			if (x0 < width && y0 < height) //respect texture sizes
			{
				RGBColor* CurRow = (RGBColor*)(pData + (y0/*blocksize*/)* pitch); //blocksize already included
				RGBColor Color = CurRow[x0/*blocksize*/];
				pixval2[pix] = (Color.r + Color.g + Color.b) / 3;
			} else { pixval2[pix] = 0; } //out of bounds
			pix++;
		}
	}
}

string getsysfld(BYTE* pData, UINT pitch, int width, int height, string sysfld)	//Exception method for sysfld00 and sysfld01
{
	UINT x = 177;
	UINT y = 155;
	RGBColor* CurRow = (RGBColor*)(pData + (y)* pitch);
	RGBColor Color = CurRow[x];
	int sysval = (Color.r + Color.g + Color.b) / 3;
	string syspage = "13";
	switch (sysval) {
	case 43: syspage = "13"; break;
	case 153: syspage = "14"; break;
	case 150: syspage = "15"; break;
	case 101: syspage = "16"; break;
	case 85: syspage = "17"; break;
	case 174: syspage = "18"; break;
	case 170: syspage = "19"; break;
	case 255: syspage = "20"; break;
	default: syspage = "13"; break;
	}
	return sysfld.substr(0, 9) + syspage;
}

string geticonfl(BYTE* pData, UINT pitch, int width, int height, string iconfl)	//Exception method for iconfl00, iconfl01, iconfl02, iconfl03, iconflmaster
{
	UINT x = 0;
	UINT y = 0;
	if (iconfl == "iconfl00_13") { x = 82; y = 150; } else if (iconfl == "iconfl01_13") { x = 175; y = 208; } else if (iconfl == "iconfl02_13") { x = 216; y = 108; } else if (iconfl == "iconfl03_13") { x = 58; y = 76; } else if (iconfl == "iconflmaster_13") { x = 215; y = 103; }

	RGBColor* CurRow = (RGBColor*)(pData + (y)* pitch);
	RGBColor Color = CurRow[x];
	int colR = Color.r;
	int colG = Color.g;
	int colB = Color.b;
	if (colR == 0) { colR++; }
	if (colG == 0) { colG++; }
	if (colB == 0) { colB++; }
	int icval = colR * colG * colB;

	string icpage = "13";
	switch (icval) {
	case 65025: icpage = "13"; break;
	case 605160: icpage = "14"; break;
	case 1191016: icpage = "15"; break;
	case 189: icpage = "16"; break;
	case 473304: icpage = "17"; break;
	case 20992: icpage = "18"; break;
	case 859625: icpage = "19"; break;
	case 551368: icpage = "20"; break;
	case 1393200: icpage = "21"; break;
	case 931500: icpage = "22"; break;
	case 1011240: icpage = "23"; break;
	case 1395640: icpage = "24"; break;
	case 1018024: icpage = "25"; break;
	case 411864: icpage = "26"; break;
	case 80064: icpage = "27"; break;
	case 4410944: icpage = "28"; break;
	default: icpage = "13"; break;
	}
	return iconfl.substr(0, iconfl.size() - 2) + icpage;
}

string getobj(uint64_t & hash) //if previously unmatched, searches through object map for objects in top left/bottom left memory quarters, finally NO_MATCH is returned
{
	objtop = 0;
	objbot = 0;
	int lastpixel = objval[63];
	for (int i = 0; i < 64; i++) {
		if (i < 32) { objtop *= 2; } else { objbot *= 2; }
		if ((objval[i] - lastpixel) >= 0) {
			if (i < 32) { objtop++; } else { objbot++; }
		}
		lastpixel = objval[i];
	}
	it = objmap.find(objtop);
	if (it != objmap.end()) {
		hash = objtop;
		return it->second;
	}
	it = objmap.find(objbot);
	if (it != objmap.end()) {
		hash = objbot;
		return it->second;
	}
	hash = 0;
	return "NO_MATCH";
}

string getfield(uint64_t & hash)	//simple sequential bit comparisons
{
	ofstream checkfile;
	//checkfile.open("tonberry/tests/hashcache_test.txt", ofstream::out| ofstream::app);
	hashval = 0;
	int lastpixel = pixval[63];
	for (int i = 0; i < 64; i++) {
		hashval *= 2;
		if ((pixval[i] - lastpixel) >= 0) { hashval++; }
		lastpixel = pixval[i];
	}
	hash_it = hashcache.find(hashval);
	if (hash_it != hashcache.end()) {
		//checkfile << "Cache hit! " << hash_it->first << "," << hash_it->second << endl;
		hash_element = *hash_it;
		hashcache.update(hash_it);
		hash = hashval;
		return hash_element.second;
	} else {
		it = hashmap.find(hashval);
		if (it != hashmap.end()) {
			hashcache.insert(it->first, it->second);
			hash = hashval;
			return it->second;
		} else {
			it = collmap.find(hashval);
			if (it != collmap.end()) {
				hash = hashval;
				return "COLLISION";
			}
		}
	}
	return getobj(hash);
}

string getfield2()	//simple sequential bit comparisons, algorithm 2
{
	hashval2 = 0;
	int lastpixel = pixval2[97];
	for (int i = 0; i < 98; i++) {
		hashval2 *= 2;
		if ((pixval2[i] - lastpixel) >= 0) { hashval2 += 1; }
		lastpixel = pixval2[i];
	}
	it2 = coll2map.find(hashval2.getNumber());
	if (it2 != coll2map.end()) { return it2->second; }
	return "NO_MATCH2";
}


typedef pair<uint64_t, HANDLE>					lru_item_t;			// type of nhcache list entry
typedef list<lru_item_t>						lru_list_t;			// type of nhcache list
typedef lru_list_t::iterator					lru_list_iter;		// type of nhcache listiterator
typedef unordered_map<uint64_t, lru_list_iter>	lru_map_t;			// type of nhcache map
typedef lru_map_t::iterator						lru_map_iter;		// type of nhcache map iterator
typedef unordered_map<HANDLE, lru_map_iter>		handlecache_t;      // type of handlecache
typedef handlecache_t::iterator					handlecache_iter;	// type of handlecache iterator

// together these make nhcache:
lru_list_t nh_list;													// newhandle list stores pair of hash and newhandle
lru_map_t nh_map;													// newhandle map:        hash  :-->  h_list pointe

// handlecache:
handlecache_t handlecache;  // handle cache:        replaced HANDLE  :-->  pointer to nhcache entry

int m = 0;
//Then the unlockrect
void GlobalContext::UnlockRect(D3DSURFACE_DESC &Desc, Bitmap &BmpUseless, HANDLE Handle) //note BmpUseless
{
	IDirect3DTexture9* pTexture = (IDirect3DTexture9*)Handle;

	String debugtype = String("");

	if (pTexture && Desc.Width < 640 && Desc.Height < 480 && Desc.Format == D3DFORMAT::D3DFMT_A8R8G8B8 && Desc.Pool == D3DPOOL::D3DPOOL_MANAGED)    //640x480 are video
	{
		D3DLOCKED_RECT Rect;
		pTexture->LockRect(0, &Rect, NULL, 0);
		UINT pitch = (UINT)Rect.Pitch;
		BYTE* pData = (BYTE*)Rect.pBits;

		ofstream debug;
		debug.open(DEBUG_LOG.string(), ofstream::out | ofstream::app);

		uint64_t hash = 0;
		Hash_Algorithm_1(pData, pitch, Desc.Width, Desc.Height);    //Run Hash_Algorithm_1
		string texturename = getfield(hash);
		debug << hash << ": " << texturename << endl;

		if (texturename == "sysfld00_13" || texturename == "sysfld01_13") { texturename = getsysfld(pData, pitch, Desc.Width, Desc.Height, texturename); } //Exception for sysfld00 and sysfld01
		if (texturename == "iconfl00_13" || texturename == "iconfl01_13" || texturename == "iconfl02_13" || texturename == "iconfl03_13" || texturename == "iconflmaster_13") { texturename = geticonfl(pData, pitch, Desc.Width, Desc.Height, texturename); } //Exception for iconfl00, iconfl01, iconfl02, iconfl03, iconflmaster

		if (texturename == "NO_MATCH") { //Handle inválido, lo borro, pero no su posible textura asociada.
			handlecache.erase(Handle);                                            // the way we deal with handlecache does not change
			debugtype = String("nomatch");
		} else { //Texture FOUND in Hash_Algorithm_1 OR is a COLLISION
			if (texturename == "COLLISION") { //Run Hash_Algorithm_2
				Hash_Algorithm_2(pData, pitch, Desc.Width, Desc.Height);
				texturename = getfield2();
				if (texturename == "NO_MATCH2") { debugtype = String("nomatch2"); }
			}

			
			string filename = "textures\\" + texturename.substr(0, 2) + "\\" + texturename.substr(0, texturename.rfind("_")) + "\\" + texturename + ".png";   

			lru_map_iter ptr_to_replacement;
			if (nh_map.size() > 0 && (ptr_to_replacement = nh_map.find(hash)) != nh_map.end()) { //casos 2 y 3

				/* UPDATE NH CACHE ACCESS ORDER */
				lru_list_iter ptr_to_nh_list = ptr_to_replacement->second;

				debug << "\tFOUND: (" << ptr_to_nh_list->first << ", " << ptr_to_nh_list->second << ")";

				// move (most-recently-accessed) list item to front of nh_list
				nh_list.erase(ptr_to_nh_list);
				debug << "Moving (" << ptr_to_nh_list->first << ", " << ptr_to_nh_list->second << ") to front of list: ";
				nh_list.push_front(lru_item_t(ptr_to_nh_list->first, ptr_to_nh_list->second));

				// update nh_map with new list item pointer
				nh_map[hash] = nh_list.begin();
				debug << "nh_map[" << hash << "] = (" << nh_map[hash]->first << ", " << nh_map[hash]->second << ")" << endl;
				/* END UPDATE NH CACHE */

				handlecache[Handle] = nh_map.find(hash);            // need to point this at the new, updated entry in nh_map (with the new list location)
			} else {//caso 1 -> hay que cargar textura
				ifstream ifile(filename);
				if (ifile.fail()) {
					debugtype = String("noreplace"); //No file, allow normal SetTexture
				} else {    //Load texture into cache
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
						for (UINT x = 0; x < Bmp.Width(); x++)   //works for textures of any size (e.g. 4-bit indexed)
						{
							RGBColor Color = Bmp[Bmp.Height() - y - 1][x];  //must flip image
							CurRow[x] = RGBColor(Color.b, Color.g, Color.r, Color.a);
						}
					}
					newtexture->UnlockRect(0); //Texture loaded
					HANDLE tempnewhandle = (HANDLE)newtexture;

					debug << "\tCREATED: (" << hash << ", " << tempnewhandle << ")";

					// we know hash is not in the nhcache, so add hash->tempnewhandle to nhcache
					nh_list.push_front(lru_item_t(hash, tempnewhandle));                         // O(1) in linked list
					debug << "Adding (" << hash << ", " << tempnewhandle << ") to front of list: ";
					nh_map[hash] = nh_list.begin();                                              // O(1) in unordered_map
					debug << "nh_map[" << hash << "] = (" << nh_map[hash]->first << ", " << nh_map[hash]->second << ")" << endl;

					/* MAKE SURE NHCACHE IS THE CORRECT SIZE */
					while (nh_list.size() > CACHE_SIZE) {                // I use "while" for completeness but this should only ever loop once
						// get pointer to last (least recent) list item
						lru_list_iter last_elem = nh_list.end();                              // O(1) in linked list
						--last_elem;

						debug << "Removing (" << last_elem->first << ", " << last_elem->second << ") from back of list: ";

						// dispose of texture
						((IDirect3DTexture9*)last_elem->second)->Release();
						last_elem->second = NULL;

						// remove from map (this is why the nh_list stores pair<hash, handle>)
						nh_map.erase(last_elem->first);                                          // O(1) in unordered_map
						lru_map_iter temp_iter = nh_map.find(last_elem->first);
						if (temp_iter == nh_map.end())
							debug << "nh_map[" << last_elem->first << "] = NULL" << endl;
						else
							debug << "!!! nh_map[" << last_elem->first << "] = (" << nh_map[last_elem->first]->first << ", " << nh_map[last_elem->first]->second << ")" << endl;
						// pop from list
						nh_list.pop_back();

						debug << "nh_list.size() = " << nh_list.size() << endl;
					}
					/* END MAKE SURE NHCACHE IS THE CORRECT SIZE */


					handlecache[Handle] = nh_map.find(hash);                 // the way we deal with handlecache does not change
				}

			}
		}
		pTexture->UnlockRect(0); //Finished reading pTextures bits

		debug << endl;
		debug.close();
	} else { //Video textures/improper format
		handlecache.erase(Handle);                                                // this is the beauty of your solution; you replaced that whole O(n^2) loop bullshit with one line ;)
	}
	if (debugtype == String("")) { debugtype = String("error"); }
	//Debug
	String debugfile = String("tonberry\\debug\\") + debugtype + String("\\") + String::ZeroPad(String(m), 3) + String(".bmp");
	D3DXSaveTextureToFile(debugfile.CString(), D3DXIFF_BMP, pTexture, NULL);
	m++; //debug
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
				newtexture = (IDirect3DTexture9*)ptr_to_replacement->second->second->second; //ptr_to_replacement->second =                 pointer to nh_map
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