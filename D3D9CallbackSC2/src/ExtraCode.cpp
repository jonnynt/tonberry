#include "Main.h"

//EXTRA CODE
			
					/*if (pngnames[i] == "bghall1a_1" || pngnames[i] == "bghall1a_3")
					{
						ofstream checkfile;
						checkfile.open ("checkfile.csv", ofstream::out | ofstream::app);
						checkfile << "SetTexture " << pngnames[i] << " " << handles[i] << " " << newhandles[i] << endl;
						checkfile.close();
					}*/
					/*String Fnamex = String("C:\\Users\\omzy\\Desktop\\preoutputX\\") + String::ZeroPad(String(m), 5) + String(".bmp");
					D3DXSaveTextureToFile(Fnamex.CString(), D3DXIFF_BMP, newtexture, NULL);
					m++;*/
								/*int substrloc = texturename.find_last_of("_");
					string fieldname = texturename.substr(0, substrloc);
					int blocknum = atoi(texturename.substr(substrloc + 1, texturename.length()).c_str());
					String filename2 = String("textures\\") + String(fieldname.c_str()) + String("_") + String(blocknum+1) + ".png";
					checkfile << filename2 << endl;
					ifstream ifile2(filename2.CString());
					if (!ifile2.fail()) //No file, allow normal SetTexture
					{
						Bitmap Bmp2;
						Bmp2.LoadPNG(filename2);
						for(UINT y = 0; y < Bmp2.Height(); y++)
						{
							RGBColor* CurRow = (RGBColor *)(newData + y * newRect.Pitch);
							UINT offset = 0;
							UINT xsize = Bmp2.Height();
							for(UINT x = offset; x < offset + xsize && x < Bmp2.Width(); x++)
							{
								RGBColor Color = Bmp2[Bmp2.Height() - y - 1][x];	//must flip image
								CurRow[x - offset + 512] = RGBColor(Color.b, Color.g, Color.r, Color.a);
							}
						}
					}*/
		//for (int b = 10; b --> 0;)	//Knock the last off, load the newest 1 to first slot
		//{
			//checkfile << "p:" << pngnames[b] << " h:" << handles[b] << " n:" << newhandles[b] << endl;
		//}
//int substrloc = texturename.find_last_of("_");	//parse texturename
			//string fieldname = texturename.substr(0, substrloc);
			//int blocknum = atoi(texturename.substr(substrloc + 1, texturename.length()).c_str());	//and page/blocknum
/*void nearestneighbor4x (BYTE* pData, UINT pitch) 
{	//Edited for 4x resize, 4 lines of 256 written to single 1024 line, then pitch
	UINT x, y, x16, y16;
	RGBColor* CurRow;
	RGBColor* DestRow;
	RGBColor Color;
	for (y = 256; y --> 0;)	//Why this syntax works and y=255;y>=0;y-- doesn't is beyond me
	{
		for (x = 256; x --> 0;)
		{
			CurRow = (RGBColor*)(pData + y / 4 * pitch);
			Color = CurRow[(y % 4) * 256 + x];
			for (x16 = 0; x16 < 4; x16++)
			{
				for (y16 = 0; y16 < 4; y16++)
				{
					DestRow = (RGBColor*)(pData + (y * 4 + y16) * pitch);
					DestRow[x * 4 + x16] = Color;
				}
			}
		}				
	}
}*/
					//D3DXCreateTextureFromFile(Device, filename.CString(), &newtexture);

					
					/*IDirect3DBaseTexture9* NewBaseTexture;
					
					if(NewTexture == NULL)
					{
						BaseTexture->Release();
						return E_OUTOFMEMORY;
					}
					else
					{
						*ppTexture = NewTexture;*/
			//pTexture still locked, have pData (Rect.pBits)
			//only replace 1st block of 256x256 VRAM	//Applies to regular size texture
			/*if (!oldbmp)
			{
				for(UINT y = 0; y < Bmp.Height(); y++)
				{
					RGBColor* CurRow = (RGBColor *)(pData + y * Rect.Pitch);
					//UINT offset = blocknum * Bmp.Height() / 2;
					UINT offset = 0;
					UINT xsize = Bmp.Height();
					for(UINT x = offset; x < offset + xsize && x < Bmp.Width(); x++)
					{
						RGBColor Color = Bmp[Bmp.Height() - y - 1][x];	//must flip image
						CurRow[x - offset] = RGBColor(Color.b, Color.g, Color.r, Color.a);
					}
				}
			}*/

//#include <bitset>
//
// GlobalContext.cpp
//
// Definition of global context.  The AI must use a global context because it is embeeded inside a DLL
// Written by Matthew Fisher
//
//map<string, uint64_t> localmap;	//!!!
    //srand(UINT(time(NULL)));
    //FPSTimer.Start(30.0f);
    //Timer.Start();
	    //Parameters.LoadFromFile("AIParameters.txt");
    //Files.Init(Parameters.OutputFileDirectory);
    //Controller.Init();
    //CheckWindowSize();
/*int comparebits (uint64_t hashval, uint64_t mapval)
{
	uint64_t xor = hashval^mapval;	//bitwise XOR to detect differences
	bitset<64> diff(xor);
	return diff.count();
}*/

/*string closesthash (uint64_t hashval)
{
	string closest = "NO_MATCH";	//default
	int mindiff = 64;
	int threshold = 9;
	uint64_t closehash = 0;
	bool longrun = 0;

	//if (beginscene) 
	{
		if (comparebits(hashval, lasthashval) < threshold && lasthashname == "NO_MATCH") //same as last but not a match
		{
			//skip algorithms
		} else {
			//search localmap first if this is same scene, if not, must be a new field
			for (it = localmap.begin(); it != localmap.end(); ++it)//run short algorithm (cheap, iterates over ~5-13 elements)
			{
				int diff = comparebits(hashval, it->second);
				if (diff < mindiff)
				{
					mindiff = diff;
					if (mindiff < threshold) { closest = it->first; closehash = it->second; }
				}
			}
			if (closest == "NO_MATCH")	//run long algorithm (expensive, iterates over 6245 elements twice), should only be done once per field
			{
				longrun = 1;
				for (it = hashmap.begin(); it != hashmap.end(); ++it)
				{
					int diff = comparebits(hashval, it->second);
					if (diff < mindiff)
					{
						mindiff = diff;
						if (mindiff < threshold) { closest = it->first; closehash = it->second; }
					}
				}
				localmap.clear(); //empty previous field
				int strloc = closest.find_last_of("_");
				string field = closest.substr(0, strloc);
				for (it = hashmap.begin(); it != hashmap.end(); ++it)	//fill localmap with new field
				{
					if (it->first.substr(0, field.length()) == field)	//if 
					{
						localmap.insert(pair<string, uint64_t>(it->first, it->second));
					}
				}
			}
		}
	//} else { //run short algorithm (cheap, iterates over ~5-13 elements)
		/*if (lastfieldname != "NO_MATCH")
		{
			for (it = localmap.begin(); it != localmap.end(); ++it)
			{
				int diff = comparebits(hashval, it->second);
				if (diff < mindiff)
				{
					mindiff = diff;
					if (mindiff < threshold) { closest = it->first; closehash = it->second; }
				}
			}
		}
	}

	
	ofstream matchfile;
	matchfile.open ("matchfile.csv", ofstream::out | ofstream::app);
	matchfile << longrun << "," << lasthashname << "," << closest << "," << mindiff << "," << hashval << "," << closehash << endl;
	matchfile.close();
	lasthashval = hashval;
	lasthashname = closest;
	return closest;
}*/
	//pixvals 0->31
	/*for (x = 0; x < 8; x++)
	{
		for (y = 0; y < 4; y++)
		{
			RGBColor* CurRow = (RGBColor*)(pData + (y*blocksize) * pitch);
			RGBColor Color = CurRow[x*blocksize];
			pixval[pix] = (Color.r + Color.g + Color.b) / 3;
			pix++;
		}				
	}
	//pixvals 32->55
	for (x = 0; x < 2; x++)
	{
		for (y = 4; y < 16; y++)
		{
			RGBColor* CurRow = (RGBColor*)(pData + (y*blocksize) * pitch);
			RGBColor Color = CurRow[x*blocksize];
			pixval[pix] = (Color.r + Color.g + Color.b) / 3;
			pix++;
		}				
	}
	//pixvals 56->63
	for (x = 3; x < 7; x+=3)
	{
		for (y = 5; y < 15; y+=3)
		{
			RGBColor* CurRow = (RGBColor*)(pData + (y*blocksize) * pitch);
			RGBColor Color = CurRow[x*blocksize];
			pixval[pix] = (Color.r + Color.g + Color.b) / 3;
			pix++;
		}				
	}*/
//int k;
//int m;
		/*String Filename = String("C:\\Users\\Omzy\\Desktop\\preoutput\\") + String::ZeroPad(String(m), 10) + String(".bmp");
		D3DXSaveTextureToFile(Filename.CString(), D3DXIFF_BMP, pTexture, NULL);
		m++;*/
		//ofstream checkfile;
		//checkfile.open ("checkfile.csv", ofstream::out | ofstream::app);

		//checkfile << texturename << "," << hashval << endl;
			//checkfile << lastfieldname << "<-last,this->" << fieldname << ",blocnum:" << blocknum << ",oldbmp:" << oldbmp;
					//checkfile << "FILE NOT FOUND" << endl;
				//checkfile << "LOADED TO MEM" << endl;
		//checkfile.close();

		//string texturename = closesthash(hashval);
		//string texturename = "bghall1a_0";
		/*int substrloc0 = texturename.find_last_of("_");	//parse texturename
		string fieldname0 = texturename.substr(0, substrloc0);*/
				//if (fieldname0 == "bghall1a")
					/*for(UINT y = 0; y < Bmp.Height(); y++)	//These alpha bits mess things up
					{
						for(UINT x = 0; x < Bmp.Width(); x++)
						{
							RGBColor C = Bmp[y][x];
							C.a = 128;
							Bmp[y][x] = C;
						}
					}*/
					//checkfile << "LOADED PNG,";
		/*String Filename = String("C:\\Users\\omzy\\Desktop\\output\\") + String::ZeroPad(String(k), 10) + String(".bmp");
		D3DXSaveTextureToFile(Filename.CString(), D3DXIFF_BMP, pTexture, NULL);
		k++;*/
	/*IDirect3DTexture9* pTexture = (IDirect3DTexture9*)SurfaceHandles[0];
	if (pTexture)
	{
		D3DSURFACE_DESC Desc;
		pTexture->GetLevelDesc(0, &Desc);
		UINT w = Desc.Width;
		UINT h = Desc.Height;
		D3DFORMAT fmt = Desc.Format; //D3DFORMAT::D3DFMT_A8R8G8B8
		//D3DRESOURCETYPE type = Desc.Type; //D3DRTYPE::D3DRTYPE_TEXTURE
		//DWORD usage = Desc.Usage;
		D3DPOOL pool = Desc.Pool; //D3DPOOL::D3DPOOL_MANAGED
		if (w == 256 && h == 256 && fmt == D3DFORMAT::D3DFMT_A8R8G8B8 && pool == D3DPOOL::D3DPOOL_MANAGED)
		{
			D3DLOCKED_RECT Rect;
			pTexture->LockRect(0, &Rect, NULL, 0);
			
			UINT pitch = (UINT)Rect.Pitch;
			BYTE* pData = (BYTE*)Rect.pBits;

			setpixvals(pData, pitch);
			uint64_t hashval = gethash();

			string texturename = closesthash(hashval);
			
			if (texturename != "NO_MATCH")
			{
				int substrloc = texturename.find_last_of("_");	//parse texturename
				string fieldname = texturename.substr(0, substrloc);
				String filename = String("C:\\Users\\omzy\\Desktop\\textures\\") + String(fieldname.c_str()) + ".png";	//into filename
				int blocknum = atoi(texturename.substr(substrloc + 1, texturename.length()).c_str());	//and blocknum

				ofstream checkfile;
				checkfile.open ("checkfile.csv", ofstream::out | ofstream::app);

				checkfile << lastfieldname << "<-last,this->" << fieldname << ",blocnum:" << blocknum << ",scenestart:" << scenestart << ",beginscene" << beginscene << ",oldbmp:" << oldbmp;
				beginscene = false;
				if (scenestart)	//Load field file
				{
					if (fieldname != lastfieldname)	//Don't reload same file every frame
					{
						ifstream ifile(filename.CString());
						if (ifile.fail()) //No file, allow normal SetTexture
						{
							//scenestart = false;
							pTexture->UnlockRect(0);
							checkfile << "FILE NOT FOUND" << endl;
							oldbmp = true;
							return;
						} 
						else
						{
							Bmp.LoadPNG(filename);
							for(UINT y = 0; y < Bmp.Height(); y++)
							{
								for(UINT x = 0; x < Bmp.Width(); x++)
								{
									RGBColor C = Bmp[y][x];
									C.a = 128;
									Bmp[y][x] = C;
								}
							}
							checkfile << "LOADED PNG,";
							oldbmp = false;
							lastfieldname = fieldname;
						}
					} else { oldbmp = false; } //needed?
					scenestart = false;
				}			

				//pTexture still locked, have pData (Rect.pBits)
				//only replace 1st block of 256x256 VRAM	//Applies to regular size texture
				if (!oldbmp)
				{
					for(UINT y = 0; y < Bmp.Height(); y++)
					{
						RGBColor* CurRow = (RGBColor *)(pData + y * Rect.Pitch);
						UINT offset = blocknum * Bmp.Height() / 2;
						for(UINT x = offset; (x < offset + Bmp.Height()) && (x < Bmp.Width()); x++)
						{
							RGBColor Color = Bmp[y][x];
							CurRow[x - offset] = RGBColor(Color.b, Color.g, Color.r, Color.a);
						}
					}
					checkfile << "LOADED TO MEM" << endl;
				}
				checkfile.close();
			}
			pTexture->UnlockRect(0);
		}
	}*/
			/*LPDIRECT3DDEVICE9 Device = g_Context->Graphics.Device();
			UINT* curpalette = NULL;
			PALETTEENTRY* palette = NULL;
			HRESULT hr = Device->GetCurrentTexturePalette(curpalette);
			if (hr == D3D_OK)
			{
				hr = Device->GetPaletteEntries(curpalette[0], palette);
				if (hr == D3D_OK)
				{
					BYTE palred = palette[0].peRed; BYTE palgreen = palette[0].peGreen; BYTE palblue = palette[0].peBlue;
					checkfile << palred << "R " << palgreen << "G " << palblue << "B " << endl;
				}
			}*/

/*int i = rand() % 1000 + 1;
	if (i > 990)
	{
		String Filename = String("C:\\Users\\Osamarah\\Desktop\\output\\") + String::ZeroPad(String(k), 10) + String(".png");
		k++;
		if (k > 1000) { k = 0; }
		BitmapSaveOptions Options;
		Options.SaveAlpha = true;
		Options.UseBGR = true;
		Bmp.SavePNG(Filename.CString(), Options);
	}*/
/*uint64_t growhash (int pixval[], uint64_t hashval, int columns)
{
    int lastpixel = 127;
    for (int i = 0; i < 16; i++)
    {
        if      (i < 1      ) { lastpixel = 127;                    }
        else if (i < columns) { lastpixel = pixval[i+(15-columns)]; }
        else if (i < 16     ) { lastpixel = pixval[i-columns];      }
        hashval *= 2;
		if ((pixval[i] - lastpixel) < 0)
		{
			//nothing
		} else {
			hashval++;
		}
    }
    return hashval;
}*/
			/*int pixval[16];
			for (int pix = 0; pix < 16; pix++)
			{
				UINT y = pix * 16;
				RGBColor* CurRow = (RGBColor*)(pData + y * pitch);
				RGBColor Color = CurRow[pix * 8];
				pixval[pix] = (Color.r + Color.g + Color.b) / 3;
			}
			hashval = growhash(pixval, hashval, 2*2*2*2);
			hashval = growhash(pixval, hashval, 2*2*2*1);
			hashval = growhash(pixval, hashval, 2*2*1*1);
			hashval = growhash(pixval, hashval, 2*1*1*1);*/

    /*int lastpixel = 127;
    for (int i = 0; i < 16; i++)
    {
        if      (i < 1      ) { lastpixel = 127;                    }
        else if (i < columns) { lastpixel = pixval[i+(15-columns)]; }
        else if (i < 16     ) { lastpixel = pixval[i-columns];      }
        hashval *= 2;
		if ((pixval[i] - lastpixel) < 0)
		{
			//nothing
		} else {
			hashval++;
		}
    }*/

		//LPDIRECT3DDEVICE9 Device = g_Context->Graphics.Device();
	/*int i = rand() % 1000 + 1;
	if (i > 990)
	{
		IDirect3DTexture9* pTexture = (IDirect3DTexture9*)SurfaceHandles[0];	
		String Filename = String("C:\\Users\\Osamarah\\Desktop\\output\\") + String::ZeroPad(String(j), 10) + String(".bmp");
		D3DXSaveTextureToFile(Filename.CString(), D3DXIFF_BMP, pTexture, NULL);
		j++;
		if (j > 1000) { j = 0; }
	}*/
	//LPDIRECT3DDEVICE9 Device = g_Context->Graphics.Device();
	/*Bitmap Bmp;
	Bmp.LoadPNG("C:\\Users\\Osamarah\\Desktop\\redcircle.png");
	for(UINT y = 0; y < Bmp.Height(); y++)
	{
		for(UINT x = 0; x < Bmp.Width(); x++)
		{
			RGBColor C = Bmp[y][x];
			C.a = 128;
			Bmp[y][x] = C;
		}
	}
	DWORD Usage = D3DUSAGE_AUTOGENMIPMAP;
	D3DPOOL Pool = D3DPOOL_MANAGED;
	D3DFORMAT Format = D3DFMT_A8R8G8B8;
	Device->CreateTexture(Bmp.Width(), Bmp.Height(), 0, Usage, Format, Pool, &xxxpTexture, NULL);
	D3DLOCKED_RECT Rect;
	xxxpTexture->LockRect(0, &Rect, NULL, 0);
    BYTE *Bytes = (BYTE *)Rect.pBits;
    for(UINT y = 0; y < Bmp.Height(); y++)
    {
        RGBColor *CurRow = (RGBColor *)(Bytes + y * Rect.Pitch);
        for(UINT x = 0; x < Bmp.Width(); x++)
        {
            RGBColor Color = Bmp[y][x];
            CurRow[x] = RGBColor(Color.b, Color.g, Color.r, Color.a);
        }
    }
    xxxpTexture->UnlockRect(0);*/


		/*Bmp.LoadPNG("C:\\Users\\Osamarah\\Desktop\\redcircle.png");
	for(UINT y = 0; y < Bmp.Height(); y++)
	{
		for(UINT x = 0; x < Bmp.Width(); x++)
		{
			RGBColor C = Bmp[y][x];
			C.a = 128;
			Bmp[y][x] = C;
		}
	}*/

	/*int i = rand() % 1000 + 1;
	String Filename = String("C:\\Users\\Osamarah\\Desktop\\output\\") + String::ZeroPad(String(i), 10) + String(".png");
    BitmapSaveOptions Options;
    Options.SaveAlpha = true;
    Options.UseBGR = true;
    Bmp.SavePNG(Filename, Options);	*/
	//LPDIRECT3DDEVICE9 Device = g_Context->Graphics.Device();
	/*IDirect3DTexture9* pTexture = (IDirect3DTexture9*)SurfaceHandles[0];
	LPDIRECT3DTEXTURE9 texture = (LPDIRECT3DTEXTURE9)&pTexture;*/
	//texture->

	/*Device->CreateTexture(Bmp.Width(), Bmp.Height(), 0, Usage, Format, Pool, &xxxpTexture, NULL);
	D3DLOCKED_RECT Rect;
	xxxpTexture->LockRect(0, &Rect, NULL, 0);
    BYTE *Bytes = (BYTE *)Rect.pBits;
    for(UINT y = 0; y < Bmp.Height(); y++)
    {
        RGBColor *CurRow = (RGBColor *)(Bytes + y * Rect.Pitch);
        for(UINT x = 0; x < Bmp.Width(); x++)
        {
            RGBColor Color = Bmp[y][x];
            CurRow[x] = RGBColor(Color.b, Color.g, Color.r, Color.a);
        }
    }
    xxxpTexture->UnlockRect(0);


	LPDIRECT3DSURFACE9 TopLevelSurface[1];
    BaseTexture->GetSurfaceLevel(0, &TopLevelSurface[0]);
	D3DSURFACE_DESC Desc;
	TopLevelSurface[0]->GetDesc(&Desc);
    RECT Rect;
    Rect.left = 0;
	Rect.right = Desc.Width;
	Rect.top = 0;
	Rect.bottom = Desc.Height;
	Bitmap Bmp;
	TopLevelSurface[0]->*/

		//LPDIRECT3DSURFACE9 pSurface = (LPDIRECT3DSURFACE9)&SurfaceHandles[0];

	/*LPDIRECT3DSURFACE9 TopLevelSurface = NULL;
	LPDIRECT3DSURFACE9 mSurface = NULL;
	D3DSURFACE_DESC Desc;
	D3DLOCKED_RECT LockedRect;
	RECT Rect;
	if (BaseTexture) { BaseTexture->GetSurfaceLevel(0, &TopLevelSurface); }
	if (TopLevelSurface) {
		TopLevelSurface->GetDesc(&Desc);		
		Rect.left = 0;
		Rect.right = Desc.Width;
		Rect.top = 0;
		Rect.bottom = Desc.Height;
		BaseTexture->LockRect(0, &LockedRect, &Rect, NULL);

		Bitmap Bmp = Bitmap(Desc.Width, Desc.Height);
		RGBColor *SurfData = (RGBColor*)LockedRect.pBits;
		for(UINT y = 0; y < Desc.Height; y++)
		{
			memcpy(Bmp[Desc.Height - 1 - y], &SurfData[y * LockedRect.Pitch / 4], Desc.Width * 4);
		}
		int i = rand() % 1000 + 1;
		String Filename = String("C:\\Users\\Osamarah\\Desktop\\output\\") + String::ZeroPad(String(i), 10) + String(".png");
		BitmapSaveOptions Options;
		Options.SaveAlpha = true;
		Options.UseBGR = true;
		Bmp.SavePNG(Filename, Options);
	}*/


		/*Bitmap Bmp = Bitmap(Desc.Width, Desc.Height);
		Bmp.Clear(RGBColor::Magenta);
		RGBColor *SurfData = (RGBColor*)LockedRect.pBits;
		for(UINT y = 0; y < Desc.Height; y++)
		{
			memcpy(Bmp[Desc.Height - 1 - y], &SurfData[y * LockedRect.Pitch / 4], Desc.Width * 4);
		}
		int i = rand() % 1000 + 1;
		String Filename = String("C:\\Users\\Osamarah\\Desktop\\output\\") + String::ZeroPad(String(i), 10) + String(".png");
		BitmapSaveOptions Options;
		Options.SaveAlpha = true;
		Options.UseBGR = true;
		Bmp.SavePNG(Filename, Options);
		mSurface->UnlockRect();





	LPDIRECT3DSURFACE9 TopLevelSurface = NULL;
	LPDIRECT3DSURFACE9 mSurface = NULL;
	D3DSURFACE_DESC Desc;
	D3DLOCKED_RECT LockedRect;
	RECT Rect;
	if (BaseTexture) { BaseTexture->GetSurfaceLevel(0, &TopLevelSurface); }
	if (TopLevelSurface) {
		TopLevelSurface->GetDesc(&Desc);		
		Rect.left = 0;
		Rect.right = Desc.Width;
		Rect.top = 0;
		Rect.bottom = Desc.Height;
		D3DXLoadSurfaceFromSurface(mSurface, NULL, &Rect, TopLevelSurface, NULL, &Rect, D3DX_FILTER_POINT, D3DCOLOR_XRGB(0, 0, 0));		
	}
	if (mSurface) {
		mSurface->LockRect(&LockedRect, &Rect, NULL);
		Bitmap Bmp = Bitmap(Desc.Width, Desc.Height);
		Bmp.Clear(RGBColor::Magenta);
		RGBColor *SurfData = (RGBColor*)LockedRect.pBits;
		for(UINT y = 0; y < Desc.Height; y++)
		{
			memcpy(Bmp[Desc.Height - 1 - y], &SurfData[y * LockedRect.Pitch / 4], Desc.Width * 4);
		}
		int i = rand() % 1000 + 1;
		String Filename = String("C:\\Users\\Osamarah\\Desktop\\output\\") + String::ZeroPad(String(i), 10) + String(".png");
		BitmapSaveOptions Options;
		Options.SaveAlpha = true;
		Options.UseBGR = true;
		Bmp.SavePNG(Filename, Options);
		mSurface->UnlockRect();
	}*/

	//if (TopLevelSurface) { TopLevelSurface->Release(); }
	//delete TopLevelSurface;

	//(IDirect3DDevice9*)Device->BitmapFromSurface(TopLevelSurface, Rect, &Bmp);

	/*int i = rand() % 100 + 1;
	if (i > 95)
	{
		//LPDIRECT3DDEVICE9 Device = g_Context->Graphics.Device();
		//LPDIRECT3DTEXTURE9 pTexture = NULL;		
		//D3DXCreateTextureFromFile(Device, "C:\\Users\\Osamarah\\Desktop\\redcircle.png", &pTexture);
		//Device->SetTexture(0,pTexture);
		//Device->SetTexture(0,xxxpTexture);
		//Device->SetTexture(1,xxxpTexture);
		//Device->SetTexture(2,xxxpTexture);
	}*/

//EXTRA
/*for(UINT y = 0; y < h; y++)
		{
			RGBColor* CurRow = (RGBColor*)(pData + y * pitch);
			for(UINT x = 0; x < w; x++)
			{
				RGBColor Color = CurRow[x];
				(Color.r + Color.g + Color.b) / 3;
				//CurRow[x] = RGBColor(Color.b, Color.g, Color.r, Color.a); //bgra writing to surface, argb from surface
			}
		}*/

/*ofstream duplicatefile;
	duplicatefile.open ("duplicates.csv", ofstream::out | ofstream::app);

	ifstream hashfile;
	hashfile.open ("hashfile.csv", ifstream::in);

	string line;
	if (hashfile.is_open() && duplicatefile.is_open())
	{
		map<uint64_t, string> hashmap; //6245 = total number of 128x256 texture blocks in ff8
		while ( getline(hashfile, line) )
		{
			int comma = line.find(",");
			string key = line.substr(0, comma);
			string valuestr = line.substr(comma + 1, line.length()).c_str();
			uint64_t value = (uint64_t)ToNumber<unsigned long long>(valuestr);
			pair<map<uint64_t, string>::iterator,bool> ret;
			ret = hashmap.insert(pair<uint64_t, string>(value, key)); //key, value for unique names, value, key for unique hashvals
			if (ret.second == false)
			{
				//duplicatefile << line << "," << valuestr << "," << value << endl;
				duplicatefile << ret.first->second << "," << key << "," << value << endl;
			}
		}
		duplicatefile.close();
		hashfile.close();
	}*/
/*void GraphicsInfo::CheckWindowSize()
{
    BOOL Success = GetClientRect(_PresentParameters.hDeviceWindow, &_ClientRect);
    Assert(Success != 0, "GetClientRect failed");

    Success = GetWindowRect(_PresentParameters.hDeviceWindow, &_WindowRect);
    Assert(Success != 0, "GetWindowRect failed");

    _WindowDimensions = Vec2i(_ClientRect.right, _ClientRect.bottom);
    g_Context->Constants.Resize(_WindowDimensions);
}

void GraphicsInfo::SetOverlay(ID3D9DeviceOverlay *Overlay)
{
    _Overlay = Overlay;
}

void FileList::Init(const String &BaseDirectory)
{
#if !defined(ULTRA_FAST) && defined(WRITING_TO_FILES)
    /Events.open((BaseDirectory + String("Events.txt")).CString());
    Assert.open((BaseDirectory + String("Assert.txt")).CString());
    Thread.open((BaseDirectory + String("Threads.txt")).CString());
    Thought.open((BaseDirectory + String("Thoughts.txt")).CString());
    Actions.open((BaseDirectory + String("Actions.txt")).CString());
#endif
}

void GlobalContext::WriteConsole(const String &Text, RGBColor Color, OverlayPanelType Panel)
{
    if(Graphics.OverlayValid())
    {
        Graphics.Overlay().WriteLine(Text, Color, UINT(Panel));
    }
}

void GlobalContext::ReportAction(const String &Description, RGBColor Color)
{
	if(g_Context->Controller.ConsoleEnabled())
	{
		WriteConsole(Description, Color, OverlayPanelActions);
	}
    g_Context->Files.Actions << Description << endl;
}*/