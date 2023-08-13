//
// +----------------------------------------------------------+
// + Author: Justin Bannerman
// + Date: July 2023
// + Desc:
// +    Simple desktop Wallpaper changer
// +    - Obtains picture list from Users Picture library
// +    - Update wallpaper to random image
// +    - To Do, grab images from web
// +    Work in progress
// +----------------------------------------------------------+
//

#include "framework.h"
#include "MonkeyWallpaperChanger.h"
using namespace std;
namespace fs = std::filesystem;

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
PROCESS_INFORMATION pi;
Timer* _timer;
bool _start = true;
std::vector<std::wstring> images;
std::default_random_engine generator{static_cast<long unsigned int>(std::chrono::high_resolution_clock::now().time_since_epoch().count())};

HWND hWnd;
HWND hWndEdit;
HWND hWndCombo;

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK       CheckChildControls(HWND, LPARAM);
void	UpdateWallpaper();
void    UpdateLocations();
void    CreateImageList();
void    LogMessage(std::wstring);
void	StartStop();
void	UpdateSelectedSet();
//void	TestCurl();

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	// Initialise Rand
	chrono::steady_clock::time_point prevChangedTime = chrono::steady_clock::now();

	// Initialize global strings
	LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadStringW(hInstance, IDC_MONKEYWALLPAPERCHANGER, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}

	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_MONKEYWALLPAPERCHANGER));

	MSG msg;

	// Main message loop:
	while (GetMessage(&msg, nullptr, 0, 0))
	{

		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int)msg.wParam;
}


//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MONKEYWALLPAPERCHANGER));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_MONKEYWALLPAPERCHANGER);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	// Check desktop size
	RECT desktopRect;
	if (!GetWindowRect(GetDesktopWindow(), &desktopRect))
		return FALSE;
	int windowWidth = 500;
	int windowHeight = 200;
	int posX = desktopRect.right - windowWidth;
	int posY = desktopRect.bottom - windowHeight - 75;

	hInst = hInstance; // Store instance handle in our global variable
	// Main Window
	hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
		posX, posY, windowWidth, windowHeight, nullptr, nullptr, hInstance, nullptr);
	if (!hWnd)
	{
		return FALSE;
	}
	// Child Control - Dropdown List
	RECT rcClient;
	GetClientRect(hWnd, &rcClient);
	HWND hWndCombo = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("COMBOBOX"), NULL, WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | CBS_HASSTRINGS, 10, 10, rcClient.right - 50, 250, hWnd, (HMENU)(int)ID_DDOWN_SETS, NULL, NULL);
	if (!hWndCombo)
	{
		return FALSE;
	}
	SendMessage(hWndCombo, CB_ADDSTRING, 0, (LPARAM)TEXT("(Default - Pictures Folder)"));
	SendMessage(hWndCombo, CB_ADDSTRING, 0, (LPARAM)TEXT("Puppies"));
	SendMessage(hWndCombo, CB_ADDSTRING, 0, (LPARAM)TEXT("Kittens"));
	SendMessage(hWndCombo, CB_ADDSTRING, 0, (LPARAM)TEXT("Anime"));
	SendMessage(hWndCombo, CB_SETCURSEL, 0, 0);

	// Child Control - Stop/Start Button
	HWND hWndButton1 = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("BUTTON"), NULL, WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX | BS_PUSHLIKE, 10, 40, 100, 25, hWnd, (HMENU)(int)ID_BTN_PAUSE, NULL, NULL);
	if (!hWndButton1)
	{
		return FALSE;
	}
	SendMessage(hWndButton1, WM_SETTEXT, 0, (LPARAM)TEXT("Pause"));

	// Child Control - Text Box for messages
	hWndEdit = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("EDIT"), NULL, WS_CHILD | WS_VISIBLE | ES_READONLY | ES_AUTOHSCROLL, 10, rcClient.bottom - 35, rcClient.right - 20, 25, hWnd, (HMENU)(int)ID_EDIT_LOG, NULL, NULL);
	if (!hWndEdit)
	{
		return FALSE;
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	// Populate image file list
	CreateImageList();

	//TestCurl();

	// Start Timer thread to periodically update wallpaper
	std::chrono::milliseconds intervalInSeconds = std::chrono::milliseconds(CHANGE_SECONDS * 1000);
	_timer = new Timer(std::function<void()>(*UpdateWallpaper), intervalInSeconds);
	_timer->start();

	return TRUE;
}


//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	RECT rcClient;

	switch (message)
	{
	case WM_COMMAND:
	{
		int wmId = LOWORD(wParam);
		int wmNC = HIWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			if (_timer != nullptr) _timer->stop();
			DestroyWindow(hWnd);
			break;
		case ID_DDOWN_SETS:
			if (wmNC == CBN_SELCHANGE)
				UpdateSelectedSet();
		case ID_BTN_PAUSE:
			if (wmNC == BN_CLICKED)
				StartStop();
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
	}
	break;
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		// TODO: Add any drawing code that uses hdc here...
		EndPaint(hWnd, &ps);
	}
	break;
	case WM_WALLPAPERCHANGED:
		std::wstring* fileNam;
		fileNam = (std::wstring*)lParam;
		LogMessage(L"Display> " + *fileNam);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_SIZE:
		GetClientRect(hWnd, &rcClient);
		EnumChildWindows(hWnd, CheckChildControls, (LPARAM)&rcClient);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

BOOL CALLBACK CheckChildControls(HWND hwndChild, LPARAM lParam)
{
	LPRECT rcParent;
	int idChild;

	// Reset position of Child Control
	idChild = GetWindowLong(hwndChild, GWL_ID);
	RECT rcClient;
	GetClientRect(hWnd, &rcClient);

	switch (idChild)
	{
	case ID_DDOWN_SETS:
		MoveWindow(hwndChild, 10, 10, rcClient.right - 10, 250, TRUE);
		break;
	case ID_BTN_PAUSE:
		MoveWindow(hwndChild, 10, 40, 100, 25, TRUE);
		//MoveWindow(hwndChild, 10, rcClient.bottom - 65, 100, 25, TRUE);
		break;
	case ID_EDIT_LOG:
		MoveWindow(hwndChild, 10, rcClient.bottom - 35, rcClient.right - 10, 25, TRUE);
		break;
	default:
		break;
	}

	ShowWindow(hwndChild, SW_SHOW);

	return TRUE;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

void UpdateLocations()
{
}


void CreateImageList()
{
	// Get Windows Pictures folder
	wchar_t* picturesPath;
	HRESULT result = SHGetKnownFolderPath(FOLDERID_Pictures, 0, NULL, &picturesPath);
	if (FAILED(result) || picturesPath == NULL)
	{
		LogMessage(L"Could not find Picture Path");
		return;
	}

	wstring wPath(picturesPath);
	LogMessage(L"Checking Picture Path - " + wPath);

	// Read recursively through Pictures folder to get image list
	int count = 0;
	fs::path folder = picturesPath;
	for (fs::recursive_directory_iterator itr(folder), end; itr != end; ++itr)
	{
		if (!fs::is_directory(itr->path()))
		{
			wstring fileName = itr->path();
			wstring suffix = fileName.substr(fileName.length() - 4);
			std::transform(suffix.begin(), suffix.end(), suffix.begin(), ::tolower);
			if (suffix == L".jpg" || suffix == L".bmp" || suffix == L".gif") {
				images.push_back(fileName);
				wstring wName(fileName);
				LogMessage(L"Add>> " + fileName);
				count++;
			}
		}
		if (count >= MAX_IMAGES)
			break;
	}
	images.shrink_to_fit();
	LogMessage(L"Images Loaded = " + to_wstring(count));
}

void UpdateWallpaper()
{
	// NOTE: UpdateWallpaper is called from separate Timer thread

	// Pick Random image from list
	int numImages = images.size();
	std::uniform_int_distribution<int> distribution(0, numImages);
	int	i = distribution(generator);

	// Change Wallpaper
	SystemParametersInfo(SPI_SETDESKWALLPAPER, 0, (LPVOID)images[i].c_str(), SPIF_UPDATEINIFILE);

	// Let main thread know the new image filename being displayed
	WaitForSingleObject(pi.hThread, 10000);
	PostMessage(hWnd, WM_WALLPAPERCHANGED, 0, (LPARAM)&images[i]);
}

string convertToString(char* a, int size)
{
	int i;
	string s = "";
	for (i = 0; i < size; i++) {
		s = s + a[i];
	}
	return s;
}

void UpdateSelectedSet()
{
	// Find index of selected set, in the drop down list
	int index = static_cast<int>(SendDlgItemMessage(hWnd, ID_DDOWN_SETS, CB_GETCURSEL, 0, 0));
	int len = static_cast<int>(SendDlgItemMessage(hWnd, ID_DDOWN_SETS, CB_GETLBTEXTLEN, 0, 0));

	wchar_t buf[MAX_COMBO_STRING_LENGTH];
	if (len < MAX_COMBO_STRING_LENGTH - 1)
	{
		SendDlgItemMessage(hWnd, ID_DDOWN_SETS, CB_GETLBTEXT, index, (LPARAM)(LPCTSTR)buf);
		// buffer contains the selected text unless CB_GETCURSEL returned CB_ERR (-1) to the index
		
		buf[len + 1] = '\0'; // Ensure zero terminated array
		std::wstring msg(buf);
		LogMessage(msg);
	}
}

void LogMessage(std::wstring msg)
{
	SendMessage(hWndEdit, WM_SETTEXT, 0, (LPARAM)msg.c_str());
}

void StartStop()
{
	_start = !_start;
	if (!_start)
		_timer->stop();
	else
		_timer->start();
}

//void TestCurl()
//{
//	CURL* curl;
//	CURLcode res;
//
//	/* In windows, this will init the winsock stuff */
//	curl_global_init(CURL_GLOBAL_ALL);
//
//	/* get a curl handle */
//	curl = curl_easy_init();
//	if (curl) {
//		/* First set the URL that is about to receive our POST. This URL can
//		   just as well be an https:// URL if that is what should receive the
//		   data. */
//		curl_easy_setopt(curl, CURLOPT_URL, "http://postit.example.com/moo.cgi");
//		/* Now specify the POST data */
//		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, "name=daniel&project=curl");
//
//		/* Perform the request, res will get the return code */
//		res = curl_easy_perform(curl);
//		/* Check for errors */
//		if (res != CURLE_OK)
//			fprintf(stderr, "curl_easy_perform() failed: %s\n",
//				curl_easy_strerror(res));
//
//		/* always cleanup */
//		curl_easy_cleanup(curl);
//	}
//	curl_global_cleanup();
//}