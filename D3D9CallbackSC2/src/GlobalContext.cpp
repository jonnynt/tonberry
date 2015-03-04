#include "Main.h"
#include <stdint.h>
#include <sstream>

//-------------
/*#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>*/
//-------------

#ifndef ULTRA_FAST
bool g_ReportingEvents = false;
#endif

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

//Global Variables
//vector<int> pixval;
int pixval[64];
//vector<int> pixval2;
int pixval2[98];
//vector<int> objval;
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
vector<String> pngnames;
//String pngnames[100]; //Store the last 100 texture pages for caching
vector<HANDLE> handles;
//HANDLE handles[100];
vector<HANDLE> newhandles;
//HANDLE newhandles[100];
const int cache_size = 100;
float resize_factor;

void initVectors ()
{
	//pixval.reserve(64);
	//pixval2.reserve(98);
	//objval.reserve(64);
	/*pngnames.reserve(100);
	handles.reserve(100);
	newhandles.reserve(100);*/
}

void loadprefs ()
{
	resize_factor = 4.0;
	ifstream prefsfile;
	prefsfile.open ("tonberry\\prefs.txt", ifstream::in);
	if (prefsfile.is_open())
	{
		string line;
		while ( getline(prefsfile, line) ) //~10000 total number of 128x256 texture blocks in ff8
		{
			if (line.substr(0, line.find("=")) == "resize_factor")
			{
				resize_factor = (float)atoi(line.substr(line.find("=") + 1, line.length()).c_str()); //make sure no spaces in prefs file
			}
		}
		prefsfile.close();
	}
}

void loadhashfile ()	//Expects hash1map.csv to be in ff8/tonberry directory
{
	ifstream hashfile;
	hashfile.open ("tonberry\\hash1map.csv", ifstream::in);
	string line;
	if (hashfile.is_open())
	{
		while ( getline(hashfile, line) ) //~10000 total number of 128x256 texture blocks in ff8
		{
			int comma = line.find(",");
			string field = line.substr(0, comma);
			string valuestr = line.substr(comma + 1, line.length()).c_str();
			uint64_t value = ToNumber<uint64_t>(valuestr);
			hashmap.insert(pair<uint64_t, string>(value, field)); //key, value for unique names, value, key for unique hashvals
		}
		hashfile.close();
	}
}

void loadcollfile ()	//Expects collisions.csv to be in ff8/tonberry directory
{
	ifstream collfile;
	collfile.open ("tonberry\\collisions.csv", ifstream::in);
	string line;
	if (collfile.is_open())
	{
		while ( getline(collfile, line) ) //~10000 total number of 128x256 texture blocks in ff8
		{
			int comma = line.find(",");
			string field = line.substr(0, comma);
			string valuestr = line.substr(comma + 1, line.length()).c_str();
			uint64_t value = ToNumber<uint64_t>(valuestr);
			collmap.insert(pair<uint64_t, string>(value, field)); //key, value for unique names, value, key for unique hashvals
		}
		collfile.close();
	}
}

void loadcoll2file ()	//Expects hash2map.csv to be in ff8/tonberry directory
{
	ifstream coll2file;
	coll2file.open ("tonberry\\hash2map.csv", ifstream::in);
	string line;
	if (coll2file.is_open())
	{
		while ( getline(coll2file, line) ) //~10000 total number of 128x256 texture blocks in ff8
		{
			int comma = line.find(",");
			string field = line.substr(0, comma);
			string valuestr = line.substr(comma + 1, line.length()).c_str();
			//BigInteger value = BigInteger(valuestr);
			//value = ToNumber<BigInt>(valuestr);
			coll2map.insert(pair<string, string>(valuestr, field)); //key, value for unique names, value, key for unique hashvals
		}
		coll2file.close();
	}
}

void loadobjfile ()	//Expects objmap.csv to be in ff8/tonberry directory
{
	ifstream objfile;
	objfile.open ("tonberry\\objmap.csv", ifstream::in);
	string line;
	if (objfile.is_open())
	{
		while ( getline(objfile, line) )
		{
			int comma = line.find(",");
			string obj = line.substr(0, comma);
			string valuestr = line.substr(comma + 1, line.length()).c_str();
			uint64_t value = ToNumber<uint64_t>(valuestr);
			objmap.insert(pair<uint64_t, string>(value, obj)); //key, value for unique names, value, key for unique hashvals
		}
		objfile.close();
	}
}

void GlobalContext::Init ()
{
	//------------
	/*_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
	_CrtSetReportMode( _CRT_ERROR, _CRTDBG_MODE_FILE );
	HANDLE hLogFile;
	hLogFile = CreateFile("c:\\log.txt", GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	_CrtSetReportFile( _CRT_ERROR, hLogFile );
	_RPT0(_CRT_ERROR,"file message\n");
	CloseHandle(hLogFile);*/
	//------------
    Graphics.Init();
	initVectors();
	loadprefs();
	loadhashfile();
	loadcollfile();
	loadcoll2file();
	loadobjfile();

	ofstream debug;
	debug.open("tonberry\\debug\\init.txt", fstream::app);
	std::time_t time = std::time(nullptr);
	debug << "Initialized " << std::asctime(std::localtime(&time));
}

void Hash_Algorithm_1 (BYTE* pData, UINT pitch, int width, int height)	//hash algorithm that preferences top and left sides
{
	int blocksize = 16;
	UINT x, y;
	int pix = 0;
	int toppix = 0;
	int botpix = 0;
	for (x = 0; x < 8; x++) //pixvals 0->31
	{
		for (y = 0; y < 4; y++)
		{
			if (x*blocksize < width && y*blocksize < height) //respect texture sizes
			{
				RGBColor* CurRow = (RGBColor*)(pData + (y*blocksize) * pitch);
				RGBColor Color = CurRow[x*blocksize];
				pixval[pix] = (Color.r + Color.g + Color.b) / 3;
				if (y*blocksize < 128) { if (toppix < 32) { objval[toppix] = pixval[pix]; toppix++; } }
				else if (botpix < 32) { objval[botpix + 32] = pixval[pix]; botpix++; }
			} else { pixval[pix] = 0; } //out of bounds
			pix++;
		}				
	}
	for (x = 0; x < 2; x++) //pixvals 32->55
	{
		for (y = 4; y < 16; y++)
		{
			if (x*blocksize < width && y*blocksize < height) //respect texture sizes
			{
				RGBColor* CurRow = (RGBColor*)(pData + (y*blocksize) * pitch);
				RGBColor Color = CurRow[x*blocksize];
				pixval[pix] = (Color.r + Color.g + Color.b) / 3;
				if (y*blocksize < 128) { if (toppix < 32) { objval[toppix] = pixval[pix]; toppix++; } }
				else if (botpix < 32) { objval[botpix + 32] = pixval[pix]; botpix++; }
			} else { pixval[pix] = 0; } //out of bounds
			pix++;
		}				
	}
	for (x = 3; x < 7; x+=3) //pixvals 56->63, note +=3
	{
		for (y = 5; y < 15; y+=3)	//note +=3
		{
			if (x*blocksize < width && y*blocksize < height) //respect texture sizes
			{
				RGBColor* CurRow = (RGBColor*)(pData + (y*blocksize) * pitch);
				RGBColor Color = CurRow[x*blocksize];
				pixval[pix] = (Color.r + Color.g + Color.b) / 3;
				if (y*blocksize < 128) { if (toppix < 32) { objval[toppix] = pixval[pix]; toppix++; } }
				else if (botpix < 32) { objval[botpix + 32] = pixval[pix]; botpix++; }
			} else { pixval[pix] = 0; } //out of bounds
			pix++;
		}				
	}
}

void Hash_Algorithm_2 (BYTE* pData, UINT pitch, int width, int height)	//hash algorithm that chooses unique pixels selected by hand
{
	int pix = 0;
	UINT x0, y0;
	UINT x[76] = {44, 0, 17, 25, 15, 111, 35, 25, 3, 46, 112, 34, 21, 1, 72, 80, 25, 32, 15, 4, 123, 16, 47, 14, 110, 78, 3, 66, 0, 86, 58, 27, 39, 4, 6, 49, 7, 71, 121, 17, 22, 16, 84, 115, 118, 119, 126, 59, 96, 88, 64, 1, 21, 31, 107, 92, 73, 116, 118, 58, 47, 18, 93, 78, 97, 106, 107, 77, 99, 13, 100, 125, 12, 33, 53, 61};
	UINT y[76] = {243, 0, 2, 19, 35, 24, 0, 12, 23, 7, 5, 0, 4, 0, 2, 218, 30, 2, 20, 23, 4, 4, 2, 8, 7, 7, 25, 0, 1, 0, 11, 15, 2, 0, 0, 1, 15, 15, 16, 7, 7, 0, 244, 245, 245, 245, 253, 203, 135, 184, 9, 15, 80, 81, 244, 245, 249, 255, 238, 237, 216, 218, 240, 216, 116, 164, 244, 247, 236, 245, 21, 59, 25, 8, 16, 108};
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

	for (x0 = 0; x0 < 44; x0+=4) //pixvals 76->97, note +=4
	{
		for (y0 = 7; y0 < 16; y0+=8) //note +=8
		{
			if (x0 < width && y0 < height) //respect texture sizes
			{
				RGBColor* CurRow = (RGBColor*)(pData + (y0/*blocksize*/) * pitch); //blocksize already included
				RGBColor Color = CurRow[x0/*blocksize*/];
				pixval2[pix] = (Color.r + Color.g + Color.b) / 3;
			} else { pixval2[pix] = 0; } //out of bounds
			pix++;
		}				
	}
}

string getsysfld (BYTE* pData, UINT pitch, int width, int height, string sysfld)	//Exception method for sysfld00 and sysfld01
{
	UINT x = 177;
	UINT y = 155;
	RGBColor* CurRow = (RGBColor*)(pData + (y) * pitch);
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

string geticonfl (BYTE* pData, UINT pitch, int width, int height, string iconfl)	//Exception method for iconfl00, iconfl01, iconfl02, iconfl03, iconflmaster
{
	UINT x = 0;
	UINT y = 0;
	if (iconfl == "iconfl00_13") { x = 82; y = 150; }
	else if (iconfl == "iconfl01_13") { x = 175; y = 208; }
	else if (iconfl == "iconfl02_13") { x = 216; y = 108; }
	else if (iconfl == "iconfl03_13") { x = 58; y = 76; }
	else if (iconfl == "iconflmaster_13") { x = 215; y = 103; }

	RGBColor* CurRow = (RGBColor*)(pData + (y) * pitch);
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
	return iconfl.substr(0, iconfl.size()-2) + icpage;
}

string getobj () //if previously unmatched, searches through object map for objects in top left/bottom left memory quarters, finally NO_MATCH is returned
{
	objtop = 0;
	objbot = 0;
	int lastpixel = objval[63];
    for (int i = 0; i < 64; i++)
    {
		if (i < 32) { objtop *= 2; }
		else { objbot *= 2; }
		if ((objval[i] - lastpixel) >= 0)
		{
			if (i < 32) { objtop++; }
			else { objbot++; }
		}
		lastpixel = objval[i];
    }
	it = objmap.find(objtop);
	if (it != objmap.end()) { return it->second; }
	it = objmap.find(objbot);
	if (it != objmap.end()) { return it->second; }
	return "NO_MATCH";
}

string getfield ()	//simple sequential bit comparisons
{
	hashval = 0;
	int lastpixel = pixval[63];
    for (int i = 0; i < 64; i++)
    {
        hashval *= 2;
		if ((pixval[i] - lastpixel) >= 0) {	hashval++; }
		lastpixel = pixval[i];
    }
	it = hashmap.find(hashval);
	if (it != hashmap.end()) { return it->second; }
	else {
		it = collmap.find(hashval);
		if (it != collmap.end()) { return "COLLISION"; }
	}
	return getobj();
}

string getfield2 ()	//simple sequential bit comparisons, algorithm 2
{
	hashval2 = 0;
	int lastpixel = pixval2[97];
    for (int i = 0; i < 98; i++)
    {
        hashval2 *= 2;
		if ((pixval2[i] - lastpixel) >= 0) {	hashval2 += 1; }
		lastpixel = pixval2[i];
    }
	it2 = coll2map.find(hashval2.getNumber());
	if (it2 != coll2map.end()) { return it2->second; }
	return "NO_MATCH2";
}

int m;
void GlobalContext::UnlockRect (D3DSURFACE_DESC &Desc, Bitmap &BmpUseless, HANDLE Handle) //note BmpUseless
{
	IDirect3DTexture9* pTexture = (IDirect3DTexture9*)Handle;	

	String debugtype = String("");

	if (pTexture && Desc.Width < 640 && Desc.Height < 480 && Desc.Format == D3DFORMAT::D3DFMT_A8R8G8B8 && Desc.Pool == D3DPOOL::D3DPOOL_MANAGED)	//640x480 are video
	{
		D3DLOCKED_RECT Rect;
		pTexture->LockRect(0, &Rect, NULL, 0);
		UINT pitch = (UINT)Rect.Pitch;
		BYTE* pData = (BYTE*)Rect.pBits;

		Hash_Algorithm_1(pData, pitch, Desc.Width, Desc.Height);	//Run Hash_Algorithm_1
		string texturename = getfield();

		if (texturename == "sysfld00_13" || texturename == "sysfld01_13") { texturename = getsysfld(pData, pitch, Desc.Width, Desc.Height, texturename); } //Exception for sysfld00 and sysfld01
		if (texturename == "iconfl00_13" || texturename == "iconfl01_13" || texturename == "iconfl02_13" || texturename == "iconfl03_13" || texturename == "iconflmaster_13") { texturename = geticonfl(pData, pitch, Desc.Width, Desc.Height, texturename); } //Exception for iconfl00, iconfl01, iconfl02, iconfl03, iconflmaster

		//ofstream checkfile;
		//checkfile.open ("checkfile.csv", ofstream::out | ofstream::app);

		if (texturename == "NO_MATCH")
		{
			for (int fn = 0; fn < handles.size(); fn++)
			{
				if (Handle == handles[fn])
				{ //Remove handle if the texture is "NO_MATCH"

					bool keeptexture = false;
					for (int nh = 0; nh < handles.size(); nh++)
					{
						if (nh != fn)
						{
							if (newhandles[fn] == newhandles[nh]) { keeptexture = true; break; }
						}
					}
					if (!keeptexture)
					{
						IDirect3DTexture9* tempTexture = (IDirect3DTexture9*)newhandles[fn];
						tempTexture->Release();
						tempTexture = NULL;
					}
					pngnames.erase(pngnames.begin() + fn);
					handles.erase(handles.begin() + fn);
					newhandles.erase(newhandles.begin() + fn);
					
					/*//if (handles[fn]) { ((IDirect3DTexture9*)handles[fn])->Release(); }
					handles[fn] = NULL;
					//if (newhandles[fn]) { ((IDirect3DTexture9*)newhandles[fn])->Release(); }
					newhandles[fn] = NULL;
					pngnames[fn] = "";*/
				}
			}
			debugtype = String("nomatch");
		} else { //Texture FOUND in Hash_Algorithm_1 OR is a COLLISION
			if (texturename == "COLLISION")
			{ //Run Hash_Algorithm_2
				Hash_Algorithm_2(pData, pitch, Desc.Width, Desc.Height);
				texturename = getfield2();
				if (texturename == "NO_MATCH2") { debugtype = String("nomatch2"); }
			}

			string filename = "textures\\" + texturename.substr(0, 2) + "\\" + texturename.substr(0, texturename.rfind("_")) + "\\" + texturename + ".png";
			//checkfile << pngnames[0] << "<-last,this->" << texturename;	
			bool incache = false;
			bool handlefound = false;
			for (int fn = 0; fn < handles.size(); fn++)
			{
				if (Handle == handles[fn])
				{	//Handle already exists
					handlefound = true;
					if (texturename == pngnames[fn].CString())
					{	//Handle appropriately exists
						incache = true;

						pngnames.erase(pngnames.begin() + fn);
						pngnames.insert(pngnames.begin(), String(texturename.c_str()));
						handles.erase(handles.begin() + fn);
						handles.insert(handles.begin(), Handle);
						HANDLE temphandle = newhandles[fn];
						newhandles.erase(newhandles.begin() + fn);
						newhandles.insert(newhandles.begin(), temphandle);
						//if (handles.size() > cache_size) { pngnames.pop_back(); handles.pop_back(); newhandles.pop_back(); }

						/*String tempstr = pngnames[fn];
						auto temphandle = handles[fn];
						auto tempnewhandle = newhandles[fn];
						for (int b = fn+1; b --> 1;)	//load the newest into first slot, shift the lists
						{
							pngnames[b] = pngnames[b-1];
							handles[b] = handles[b-1];
							newhandles[b] = newhandles[b-1];
						}					
						pngnames[0] = tempstr; //Add the new texture references
						handles[0] = temphandle;
						newhandles[0] = tempnewhandle;*/
						//checkfile << "RETRIEVED FROM CACHE" << endl;
					} else { //Handle inappropriately exists (overwritten)
						incache = false;

						bool keeptexture = false;
						for (int nh = 0; nh < handles.size(); nh++)
						{
							if (nh != fn)
							{
								if (newhandles[fn] == newhandles[nh]) { keeptexture = true; break; }
							}
						}
						if (!keeptexture)
						{
							IDirect3DTexture9* tempTexture = (IDirect3DTexture9*)newhandles[fn];
							tempTexture->Release();
							tempTexture = NULL;
						}
						pngnames.erase(pngnames.begin() + fn);
						handles.erase(handles.begin() + fn);
						newhandles.erase(newhandles.begin() + fn);

						/*//if (handles[fn]) { ((IDirect3DTexture9*)handles[fn])->Release(); }
						handles[fn] = NULL;
						//if (newhandles[fn]) { ((IDirect3DTexture9*)newhandles[fn])->Release(); }
						newhandles[fn] = NULL;
						pngnames[fn] = "";*/
					}
					break;
				}
			}
			if (!handlefound)
			{	//Handle does not exist yet
				bool texturefound = false;
				for (int fn = 0; fn < handles.size(); fn++)
				{
					if (texturename == pngnames[fn].CString())
					{	//New handle but texture in cache
						texturefound = true;
						incache = true;

						pngnames.insert(pngnames.begin(), String(texturename.c_str()));
						handles.insert(handles.begin(), Handle);
						HANDLE temphandle = newhandles[fn];
						newhandles.insert(newhandles.begin(), temphandle);
						if (handles.size() > cache_size) { pngnames.pop_back(); handles.pop_back(); newhandles.pop_back(); }

						/*auto newhandlestemp = newhandles[fn];
						for (int b = handles.size(); b --> 1;)	//Knock the last off, load the newest into first slot
						{
							pngnames[b] = pngnames[b-1];
							handles[b] = handles[b-1];
							newhandles[b] = newhandles[b-1];
						}					
						pngnames[0] = String(texturename.c_str()); //Add the new texture references
						handles[0] = Handle;
						newhandles[0] = newhandlestemp;	//New texture handle*/
						debugtype = String("replaced");
						break;
					}
				}
				if (!texturefound)
				{	//New handle and texture not yet cached
					incache = false;
				}
			}
			if (!incache) //Don't reload same file every frame
			{
				ifstream ifile(filename);
				if (ifile.fail()) {	debugtype = String("noreplace"); //No file, allow normal SetTexture
				} else {	//Load texture into cache
					LPDIRECT3DDEVICE9 Device = g_Context->Graphics.Device();
					IDirect3DTexture9* newtexture;	
					Bitmap Bmp;
					Bmp.LoadPNG(String(filename.c_str()));
					DWORD Usage = D3DUSAGE_AUTOGENMIPMAP;
					D3DPOOL Pool = D3DPOOL_MANAGED;
					D3DFORMAT Format = D3DFMT_A8R8G8B8;
					Device->CreateTexture(int(resize_factor*(float)Desc.Width), int(resize_factor*(float)Desc.Height), 0, Usage, Format, Pool, &newtexture, NULL);
					D3DLOCKED_RECT newRect;
					newtexture->LockRect(0, &newRect, NULL, 0);
					BYTE* newData = (BYTE *)newRect.pBits;
					for(UINT y = 0; y < Bmp.Height(); y++)
					{
						RGBColor* CurRow = (RGBColor *)(newData + y * newRect.Pitch);
						for(UINT x = 0; x < Bmp.Width(); x++)	//works for textures of any size (e.g. 4-bit indexed)
						{
							RGBColor Color = Bmp[Bmp.Height() - y - 1][x];	//must flip image
							CurRow[x] = RGBColor(Color.b, Color.g, Color.r, Color.a);
						}
					}
					newtexture->UnlockRect(0); //Texture loaded

					pngnames.insert(pngnames.begin(), String(texturename.c_str()));
					handles.insert(handles.begin(), Handle);
					HANDLE tempnewhandle = (HANDLE)newtexture;
					newhandles.insert(newhandles.begin(), tempnewhandle);
					if (handles.size() > cache_size) { pngnames.pop_back(); handles.pop_back(); newhandles.pop_back(); }

					for (int fn = 0; fn < handles.size(); fn++)
					{
						if (tempnewhandle == handles[fn])
						{
							bool keeptexture = false;
							for (int nh = 0; nh < handles.size(); nh++)
							{
								if (nh != fn)
								{
									if (newhandles[fn] == newhandles[nh]) { keeptexture = true; break; }
								}
							}
							if (!keeptexture)
							{
								IDirect3DTexture9* tempTexture = (IDirect3DTexture9*)newhandles[fn];
								tempTexture->Release();
								tempTexture = NULL;
							}

							pngnames.erase(pngnames.begin() + fn);
							handles.erase(handles.begin() + fn);
							newhandles.erase(newhandles.begin() + fn);
						}
					}
					
					/*for (int b = handles.size(); b --> 1;)	//Knock the last off, load the newest into first slot
					{
						pngnames[b] = pngnames[b-1];
						handles[b] = handles[b-1];
						newhandles[b] = newhandles[b-1];
					}					
					pngnames[0] = String(texturename.c_str()); //Add the new texture references
					handles[0] = Handle;
					newhandles[0] = (HANDLE)newtexture;	//New texture handle*/
					//checkfile << pngnames[0] << " " << handles[0] << " " << newhandles[0] << endl;
					debugtype = String("replaced");
				}
			}	
		}
		/*checkfile << m << endl;
		for (int h = 0; h < handles.size(); h++)
		{
			checkfile << h << " : " << pngnames[h] << " : " << handles[h] << " : " << newhandles[h] << endl;
		}*/
		//checkfile << "CREATE " << m << ": " << texturename << endl;
		//checkfile.close();
		pTexture->UnlockRect(0); //Finished reading pTextures bits
	} else { //Video textures/improper format
		for (int fn = 0; fn < handles.size(); fn++)
		{
			if (Handle == handles[fn])
			{ //Remove handle since the texture is not replaced (overwritten)
				//if (handles[fn]) { ((IDirect3DTexture9*)handles[fn])->Release(); }
				bool keeptexture = false;
				for (int nh = 0; nh < handles.size(); nh++)
				{
					if (nh != fn)
					{
						if (newhandles[fn] == newhandles[nh]) { keeptexture = true; break; }
					}
				}
				if (!keeptexture)
				{
					IDirect3DTexture9* tempTexture = (IDirect3DTexture9*)newhandles[fn];
					tempTexture->Release();
					tempTexture = NULL;
				}
				pngnames.erase(pngnames.begin() + fn);
				handles.erase(handles.begin() + fn);
				newhandles.erase(newhandles.begin() + fn);
				/*handles[fn] = NULL;
				//if (newhandles[fn]) { ((IDirect3DTexture9*)newhandles[fn])->Release(); }
				newhandles[fn] = NULL;
				pngnames[fn] = "";*/
			}
		}
		debugtype = String("unsupported");
	}
	if (debugtype == String("")) { debugtype = String("error"); }
	//Debug
	String debugfile = String("tonberry\\debug\\") + debugtype + String("\\") + String::ZeroPad(String(m), 3) + String(".bmp");
	D3DXSaveTextureToFile(debugfile.CString(), D3DXIFF_BMP, pTexture, NULL);
	m++; //debug
}

bool GlobalContext::SetTexture (DWORD Stage, HANDLE* SurfaceHandles, UINT SurfaceHandleCount)
{
	//ofstream checkfile;
	//checkfile.open ("checkfile.csv", ofstream::out | ofstream::app);
	//checkfile << "SET" << endl;
	//checkfile.close();
	IDirect3DTexture9* newtexture = NULL;
	for (int j = 0; j < SurfaceHandleCount; j++)
	{
		if (SurfaceHandles[j])
		{		
			for (int i = 0; i < handles.size(); i++)
			{
				if (handles[i] == SurfaceHandles[j])	//SurfaceHandles[0] only necessary?
				{	
					newtexture = (IDirect3DTexture9*)newhandles[i];
					if (newtexture)
					{
						g_Context->Graphics.Device()->SetTexture(Stage, newtexture);
						//((IDirect3DTexture9*)SurfaceHandles[j])->Release();
						return true;
					}	//Texture replaced!
				}
			}
		}
	}
	return false;
}

//Unused functions
void GlobalContext::UpdateSurface(D3DSURFACE_DESC &Desc, Bitmap &Bmp, HANDLE Handle) {}
void GlobalContext::Destroy(HANDLE Handle) {}
void GlobalContext::CreateTexture (D3DSURFACE_DESC &Desc, Bitmap &Bmp, HANDLE Handle, IDirect3DTexture9** ppTexture) {}
void GlobalContext::BeginScene () {}