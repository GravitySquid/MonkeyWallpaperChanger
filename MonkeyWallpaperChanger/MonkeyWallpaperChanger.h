#pragma once
#define CURL_STATICLIB

#include "resource.h"
#include <filesystem>
#include <iostream>
#include <chrono>
#include <thread>
#include <string>
#include <vector>
#include <fstream>
#include "Timer.h"
#include <shlobj.h>
#include <sstream>
#include <random>
#include <WinUser.h>
//#include <curl/curl.h>

#define CHANGE_SECONDS 10
#define MAX_IMAGES 50000
#define MAX_COMBO_STRING_LENGTH 500
#define ID_EDIT_LOG		200
#define ID_DDOWN_SETS	201
#define ID_BTN_PAUSE	202

#define WM_WALLPAPERCHANGED (WM_USER + 0)

