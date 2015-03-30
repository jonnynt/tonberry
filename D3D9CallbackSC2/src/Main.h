//
// Main.h
//
// Precompiled header for the D3D9 Wrapper
// Written by Matthew Fisher
//

#pragma once

#define NOMINMAX

#include "Config.h"
#include "CompileOptions.h"

#include <windows.h>
#include <WinUser.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string>
#include <fstream>

namespace D3D9Base
{
#include <d3d9.h>
#include <d3dx9.h>
}
using namespace D3D9Base;

#define ULONG ::ULONG
#define SHORT ::SHORT
#define UCHAR ::UCHAR

#include "BigInteger.h"
#include "Engine.h"
#include "DisplayOptions.h"
#include "d3d9CallbackStructures.h"
#include "GlobalContext.h"


