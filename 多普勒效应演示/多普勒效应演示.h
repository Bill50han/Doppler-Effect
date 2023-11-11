#pragma once

#include "resource.h"

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <mmsystem.h>
#include <dsound.h>
#include <math.h>

#pragma comment (lib,"dsound.lib")
#pragma comment (lib,"Winmm.lib")

// Copy from MSDN   
#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#define IDC_START 200
#define IDC_MOVE  201
