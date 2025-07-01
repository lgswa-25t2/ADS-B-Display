//---------------------------------------------------------------------------

// Prevent WinCrypt defines from being overridden by OpenSSL
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define NOGDI
#define NOUSER

// Suppress WinCrypt warnings and prevent OpenSSL conflicts
#pragma warning(disable: 4005) // macro redefinition
#pragma warning(disable: 4996) // deprecated functions
#pragma warning(disable: 4091) // typedef ignored

// Prevent OpenSSL from overriding Windows crypto definitions
#define OPENSSL_NO_WINCRYPT
#define OPENSSL_NO_ENGINE
#define OPENSSL_NO_DYNAMIC_ENGINE
#define OPENSSL_NO_EC
#define OPENSSL_NO_DH
#define OPENSSL_NO_DSA

// Define Windows crypto types before OpenSSL includes them
#ifndef WINCRYPT_DEFINED
#define WINCRYPT_DEFINED
#define CRYPT_STRING_BASE64 0x00000001
#define CRYPT_STRING_HEX 0x00000004
#define CRYPT_STRING_HEXRAW 0x0000000c
#define CRYPT_STRING_HEXASCII 0x00000005
#define CRYPT_STRING_BASE64HEADER 0x00000000
#define CRYPT_STRING_BASE64REQUESTHEADER 0x00000003
#define CRYPT_STRING_HEXADDR 0x0000000a
#define CRYPT_STRING_HEXASCIIADDR 0x0000000b
#define CRYPT_STRING_HEXRAWADDR 0x0000000e
#define CRYPT_STRING_BASE64X509CRLHEADER 0x00000009
#define CRYPT_STRING_HEXASCIIADDR 0x0000000b
#define CRYPT_STRING_HEXRAWADDR 0x0000000e
#define CRYPT_STRING_HEXRAW 0x0000000c
#define CRYPT_STRING_HEXASCII 0x00000005
#define CRYPT_STRING_HEXADDR 0x0000000a
#define CRYPT_STRING_HEXASCIIADDR 0x0000000b
#define CRYPT_STRING_HEXRAWADDR 0x0000000e
#define CRYPT_STRING_BASE64X509CRLHEADER 0x00000009
#define CRYPT_STRING_HEXASCIIADDR 0x0000000b
#define CRYPT_STRING_HEXRAWADDR 0x0000000e
#endif

// Include Windows headers first
//inlude <windows.h>
//nclude <wincrypt.h>

// Then include VCL and other headers
#include <vcl.h>
#include <new>
#include <math.h>
#include <dir.h>
#include <float.h>
#include <stdio.h>
#include <stdlib.h>
#include <filesystem>
#include <fileapi.h>
#include <chrono>
#include <ShellAPI.h>
#include <IniFiles.hpp>

#pragma hdrstop

// Include project headers last
#include "DisplayGUI.h"
#include "AreaDialog.h"
#include "ntds2d.h"
#include "LatLonConv.h"
#include "PointInPolygon.h"
#include "DecodeRawADS_B.h"
#include "ght_hash_table.h"
#include "dms.h"
#include "Aircraft.h"
#include "TimeFunctions.h"
#include "SBS_Message.h"
#include "CPA.h"
#include "AircraftDB.h"
#include "AircraftAPI.h"
#include "csv.h"
#include "AirportDB.h"
#include "AirportDataManager.h"
#include "Logger.h"

#define AIRCRAFT_DATABASE_URL "https://opensky-network.org/datasets/metadata/aircraftDatabase.zip"
#define AIRCRAFT_DATABASE_FILE "aircraftDatabase.csv"
#define ARTCC_BOUNDARY_FILE "Ground_Level_ARTCC_Boundary_Data_2025-05-15.csv"

#define MAP_CENTER_LAT 40.73612;
#define MAP_CENTER_LON -80.33158;

#define BIG_QUERY_UPLOAD_COUNT 50000
#define BIG_QUERY_RUN_FILENAME "SimpleCSVtoBigQuery.py"
#define LEFT_MOUSE_DOWN 1
#define RIGHT_MOUSE_DOWN 2
#define MIDDLE_MOUSE_DOWN 4

#define PROGRESSBAR_RELEASE_MODE true
// #define PROGRESSBAR_DEBUGGING

#define BG_INTENSITY 0.37
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma link "OpenGLPanel"
#pragma link "Map\libgefetch\Win64\Release\libgefetch.a"
#pragma link "Map\zlib\Win64\Release\zlib.a"
#pragma link "Map\jpeg\Win64\Release\jpeg.a"
#pragma link "Map\png\Win64\Release\png.a"
#pragma link "HashTable\Lib\Win64\Release\HashTableLib.a"
#pragma link "cspin"
#pragma resource "*.dfm"
TForm1 *Form1;
//---------------------------------------------------------------------------
static void RunPythonScript(AnsiString scriptPath, AnsiString args);
static bool DeleteFilesWithExtension(AnsiString dirPath, AnsiString extension);
static int FinshARTCCBoundary(void);
//---------------------------------------------------------------------------

static char *stristr(const char *String, const char *Pattern);
static const char *strnistr(const char *pszSource, DWORD dwLength, const char *pszFind);
int globalTrackbarValue = 1;
extern ght_hash_table_t *AircraftDBHashTable;

//---------------------------------------------------------------------------
uint32_t createRGB(uint8_t r, uint8_t g, uint8_t b)
{
    return ((uint32_t)r << 16) | ((uint32_t)g << 8) | b;
}
//---------------------------------------------------------------------------
uint32_t PopularColors[] = {
    createRGB(255, 0, 0),     // Red
    createRGB(0, 255, 0),     // Green
    createRGB(0, 0, 255),     // Blue
    createRGB(255, 255, 0),   // Yellow
    createRGB(255, 165, 0),   // Orange
    createRGB(255, 192, 203), // Pink
    createRGB(0, 255, 255),   // Cyan
    createRGB(255, 0, 255),   // Magenta
    createRGB(255, 255, 255), // White
    // createRGB(0, 0, 0),        // Black
    createRGB(128, 128, 128), // Gray
    createRGB(165, 42, 42)    // Brown
};

int NumColors = sizeof(PopularColors) / sizeof(PopularColors[0]);
unsigned int CurrentColor = 0;
__int64 LastHeartbeatTime = 0;
bool RawTimeoutPopupShown = true;
__int64 LastSBSDataReceiveTime = 0;
bool SBSTimeoutPopupShown = true;
bool RawConnectionLostShown; // Raw 연결 끊김 팝업 표시 여부
bool SBSConnectionLostShown; // SBS 연결 끊김 팝업 표시 여부

//---------------------------------------------------------------------------
typedef struct
{
    union
    {
        struct
        {
            System::Byte Red;
            System::Byte Green;
            System::Byte Blue;
            System::Byte Alpha;
        };
        struct
        {
            TColor Cl;
        };
        struct
        {
            COLORREF Rgb;
        };
    };

} TMultiColor;

//---------------------------------------------------------------------------
static const char *strnistr(const char *pszSource, DWORD dwLength, const char *pszFind)
{
    DWORD dwIndex = 0;
    DWORD dwStrLen = 0;
    const char *pszSubStr = NULL;

    // check for valid arguments
    if (!pszSource || !pszFind)
    {
        return pszSubStr;
    }

    dwStrLen = strlen(pszFind);

    // can pszSource possibly contain pszFind?
    if (dwStrLen > dwLength)
    {
        return pszSubStr;
    }

    while (dwIndex <= dwLength - dwStrLen)
    {
        if (0 == strnicmp(pszSource + dwIndex, pszFind, dwStrLen))
        {
            pszSubStr = pszSource + dwIndex;
            break;
        }

        dwIndex++;
    }

    return pszSubStr;
}
//---------------------------------------------------------------------------
static char *stristr(const char *String, const char *Pattern)
{
    char *pptr, *sptr, *start;
    size_t slen, plen;

    for (start = (char *)String, pptr = (char *)Pattern, slen = strlen(String), plen = strlen(Pattern);
         slen >= plen; start++, slen--)
    {
        /* find start of pattern in string */
        while (toupper(*start) != toupper(*Pattern))
        {
            start++;
            slen--;

            /* if pattern longer than string */

            if (slen < plen)
                return (NULL);
        }

        sptr = start;
        pptr = (char *)Pattern;

        while (toupper(*sptr) == toupper(*pptr))
        {
            sptr++;
            pptr++;

            /* if end of pattern then pattern was found */

            if ('\0' == *pptr)
                return (start);
        }
    }
    return (NULL);
}
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
__fastcall TForm1::TForm1(TComponent *Owner)
    : TForm(Owner)
{
    // 키 이벤트를 Form에서 받을 수 있도록 설정
    this->KeyPreview = true;
    this->OnKeyDown = FormKeyDown;
    this->OnKeyUp = FormKeyUp;

    // Initialize IP history
    LoadIpHistory();

    // Initialize cache cleanup time
    lastCleanupTime = std::chrono::system_clock::now();

    // Create vertical scrollbar
    MapVScrollBar = new TScrollBar(this);
    MapVScrollBar->Parent = this; // 스크롤바를 폼에 직접 추가 (ObjectDisplay와 같은 레벨)
    MapVScrollBar->Kind = sbVertical;
    MapVScrollBar->Align = alNone;  // 자동 정렬 비활성화
    MapVScrollBar->Width = 17;      // 스크롤바 너비 설정
    MapVScrollBar->Visible = false; // 처음에는 숨김
    MapVScrollBar->TabStop = false; // 키 이벤트를 가로채지 않도록 설정
    MapVScrollBar->OnScroll = MapVScrollBarScroll;

    // Create horizontal scrollbar
    MapHScrollBar = new TScrollBar(this);
    MapHScrollBar->Parent = this; // 스크롤바를 폼에 직접 추가 (ObjectDisplay와 같은 레벨)
    MapHScrollBar->Kind = sbHorizontal;
    MapHScrollBar->Align = alNone;  // 자동 정렬 비활성화
    MapHScrollBar->Height = 17;     // 스크롤바 높이 설정
    MapHScrollBar->Visible = false; // 처음에는 숨김
    MapHScrollBar->TabStop = false; // 키 이벤트를 가로채지 않도록 설정
    MapHScrollBar->OnScroll = MapHScrollBarScroll;

    // ObjectDisplay는 원래 부모로 복원 (지도 표시용)
    // ObjectDisplay->Parent = Panel7; // 이 줄 제거
    // ObjectDisplay->Align = alClient; // 이 줄 제거
    ObjectDisplay->TabStop = false; // ObjectDisplay가 포커스를 받지 않도록 설정

    AircraftDBPathFileName = ExtractFilePath(ExtractFileDir(Application->ExeName)) + AnsiString("..\\AircraftDB\\") + AIRCRAFT_DATABASE_FILE;
    ARTCCBoundaryDataPathFileName = ExtractFilePath(ExtractFileDir(Application->ExeName)) + AnsiString("..\\ARTCC_Boundary_Data\\") + ARTCC_BOUNDARY_FILE;
    BigQueryPath = ExtractFilePath(ExtractFileDir(Application->ExeName)) + AnsiString("..\\BigQuery\\");
    BigQueryPythonScript = BigQueryPath + AnsiString(BIG_QUERY_RUN_FILENAME);
    DeleteFilesWithExtension(BigQueryPath, "csv");
    BigQueryLogFileName = BigQueryPath + "BigQuery.log";
    DeleteFileA(BigQueryLogFileName.c_str());
    CurrentSpriteImage = 0;
    InitDecodeRawADS_B();
    RecordRawStream = NULL;
    PlayBackRawStream = NULL;
    TrackHook.Valid_CC = false;
    TrackHook.Valid_CPA = false;

    // Initialize BigQuery client
	bigquery_initialized_ = false;
	bigquery_credentials_path_ = BigQueryPath + "YourJsonFile.json";
	bigquery_auto_update_enabled_ = false;
	bigquery_update_interval_seconds_ = 30;
	last_tracked_icao_ = "";
	InitializeBigQuery();

	// Initialize Planned Route
	planned_route_enabled_ = true;
	planned_route_icao_ = "";

	HashTable = ght_create(50000);

    if (!HashTable)
    {
        throw Sysutils::Exception("Create Hash Failed");
    }
    ght_set_rehash(HashTable, TRUE);

    AreaTemp = NULL;
    Areas = new TList;

    MouseDown = false;

    MapCenterLat = MAP_CENTER_LAT;
    MapCenterLon = MAP_CENTER_LON;

    LoadMapFromInternet = LiveMapCheckbox->Checked;
    MapComboBox->ItemIndex = GoogleMaps;
    SelectedMapIndex = GoogleMaps;

    panelsVisible = true;

    Panel4->VertScrollBar->Position = 0;
    Panel4->HorzScrollBar->Position = 0;

    this->ShowHint = true;
    // PanelTitle1에 tooltip 설정
    PanelTitle1->ShowHint = true;
    PanelTitle1->Hint = "Click to hide Control Menu";
    PanelTitle1->Cursor = crHandPoint;

    // Hint 표시 시간 조정 (선택사항)
    Application->HintPause = 500;      // 0.5초 후 표시
    Application->HintHidePause = 5000; // 5초 후 숨김

    LoadMap(MapComboBox->ItemIndex);

    g_EarthView->m_Eye.h /= pow(1.3, 18); // pow(1.3,43);
    SetMapCenter(g_EarthView->m_Eye.x, g_EarthView->m_Eye.y);

    // 스크롤바 초기화
    UpdateScrollBarRanges();
    UpdateScrollBarPositions();

    // 초기화
    InitializePlaybackProgress();

    TimeToGoTrackBar->Position = 120;
    BigQueryCSV = NULL;
    BigQueryRowCount = 0;
    BigQueryFileCount = 0;
    InitAircraftDB(AircraftDBPathFileName);
    printf("init complete\n");
    LOG_ERROR("ADS-B Display Application Started");
    // init airport db
    AnsiString AirportDBFile = ExtractFilePath(ExtractFileDir(Application->ExeName)) + AnsiString("..\\AirportDB\\") + "airports.csv";
    if (FileExists(AirportDBFile))
    {
        if (!InitAirportDB(AirportDBFile))
        {
            ShowMessage("Failed to load Airport Database");
        }
    }
    else
    {
        ShowMessage("Airport Database file not found: " + AirportDBFile);
    }

    // init AirportDataManager
    airportManager = new AirportDataManager();

    // load AirportData from hashtable
    // printf("=== Airport Data Loading Debug ===\n");
    auto airportData = GetAllAirportsFromHashTable();
    // printf("GetAllAirportsFromHashTable returned %d airports\n", (int)airportData.size());

    if (!airportData.empty())
    {
        // print airport info for first 5 airports for debugging
        /*
        for (int i = 0; i < min(5, (int)airportData.size()); i++) {
            printf("Airport %d: ICAO=%s, Name=%s, Lat=%.6f, Lon=%.6f\n",
                   i, airportData[i].ICAO.c_str(), airportData[i].Name.c_str(),
                   airportData[i].Latitude, airportData[i].Longitude);
        } */

        airportManager->loadAirportsFromHashTable(airportData);
        printf("Successfully loaded %d airports into AirportDataManager\n", (int)airportData.size());
    }
    else
    {
        printf("No airport data found in hash table\n");
    }
    // printf("=== End Airport Data Loading Debug ===\n");

    // init Area filter variables
    selectedFilterAreas = new TList();
    areaFilterEnabled = false;
    // Set to initial state
    ClearAircraftInfo();

    lastCleanupTime = std::chrono::system_clock::now();
    // Initial Trackbar Value
    PlaybackSpeedPanel->Visible = false;
    AircraftTypeFilterComboBox->ItemIndex = 0; // "All" 선택
    SelectedAircraftTypeFilter = 0;
    SelectedAltitudeFilter = 0; 
    SelectedSpeedFilter = 0;

    // 거리 계산 스레드 초기화
    aircraftAirportDistanceResult = nullptr;
    distanceCalculationThread = nullptr;

    // 거리 계산 스레드 시작
    startDistanceCalculationThread();

    // 연결 재시도 관련 변수 초기화
    RawConnectionLostShown = false;
    SBSConnectionLostShown = false;
    LastRawConnectionCheck = 0;
    LastSBSConnectionCheck = 0;

    // 초기 연결 상태 표시
    UpdateRawConnectionStatus("Disconnected");
    UpdateSBSConnectionStatus("Disconnected");

    // 항공기 간 거리 계산 스레드 초기화
    aircraftAircraftDistanceResult = nullptr;
    aircraftDistanceCalculationThread = nullptr;

    // 항공기 간 거리 계산 스레드 시작
    startAircraftDistanceCalculationThread();

    // ComboBox 초기화
    AltitudeFilterComboBox->Items->Clear();
    AltitudeFilterComboBox->Items->Add("All Altitudes");
    AltitudeFilterComboBox->Items->Add("35000+ feet");
    AltitudeFilterComboBox->Items->Add("25000-35000 feet");
    AltitudeFilterComboBox->Items->Add("15000-25000 feet");
    AltitudeFilterComboBox->Items->Add("8000-15000 feet");
    AltitudeFilterComboBox->Items->Add("3000-8000 feet");
    AltitudeFilterComboBox->Items->Add("1000-3000 feet");
    AltitudeFilterComboBox->Items->Add("Below 1000 feet");
    AltitudeFilterComboBox->ItemIndex = 0;

    SpeedFilterComboBox->Items->Clear();
    SpeedFilterComboBox->Items->Add("All Speeds");
    SpeedFilterComboBox->Items->Add("500+ knots");
    SpeedFilterComboBox->Items->Add("400-500 knots");
    SpeedFilterComboBox->Items->Add("300-400 knots");
    SpeedFilterComboBox->Items->Add("200-300 knots");
    SpeedFilterComboBox->Items->Add("Below 200 knots");
    SpeedFilterComboBox->ItemIndex = 0;

    // 빠른 클릭 감지 변수 초기화
    rapidClickCount = 0;
    firstClickTime = 0;

    // Initialize Planned Route Manager
    plannedRouteManager = new AircraftPlannedRoute();
}
//---------------------------------------------------------------------------
__fastcall TForm1::~TForm1()
{
    if (Font2DSmall)
    {
        delete Font2DSmall;
        Font2DSmall = NULL;
    }
    // Save IP history before closing
    SaveIpHistory();

    // Clean up IP history
    if (SBSIpHistory)
    {
        delete SBSIpHistory;
        SBSIpHistory = NULL;
    }
    if (RawIpHistory)
    {
        delete RawIpHistory;
        RawIpHistory = NULL;
    }

    Timer1->Enabled = false;
    Timer2->Enabled = false;
    delete g_EarthView;
    if (g_GETileManager)
    {
        delete g_GETileManager;
    }
    delete g_MasterLayer;
    delete g_Storage;
    if (LoadMapFromInternet)
    {
        if (g_Keyhole)
            delete g_Keyhole;
    }
    // cleanup AirportDB
    CleanupAirportDB();

    // delete AirportDataManager
    if (airportManager)
    {
        delete airportManager;
        airportManager = nullptr;
    }

    if (selectedFilterAreas)
    {
        selectedFilterAreas->Clear();
        delete selectedFilterAreas;
    }

    // 거리 계산 스레드 중지
    stopDistanceCalculationThread();

    // 항공기 간 거리 계산 스레드 중지
    stopAircraftDistanceCalculationThread();
}
//---------------------------------------------------------------------------
void __fastcall TForm1::SetMapCenter(double &x, double &y)
{
    double siny;
    x = (MapCenterLon + 0.0) / 360.0;
    siny = sin(MapCenterLat * DEG_TO_RAD);
    siny = fmin(fmax(siny, -0.9999), 0.9999);
    y = (log((1 + siny) / (1 - siny)) / (4 * M_PI));
}
//---------------------------------------------------------------------------
void __fastcall TForm1::ObjectDisplayInit(TObject *Sender)
{
    glViewport(0, 0, (GLsizei)ObjectDisplay->Width, (GLsizei)ObjectDisplay->Height);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glEnable(GL_LINE_STIPPLE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    NumSpriteImages = MakeAirplaneImages();
    MakeAirTrackFriend();
    MakeAirTrackHostile();
    MakeAirTrackUnknown();
    MakePoint();
    MakeTrackHook();
    g_EarthView->Resize(ObjectDisplay->Width, ObjectDisplay->Height);
    glPushAttrib(GL_LINE_BIT);
    glPopAttrib();
    printf("OpenGL Version %s\n", glGetString(GL_VERSION));
    if (LiveMapCheckbox && MapComboBox)
    {
        bool supportsOnline = (MapComboBox->ItemIndex == 0 || MapComboBox->ItemIndex == 4);
        LiveMapCheckbox->Enabled = supportsOnline;
        if (!supportsOnline)
        {
            LiveMapCheckbox->Checked = false;
        }
        LoadMapFromInternet = LiveMapCheckbox->Checked;
    }
    // 작은 폰트 생성
    TFont *smallFont = new TFont();
    smallFont->Name = "Arial";
    smallFont->Style = TFontStyles() << fsBold;
    smallFont->Size = 12; // 작은 크기
    Font2DSmall = ObjectDisplay->Create2DFont(smallFont, 32, 224);
    delete smallFont;
}
//---------------------------------------------------------------------------

void __fastcall TForm1::ObjectDisplayResize(TObject *Sender)
{
    double Value;
    // ObjectDisplay->Width=ObjectDisplay->Height;
    glViewport(0, 0, (GLsizei)ObjectDisplay->Width, (GLsizei)ObjectDisplay->Height);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glEnable(GL_LINE_STIPPLE);
    // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    g_EarthView->Resize(ObjectDisplay->Width, ObjectDisplay->Height);

    // 스크롤바 범위와 위치 업데이트
    UpdateScrollBarRanges();
    UpdateScrollBarPositions();
}
//---------------------------------------------------------------------------
void __fastcall TForm1::ObjectDisplayPaint(TObject *Sender)
{

    if (DrawMap->Checked)
    {
        glClearColor(0.0, 0.0, 0.0, 0.0);
    }
    else
    {
        glClearColor(0.94, 0.94, 0.96, 1.0); // background color
    }
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    g_EarthView->Animate();
    g_EarthView->Render(DrawMap->Checked);
    g_GETileManager->Cleanup();
    Mw1 = Map_w[1].x - Map_w[0].x;
    Mw2 = Map_v[1].x - Map_v[0].x;
    Mh1 = Map_w[1].y - Map_w[0].y;
    Mh2 = Map_v[3].y - Map_v[0].y;

    xf = Mw1 / Mw2;
    yf = Mh1 / Mh2;

    DrawObjects();
}
//---------------------------------------------------------------------------
void __fastcall TForm1::Timer1Timer(TObject *Sender)
{
    __int64 CurrentTime;

    CurrentTime = GetCurrentTimeInMsec();
    SystemTime->Caption = TimeToChar(CurrentTime);

    // monitoring cache status (per 5 secs)
    static __int64 lastCacheCheck = 0;
    if (CurrentTime - lastCacheCheck > 5000)
    {
        if (g_GETileManager)
        {
            int currentTextures = g_GETileManager->GetTextureCount();
            int maxTextures = g_GETileManager->GetMaxTextures();
            /*printf("Cache Status: %d/%d textures (%.1f%% full)\n",
                   currentTextures, maxTextures,
                   (float)currentTextures / maxTextures * 100.0f);*/
        }
        lastCacheCheck = CurrentTime;
    }

    // Check RawData Timeout
    __int64 now = GetCurrentTimeInMsec();
    if (RawConnectButton->Caption == "Raw Disconnect")
    {
        // 연결 상태 체크 (간단한 방법으로 부하 줄임)
        bool isConnected = false;
        static __int64 lastConnectionCheck = 0;

        // 2초마다만 연결 상태 체크 (부하 줄임)
        if (now - lastConnectionCheck > 2000)
        {
            try
            {
                // 기본 Connected() 체크만 사용
                isConnected = Form1->IdTCPClientRaw->Connected();

                // 간단한 소켓 에러 체크만
                if (isConnected && Form1->IdTCPClientRaw->Socket && Form1->IdTCPClientRaw->Socket->Binding)
                {
                    SOCKET sockHandle = Form1->IdTCPClientRaw->Socket->Binding->Handle;
                    if (sockHandle != INVALID_SOCKET)
                    {
                        int optval;
                        int optlen = sizeof(optval);
                        if (getsockopt(sockHandle, SOL_SOCKET, SO_ERROR, (char *)&optval, &optlen) != 0 || optval != 0)
                        {
                            isConnected = false;
                            printf("Socket error detected: %d\n", optval);
                        }
                    }
                    else
                    {
                        isConnected = false;
                    }
                }
                lastConnectionCheck = now;
            }
            catch (...)
            {
                isConnected = false;
                printf("Connection check exception\n");
                lastConnectionCheck = now;
            }
        }
        else
        {
            // 체크 간격이 아닌 경우 이전 결과 사용
            isConnected = Form1->IdTCPClientRaw->Connected();
        }

        // WiFi 끊김/파이 전원 OFF 감지 (연결이 끊어진 경우)
        if (!isConnected)
        {
            if (!RawConnectionLostShown)
            {
                RawConnectionLostShown = true;
                ShowRawConnectionLostDialog(); // Raw 전용 다이얼로그 표시
            }
        }
        // 데이터 없음 감지 (연결은 살아있지만 데이터가 안 옴)
		else if (!RawConnectionLostShown && !RawTimeoutPopupShown && (now - LastHeartbeatTime > 30000)) // 30초
        {
            RawTimeoutPopupShown = true;
            ShowRawTimeoutDialog(); // 타임아웃 다이얼로그 표시 (연결은 유지)
        }
    }

    // Check SBSData Timeout
    if (SBSConnectButton->Caption == "SBS Disconnect")
    {
        // SBS 연결 상태 체크
        bool isSBSConnected = false;

        // 2초마다만 연결 상태 체크 (부하 줄임)
        if (now - LastSBSConnectionCheck > 2000)
        {
            try
            {
                // 기본 Connected() 체크만 사용
                isSBSConnected = Form1->IdTCPClientSBS->Connected();

                // 간단한 소켓 에러 체크만
                if (isSBSConnected && Form1->IdTCPClientSBS->Socket && Form1->IdTCPClientSBS->Socket->Binding)
                {
                    SOCKET sockHandle = Form1->IdTCPClientSBS->Socket->Binding->Handle;
                    if (sockHandle != INVALID_SOCKET)
                    {
                        int optval;
                        int optlen = sizeof(optval);
                        if (getsockopt(sockHandle, SOL_SOCKET, SO_ERROR, (char *)&optval, &optlen) != 0 || optval != 0)
                        {
                            isSBSConnected = false;
                            printf("SBS Socket error detected: %d\n", optval);
                        }
                    }
                    else
                    {
                        isSBSConnected = false;
                    }
                }
                LastSBSConnectionCheck = now;
            }
            catch (...)
            {
                isSBSConnected = false;
                printf("SBS Connection check exception\n");
                LastSBSConnectionCheck = now;
            }
        }
        else
        {
            // 체크 간격이 아닌 경우 이전 결과 사용
            isSBSConnected = Form1->IdTCPClientSBS->Connected();
        }

        // SBS 연결 끊김 감지
        if (!isSBSConnected)
        {
            if (!SBSConnectionLostShown)
            {
                SBSConnectionLostShown = true;
                ShowSBSConnectionLostDialog(); // SBS 전용 다이얼로그 표시
            }
        }
        // SBS 데이터 타임아웃 체크 (기존 로직)
		else if (!SBSConnectionLostShown && !SBSTimeoutPopupShown && (now - LastSBSDataReceiveTime > 30000)) // 30 Sec
        {
            SBSTimeoutPopupShown = true;
            ShowSBSTimeoutDialog(); // 타임아웃 다이얼로그 표시 (연결은 유지)
        }
    }

    // 모든 항공기의 오래된 이동 경로 정리 (5초마다)
    static __int64 lastHistoryPurge = 0;
    if (CurrentTime - lastHistoryPurge > 5000) // 5초마다 실행
    {
        uint32_t *Key;
        ght_iterator_t iterator;
        TADS_B_Aircraft *Data;

        for (Data = (TADS_B_Aircraft *)ght_first(HashTable, &iterator, (const void **)&Key);
             Data; Data = (TADS_B_Aircraft *)ght_next(HashTable, &iterator, (const void **)&Key))
        {
            // Playback 중일 때만 각 항공기의 30초 이상된 이동 경로 삭제
            if (PlayBackSBSStream != NULL)
            {
                PurgeOldHistory(Data, CurrentTime);
            }
        }
        
        lastHistoryPurge = CurrentTime;
    }

    ObjectDisplay->Repaint();
    UpdateRawConnectionStatus(RawConnectButton->Caption);
    UpdateSBSConnectionStatus(SBSConnectButton->Caption);
}
//---------------------------------------------------------------------------
void __fastcall TForm1::DrawObjects(void)
{
    // Cache remove
    cleanupOldCache();

    double ScrX, ScrY;
    int ViewableAircraft = 0;

    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_POINT_SMOOTH);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
    glLineWidth(3.0);
    glPointSize(4.0);

    LatLon2XY(MapCenterLat, MapCenterLon, ScrX, ScrY);
    // draw all airports in the map by checkbox
    DrawAllAirports();
    if (DrawMap->Checked)
    {
        glColor4f(1, 1, 1, 1.0); // white
    }
    else
    {
        glColor4f(0.3, 0.3, 0.3, 0.8); // dark gray
    }

    uint32_t *Key;
    ght_iterator_t iterator;
    TADS_B_Aircraft *Data, *DataCPA;
    TAircraftData *a = NULL;

    DWORD i, j, Count;
    int cell[10][10] = {
        0,
    };
    int cellWidth = ObjectDisplay->Width / 10 - 1;
    int cellHeight = ObjectDisplay->Height / 10 - 1;
    int cellSize = std::min(cellWidth, cellHeight) / 2;
    int cellMin = cellSize / 5;
    int cellMax = cellSize;
    double cellDrawZoomRate = 0.00005;
    double textDrawZoomRate = 0.00003; // threshold for displaying text

    if (AreaTemp)
    {
        glPointSize(3.0);
        for (DWORD i = 0; i < AreaTemp->NumPoints; i++)
            LatLon2XY(AreaTemp->Points[i][1], AreaTemp->Points[i][0],
                      AreaTemp->PointsAdj[i][0], AreaTemp->PointsAdj[i][1]);

        glBegin(GL_POINTS);
        for (DWORD i = 0; i < AreaTemp->NumPoints; i++)
        {
            glVertex2f(AreaTemp->PointsAdj[i][0],
                       AreaTemp->PointsAdj[i][1]);
        }
        glEnd();
        glBegin(GL_LINE_STRIP);
        for (DWORD i = 0; i < AreaTemp->NumPoints; i++)
        {
            glVertex2f(AreaTemp->PointsAdj[i][0],
                       AreaTemp->PointsAdj[i][1]);
        }
        glEnd();
    }
    Count = Areas->Count;
    for (i = 0; i < Count; i++)
    {
        TArea *Area = (TArea *)Areas->Items[i];
        TMultiColor MC;

        MC.Rgb = ColorToRGB(Area->Color);
        if (Area->Selected)
        {
            glLineWidth(4.0);
            glPushAttrib(GL_LINE_BIT);
            glLineStipple(3, 0xAAAA);
        }

        glColor4f(MC.Red / 255.0, MC.Green / 255.0, MC.Blue / 255.0, 1.0);
        glBegin(GL_LINE_LOOP);
        for (j = 0; j < Area->NumPoints; j++)
        {
            LatLon2XY(Area->Points[j][1], Area->Points[j][0], ScrX, ScrY);
            glVertex2f(ScrX, ScrY);
        }
        glEnd();
        if (Area->Selected)
        {
            glPopAttrib();
            glLineWidth(2.0);
        }

        glColor4f(MC.Red / 255.0, MC.Green / 255.0, MC.Blue / 255.0, 0.4);

        for (j = 0; j < Area->NumPoints; j++)
        {
            LatLon2XY(Area->Points[j][1], Area->Points[j][0],
                      Area->PointsAdj[j][0], Area->PointsAdj[j][1]);
        }
        TTriangles *Tri = Area->Triangles;

        while (Tri)
        {
            glBegin(GL_TRIANGLES);
            glVertex2f(Area->PointsAdj[Tri->indexList[0]][0],
                       Area->PointsAdj[Tri->indexList[0]][1]);
            glVertex2f(Area->PointsAdj[Tri->indexList[1]][0],
                       Area->PointsAdj[Tri->indexList[1]][1]);
            glVertex2f(Area->PointsAdj[Tri->indexList[2]][0],
                       Area->PointsAdj[Tri->indexList[2]][1]);
            glEnd();
            Tri = Tri->next;
        }
    }

    TAircraftTypeFilter selectedAircraftTypeFilter = (TAircraftTypeFilter)SelectedAircraftTypeFilter;
    AircraftCountLabel->Caption = IntToStr((int)ght_size(HashTable));

    __int64 CurrentTime = GetCurrentTimeInMsec();

    bool isDataThreadWorking = Form1->IdTCPClientRaw->Connected() || Form1->IdTCPClientSBS->Connected();
    bool isConnectClicked = RawConnectButton->Caption == "Raw Disconnect" || SBSConnectButton->Caption == "SBS Disconnect";

    for (Data = (TADS_B_Aircraft *)ght_first(HashTable, &iterator, (const void **)&Key);
         Data; Data = (TADS_B_Aircraft *)ght_next(HashTable, &iterator, (const void **)&Key))
    {
        // Check if aircraft has position data (either real or predicted)
        bool hasPosition = Data->HaveLatLon;
        double displayLat, displayLon, displayAlt;
        double displayHeading, displaySpeed;
        bool isPredicted = false;

        if (Data->HaveLatLon)
        {
            if (!isDataThreadWorking && isConnectClicked)
            {
                // Calculate dead reckoning for the first time
                CalculateDeadReckoningPosition(Data, CurrentTime);
                displayLat = Data->PredictedLatitude;
                displayLon = Data->PredictedLongitude;
                displayAlt = Data->PredictedAltitude;
                displayHeading = Data->LastKnownHeading;
                displaySpeed = Data->LastKnownSpeed;
                isPredicted = true;
                hasPosition = true;
            }
            else
            {
                displayLat = Data->Latitude;
                displayLon = Data->Longitude;
                displayAlt = Data->Altitude;
                displayHeading = Data->Heading;
                displaySpeed = Data->Speed;
                isPredicted = false;
            }

            // 1. Aircraft type 판별 (한 번만)
            bool isHelicopter = aircraft_is_helicopter(Data->ICAO, NULL);
            bool isMilitary = IsAircraftMilitary(Data->ICAO);
            const TAircraftData *a = (TAircraftData *)ght_get(AircraftDBHashTable, sizeof(Data->ICAO), &Data->ICAO);
            bool isKnownCivilian = (a != NULL && !isHelicopter && !isMilitary);
            bool isUnknown = (!a && !isHelicopter && !isMilitary);

            // 2. 필터링 체크
            bool shouldShow = true;
            switch (selectedAircraftTypeFilter)
            {
            case atfHelicopters:
                shouldShow = isHelicopter;
                break;
            case atfMilitary:
                shouldShow = isMilitary;
                break;
            case atfKnownCivilian:
                shouldShow = isKnownCivilian;
                break;
            case atfUnknown:
                shouldShow = isUnknown;
                break;
            case atfAll:
            default:
                shouldShow = true;
                break;
            }

            if (!shouldShow)
            {
                continue; // 이 항공기는 건너뛰기
            }
            // 고도 필터링 추가
            if (!IsAircraftInAltitudeFilter(Data))
            {
                continue;
            }
            
            // 속도 필터링 추가
            if (!IsAircraftInSpeedFilter(Data))
            {
                continue;
            }
            
        
            // feature selectedAreas
            if (!IsAircraftInSelectedAreas(Data))
            {
                continue;
            }

            ViewableAircraft++;
            double aircraftX, aircraftY;

            // 별도 스레드에서 계산된 결과 사용
            bool isNearAirport = isAircraftNearAirport(Data->ICAO);

            // Only update history for real positions, not predicted ones
            if (!isPredicted)
            {
                UpdateAircraftHistory(Data);
            }

            LatLon2XY(displayLat, displayLon, ScrX, ScrY);

            if (ScrX >= 0 && ScrX <= ObjectDisplay->Width &&
                ScrY >= 0 && ScrY <= ObjectDisplay->Height)
            {
                int y = ScrY / cellHeight;
                int x = ScrX / cellWidth;
                if (x >= 0 && x < 10 && y >= 0 && y < 10)
                {
                    cell[y][x] += 1;
                }
            }
            else
            {
                continue;
            }

            // 색깔 설정 (이미 판별된 값 사용)
            if (isHelicopter)
            {
                glColor4f(1.0f, 0.65f, 0.0f, 1.0f); // Orange
            }
            else if (isMilitary)
            {
                glColor4f(0.0f, 1.0f, 0.0f, 1.0f); // Green
            }
            else if (isKnownCivilian)
            {
                glColor4f(1.0, 0.0, 1.0, 1.0); // Magenta
            }
            else
            {                                       // isUnknown
                glColor4f(0.0f, 0.75f, 1.0f, 1.0f); // Light blue
            }

            // Make predicted aircraft semi-transparent and add red tint
            if (isPredicted)
            {
                float currentColor[4];
                glGetFloatv(GL_CURRENT_COLOR, currentColor);
                glColor4f(currentColor[0] + 0.3f, currentColor[1] * 0.7f, currentColor[2] * 0.7f, 0.7f);
            }

            if (airportManager && isNearAirport)
            {
                glColor4f(1.0, 1.0, 0.0, 1.0); // yellow
            }

            // If aircraft has no speed/heading data, make it semi-transparent
            if (!Data->HaveSpeedAndHeading)
            {
                displayHeading = 0.0;
                // Make unknown heading aircraft semi-transparent
                float currentColor[4];
                glGetFloatv(GL_CURRENT_COLOR, currentColor);
                glColor4f(currentColor[0], currentColor[1], currentColor[2], 0.6f);
            }

            if (xf < cellDrawZoomRate)
            {
                DrawAirplaneImage(ScrX, ScrY, 0.8, displayHeading, Data->SpriteImage);

                // 줌 레벨에 따라 텍스트 표시 여부 결정
                bool showText = true;
                bool useSmallText = false;

                if (xf > textDrawZoomRate * 0.2)
                {                        // 중간 줌 레벨
                    useSmallText = true; // 작은 텍스트 사용
                }
                if (xf > textDrawZoomRate)
                { // 너무 멀면 텍스트 안 보이기
                    showText = false;
                }

                if (showText)
                {
                    // text color
                    if (DrawMap->Checked)
                    {
                        switch (SelectedMapIndex)
                        {
                        case 0: // GoogleMaps
                            glColor4f(0.92, 0.92, 0.96, 1.0);
                            break;
                        case 1:
                        case 2:
                        case 3:
                        case 4: // OpenStreetMaps
                            glColor4f(0.0, 0.0, 0.0, 1.0);
                            break;
                        default:
                            glColor4f(0.92, 0.92, 0.96, 1.0);
                        }
                    }
                    else
                        glColor4f(0.0, 0.0, 0.0, 1.0);

                    glRasterPos2i(ScrX + 15, ScrY - 10);

                    if (useSmallText && Font2DSmall)
                    {
                        ObjectDisplay->Draw2DText(Font2DSmall, Data->HexAddr);
                    }
                    else
                    {
                        ObjectDisplay->Draw2DText(Data->HexAddr);
                    }
                }
            }

            if ((Data->HaveSpeedAndHeading) && (TimeToGoCheckBox->State == cbChecked) && xf < cellDrawZoomRate)
            {
                double lat, lon, az;
                if (VDirect(displayLat, displayLon,
                            displayHeading, displaySpeed / 3060.0 * TimeToGoTrackBar->Position, &lat, &lon, &az) == OKNOERROR)
                {
                    double ScrX2, ScrY2;
                    LatLon2XY(lat, lon, ScrX2, ScrY2);

                    // 고도에 따른 색상 결정 (속도 대신 고도 사용)
                    float r, g, b, alpha;
                    if (Data->HaveAltitude)
                    {
                        GetAltitudeLineColor(displayAlt, r, g, b, alpha);
                    }
                    else
                    {
                        // 고도 정보가 없으면 회색으로 표시
                        r = 0.5f;
                        g = 0.5f;
                        b = 0.5f;
                        alpha = 0.8f;
                    }

                    // Make prediction lines more transparent
                    if (isPredicted)
                    {
                        alpha *= 0.6f;
                    }

                    if (DrawMap->Checked)
                    {
                        glColor4f(r, g, b, alpha); // 동적 색상 적용
                    }
                    else
                    {
                        // 맵이 없을 때는 약간 더 진하게
                        glColor4f(r * 0.8f, g * 0.8f, b * 0.8f, alpha * 0.9f);
                    }
                    glLineWidth(2.0f);

                    glBegin(GL_LINE_STRIP);
                    glVertex2f(ScrX, ScrY);
                    glVertex2f(ScrX2, ScrY2);
                    glEnd();
                    // 라인 두께 원복
                    glLineWidth(1.0f);
                }
            }
        }
    }
    ViewableAircraftCountLabel->Caption = ViewableAircraft;
    // feature selectedAreas (multiple filter)
    if (areaFilterEnabled && selectedFilterAreas->Count > 0)
    {
        AnsiString filterInfo = "Filtered by " + IntToStr(selectedFilterAreas->Count) + " area(s): ";
        for (int i = 0; i < selectedFilterAreas->Count; i++)
        {
            TArea *area = (TArea *)selectedFilterAreas->Items[i];
            if (area)
            {
                if (i > 0)
                    filterInfo += ", ";
                filterInfo += area->Name;
            }
        }
        printf("%s\n", filterInfo.c_str());
    }
    if (TrackHook.Valid_CC)
    {
        Data = (TADS_B_Aircraft *)ght_get(HashTable, sizeof(TrackHook.ICAO_CC), (void *)&TrackHook.ICAO_CC);
        if (Data)
        {
            a = (TAircraftData *)ght_get(AircraftDBHashTable, sizeof(Data->ICAO), &Data->ICAO);

            ICAOLabel->Caption = Data->HexAddr;
            if (Data->HaveFlightNum)
                FlightNumLabel->Caption = Data->FlightNum;
            else
                FlightNumLabel->Caption = "N/A";

        	 if (Data->HaveLatLon)
            {
                CLatiLabel->Caption = DMS::DegreesMinutesSecondsLat(Data->Latitude).c_str();
                CLongtLabel->Caption = DMS::DegreesMinutesSecondsLon(Data->Longitude).c_str();
            }
            else
            {
                CLatiLabel->Caption = "N/A";
                CLongtLabel->Caption = "N/A";
            }

            if (Data->HaveSpeedAndHeading)
            {
                SpdLabel->Caption = FloatToStrF(Data->Speed, ffFixed, 12, 2) + " KTS  VRATE:" + FloatToStrF(Data->VerticalRate, ffFixed, 12, 2);
                HdgLabel->Caption = FloatToStrF(Data->Heading, ffFixed, 12, 2) + " DEG";
            }
            else
            {
                SpdLabel->Caption = "N/A";
                HdgLabel->Caption = "N/A";
            }
            if (Data->Altitude)
                AltLabel->Caption = FloatToStrF(Data->Altitude, ffFixed, 12, 2) + " FT";
            else
                AltLabel->Caption = "N/A";

            MsgCntLabel->Caption = "Raw: " + IntToStr((int)Data->NumMessagesRaw) + " SBS: " + IntToStr((int)Data->NumMessagesSBS);
            TrkLastUpdateTimeLabel->Caption = TimeToChar(Data->LastSeen);

            glColor4f(1.0, 0.0, 0.0, 1.0);
            LatLon2XY(Data->Latitude, Data->Longitude, ScrX, ScrY);
            DrawTrackHook(ScrX, ScrY);

            bool isDrawn = false;
            // Draw planned route for selected aircraft by fetching from http://flightaware.com
            if (Data->HaveFlightNum)
            {
                std::string flightName(Data->FlightNum);
                auto waypoints = plannedRouteManager->GetWaypoints(flightName);
                if (waypoints.size() > 2)
                {
                    isDrawn = true;

                    // draw curved dotted line connecting waypoints
                    glColor4f(1.0, 0.0, 0.0, 1.0); // red with full opacity
                    glLineWidth(2.0);
                    glEnable(GL_LINE_STIPPLE);
                    glLineStipple(1, 0x00FF); // Dashed line pattern

                    glBegin(GL_LINE_STRIP);
                    for (const auto& waypoint : waypoints)
                    {
                        double routeScrX, routeScrY;
                        LatLon2XY(waypoint.first, waypoint.second, routeScrX, routeScrY);
                        glVertex2f(routeScrX, routeScrY);
                    }
                    glEnd();

                    glDisable(GL_LINE_STIPPLE);
                }
            }

            // Draw planned route for selected aircraft if available
            if (!isDrawn && a && a->route_size >= 2)
			{
				// Set red dashed line style
                glColor4f(1.0, 0.0, 0.0, 1.0); // red with full opacity
                glLineWidth(2.0);
                glEnable(GL_LINE_STIPPLE);
                glLineStipple(1, 0x00FF); // Dashed line pattern
                
                glBegin(GL_LINE_STRIP);
                for (int i = 0; i < a->route_size; i++)
                {
                    double routeLat = a->route_latitude[i];
                    double routeLon = a->route_longitude[i];
                    
                    // Validate coordinates
                    if (fabs(routeLat) <= 90.0 && fabs(routeLon) <= 180.0)
                    {
                        double routeScrX, routeScrY;
                        LatLon2XY(routeLat, routeLon, routeScrX, routeScrY);
                        glVertex2f(routeScrX, routeScrY);
                    }
                }
                glEnd();
                
                // Reset line style
                glDisable(GL_LINE_STIPPLE);
				glLineWidth(3.0);
            }

			// Display Tracking history
            if (Data && Data->HistoryCount > 0 && Data->HistoryIndex >= 0 && Data->HistoryIndex < FLIGHT_TRACK_HISTORY_COUNT)
            {
                // printf("[Data] %s HistoryCount=%d HistoryIndex=%d\n", Data->HexAddr, Data->HistoryCount, Data->HistoryIndex);
                glBegin(GL_LINE_STRIP);
                for (int i = 0; i < Data->HistoryCount && i < FLIGHT_TRACK_HISTORY_COUNT; i++)
                {
                    int idx = (Data->HistoryIndex - i - 1 + FLIGHT_TRACK_HISTORY_COUNT) % FLIGHT_TRACK_HISTORY_COUNT;

                    // Error Handling
                    if (idx < 0 || idx >= FLIGHT_TRACK_HISTORY_COUNT)
                    {
                        // printf("[NG] Invalid history: %s idx=%d\n", Data->HexAddr, idx);
                        continue;
                    }

                    // Error Handling - -90 < LAT < 90, -180 < LON < 180
                    if (fabs(Data->PrevLatitude[idx]) > 90.0 || fabs(Data->PrevLongitude[idx]) > 180.0)
                    {
                        // printf("[NG] Invalid history: %s idx=%d lat=%.6f lon=%.6f\n",    Data->HexAddr, idx, Data->PrevLatitude[idx], Data->PrevLongitude[idx]);
                        continue;
                    }

                    // Error Handling - 0.000000
                    if (fabs(Data->PrevLatitude[idx]) < 0.01 || fabs(Data->PrevLongitude[idx]) < 0.01)
                    {
                        // printf("[NG] Invalid history: 0.000000 %s idx=%d %f %f\n", Data->HexAddr, idx, Data->PrevLatitude[idx], Data->PrevLongitude[idx]);
                        continue;
                    }

                    // Error Handling - Check difference with previous value
                    if (i > 0)
                    {
                        int prevIdx = (Data->HistoryIndex - i + FLIGHT_TRACK_HISTORY_COUNT) % FLIGHT_TRACK_HISTORY_COUNT;
                        double latDiff = fabs(Data->PrevLatitude[idx] - Data->PrevLatitude[prevIdx]);
                        double lonDiff = fabs(Data->PrevLongitude[idx] - Data->PrevLongitude[prevIdx]);
                        if (latDiff > 0.8 || lonDiff > 0.8)
                        {
                            printf("Too big diff: %s idx=%d prevIdx=%d latDiff=%.6f lonDiff=%.6f\n", Data->HexAddr, idx, prevIdx, latDiff, lonDiff);
                            continue;
                        }
                    }

                    glColor4f(1.0, 1.0, 1.0, 0.7);
                    glLineWidth(3.0);
                    double historyScrX, historyScrY;
                    LatLon2XY(Data->PrevLatitude[idx], Data->PrevLongitude[idx], historyScrX, historyScrY);
                    glVertex2f(historyScrX, historyScrY);

                    //printf("[OK] Valid Aircraft History %s idx=%d %f %f\n", Data->HexAddr, idx, Data->PrevLatitude[idx], Data->PrevLongitude[idx]);
                }
                glEnd();
            }
        }
        else
        {
            TrackHook.Valid_CC = false;
            ICAOLabel->Caption = "N/A";
            FlightNumLabel->Caption = "N/A";
            CLatiLabel->Caption = "N/A";
            CLongtLabel->Caption = "N/A";
            SpdLabel->Caption = "N/A";
            HdgLabel->Caption = "N/A";
            AltLabel->Caption = "N/A";
            MsgCntLabel->Caption = "N/A";
            TrkLastUpdateTimeLabel->Caption = "N/A";
        }
    }
    if (TrackHook.Valid_CPA)
    {
        bool CpaDataIsValid = false;
        DataCPA = (TADS_B_Aircraft *)ght_get(HashTable, sizeof(TrackHook.ICAO_CPA), (void *)&TrackHook.ICAO_CPA);
        if ((DataCPA) && (TrackHook.Valid_CC))
        {
            double tcpa, cpa_distance_nm, vertical_cpa;
            double lat1, lon1, lat2, lon2, junk;
            if (computeCPA(Data->Latitude, Data->Longitude, Data->Altitude,
                           Data->Speed, Data->Heading, Data->VerticalRate,
                           DataCPA->Latitude, DataCPA->Longitude, DataCPA->Altitude,
                           DataCPA->Speed, DataCPA->Heading, DataCPA->VerticalRate,
                           tcpa, cpa_distance_nm, vertical_cpa))
            {
                if (VDirect(Data->Latitude, Data->Longitude,
                            Data->Heading, Data->Speed / 3600.0 * tcpa, &lat1, &lon1, &junk) == OKNOERROR)
                {
                    if (VDirect(DataCPA->Latitude, DataCPA->Longitude,
                                DataCPA->Heading, DataCPA->Speed / 3600.0 * tcpa, &lat2, &lon2, &junk) == OKNOERROR)
                    {
                        glColor4f(0.0, 1.0, 0.0, 1.0);
                        glBegin(GL_LINE_STRIP);
                        LatLon2XY(Data->Latitude, Data->Longitude, ScrX, ScrY);
                        glVertex2f(ScrX, ScrY);
                        LatLon2XY(lat1, lon1, ScrX, ScrY);
                        glVertex2f(ScrX, ScrY);
                        glEnd();
                        glBegin(GL_LINE_STRIP);
                        LatLon2XY(DataCPA->Latitude, DataCPA->Longitude, ScrX, ScrY);
                        glVertex2f(ScrX, ScrY);
                        LatLon2XY(lat2, lon2, ScrX, ScrY);
                        glVertex2f(ScrX, ScrY);
                        glEnd();
                        glColor4f(1.0, 0.0, 0.0, 1.0);
                        glBegin(GL_LINE_STRIP);
                        LatLon2XY(lat1, lon1, ScrX, ScrY);
                        glVertex2f(ScrX, ScrY);
                        LatLon2XY(lat2, lon2, ScrX, ScrY);
                        glVertex2f(ScrX, ScrY);
                        glEnd();
                        CpaTimeValue->Caption = TimeToChar(tcpa * 1000);
                        CpaDistanceValue->Caption = FloatToStrF(cpa_distance_nm, ffFixed, 10, 2) + " NM VDIST: " + IntToStr((int)vertical_cpa) + " FT";
                        CpaDataIsValid = true;
                    }
                }
            }
        }
        if (!CpaDataIsValid)
        {
            TrackHook.Valid_CPA = false;
            CpaTimeValue->Caption = "None";
            CpaDistanceValue->Caption = "None";
        }
    }

    if (a != NULL && a->airport_size > 0)
    {
        for (i = 0; i < a->airport_size; i++)
        {
            DrawAirportIcon(a->airport_lat[i], a->airport_lon[i], (i == 0) ? true : false);
            DrawAirportInfo(a->airport_lat[i], a->airport_lon[i], a->airport_iata[i].c_str(), (i == 0) ? true : false);
        }
	}

    // Draw Cells(white bubbles) instead of whole aircrafts for the performance and usability,
    // when zoomRate(xf) >= cellDrawZoomRate
    int s = 0;
    if (xf >= cellDrawZoomRate)
    {
        for (j = 0; j < 10; j++)
        {
            for (i = 0; i < 10; i++)
            {
                gCell[j][i] = cell[j][i];

                if (cell[j][i] > 0)
                {

                    s = cell[j][i];

                    if (cell[j][i] < cellMin)
                    {
                        s = cellMin;
                    }
                    else if (cell[j][i] > cellMax)
                    {
                        s = cellMax;
                    }

                    DrawCircleWithNumber(
                        (float)(cellWidth * (i + 1) - cellWidth / 2),
                        (float)(cellHeight * (j + 1) - cellHeight / 2),
                        s,
                        cell[j][i]);
                }
            }
        }
    }

    DrawCenterCross();

    if (!isDataThreadWorking && isConnectClicked && ght_size(HashTable) > 0)
    {
        DrawDeadReckoningStatusBar();
    }

    // SBS 재생 중일 때 Progress 업데이트
    if (SBSPlaybackButton->Caption == "Stop SBS Playback" && PlayBackSBSStream)
    {
#if PROGRESSBAR_RELEASE_MODE == true
        UpdatePlaybackProgress(); // Progressbar TBD
#endif
    }
}

void __fastcall TForm1::DrawCenterCross(void)
{
    // 화면 중앙 좌표 계산
    int screenWidth = ObjectDisplay->Width;
    int screenHeight = ObjectDisplay->Height;
    double centerX = screenWidth / 2.0;
    double centerY = screenHeight / 2.0;

    // 십자가 선 두께 설정
    glLineWidth(3.0);

    // 십자가 선 색상 설정
    glColor4f(1.0, 1.0, 1.0, 1.0);

    glBegin(GL_LINE_STRIP);
    glVertex2f(centerX - 20.0, centerY);
    glVertex2f(centerX + 20.0, centerY);
    glEnd();

    glBegin(GL_LINE_STRIP);
    glVertex2f(centerX, centerY - 20.0);
    glVertex2f(centerX, centerY + 20.0);
    glEnd();
}


int __fastcall TForm1::getAirplaneType(uint32_t addr)
{
    int rtn = 0;
    if (aircraft_is_helicopter(addr, NULL))
    {
        rtn = 46; // Orange for helicopters
    }
    else if (IsAircraftMilitary(addr))
    {
        rtn = 7; // Fluorescent green for military
    }
    else
    {
        rtn = Form1->CurrentSpriteImage;
    }
    return rtn;
}

void __fastcall TForm1::DrawCircleWithNumber(float x, float y, float radius, int number)
{
    // Draw Circle
    glBegin(GL_TRIANGLE_FAN);
    glColor4f(1.0f, 1.0f, 1.0f, 0.5f);
    glVertex2f(x, y);
    for (int i = 0; i <= 360; i += 10)
    {
        float angle = i * DEG_TO_RAD;
        glVertex2f(x + radius * cos(angle), y + radius * sin(angle));
    }
    glEnd();

    // Draw Circle outline
    /*
    glBegin(GL_LINE_LOOP);
    glColor4f(0.0f, 0.0f, 0.0f, 1.0f);
    for(int i = 0; i < 360; i += 10) {
        float angle = i * M_PI / 180.0f;
        glVertex2f(x + radius * cos(angle), y + radius * sin(angle));
    }
    glEnd();
    */

    // Draw Number
    char numStr[10];
    sprintf(numStr, "%d", number);

    // Calculate Number Position (Center Align)
    int textWidth = strlen(numStr) * 8;
    float textX = x - textWidth / 2.0f - textWidth / 2.0f;
    float textY = y - 10.0f;

    // Draw Number
    glColor4f(0.1f, 0.1f, 0.1f, 1.0f);
    glRasterPos2i(textX, textY);
    ObjectDisplay->Draw2DText(numStr);
}

//---------------------------------------------------------------------------
// Dead reckoning 상태바를 그리는 함수
void __fastcall TForm1::DrawDeadReckoningStatusBar(void)
{
    // 화면 상단에 빨간 막대 그리기
    int barHeight = 70;
    int barY = ObjectDisplay->Height - barHeight;

    // 빨간 배경 막대
    glColor4f(0.8f, 0.1f, 0.1f, 0.9f); // 빨간색, 약간 투명
    glBegin(GL_QUADS);
    glVertex2f(0, barY);
    glVertex2f(ObjectDisplay->Width, barY);
    glVertex2f(ObjectDisplay->Width, barY + barHeight);
    glVertex2f(0, barY + barHeight);
    glEnd();

    // 텍스트 그리기
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

    // 메인 메시지
    AnsiString mainMessage = "Connection disconnected. Dead reckoning mode is running.";
    int textX = ObjectDisplay->Width - (ObjectDisplay->Width / 2) - 300;
    int textY = barY + barHeight / 2 - 15;

    glRasterPos2i(textX, textY);
    ObjectDisplay->Draw2DText(mainMessage);
}

//---------------------------------------------------------------------------
void __fastcall TForm1::ObjectDisplayMouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y)
{
    if (this->Visible && this->Enabled)
        this->SetFocus();
    if (Button == mbLeft)
    {
        // 빠른 클릭 감지 로직 추가
        __int64 currentTime = GetCurrentTimeInMsec();

        if (rapidClickCount == 0)
        {
            // 첫 번째 클릭
            firstClickTime = currentTime;
            rapidClickCount = 1;
        }
        else
        {
            // 후속 클릭들
            if (currentTime - firstClickTime <= RAPID_CLICK_TIME_WINDOW)
            {
                // 시간 윈도우 내의 클릭
                rapidClickCount++;

                if (rapidClickCount >= RAPID_CLICK_THRESHOLD)
                {
                    // 10회 클릭 달성!
                    ShowSecretDialog();
                    rapidClickCount = 0;  // 카운터 리셋
                    firstClickTime = 0;
                    return; // 다른 처리 건너뛰기
                }
            }
            else
            {
                // 시간 윈도우 초과, 카운터 리셋
                firstClickTime = currentTime;
                rapidClickCount = 1;
            }
        }

        // Cell 클릭 감지 및 zoom-in 기능 추가
        if (CheckCellClickAndZoom(X, Y))
        {
            // Cell 클릭으로 zoom-in이 처리되었으면 다른 처리를 하지 않음
            return;
        }

        if (Shift.Contains(ssCtrl))
        {
        }
        else
        {
            g_MouseLeftDownX = X;
            g_MouseLeftDownY = Y;
            g_MouseDownMask |= LEFT_MOUSE_DOWN;
            g_EarthView->StartDrag(X, Y, NAV_DRAG_PAN);
        }
    }
    else if (Button == mbRight)
    {
        if (AreaTemp)
        {
            if (AreaTemp->NumPoints < MAX_AREA_POINTS)
            {
                AddPoint(X, Y);
            }
            else
            {
                ShowMessage("Max Area Points Reached");
            }
        }
        else
        {
            if (Shift.Contains(ssCtrl))
            {
                HookTrack(X, Y, true);
            }
            else
            {
                HookTrack(X, Y, false);
            }
        }
    }
    else if (Button == mbMiddle)
    {
        ResetXYOffset();
    }
}

// Cell 클릭 감지 및 zoom-in 처리
bool __fastcall TForm1::CheckCellClickAndZoom(int X, int Y)
{
    // 현재 줌 레벨 확인
    double cellDrawZoomRate = 0.00005;

    // Cell이 표시되는 줌 레벨이 아니면 false 반환
    if (xf < cellDrawZoomRate)
    {
        return false;
    }

    Y = ObjectDisplay->Height - Y;

    // Cell 크기 계산 (DrawObjects 함수와 동일한 로직)
    int cellWidth = ObjectDisplay->Width / 10 - 1;
    int cellHeight = ObjectDisplay->Height / 10 - 1;

    // 클릭한 위치의 cell 좌표 계산
    int cellX = X / cellWidth;
    int cellY = Y / cellHeight;

    // 유효한 cell 범위인지 확인
    if (cellX < 0 || cellX >= 10 || cellY < 0 || cellY >= 10)
    {
        return false;
    }

    // Cell 중심점 계산
    int cellCenterX = cellWidth * (cellX + 1) - cellWidth / 2;
    int cellCenterY = cellHeight * (cellY + 1) - cellHeight / 2;

    // Cell 반지름 계산 (DrawObjects 함수와 동일한 로직)
    int cellSize = std::min(cellWidth, cellHeight) / 2;
    int cellMin = cellSize / 5;
    int cellMax = cellSize;

    // 클릭 위치와 cell 중심점의 거리 계산
    int distance = sqrt((X - cellCenterX) * (X - cellCenterX) +
                        (Y - cellCenterY) * (Y - cellCenterY));

    int s = gCell[cellY][cellX];
    if (gCell[cellY][cellX] < cellMin)
    {
        s = cellMin;
    }
    else if (gCell[cellY][cellX] > cellMax)
    {
        s = cellMax;
    }

    // Cell 내부를 클릭했는지 확인
    if (distance <= s)
    {
        g_EarthView->ZoomAtPoint(cellCenterX, ObjectDisplay->Height - cellCenterY, NAV_ZOOM_IN);

        // 스크롤바 업데이트
        UpdateScrollBarRanges();
        UpdateScrollBarPositions();

        // 화면 갱신
        ObjectDisplay->Repaint();

        return true;
    }

    return false; // Cell 클릭이 아님
}

//---------------------------------------------------------------------------
void __fastcall TForm1::UpdateAircraftHistory(TADS_B_Aircraft *aircraft)
{
    // printf("UpdateAircraftHistory Called\n");
    if (!aircraft->HaveLatLon)
        return;

    // Check index range
    if (aircraft->HistoryIndex < 0 || aircraft->HistoryIndex >= FLIGHT_TRACK_HISTORY_COUNT)
    {
        aircraft->HistoryIndex = 0;
        aircraft->HistoryCount = 0;
    }

    // 위치 변경 감지를 위한 정밀도 설정 (약 10미터 정도)
    const double POSITION_CHANGE_THRESHOLD = 0.0001; // 위도/경도 기준

    // Compare previous loc and current loc
    bool shouldUpdate = true;
    bool positionChanged = true;

    if (aircraft->HistoryCount > 0)
    {
        int prevIdx = (aircraft->HistoryIndex - 1 + FLIGHT_TRACK_HISTORY_COUNT) % FLIGHT_TRACK_HISTORY_COUNT;

        // 이전 위치와 현재 위치의 차이 계산
        double latDiff = fabs(aircraft->PrevLatitude[prevIdx] - aircraft->Latitude);
        double lonDiff = fabs(aircraft->PrevLongitude[prevIdx] - aircraft->Longitude);

        // 위치 변경 감지
        if (latDiff < POSITION_CHANGE_THRESHOLD && lonDiff < POSITION_CHANGE_THRESHOLD)
        {
            positionChanged = false;
        }

        // Same data, not save
        if (aircraft->PrevLatitude[prevIdx] == aircraft->Latitude &&
            aircraft->PrevLongitude[prevIdx] == aircraft->Longitude)
        {
            shouldUpdate = false;
        }
    }
    else
    {
        // 첫 번째 데이터인 경우 LastPositionChangeTime 초기화
        aircraft->LastPositionChangeTime = aircraft->LastSeen;
    }

    if (shouldUpdate)
    {
        int idx = aircraft->HistoryIndex;

        // Save current position
        aircraft->PrevLatitude[idx] = aircraft->Latitude;
        aircraft->PrevLongitude[idx] = aircraft->Longitude;
        aircraft->PrevAltitude[idx] = aircraft->Altitude;
        aircraft->PrevTimestamp[idx] = aircraft->LastSeen;

        //printf("[OK] Valid Aircraft History LastSeen %lld\n", aircraft->LastSeen);

        // Circular buffer index update
        aircraft->HistoryIndex = (aircraft->HistoryIndex + 1) % FLIGHT_TRACK_HISTORY_COUNT;
        if (aircraft->HistoryCount < FLIGHT_TRACK_HISTORY_COUNT)
        {
            aircraft->HistoryCount++;
        }
    }

    // 실제 위치가 변경된 경우에만 LastPositionChangeTime 업데이트
    if (positionChanged)
    {
        aircraft->LastPositionChangeTime = aircraft->LastSeen;
        //printf("[MOVE] Aircraft %s position changed at %lld\n", aircraft->HexAddr, aircraft->LastSeen);
    }
}

//---------------------------------------------------------------------------
// 개선된 PurgeOldHistory 함수
void __fastcall TForm1::PurgeOldHistory(TADS_B_Aircraft *aircraft, __int64 currentTime)
{
    if (!aircraft || aircraft->HistoryCount == 0)
        return;

    const __int64 HISTORY_TIMEOUT_MS = 30000; // 30초
    // 두 가지 조건 중 하나라도 만족하면 경로 삭제
    bool shouldClearHistory = false;
    AnsiString reason = "";
    
    // 조건 1: 30초 동안 데이터가 안 들어옴
    __int64 timeSinceLastData = currentTime - aircraft->LastSeen;
    if (timeSinceLastData >= HISTORY_TIMEOUT_MS)
    {
        shouldClearHistory = true;
        reason = "No data for " + IntToStr((int)(timeSinceLastData / 1000)) + " seconds";
    }
    
    // 조건 2: 30초 동안 위치 변경이 없음 (데이터는 들어오지만 같은 위치)
    __int64 timeSinceLastMove = currentTime - aircraft->LastPositionChangeTime;
    if (timeSinceLastMove >= HISTORY_TIMEOUT_MS)
    {
        shouldClearHistory = true;
        if (reason.Length() > 0) reason += " and ";
        reason += "No position change for " + IntToStr((int)(timeSinceLastMove / 1000)) + " seconds";
    }
    
    if (shouldClearHistory)
    {
        // 경로 히스토리 완전 삭제
        aircraft->HistoryCount = 0;
        aircraft->HistoryIndex = 0;
        
        // 배열 초기화 (메모리 정리)
        memset(aircraft->PrevLatitude, 0, sizeof(aircraft->PrevLatitude));
        memset(aircraft->PrevLongitude, 0, sizeof(aircraft->PrevLongitude));
        memset(aircraft->PrevAltitude, 0, sizeof(aircraft->PrevAltitude));
        memset(aircraft->PrevTimestamp, 0, sizeof(aircraft->PrevTimestamp));
        
        printf("[CLEAR] Aircraft %s track history cleared: %s\n", aircraft->HexAddr, reason.c_str());
    }
}

//---------------------------------------------------------------------------

void __fastcall TForm1::ObjectDisplayMouseUp(TObject *Sender,
                                             TMouseButton Button, TShiftState Shift, int X, int Y)
{
    if (Button == mbLeft)
        g_MouseDownMask &= ~LEFT_MOUSE_DOWN;
}
//---------------------------------------------------------------------------
void __fastcall TForm1::ObjectDisplayMouseMove(TObject *Sender, TShiftState Shift, int X, int Y)
{
    int X1, Y1;
    double VLat, VLon;
    int i;
    Y1 = (ObjectDisplay->Height - 1) - Y;
    X1 = X;
    if ((X1 >= Map_v[0].x) && (X1 <= Map_v[1].x) &&
        (Y1 >= Map_v[0].y) && (Y1 <= Map_v[3].y))

    {
        pfVec3 Point;
        VLat = atan(sinh(M_PI * (2 * (Map_w[1].y - (yf * (Map_v[3].y - Y1)))))) * RAD_TO_DEG;
        VLon = (Map_w[1].x - (xf * (Map_v[1].x - X1))) * 360.0;
        Lat->Caption = DMS::DegreesMinutesSecondsLat(VLat).c_str();
        Lon->Caption = DMS::DegreesMinutesSecondsLon(VLon).c_str();
        Point[0] = VLon;
        Point[1] = VLat;
        Point[2] = 0.0;

        for (i = 0; i < Areas->Count; i++)
        {
            TArea *Area = (TArea *)Areas->Items[i];
            if (PointInPolygon(Area->Points, Area->NumPoints, Point))
            {
#if 0
          MsgLog->Lines->Add("In Polygon "+ Area->Name);
#endif
            }
        }
    }

    if (g_MouseDownMask & LEFT_MOUSE_DOWN)
    {
        g_EarthView->Drag(g_MouseLeftDownX, g_MouseLeftDownY, X, Y, NAV_DRAG_PAN);
        // LatLon2XY(g_EarthView->m_Eye.x, g_EarthView->m_Eye.y, MapCenterLat, MapCenterLon);
        ObjectDisplay->Repaint();
    }
}
//---------------------------------------------------------------------------
void __fastcall TForm1::ResetXYOffset(void)
{
    SetMapCenter(g_EarthView->m_Eye.x, g_EarthView->m_Eye.y);
    ObjectDisplay->Repaint();
}
//---------------------------------------------------------------------------
void __fastcall TForm1::Exit1Click(TObject *Sender)
{
    Close();
}
//---------------------------------------------------------------------------
void __fastcall TForm1::AddPoint(int X, int Y)
{
    double Lat, Lon;

    if (XY2LatLon2(X, Y, Lat, Lon) == 0)
    {

        AreaTemp->Points[AreaTemp->NumPoints][1] = Lat;
        AreaTemp->Points[AreaTemp->NumPoints][0] = Lon;
        AreaTemp->Points[AreaTemp->NumPoints][2] = 0.0;
        AreaTemp->NumPoints++;
        ObjectDisplay->Repaint();
    }
}
//---------------------------------------------------------------------------
void __fastcall TForm1::HookTrack(int X, int Y, bool CPA_Hook)
{
    double VLat, VLon, dlat, dlon, Range;
    int X1, Y1;
    uint32_t *Key;

    uint32_t Current_ICAO;
    double MinRange;
    ght_iterator_t iterator;
    TADS_B_Aircraft *Data;

    Y1 = (ObjectDisplay->Height - 1) - Y;
    X1 = X;

    if ((X1 < Map_v[0].x) || (X1 > Map_v[1].x) ||
        (Y1 < Map_v[0].y) || (Y1 > Map_v[3].y))
        return;

    VLat = atan(sinh(M_PI * (2 * (Map_w[1].y - (yf * (Map_v[3].y - Y1)))))) * RAD_TO_DEG;
    VLon = (Map_w[1].x - (xf * (Map_v[1].x - X1))) * 360.0;

    MinRange = 16.0;

    for (Data = (TADS_B_Aircraft *)ght_first(HashTable, &iterator, (const void **)&Key);
         Data; Data = (TADS_B_Aircraft *)ght_next(HashTable, &iterator, (const void **)&Key))
    {
        if (Data->HaveLatLon)
        {
            dlat = VLat - Data->Latitude;
            dlon = VLon - Data->Longitude;
            Range = sqrt(dlat * dlat + dlon * dlon);
            if (Range < MinRange)
            {
                Current_ICAO = Data->ICAO;
                MinRange = Range;
            }
        }
    }
    if (MinRange < 0.2)
    {
        TADS_B_Aircraft *ADS_B_Aircraft = (TADS_B_Aircraft *)
            ght_get(HashTable, sizeof(Current_ICAO),
                    &Current_ICAO);
        if (ADS_B_Aircraft)
        {
            if (!CPA_Hook)
            {
                TrackHook.Valid_CC = true;
                TrackHook.ICAO_CC = ADS_B_Aircraft->ICAO;

                // Get local aircraft info
                const char *info = GetAircraftDBInfo(ADS_B_Aircraft->ICAO);

                printf("info: %s\n", info);
                if (ADS_B_Aircraft->HaveFlightNum)
                {
                    bool isExist = false;
                    const char *additionalInfo = GetAircraftAPIInfo(ADS_B_Aircraft->ICAO, ADS_B_Aircraft->FlightNum, &isExist);
                    //printf("additionalInfo: %s\n", additionalInfo);

                    if (isExist)
                    {
                        ObjectDisplay->Repaint();
                    }
                }
                // Update aircraft information panel
                UpdateAircraftInfo(ADS_B_Aircraft);
            }
            else
            {
                TrackHook.Valid_CPA = true;
                TrackHook.ICAO_CPA = ADS_B_Aircraft->ICAO;
            }
        }
    }
    else
    {
        if (!CPA_Hook)
        {
            TrackHook.Valid_CC = false;
            ICAOLabel->Caption = "N/A";
            FlightNumLabel->Caption = "N/A";
            CLatiLabel->Caption = "N/A";
            CLongtLabel->Caption = "N/A";
            SpdLabel->Caption = "N/A";
            HdgLabel->Caption = "N/A";
            AltLabel->Caption = "N/A";
            MsgCntLabel->Caption = "N/A";
            TrkLastUpdateTimeLabel->Caption = "N/A";
        }
        else
        {
            TrackHook.Valid_CPA = false;
            CpaTimeValue->Caption = "None";
            CpaDistanceValue->Caption = "None";
        }
    }
}
//---------------------------------------------------------------------------
void __fastcall TForm1::LatLon2XY(double lat, double lon, double &x, double &y)
{
    x = (Map_v[1].x - ((Map_w[1].x - (lon / 360.0)) / xf));
    y = Map_v[3].y - (Map_w[1].y / yf) + (asinh(tan(lat * DEG_TO_RAD)) / (2 * M_PI * yf));
}
//---------------------------------------------------------------------------
int __fastcall TForm1::XY2LatLon2(int x, int y, double &lat, double &lon)
{
    double Lat, Lon, dlat, dlon, Range;
    int X1, Y1;

    Y1 = (ObjectDisplay->Height - 1) - y;
    X1 = x;

    if ((X1 < Map_v[0].x) || (X1 > Map_v[1].x) ||
        (Y1 < Map_v[0].y) || (Y1 > Map_v[3].y))
        return -1;

    lat = atan(sinh(M_PI * (2 * (Map_w[1].y - (yf * (Map_v[3].y - Y1)))))) * RAD_TO_DEG;
    lon = (Map_w[1].x - (xf * (Map_v[1].x - X1))) * 360.0;

    return 0;
}
//---------------------------------------------------------------------------
void __fastcall TForm1::ZoomInClick(TObject *Sender)
{
    g_EarthView->SingleMovement(NAV_ZOOM_IN);
    UpdateScrollBarRanges();    // 줌 후 스크롤바 범위 업데이트
    UpdateScrollBarPositions(); // 스크롤바 위치 업데이트
    ObjectDisplay->Repaint();
}
//---------------------------------------------------------------------------

void __fastcall TForm1::ZoomOutClick(TObject *Sender)
{
    g_EarthView->SingleMovement(NAV_ZOOM_OUT);
    UpdateScrollBarRanges(); // 줌 후 스크롤바 범위 업데이트
    ObjectDisplay->Repaint();
}
//---------------------------------------------------------------------------
void __fastcall TForm1::Purge(void)
{
    uint32_t *Key;
    ght_iterator_t iterator;
    TADS_B_Aircraft *Data;
    void *p;
    __int64 CurrentTime = GetCurrentTimeInMsec();
    __int64 StaleTimeInMs = CSpinStaleTime->Value * 1000;

    if (PurgeStale->Checked == false)
        return;

    for (Data = (TADS_B_Aircraft *)ght_first(HashTable, &iterator, (const void **)&Key);
         Data; Data = (TADS_B_Aircraft *)ght_next(HashTable, &iterator, (const void **)&Key))
    {
        if ((CurrentTime - Data->LastSeen) >= StaleTimeInMs)
        {
            p = ght_remove(HashTable, sizeof(*Key), Key);
            if (!p)
            {
                LOG_ERROR("Removing the current iterated entry failed! This is a BUG");
                ShowMessage("Removing the current iterated entry failed! This is a BUG\n");
            }

            delete Data;
        }
    }
}
//---------------------------------------------------------------------------
void __fastcall TForm1::Timer2Timer(TObject *Sender)
{
    Purge();
}
//---------------------------------------------------------------------------
void __fastcall TForm1::PurgeButtonClick(TObject *Sender)
{
    uint32_t *Key;
    ght_iterator_t iterator;
    TADS_B_Aircraft *Data;
    void *p;

    for (Data = (TADS_B_Aircraft *)ght_first(HashTable, &iterator, (const void **)&Key);
         Data; Data = (TADS_B_Aircraft *)ght_next(HashTable, &iterator, (const void **)&Key))
    {
        p = ght_remove(HashTable, sizeof(*Key), Key);
        if (!p)
        {
            LOG_ERROR("Removing the current iterated entry failed! This is a BUG");
            ShowMessage("Removing the current iterated entry failed! This is a BUG\n");
        }

        delete Data;
    }
}
//---------------------------------------------------------------------------
void __fastcall TForm1::InsertClick(TObject *Sender)
{
    Insert->Enabled = false;
		Insert->Color = clCream;
    LoadARTCCBoundaries1->Enabled = false;
    Complete->Enabled = true;
    Cancel->Enabled = true;
    Complete->Color = clMoneyGreen;
    Cancel->Color = clMoneyGreen;
    // Delete->Enabled=false;

    AreaTemp = new TArea;
    AreaTemp->NumPoints = 0;
    AreaTemp->Name = "";
    AreaTemp->Selected = false;
    AreaTemp->Triangles = NULL;
}
//---------------------------------------------------------------------------
void __fastcall TForm1::CancelClick(TObject *Sender)
{
    TArea *Temp;
    Temp = AreaTemp;
    AreaTemp = NULL;
    delete Temp;
    Insert->Enabled = true;
	Insert->Color = clMoneyGreen;
    Complete->Enabled = false;
	Complete->Color = clCream;
    Cancel->Enabled = false;
	Cancel->Color = clCream;
    LoadARTCCBoundaries1->Enabled = true;
    // if (Areas->Count>0)  Delete->Enabled=true;
    // else   Delete->Enabled=false;
}
//---------------------------------------------------------------------------
void __fastcall TForm1::CompleteClick(TObject *Sender)
{

    int or1 = orientation2D_Polygon(AreaTemp->Points, AreaTemp->NumPoints);
    if (or1 == 0)
    {
        ShowMessage("Degenerate Polygon");
        CancelClick(NULL);
        return;
    }
    if (or1 == CLOCKWISE)
    {
        DWORD i;

        memcpy(AreaTemp->PointsAdj, AreaTemp->Points, sizeof(AreaTemp->Points));
        for (i = 0; i < AreaTemp->NumPoints; i++)
        {
            memcpy(AreaTemp->Points[i],
                   AreaTemp->PointsAdj[AreaTemp->NumPoints - 1 - i], sizeof(pfVec3));
        }
    }
    if (checkComplex(AreaTemp->Points, AreaTemp->NumPoints))
    {
        ShowMessage("Polygon is Complex");
        CancelClick(NULL);
        return;
    }

    AreaConfirm->ShowDialog();
}
//---------------------------------------------------------------------------
void __fastcall TForm1::AreaListViewSelectItem(TObject *Sender, TListItem *Item,
                                               bool Selected)
{
    if (!Item)
        return;

    TArea *AreaS = (TArea *)Item->Data;
    if (!AreaS)
        return;

    // Ctrl 키 상태 확인
    bool ctrlPressed = (GetKeyState(VK_CONTROL) & 0x8000) != 0;

    printf("AreaListViewSelectItem: Area=%s, Selected=%s, Ctrl=%s\n",
           AreaS->Name.c_str(), Selected ? "true" : "false", ctrlPressed ? "true" : "false");

    if (Selected)
    {
        // 아이템이 선택된 경우
        AreaS->Selected = true;

        if (ctrlPressed)
        {
            // Ctrl+클릭: 다중 선택 모드 - 해당 Area를 필터에 추가
            AddAreaToFilter(AreaS);
        }
        else
        {
            // 일반 클릭: 단일 선택 모드 - 다른 모든 선택 해제 후 해당 Area만 선택

            // 다른 모든 Area들의 선택 상태 해제 (UI와 내부 상태 모두)
            for (int i = 0; i < AreaListView->Items->Count; i++)
            {
                TListItem *listItem = AreaListView->Items->Item[i];
                TArea *area = (TArea *)listItem->Data;
                if (area && area != AreaS)
                {
                    area->Selected = false;
                    // UI에서도 선택 해제 (현재 처리중인 아이템 제외)
                    if (listItem != Item)
                    {
                        listItem->Selected = false;
                    }
                }
            }

            // 필터 클리어 후 현재 Area만 추가
            ClearAreaFilter();
            AddAreaToFilter(AreaS);
        }

        Delete->Enabled = true;
		Delete->Color = clMoneyGreen;
    }
    else
    {
        // 아이템이 선택 해제된 경우
        AreaS->Selected = false;
        RemoveAreaFromFilter(AreaS);

        // 선택된 아이템이 있는지 확인
        bool hasSelected = false;
        for (int i = 0; i < AreaListView->Items->Count; i++)
        {
            if (AreaListView->Items->Item[i]->Selected)
            {
                hasSelected = true;
                break;
            }
        }
        Delete->Enabled = hasSelected;
		Delete->Color = hasSelected?clMoneyGreen:clCream;
    }

    ObjectDisplay->Repaint();
}

//---------------------------------------------------------------------------
void __fastcall TForm1::DeleteClick(TObject *Sender)
{
    int i = 0;

    while (i < AreaListView->Items->Count)
    {
        if (AreaListView->Items->Item[i]->Selected)
        {
            TArea *Area;
            int Index;

            Area = (TArea *)AreaListView->Items->Item[i]->Data;
            for (Index = 0; Index < Areas->Count; Index++)
            {
                if (Area == Areas->Items[Index])
                {
                    Areas->Delete(Index);
                    AreaListView->Items->Item[i]->Delete();
                    TTriangles *Tri = Area->Triangles;
                    while (Tri)
                    {
                        TTriangles *temp = Tri;
                        Tri = Tri->next;
                        free(temp->indexList);
                        free(temp);
                    }
                    delete Area;
                    break;
                }
            }
        }
        else
        {
            ++i;
        }
    }
    // if (Areas->Count>0)  Delete->Enabled=true;
    // else   Delete->Enabled=false;

    ObjectDisplay->Repaint();
}
//---------------------------------------------------------------------------
void __fastcall TForm1::AreaListViewCustomDrawItem(TCustomListView *Sender,
                                                   TListItem *Item, TCustomDrawState State, bool &DefaultDraw)
{
    TRect R;
    int Left;
    AreaListView->Canvas->Brush->Color = AreaListView->Color;
    AreaListView->Canvas->Font->Color = AreaListView->Font->Color;
    R = Item->DisplayRect(drBounds);
    AreaListView->Canvas->FillRect(R);

    AreaListView->Canvas->TextWidth(Item->Caption);

    AreaListView->Canvas->TextOut(2, R.Top, Item->Caption);

    Left = AreaListView->Column[0]->Width;

    for (int i = 0; i < Item->SubItems->Count; i++)
    {
        R = Item->DisplayRect(drBounds);
        R.Left = R.Left + Left;
        TArea *Area = (TArea *)Item->Data;
        AreaListView->Canvas->Brush->Color = Area->Color;
        AreaListView->Canvas->FillRect(R);
    }

    if (Item->Selected)
    {
        R = Item->DisplayRect(drBounds);
        AreaListView->Canvas->DrawFocusRect(R);
    }
    DefaultDraw = false;
}
//---------------------------------------------------------------------------
void __fastcall TForm1::DeleteAllAreas(void)
{
    int i = 0;

    while (AreaListView->Items->Count)
    {

        TArea *Area;
        int Index;

        Area = (TArea *)AreaListView->Items->Item[i]->Data;
        for (Index = 0; Index < Areas->Count; Index++)
        {
            if (Area == Areas->Items[Index])
            {
                Areas->Delete(Index);
                AreaListView->Items->Item[i]->Delete();
                TTriangles *Tri = Area->Triangles;
                while (Tri)
                {
                    TTriangles *temp = Tri;
                    Tri = Tri->next;
                    free(temp->indexList);
                    free(temp);
                }
                delete Area;
                break;
            }
        }
    }

    ObjectDisplay->Repaint();
}
//---------------------------------------------------------------------------
void __fastcall TForm1::FormMouseWheel(TObject *Sender, TShiftState Shift,
                                       int WheelDelta, TPoint &MousePos, bool &Handled)
{
    /*
     if (WheelDelta>0)
          g_EarthView->SingleMovement(NAV_ZOOM_IN);
     else g_EarthView->SingleMovement(NAV_ZOOM_OUT);
      ObjectDisplay->Repaint();
    */

    // 마우스 위치가 ObjectDisplay 내부인지 확인
    if (MousePos.x >= 0 && MousePos.x < ObjectDisplay->Width &&
        MousePos.y >= 0 && MousePos.y < ObjectDisplay->Height)
    {

        // 마우스 위치 기준 줌
        if (WheelDelta > 0)
        {
            g_EarthView->ZoomAtPoint(MousePos.x, MousePos.y, NAV_ZOOM_IN);
        }
        else
        {
            g_EarthView->ZoomAtPoint(MousePos.x, MousePos.y, NAV_ZOOM_OUT);
        }

        // 줌 후 스크롤바 업데이트
        UpdateScrollBarRanges();
        UpdateScrollBarPositions();
        ObjectDisplay->Repaint();
    }
    else
    {
        // 화면 밖이면 Do Nothing
    }
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
void __fastcall TTCPClientRawHandleThread::HandleInput(void)
{
    modeS_message mm;
    TDecodeStatus Status;
    __int64 CurrentTime;
    CurrentTime = GetCurrentTimeInMsec();

    // Form1->MsgLog->Lines->Add(StringMsgBuffer);

    if (Form1->RecordRawStream)
    {
        //__int64 CurrentTime;
        // CurrentTime=GetCurrentTimeInMsec();
        Form1->RecordRawStream->WriteLine(IntToStr(CurrentTime));
        Form1->RecordRawStream->WriteLine(StringMsgBuffer);
    }

    Status = decode_RAW_message(StringMsgBuffer, &mm);
    if (Status == HaveMsg)
    {
        TADS_B_Aircraft *ADS_B_Aircraft;
        uint32_t addr;

        addr = (mm.AA[0] << 16) | (mm.AA[1] << 8) | mm.AA[2];

        ADS_B_Aircraft = (TADS_B_Aircraft *)ght_get(Form1->HashTable, sizeof(addr), &addr);
        if (ADS_B_Aircraft)
        {
            // Form1->MsgLog->Lines->Add("Retrived");
        }
        else
        {
            ADS_B_Aircraft = new TADS_B_Aircraft;
            if (!ADS_B_Aircraft)
            {
                LOG_ERROR("Memory allocation failed for new aircraft");
                printf("Memory allocation failed for new aircraft\n");
                return;
            }

            memset(ADS_B_Aircraft, 0, sizeof(TADS_B_Aircraft));

            ADS_B_Aircraft->ICAO = addr;
            snprintf(ADS_B_Aircraft->HexAddr, sizeof(ADS_B_Aircraft->HexAddr), "%06X", (int)addr);
            ADS_B_Aircraft->NumMessagesSBS = 0;
            ADS_B_Aircraft->NumMessagesRaw = 0;
            ADS_B_Aircraft->VerticalRate = 0;
            ADS_B_Aircraft->HaveAltitude = false;
            ADS_B_Aircraft->HaveLatLon = false;
            ADS_B_Aircraft->HaveSpeedAndHeading = false;
            ADS_B_Aircraft->HaveFlightNum = false;
            ADS_B_Aircraft->SpriteImage = Form1->getAirplaneType(addr);
            // init value for tracking
            ADS_B_Aircraft->HistoryIndex = 0;
            ADS_B_Aircraft->HistoryCount = 0;
            // 위치 변경 시간 초기화
            ADS_B_Aircraft->LastPositionChangeTime = GetCurrentTimeInMsec();
            memset(ADS_B_Aircraft->PrevLatitude, 0, sizeof(ADS_B_Aircraft->PrevLatitude));
            memset(ADS_B_Aircraft->PrevLongitude, 0, sizeof(ADS_B_Aircraft->PrevLongitude));
            memset(ADS_B_Aircraft->PrevAltitude, 0, sizeof(ADS_B_Aircraft->PrevAltitude));
            memset(ADS_B_Aircraft->PrevTimestamp, 0, sizeof(ADS_B_Aircraft->PrevTimestamp));

            if (Form1->CycleImages->Checked)
            {
                Form1->CurrentSpriteImage = (Form1->CurrentSpriteImage + 1) % Form1->NumSpriteImages;
            }
            if (ght_insert(Form1->HashTable, ADS_B_Aircraft, sizeof(addr), &addr) < 0)
            {
                printf("ght_insert Error - Should Not Happen\n");
            }
        }

        RawToAircraft(&mm, ADS_B_Aircraft);
        LastHeartbeatTime = GetCurrentTimeInMsec();
        RawTimeoutPopupShown = false;
        RawConnectionLostShown = false;
    }
    else if (Status == MsgHeartBeat)
    {
        LastHeartbeatTime = GetCurrentTimeInMsec();
        RawTimeoutPopupShown = false;
        RawConnectionLostShown = false;
    }
    else
    {
        // Not User Scene
        // ShowMessage("Error while connecting: E%.2d"+Status);
    }

    // printf("[%lld]PI Raw Decode code:%d\n", CurrentTime, Status);
}
//---------------------------------------------------------------------------
void __fastcall TForm1::RawConnectButtonClick(TObject *Sender)
{
    if ((RawConnectButton->Caption == "Raw Connect") && (Sender != NULL))
    {
        // Disable button to prevent multiple clicks
        RawConnectButton->Enabled = false;
		RawConnectButton->Color = clCream;
        RawConnectButton->Caption = "Connecting...";

        // Start connection in separate thread to keep UI responsive
        TConnectionThread *connectionThread = new TConnectionThread(RawIpAddress->Text, 30002, false);
        connectionThread->Resume();
    }
	else
	{
    	if (TCPClientRawHandleThread && TCPClientRawHandleThread->Handle)
		{
			try {
				TCPClientRawHandleThread->Terminate();
				TCPClientRawHandleThread->WaitFor();
			}
			catch (...) {
				printf("Error: Raw thread termination failed\n");
			}
		}

		if (IdTCPClientRaw->Connected())
		{
			IdTCPClientRaw->Disconnect();
		}

		IdTCPClientRaw->IOHandler->InputBuffer->Clear();
        RawConnectButton->Caption = "Raw Connect";
        RawPlaybackButton->Enabled = true;
		RawConnectButton->Color = clMoneyGreen;
    }
}
//---------------------------------------------------------------------------
void __fastcall TForm1::IdTCPClientRawConnected(TObject *Sender)
{
    // SetKeepAliveValues(const AEnabled: Boolean; const ATimeMS, AInterval: Integer);
    // WiFi 끊김 빠른 감지를 위한 더 짧은 KeepAlive 설정 (1초 간격, 1초 재시도)
    IdTCPClientRaw->Socket->Binding->SetKeepAliveValues(true, 1 * 1000, 1 * 1000);
    // ReadTimeout 설정 (3초) - WiFi 끊김 빠른 감지용
    IdTCPClientRaw->ReadTimeout = 3000;
    RawConnectButton->Caption = "Raw Disconnect";
    RawPlaybackButton->Enabled = false;
	RawConnectButton->Color = clCream;
    RawTimeoutPopupShown = false;
    RawConnectionLostShown = false; // 연결 재시도 플래그 리셋
    LastHeartbeatTime = GetCurrentTimeInMsec();
}
//---------------------------------------------------------------------------
void __fastcall TForm1::IdTCPClientRawDisconnected(TObject *Sender)
{
	// 안전한 스레드 종료
	if (TCPClientRawHandleThread && TCPClientRawHandleThread->Handle)
	{
		try {
				TCPClientRawHandleThread->Terminate();
				TCPClientRawHandleThread->WaitFor();
			}
		catch (...) {
			printf("Error: Raw thread termination failed\n");
		}
	}
	RawTimeoutPopupShown = true;
    RawConnectionLostShown = true;
}
//---------------------------------------------------------------------------
void __fastcall TForm1::RawRecordButtonClick(TObject *Sender)
{
    if (RawRecordButton->Caption == "Raw Record")
    {
        if (RecordRawSaveDialog->Execute())
        {
            // First, check if the file exists.
            if (FileExists(RecordRawSaveDialog->FileName))
                ShowMessage("File " + RecordRawSaveDialog->FileName + "already exists. Cannot overwrite.");
            else
            {
                // Open a file for writing. Creates the file if it doesn't exist, or overwrites it if it does.
                RecordRawStream = new TStreamWriter(RecordRawSaveDialog->FileName, false);
                if (RecordRawStream == NULL)
                {
                    ShowMessage("Cannot Open File " + RecordRawSaveDialog->FileName);
                }
                else
                    RawRecordButton->Caption = "Stop Raw Recording";
            }
        }
    }
    else
    {
        delete RecordRawStream;
        RecordRawStream = NULL;
        RawRecordButton->Caption = "Raw Record";
    }
}
//---------------------------------------------------------------------------
void __fastcall TForm1::RawPlaybackButtonClick(TObject *Sender)
{
    if ((RawPlaybackButton->Caption == "Raw Playback") && (Sender != NULL))
    {
        if (PlaybackRawDialog->Execute())
        {
            // First, check if the file exists.
            if (!FileExists(PlaybackRawDialog->FileName))
                ShowMessage("File " + PlaybackRawDialog->FileName + " does not exist");
            else
            {
                // Open a file for writing. Creates the file if it doesn't exist, or overwrites it if it does.
                PlayBackRawStream = new TStreamReader(PlaybackRawDialog->FileName);
                if (PlayBackRawStream == NULL)
                {
                    ShowMessage("Cannot Open File " + PlaybackRawDialog->FileName);
                }
                else
                {
                    TCPClientRawHandleThread = new TTCPClientRawHandleThread(true);
                    TCPClientRawHandleThread->UseFileInsteadOfNetwork = true;
                    TCPClientRawHandleThread->First = true;
                    TCPClientRawHandleThread->FreeOnTerminate = TRUE;
                    TCPClientRawHandleThread->Resume();
                    RawPlaybackButton->Caption = "Stop Raw Playback";
                    RawConnectButton->Enabled = false;
					RawConnectButton->Color = clCream;
                    PlaybackSpeedPanel->Visible = true;
                }
            }
        }
    }
    else
	{
		// 안전한 스레드 종료
		if (TCPClientRawHandleThread && TCPClientRawHandleThread->Handle)
		{
			try {
				TCPClientRawHandleThread->Terminate();
				TCPClientRawHandleThread->WaitFor();
			}
			catch (...) {
				printf("Error: Raw thread termination failed\n");
			}
		}

        delete PlayBackRawStream;
        PlayBackRawStream = NULL;
        RawPlaybackButton->Caption = "Raw Playback";
        RawConnectButton->Enabled = true;
        RawConnectButton->Color = clMoneyGreen;
        PlaybackSpeedPanel->Visible = false;
        PlaybackSpeedTrackBar->Position = 0;
    }
}
//---------------------------------------------------------------------------
// Constructor for the thread class
__fastcall TTCPClientRawHandleThread::TTCPClientRawHandleThread(bool value) : TThread(value)
{
    FreeOnTerminate = true; // Automatically free the thread object after execution
}
//---------------------------------------------------------------------------
// Destructor for the thread class
__fastcall TTCPClientRawHandleThread::~TTCPClientRawHandleThread()
{
    // Clean up resources if needed
}
//---------------------------------------------------------------------------
// Execute method where the thread's logic resides
void __fastcall TTCPClientRawHandleThread::Execute(void)
{
    __int64 Time, SleepTime;
    while (!Terminated)
    {
        if (!UseFileInsteadOfNetwork)
        {
            try
            {
                if (!Form1->IdTCPClientRaw->Connected())
                {
                    Terminate();
                    break;
                }

                // Check if data is available before reading
                if (Form1->IdTCPClientRaw->IOHandler->InputBuffer->Size > 0)
                {
                    StringMsgBuffer = Form1->IdTCPClientRaw->IOHandler->ReadLn();
                }
                else
                {
                    // No data available, sleep briefly to prevent busy waiting
                    Sleep(10);
                    continue;
                }
            }
            catch (const EIdReadTimeout &e)
            {
                // Handle read timeout specifically
                printf("Raw Read timeout: %s\n", AnsiString(e.Message).c_str());
                TThread::Synchronize(StopTCPClient);
                break;
            }
            catch (const EIdException &e)
            {
                // Handle other Indy exceptions
                printf("Raw Indy exception: %s\n", AnsiString(e.Message).c_str());
                TThread::Synchronize(StopTCPClient);
                break;
            }
            catch (...)
            {
                printf("Raw General exception\n");
                TThread::Synchronize(StopTCPClient);
                break;
            }
        }
        else
        {
            try
            {
                if (Form1->PlayBackRawStream->EndOfStream)
                {
                    printf("End Raw Playback 1\n");
                    TThread::Synchronize(StopPlayback);
                    break;
                }
                StringMsgBuffer = Form1->PlayBackRawStream->ReadLine();
                Time = StrToInt64(StringMsgBuffer);
                if (First)
                {
                    First = false;
                    LastTime = Time;
                }

                int SpeedFactor = globalTrackbarValue;
                // printf("SpeedFactor: %d\n", SpeedFactor);
                if (SpeedFactor < 1)
                    SpeedFactor = 1;
                SleepTime = (Time - LastTime) / SpeedFactor;
                LastTime = Time;

                if (SleepTime > 0)
                {
                    Sleep(SleepTime);
                }

                if (Form1->PlayBackRawStream->EndOfStream)
                {
                    printf("End Raw Playback 2\n");
                    TThread::Synchronize(StopPlayback);
                    break;
                }
                StringMsgBuffer = Form1->PlayBackRawStream->ReadLine();
            }
            catch (...)
            {
                printf("Raw Playback Exception\n");
                TThread::Synchronize(StopPlayback);
                break;
            }
        }

        // Only process if we have data
        if (StringMsgBuffer.Length() > 0)
        {
            try
            {
                // Synchronize method to safely access UI components
                TThread::Synchronize(HandleInput);
            }
            catch (...)
            {
                ShowMessage("TTCPClientRawHandleThread::Execute Exception 3");
            }
        }
    }
}
//---------------------------------------------------------------------------
void __fastcall TTCPClientRawHandleThread::StopPlayback(void)
{
    Form1->RawPlaybackButtonClick(NULL);
}
//---------------------------------------------------------------------------
void __fastcall TTCPClientRawHandleThread::StopTCPClient(void)
{
    Form1->RawConnectButtonClick(NULL);
}
//---------------------------------------------------------------------------
void __fastcall TForm1::CycleImagesClick(TObject *Sender)
{
    CurrentSpriteImage = 0;
}
//---------------------------------------------------------------------------
void __fastcall TForm1::SBSConnectButtonClick(TObject *Sender)
{
    if ((SBSConnectButton->Caption == "SBS Connect") && (Sender != NULL))
    {
        // Disable button to prevent multiple clicks
        SBSConnectButton->Enabled = false;
				SBSConnectButton->Color = clCream;
        SBSConnectButton->Caption = "Connecting...";

        // Start connection in separate thread to keep UI responsive
        TConnectionThread *connectionThread = new TConnectionThread(SBSIpAddress->Text, 5002, true);
        connectionThread->Resume();
	}
	else
	{
		// 안전한 스레드 종료
		if (TCPClientSBSHandleThread && TCPClientSBSHandleThread->Handle)
		{
			try {
				TCPClientSBSHandleThread->Terminate();
				TCPClientSBSHandleThread->WaitFor();
			}
			catch (...) {
				printf("Error: SBS thread termination failed\n");
			}

		}

		if (IdTCPClientSBS->Connected())
		{
			IdTCPClientSBS->Disconnect();
		}

		IdTCPClientSBS->IOHandler->InputBuffer->Clear();
		SBSConnectButton->Caption = "SBS Connect";
        SBSPlaybackButton->Enabled = true;
		SBSConnectButton->Color = clMoneyGreen;
	}
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
void __fastcall TTCPClientSBSHandleThread::HandleInput(void)
{
    modeS_message mm;
    TDecodeStatus Status;

    // Form1->MsgLog->Lines->Add(StringMsgBuffer);
    if (Form1->RecordSBSStream)
    {
        __int64 CurrentTime;
        CurrentTime = GetCurrentTimeInMsec();
        Form1->RecordSBSStream->WriteLine(IntToStr(CurrentTime));
        Form1->RecordSBSStream->WriteLine(StringMsgBuffer);
    }

    if (Form1->BigQueryCSV)
    {
        Form1->BigQueryCSV->WriteLine(StringMsgBuffer);
        Form1->BigQueryRowCount++;
        if (Form1->BigQueryRowCount >= BIG_QUERY_UPLOAD_COUNT)
        {
            Form1->CloseBigQueryCSV();
            // printf("string is:%s\n", Form1->BigQueryPythonScript.c_str());
            RunPythonScript(Form1->BigQueryPythonScript, Form1->BigQueryPath + " " + Form1->BigQueryCSVFileName);
            Form1->CreateBigQueryCSV();
        }
    }

    // Detect SBS Message Timeout
    if (StringMsgBuffer.Length() > 0)
    {
        LastSBSDataReceiveTime = GetCurrentTimeInMsec();
        SBSTimeoutPopupShown = false;
        SBSConnectionLostShown = false;
    }

    // Process SBS message - this should be fast and not block
    try
    {
        SBS_Message_Decode(StringMsgBuffer.c_str());
    }
    catch (...)
    {
        // Log error but don't crash the thread
        printf("Error in SBS_Message_Decode\n");
    }
}
//---------------------------------------------------------------------------
// Constructor for the thread class
__fastcall TTCPClientSBSHandleThread::TTCPClientSBSHandleThread(bool value) : TThread(value)
{
    FreeOnTerminate = true; // Automatically free the thread object after execution
    SeekRequested = false;
    SeekTargetTime = 0;
}
//---------------------------------------------------------------------------
// Destructor for the thread class
__fastcall TTCPClientSBSHandleThread::~TTCPClientSBSHandleThread()
{
    // Clean up resources if needed
}
//---------------------------------------------------------------------------
// TTCPClientSBSHandleThread 클래스에 추가
void TTCPClientSBSHandleThread::RequestSeek(__int64 targetTime)
{
    SeekTargetTime = targetTime;
    SeekRequested = true;
}
//---------------------------------------------------------------------------
// Execute method where the thread's logic resides
void __fastcall TTCPClientSBSHandleThread::Execute(void)
{
    __int64 Time, SleepTime;
    while (!Terminated)
    {
        // Seek 요청 처리
        if (SeekRequested && UseFileInsteadOfNetwork)
        {
            SeekRequested = false;
            printf("Processing seek request to time: %lld\n", SeekTargetTime);

            // Form1의 SeekToPosition 호출
            TThread::Synchronize(this, [this]()
                                 { Form1->SeekToPosition(SeekTargetTime); });

            // 스레드 변수 초기화
            First = true;
            LastTime = SeekTargetTime;
            printf("Seek completed, resetting thread variables\n");
            continue;
        }

        if (!UseFileInsteadOfNetwork)
        {
            try
            {
                if (!Form1->IdTCPClientSBS->Connected())
                {
                    Terminate();
					continue;
                }

                // Check if data is available before reading
                if (Form1->IdTCPClientSBS->IOHandler->InputBuffer->Size > 0)
                {
                    StringMsgBuffer = Form1->IdTCPClientSBS->IOHandler->ReadLn();
                }
                else
                {
                    // No data available, sleep briefly to prevent busy waiting
                    Sleep(10);
                    continue;
                }
            }
            catch (const EIdReadTimeout &e)
            {
                // Handle read timeout specifically
                printf("SBS Read timeout: %s\n", AnsiString(e.Message).c_str());
                TThread::Synchronize(StopTCPClient);
                break;
            }
            catch (const EIdException &e)
            {
                // Handle other Indy exceptions
                printf("SBS Indy exception: %s\n", AnsiString(e.Message).c_str());
                TThread::Synchronize(StopTCPClient);
                break;
            }
            catch (...)
            {
                printf("SBS General exception\n");
                TThread::Synchronize(StopTCPClient);
                break;
            }
        }
        else
        {
            try
            {
                if (Form1->PlayBackSBSStream->EndOfStream)
                {
                    printf("End SBS Playback\n");
                    TThread::Synchronize(StopPlayback);
                    break;
                }

                // 타임스탬프 라인을 찾을 때까지 읽기
                bool foundTimestamp = false;
                int attempts = 0;
                const int maxAttempts = 10; // 무한 루프 방지

                while (!foundTimestamp && attempts < maxAttempts && !Form1->PlayBackSBSStream->EndOfStream)
                {
                    StringMsgBuffer = Form1->PlayBackSBSStream->ReadLine();

                    try
                    {
                        Time = StrToInt64(StringMsgBuffer);
                        foundTimestamp = true;
                    }
                    catch (...)
                    {
                        attempts++;
                        continue;
                    }
                }

                if (!foundTimestamp)
                {
                    printf("Could not find valid timestamp after %d attempts\n", maxAttempts);
                    TThread::Synchronize(StopPlayback);
                    break;
                }

                if (First)
                {
                    First = false;
                    LastTime = Time;
                }
                int SpeedFactor = globalTrackbarValue;
                // printf("SpeedFactor: %d\n", SpeedFactor);
                if (SpeedFactor < 1)
                    SpeedFactor = 1;
                SleepTime = (Time - LastTime) / SpeedFactor;
                LastTime = Time;

                if (SleepTime > 0)
                {
                    Sleep(SleepTime);
                }
                if (Form1->PlayBackSBSStream->EndOfStream)
                {
                    printf("End SBS Playback 2\n");
                    TThread::Synchronize(StopPlayback);
                    break;
                }

                // 여기가 핵심! PlaybackCurrentTime 업데이트 및 Progress 업데이트
                Form1->PlaybackCurrentTime = LastTime;
                StringMsgBuffer = Form1->PlayBackSBSStream->ReadLine();
            }
            catch (Exception &e)
            {
                printf("SBS Playback Exception\n");
                TThread::Synchronize(StopPlayback);
                break;
            }
        }

        // Only process if we have data
        if (StringMsgBuffer.Length() > 0)
        {
            try
            {
                // Synchronize method to safely access UI components
                TThread::Synchronize(HandleInput);
            }
            catch (...)
            {
                LOG_ERROR("TTCPClientSBSHandleThread::Execute Exception");
                ShowMessage("TTCPClientSBSHandleThread::Execute Exception");
            }
        }
    }
}
//---------------------------------------------------------------------------
void __fastcall TTCPClientSBSHandleThread::StopPlayback(void)
{
    Form1->SBSPlaybackButtonClick(NULL);
}
//---------------------------------------------------------------------------
void __fastcall TTCPClientSBSHandleThread::StopTCPClient(void)
{
    Form1->SBSConnectButtonClick(NULL);
}
//---------------------------------------------------------------------------
void __fastcall TForm1::SBSRecordButtonClick(TObject *Sender)
{
    if (SBSRecordButton->Caption == "SBS Record")
    {
        if (RecordSBSSaveDialog->Execute())
        {
            // First, check if the file exists.
            if (FileExists(RecordSBSSaveDialog->FileName))
                ShowMessage("File " + RecordSBSSaveDialog->FileName + "already exists. Cannot overwrite.");
            else
            {
                // Open a file for writing. Creates the file if it doesn't exist, or overwrites it if it does.
                RecordSBSStream = new TStreamWriter(RecordSBSSaveDialog->FileName, false);
                if (RecordSBSStream == NULL)
                {
                    ShowMessage("Cannot Open File " + RecordSBSSaveDialog->FileName);
                }
                else
                {
                    SBSRecordButton->Caption = "Stop SBS Recording";
                }
            }
        }
    }
    else
    {
        delete RecordSBSStream;
        RecordSBSStream = NULL;
        SBSRecordButton->Caption = "SBS Record";
    }
}
//---------------------------------------------------------------------------
void __fastcall TForm1::SBSPlaybackButtonClick(TObject *Sender)
{
    if ((SBSPlaybackButton->Caption == "SBS Playback") && (Sender != NULL))
    {
        if (PlaybackSBSDialog->Execute())
        {
            // First, check if the file exists.
            if (!FileExists(PlaybackSBSDialog->FileName)){
                ShowMessage("File " + PlaybackSBSDialog->FileName + " does not exist");
            }
            else
            {
                // Open a file for writing. Creates the file if it doesn't exist, or overwrites it if it does.
                PlayBackSBSStream = new TStreamReader(PlaybackSBSDialog->FileName);
                if (PlayBackSBSStream == NULL)
                {
                    ShowMessage("Cannot Open File " + PlaybackSBSDialog->FileName);
                }
                else
                {
                    // 파일 인덱스 구축
#if PROGRESSBAR_RELEASE_MODE == true
                    BuildFileIndex();                      // Progress TBD
                    PlaybackProgressPanel->Visible = true; // Panel 전체를 보이도록  Progress TBD
#endif
                    TCPClientSBSHandleThread = new TTCPClientSBSHandleThread(true);
                    TCPClientSBSHandleThread->UseFileInsteadOfNetwork = true;
                    TCPClientSBSHandleThread->First = true;
                    TCPClientSBSHandleThread->FreeOnTerminate = TRUE;
                    TCPClientSBSHandleThread->Resume();
                    SBSPlaybackButton->Caption = "Stop SBS Playback";
                    SBSConnectButton->Enabled = false;
					SBSConnectButton->Color = clCream;
                    PlaybackSpeedPanel->Visible = true;
                }
            }
        }
    }
    else
    {
		// 안전한 스레드 종료
		if (TCPClientSBSHandleThread && TCPClientSBSHandleThread->Handle)
		{
			try {
				TCPClientSBSHandleThread->Terminate();
				TCPClientSBSHandleThread->WaitFor();
			}
			catch (...) {
				printf("Error: SBS thread termination failed\n");
			}
		}
		delete PlayBackSBSStream;
		PlayBackSBSStream = NULL;
        SBSPlaybackButton->Caption = "SBS Playback";
        SBSConnectButton->Enabled = true;
		SBSConnectButton->Color = clMoneyGreen;
        PlaybackSpeedPanel->Visible = false;
        PlaybackProgressPanel->Visible = false; // Panel 전체를 숨기도록
        PlaybackPaused = false;
        PlaybackSpeedTrackBar->Position = 0;
    }
}
//---------------------------------------------------------------------------

void __fastcall TForm1::IdTCPClientSBSConnected(TObject *Sender)
{
    // SetKeepAliveValues(const AEnabled: Boolean; const ATimeMS, AInterval: Integer);
    // WiFi 끊김 빠른 감지를 위한 더 짧은 KeepAlive 설정 (1초 간격, 1초 재시도) - Raw와 동일
    IdTCPClientSBS->Socket->Binding->SetKeepAliveValues(true, 1 * 1000, 1 * 1000);
    // ReadTimeout 설정 (3초) - WiFi 끊김 빠른 감지용 - Raw와 동일
    IdTCPClientSBS->ReadTimeout = 3000;
    SBSConnectButton->Caption = "SBS Disconnect";
    SBSPlaybackButton->Enabled = false;
	SBSConnectButton->Color = clCream;
    SBSTimeoutPopupShown = false;
    SBSConnectionLostShown = false; // 연결 재시도 플래그 리셋
    LastSBSDataReceiveTime = GetCurrentTimeInMsec();
}
//---------------------------------------------------------------------------
void __fastcall TForm1::IdTCPClientSBSDisconnected(TObject *Sender)
{
	// 안전한 스레드 종료
	if (TCPClientSBSHandleThread && TCPClientSBSHandleThread->Handle)
	{
		try {
			TCPClientSBSHandleThread->Terminate();
			TCPClientSBSHandleThread->WaitFor();
		}
		catch (...) {
			printf("Error: SBS thread termination failed\n");
		}
	}
    SBSTimeoutPopupShown = true;
}
//---------------------------------------------------------------------------

void __fastcall TForm1::TimeToGoTrackBarChange(TObject *Sender)
{
    _int64 hmsm;
    hmsm = TimeToGoTrackBar->Position * 1000;
    TimeToGoText->Caption = TimeToChar(hmsm);
}
//---------------------------------------------------------------------------
void __fastcall TForm1::LoadMap(int Type)
{
    if (!LiveMapCheckbox->Checked)
    {
        LoadMapFromInternet = false;
    }
    printf("LoadMapFromInternet = %s\n", LoadMapFromInternet ? "TRUE" : "FALSE");
    info("%s: Loading map type %d\n", __func__, Type);
    AnsiString HomeDir = ExtractFilePath(ExtractFileDir(Application->ExeName));
    if (Type == GoogleMaps)
    {
        HomeDir += "..\\GoogleMap";
        if (LoadMapFromInternet)
            HomeDir += "_Live\\";
        else
            HomeDir += "\\";
        std::string cachedir;
        cachedir = HomeDir.c_str();

        if (mkdir(cachedir.c_str()) != 0 && errno != EEXIST)
            throw Sysutils::Exception("Can not create cache directory");

        g_Storage = new FilesystemStorage(cachedir, true);
        if (LoadMapFromInternet)
        {
            g_Keyhole = new KeyholeConnection(GoogleMaps);
            g_Keyhole->SetSaveStorage(g_Storage);
            g_Storage->SetNextLoadStorage(g_Keyhole);
        }
    }
    else if (Type == SkyVector_VFR)
    {
        LoadMapFromInternet = false;
        HomeDir += "..\\VFR_Map";
        if (LoadMapFromInternet)
            HomeDir += "_Live\\";
        else
            HomeDir += "\\";
        std::string cachedir;
        cachedir = HomeDir.c_str();

        if (mkdir(cachedir.c_str()) != 0 && errno != EEXIST)
            throw Sysutils::Exception("Can not create cache directory");

        g_Storage = new FilesystemStorage(cachedir, true);
        if (LoadMapFromInternet)
        {
            g_Keyhole = new KeyholeConnection(SkyVector_VFR);
            g_Keyhole->SetSaveStorage(g_Storage);
            g_Storage->SetNextLoadStorage(g_Keyhole);
        }
    }
    else if (Type == SkyVector_IFR_Low)
    {
        LoadMapFromInternet = false;
        HomeDir += "..\\IFR_Low_Map";
        if (LoadMapFromInternet)
            HomeDir += "_Live\\";
        else
            HomeDir += "\\";
        std::string cachedir;
        cachedir = HomeDir.c_str();

        if (mkdir(cachedir.c_str()) != 0 && errno != EEXIST)
            throw Sysutils::Exception("Can not create cache directory");

        g_Storage = new FilesystemStorage(cachedir, true);
        if (LoadMapFromInternet)
        {
            g_Keyhole = new KeyholeConnection(SkyVector_IFR_Low);
            g_Keyhole->SetSaveStorage(g_Storage);
            g_Storage->SetNextLoadStorage(g_Keyhole);
        }
    }
    else if (Type == SkyVector_IFR_High)
    {
        LoadMapFromInternet = false;
        HomeDir += "..\\IFR_High_Map";
        if (LoadMapFromInternet)
            HomeDir += "_Live\\";
        else
            HomeDir += "\\";
        std::string cachedir;
        cachedir = HomeDir.c_str();

        if (mkdir(cachedir.c_str()) != 0 && errno != EEXIST)
            throw Sysutils::Exception("Can not create cache directory");

        g_Storage = new FilesystemStorage(cachedir, true);
        if (LoadMapFromInternet)
        {
            g_Keyhole = new KeyholeConnection(SkyVector_IFR_High);
            g_Keyhole->SetSaveStorage(g_Storage);
            g_Storage->SetNextLoadStorage(g_Keyhole);
        }
    }
    else if (Type == OpenStreetMaps)
    {
        HomeDir += "..\\OpenstreetMap";
        if (LoadMapFromInternet)
            HomeDir += "_Live\\";
        else
            HomeDir += "\\";
        std::string cachedir;
        cachedir = HomeDir.c_str();

        if (mkdir(cachedir.c_str()) != 0 && errno != EEXIST)
            throw Sysutils::Exception("Can not create cache directory");

        g_Storage = new FilesystemStorage(cachedir, true, OpenStreetMaps);
        if (LoadMapFromInternet)
        {
            g_Keyhole = new KeyholeConnection(OpenStreetMaps);
            g_Keyhole->SetSaveStorage(g_Storage);
            g_Storage->SetNextLoadStorage(g_Keyhole);
        }
    }
    g_GETileManager = new TileManager(g_Storage);

    // dynamic cache by system memory
    MEMORYSTATUSEX memInfo;
    memInfo.dwLength = sizeof(MEMORYSTATUSEX);
    if (GlobalMemoryStatusEx(&memInfo))
    {
        DWORDLONG totalPhysMB = memInfo.ullTotalPhys / (1024 * 1024);
        int maxTextures = 500; // default

        if (totalPhysMB >= 16384)
        { // 16GB
            maxTextures = 2000;
        }
        else if (totalPhysMB >= 8192)
        { // 8GB
            maxTextures = 1000;
        }
        else if (totalPhysMB >= 4096)
        { // 4GB
            maxTextures = 500;
        }
        else
        { // 4GB
            maxTextures = 200;
        }

        g_GETileManager->SetMaxTextures(maxTextures);
        printf("System RAM: %lld MB, Max Textures: %d\n", totalPhysMB, maxTextures);
    }
    if (Type == OpenStreetMaps)
    {
        g_MasterLayer = new OsmLayer(g_GETileManager);
    }
    else
    {
        g_MasterLayer = new GoogleLayer(g_GETileManager);
    }

    g_EarthView = new FlatEarthView(g_MasterLayer);
    g_EarthView->Resize(ObjectDisplay->Width, ObjectDisplay->Height);
}
//---------------------------------------------------------------------------
void __fastcall TForm1::MapComboBoxChange(TObject *Sender)
{
    // Do nothing on Change event (it's triggered by mouse focus only). Map changes are handled on CloseUp event
}

//---------------------------------------------------------------------------
void __fastcall TForm1::MapComboBoxCloseUp(TObject *Sender)
{
    // Only process actual map changes when the ComboBox closes
    if (SelectedMapIndex != MapComboBox->ItemIndex)
    {
        // 새로운 맵 타입의 온라인 지원 여부 확인
        int newMapIndex = MapComboBox->ItemIndex;
        bool newSupportsOnline = (newMapIndex == 0 || newMapIndex == 4);

        // 지원하지 않는 맵으로 변경하는 경우 LiveMapCheckbox 체크 해제
        if (!newSupportsOnline)
        {
            LiveMapCheckbox->Checked = false;
            LoadMapFromInternet = false;
        }
        else
        {
            LoadMapFromInternet = LiveMapCheckbox->Checked;
        }

        double m_Eyeh = g_EarthView->m_Eye.h;
        double m_Eyex = g_EarthView->m_Eye.x;
        double m_Eyey = g_EarthView->m_Eye.y;

        Timer1->Enabled = false;
        Timer2->Enabled = false;

        // 안전한 삭제 순서 - 의존성 역순으로
        if (g_EarthView)
        {
            delete g_EarthView;
            g_EarthView = NULL;
        }
        if (g_MasterLayer)
        {
            delete g_MasterLayer;
            g_MasterLayer = NULL;
        }
        if (g_GETileManager)
        {
            delete g_GETileManager;
            g_GETileManager = NULL;
        }
        if (g_Storage)
        {
            delete g_Storage;
            g_Storage = NULL;
        }
        if (g_Keyhole)
        { // 조건 제거 - 항상 체크
            delete g_Keyhole;
            g_Keyhole = NULL;
        }

        // update map index that is really selected
        SelectedMapIndex = MapComboBox->ItemIndex;

        // 새로운 맵 로드
        if (MapComboBox->ItemIndex == GoogleMaps)
            LoadMap(GoogleMaps);
        else if (MapComboBox->ItemIndex == SkyVector_VFR)
            LoadMap(SkyVector_VFR);
        else if (MapComboBox->ItemIndex == SkyVector_IFR_Low)
            LoadMap(SkyVector_IFR_Low);
        else if (MapComboBox->ItemIndex == SkyVector_IFR_High)
            LoadMap(SkyVector_IFR_High);
        else if (MapComboBox->ItemIndex == OpenStreetMaps)
            LoadMap(OpenStreetMaps);

        // 뷰 상태 복원
        if (g_EarthView)
        { // LoadMap에서 새로 생성되었는지 확인
            g_EarthView->m_Eye.h = m_Eyeh;
            g_EarthView->m_Eye.x = m_Eyex;
            g_EarthView->m_Eye.y = m_Eyey;
        }

        Timer1->Enabled = true;
        Timer2->Enabled = true;
    }
}
//---------------------------------------------------------------------------

void __fastcall TForm1::BigQueryCheckBoxClick(TObject *Sender)
{
    if (BigQueryCheckBox->State == cbChecked)
        CreateBigQueryCSV();
    else
    {
        CloseBigQueryCSV();
        RunPythonScript(BigQueryPythonScript, BigQueryPath + " " + BigQueryCSVFileName);
    }
}
//---------------------------------------------------------------------------
void __fastcall TForm1::CreateBigQueryCSV(void)
{
    AnsiString HomeDir = ExtractFilePath(ExtractFileDir(Application->ExeName));
    BigQueryCSVFileName = "BigQuery" + UIntToStr(BigQueryFileCount) + ".csv";
    BigQueryRowCount = 0;
    BigQueryFileCount++;
    BigQueryCSV = new TStreamWriter(HomeDir + "..\\BigQuery\\" + BigQueryCSVFileName, false);
    if (BigQueryCSV == NULL)
    {
        ShowMessage("Cannot Open BigQuery CSV File " + HomeDir + "..\\BigQuery\\" + BigQueryCSVFileName);
        BigQueryCheckBox->State = cbUnchecked;
    }
    AnsiString Header = AnsiString("Message Type,Transmission Type,SessionID,AircraftID,HexIdent,FlightID,Date_MSG_Generated,Time_MSG_Generated,Date_MSG_Logged,Time_MSG_Logged,Callsign,Altitude,GroundSpeed,Track,Latitude,Longitude,VerticalRate,Squawk,Alert,Emergency,SPI,IsOnGround");
    BigQueryCSV->WriteLine(Header);
}
//--------------------------------------------------------------------------
void __fastcall TForm1::CloseBigQueryCSV(void)
{
    if (BigQueryCSV)
    {
        delete BigQueryCSV;
        BigQueryCSV = NULL;
    }
}
//--------------------------------------------------------------------------
static void RunPythonScript(AnsiString scriptPath, AnsiString args)
{
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    AnsiString commandLine = "python " + scriptPath + " " + args;
    char *cmdLineCharArray = new char[strlen(commandLine.c_str()) + 1];
    strcpy(cmdLineCharArray, commandLine.c_str());
#define LOG_PYTHON 1
#if LOG_PYTHON
    // printf("%s\n", cmdLineCharArray);
    SECURITY_ATTRIBUTES sa;
    sa.nLength = sizeof(sa);
    sa.lpSecurityDescriptor = NULL;
    sa.bInheritHandle = TRUE;
    HANDLE h = CreateFileA(Form1->BigQueryLogFileName.c_str(),
                           FILE_APPEND_DATA,
                           FILE_SHARE_WRITE | FILE_SHARE_READ,
                           &sa,
                           OPEN_ALWAYS,
                           FILE_ATTRIBUTE_NORMAL,
                           NULL);

    si.hStdInput = NULL;
    si.hStdOutput = h;
    si.hStdError = h; // Redirect standard error as well, if needed
    si.dwFlags |= STARTF_USESTDHANDLES;
#endif
    if (!CreateProcessA(
            nullptr,          // No module name (use command line)
            cmdLineCharArray, // Command line
            nullptr,          // Process handle not inheritable
            nullptr,          // Thread handle not inheritable
#if LOG_PYTHON
            TRUE,
#else
            FALSE, // Set handle inheritance to FALSE
#endif
            CREATE_NO_WINDOW, // Don't create a console window
            nullptr,          // Use parent's environment block
            nullptr,          // Use parent's starting directory
            &si,              // Pointer to STARTUPINFO structure
            &pi))             // Pointer to PROCESS_INFORMATION structure
    {
        std::cerr << "CreateProcess failed (" << GetLastError() << ").\n";
        delete[] cmdLineCharArray;
        return;
    }

    // Optionally, detach from the process
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    delete[] cmdLineCharArray;
}

//--------------------------------------------------------------------------
void __fastcall TForm1::UseSBSRemoteClick(TObject *Sender)
{
    SBSIpAddress->Text = "data.adsbhub.org";
}
//---------------------------------------------------------------------------

void __fastcall TForm1::UseSBSLocalClick(TObject *Sender)
{
    SBSIpAddress->Text = "128.237.96.41";
}
//---------------------------------------------------------------------------
static bool DeleteFilesWithExtension(AnsiString dirPath, AnsiString extension)
{
    AnsiString searchPattern = dirPath + "\\*." + extension;
    WIN32_FIND_DATAA findData;

    HANDLE hFind = FindFirstFileA(searchPattern.c_str(), &findData);

    if (hFind == INVALID_HANDLE_VALUE)
    {
        return false; // No files found or error
    }

    do
    {
        AnsiString filePath = dirPath + "\\" + findData.cFileName;
        if (DeleteFileA(filePath.c_str()) == 0)
        {
            FindClose(hFind);
            return false; // Failed to delete a file
        }
    } while (FindNextFileA(hFind, &findData) != 0);

    FindClose(hFind);
    return true;
}
static bool IsFirstRow = true;
static bool CallBackInit = false;
//---------------------------------------------------------------------------
static int CSV_callback_ARTCCBoundaries(struct CSV_context *ctx, const char *value)
{
    int rc = 1;
    static char LastArea[512];
    static char Area[512];
    static char Lat[512];
    static char Lon[512];
    int Deg, Min, Sec, Hsec;
    char Dir;

    if (ctx->field_num == 0)
    {
        strcpy(Area, value);
    }
    else if (ctx->field_num == 3)
    {
        strcpy(Lat, value);
    }
    else if (ctx->field_num == 4)
    {
        strcpy(Lon, value);
    }

    if (ctx->field_num == (ctx->num_fields - 1))
    {
        float fLat, fLon;
        if (!IsFirstRow)
        {
            if (!CallBackInit)
            {
                strcpy(LastArea, Area);
                CallBackInit = true;
            }
            if (strcmp(LastArea, Area) != 0)
            {

                if (FinshARTCCBoundary())
                {
                    printf("Load ERROR ID %s\n", LastArea);
                }
                else
                    printf("Loaded ID %s\n", LastArea);
                strcpy(LastArea, Area);
            }
            if (Form1->AreaTemp == NULL)
            {
                Form1->AreaTemp = new TArea;
                Form1->AreaTemp->NumPoints = 0;
                Form1->AreaTemp->Name = Area;
                Form1->AreaTemp->Selected = false;
                Form1->AreaTemp->Triangles = NULL;
                printf("Loading ID %s\n", Area);
            }
            if (sscanf(Lat, "%2d%2d%2d%2d%c", &Deg, &Min, &Sec, &Hsec, &Dir) != 5)
                printf("Latitude Parse Error\n");
            fLat = Deg + Min / 60.0 + Sec / 3600.0 + Hsec / 360000.00;
            if (Dir == 'S')
                fLat = -fLat;

            if (sscanf(Lon, "%3d%2d%2d%2d%c", &Deg, &Min, &Sec, &Hsec, &Dir) != 5)
                printf("Longitude Parse Error\n");
            fLon = Deg + Min / 60.0 + Sec / 3600.0 + Hsec / 360000.00;
            if (Dir == 'W')
                fLon = -fLon;
            // printf("%f, %f\n",fLat,fLon);
            if (Form1->AreaTemp->NumPoints < MAX_AREA_POINTS)
            {
                Form1->AreaTemp->Points[Form1->AreaTemp->NumPoints][1] = fLat;
                Form1->AreaTemp->Points[Form1->AreaTemp->NumPoints][0] = fLon;
                Form1->AreaTemp->Points[Form1->AreaTemp->NumPoints][2] = 0.0;
                Form1->AreaTemp->NumPoints++;
            }
            else
                printf("Max Area Points Reached\n");
        }
        if (IsFirstRow)
            IsFirstRow = false;
    }
    return (rc);
}
//---------------------------------------------------------------------------
bool __fastcall TForm1::LoadARTCCBoundaries(AnsiString FileName)
{
    CSV_context csv_ctx;
    memset(&csv_ctx, 0, sizeof(csv_ctx));
    csv_ctx.file_name = FileName.c_str();
    csv_ctx.delimiter = ',';
    csv_ctx.callback = CSV_callback_ARTCCBoundaries;
    csv_ctx.line_size = 2000;
    IsFirstRow = true;
    CallBackInit = false;
    if (!CSV_open_and_parse_file(&csv_ctx))
    {
        printf("Parsing of \"%s\" failed: %s\n", FileName.c_str(), strerror(errno));
        return (false);
    }
    if ((Form1->AreaTemp != NULL) && (Form1->AreaTemp->NumPoints > 0))
    {
        char Area[512];
        strcpy(Area, Form1->AreaTemp->Name.c_str());
        if (FinshARTCCBoundary())
        {
            printf("Loaded ERROR ID %s\n", Area);
        }
        else
            printf("Loaded ID %s\n", Area);
    }
    printf("Done\n");
    return (true);
}
//---------------------------------------------------------------------------
void __fastcall TForm1::LoadARTCCBoundaries1Click(TObject *Sender)
{
    LoadARTCCBoundaries(ARTCCBoundaryDataPathFileName);
}
//---------------------------------------------------------------------------
static int FinshARTCCBoundary(void)
{
    int or1 = orientation2D_Polygon(Form1->AreaTemp->Points, Form1->AreaTemp->NumPoints);
    if (or1 == 0)
    {
        TArea *Temp;
        Temp = Form1->AreaTemp;
        Form1->AreaTemp = NULL;
        delete Temp;
        printf("Degenerate Polygon\n");
        return (-1);
    }
    if (or1 == CLOCKWISE)
    {
        DWORD i;

        memcpy(Form1->AreaTemp->PointsAdj, Form1->AreaTemp->Points, sizeof(Form1->AreaTemp->Points));
        for (i = 0; i < Form1->AreaTemp->NumPoints; i++)
        {
            memcpy(Form1->AreaTemp->Points[i],
                   Form1->AreaTemp->PointsAdj[Form1->AreaTemp->NumPoints - 1 - i], sizeof(pfVec3));
        }
    }
    if (checkComplex(Form1->AreaTemp->Points, Form1->AreaTemp->NumPoints))
    {
        TArea *Temp;
        Temp = Form1->AreaTemp;
        Form1->AreaTemp = NULL;
        delete Temp;
        printf("Polygon is Complex\n");
        return (-2);
    }
    DWORD Row, Count, i;

    Count = Form1->Areas->Count;
    for (i = 0; i < Count; i++)
    {
        TArea *Area = (TArea *)Form1->Areas->Items[i];
        if (Area->Name == Form1->AreaTemp->Name)
        {

            TArea *Temp;
            Temp = Form1->AreaTemp;
            printf("Duplicate Area Name %s\n", Form1->AreaTemp->Name.c_str());
            Form1->AreaTemp = NULL;
            delete Temp;
            return (-3);
        }
    }

    triangulatePoly(Form1->AreaTemp->Points, Form1->AreaTemp->NumPoints,
                    &Form1->AreaTemp->Triangles);

    Form1->AreaTemp->Color = TColor(PopularColors[CurrentColor]);
    CurrentColor++;
    CurrentColor = CurrentColor % NumColors;
    Form1->Areas->Add(Form1->AreaTemp);
    Form1->AreaListView->Items->BeginUpdate();
    Form1->AreaListView->Items->Add();
    Row = Form1->AreaListView->Items->Count - 1;
    Form1->AreaListView->Items->Item[Row]->Caption = Form1->AreaTemp->Name;
    Form1->AreaListView->Items->Item[Row]->Data = Form1->AreaTemp;
    Form1->AreaListView->Items->Item[Row]->SubItems->Add("");
    Form1->AreaListView->Items->EndUpdate();
    Form1->AreaTemp = NULL;
    return 0;
}
//---------------------------------------------------------------------------

void __fastcall TForm1::DisplayAirportCheckBoxClick(TObject *Sender)
{

    if (DisplayAirportCheckBox->Checked && airportManager)
    {
        printf("Airport display should be enabled\n");
    }
    else
    {
        printf("Airport display is disabled\n");
    }
}
//---------------------------------------------------------------------------

void __fastcall TForm1::DrawAirportIcon(double lat, double lon, bool isDeparture)
{
    double ScrX, ScrY;
    LatLon2XY(lat, lon, ScrX, ScrY);

    // Set color based on departure/arrival
    if (isDeparture)
    {
        glColor4f(0.0, 1.0, 0.0, 1.0); // Green for departure
    }
    else
    {
        glColor4f(1.0, 0.0, 0.0, 1.0); // Red for arrival
    }

    // Draw airport icon (simple cross)
    glLineWidth(2.0);
    glBegin(GL_LINES);
    glVertex2f(ScrX - 10, ScrY);
    glVertex2f(ScrX + 10, ScrY);
    glVertex2f(ScrX, ScrY - 10);
    glVertex2f(ScrX, ScrY + 10);
    glEnd();

    // Draw circle around the cross
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < 360; i += 10)
    {
        double angle = i * DEG_TO_RAD;
        glVertex2f(ScrX + 15 * cos(angle), ScrY + 15 * sin(angle));
    }
    glEnd();
}

void __fastcall TForm1::DrawAirportInfo(double lat, double lon, const char *name, bool isDeparture)
{
    double ScrX, ScrY;
    LatLon2XY(lat, lon, ScrX, ScrY);

    // Draw airport name
    glColor4f(1.0, 1.0, 0.0, 1.0);
    glRasterPos2i(ScrX + 20, ScrY + 20);
    ObjectDisplay->Draw2DText(name);
}

// 캐시된 거리 계산 함수 구현
double TForm1::getCachedDistance(uint32_t aircraftICAO, const std::string &airportICAO,
                                 double aircraftLat, double aircraftLon,
                                 double airportLat, double airportLon)
{
    auto now = std::chrono::system_clock::now();
    auto key = std::make_pair(aircraftICAO, airportICAO);

    // 캐시에서 거리 찾기
    auto it = distanceCache.find(key);
    if (it != distanceCache.end())
    {
        // 캐시가 만료되지 않았는지 확인
        auto age = std::chrono::duration_cast<std::chrono::milliseconds>(
                       now - it->second.timestamp)
                       .count();
        if (age < CACHE_EXPIRY_MS)
        {
            return it->second.distance;
        }
    }

    // 캐시에 없거나 만료된 경우 새로 계산
    double dlat = aircraftLat - airportLat;
    double dlon = aircraftLon - airportLon;
    double latDist = dlat * 60.0;
    double lonDist = dlon * 60.0 * cos(aircraftLat * DEG_TO_RAD);
    double distance = sqrt(latDist * latDist + lonDist * lonDist);

    // 결과를 캐시에 저장
    DistanceCache cache;
    cache.distance = distance;
    cache.timestamp = now;
    distanceCache[key] = cache;

    return distance;
}

// 캐시 정리 함수 구현
void TForm1::cleanupOldCache()
{
    auto now = std::chrono::system_clock::now();

    // 마지막 정리 이후 일정 시간이 지났는지 확인
    auto timeSinceLastCleanup = std::chrono::duration_cast<std::chrono::milliseconds>(
                                    now - lastCleanupTime)
                                    .count();
    if (timeSinceLastCleanup < CACHE_CLEANUP_INTERVAL_MS)
    {
        return;
    }

    // 오래된 캐시 항목 제거
    for (auto it = distanceCache.begin(); it != distanceCache.end();)
    {
        auto age = std::chrono::duration_cast<std::chrono::milliseconds>(
                       now - it->second.timestamp)
                       .count();
        if (age > CACHE_MAX_AGE_MS)
        {
            it = distanceCache.erase(it);
        }
        else
        {
            ++it;
        }
    }

    lastCleanupTime = now;
}

//---------------------------------------------------------------------------
void __fastcall TForm1::PlaybackSpeedTrackBarChanged(TObject *Sender)
{
    int trackBarValue = PlaybackSpeedTrackBar->Position;
    globalTrackbarValue = trackBarValue;
    printf("PlaybackSpeedTrackBar Changed - Position: %d\n", trackBarValue);
}
//---------------------------------------------------------------------------
void __fastcall TForm1::AboutADSBDisplay1Click(TObject *Sender)
{
    printf("About Clicked\n");
    ShowMessage("ADS-B-Display Version 25.07.03\n"
                "Copyright 2025 Solvit. All Rights Reserved.\n"
                "Product by SW Architect #2 Challengers\n");
}
//---------------------------------------------------------------------------

void __fastcall TForm1::UserManual1Click(TObject *Sender)
{
    printf("User Manual Clicked\n");
    ShellExecute(0, L"open", L"https://github.com/lgswa-25t2/ADS-B-Display/blob/main/docs/usermanual/User_Guide_EN.md", NULL, NULL, SW_SHOWNORMAL);
}

void __fastcall TForm1::ShowSecretDialog()
{
    printf("Secret dialog triggered by rapid clicks!\n");
    ShowMessage("-> SECRET FEATURE UNLOCKED! <-\n\n"
                "You discovered the hidden Easter Egg!\n"
                "Rapid clicking 10 times in 2 seconds unlocks this message.\n\n"
                "## ADS-B Display - Advanced Features\n"
                "* Performance - Being super fast application speed\n"
                "* Userablility - Special userablility adapted\n"
		        "* Modifiability - Enhanced Modifiability provided\n"
                "* Resiliency - Self-healing mechanisms enabled\n"
                "* Call to Leader of LG SA Team 2 Leader\n\n"
                "Keep exploring the skies!!");
}

void __fastcall TForm1::UpdateAircraftInfo(TADS_B_Aircraft *Data)
{
    if (!Data)
    {
        return;
    }

    // Get aircraft information from database
    const TAircraftData *a = (TAircraftData *)ght_get(AircraftDBHashTable, sizeof(Data->ICAO), &Data->ICAO);

    if (a)
    {
        // Update aircraft metadata in right panel
        SerialNum->Caption = SafeAnsiString(a->Fields[AC_DB_SerialNumber].c_str());
        Manufacturer->Caption = SafeAnsiString(a->Fields[AC_DB_ManufacturerName].c_str());
        Model->Caption = SafeAnsiString(a->Fields[AC_DB_Model].c_str());
        MFRYear->Caption = SafeAnsiString(a->Fields[AC_DB_Built].c_str());
        CeritificatedInfo->Caption = SafeAnsiString(a->Fields[AC_DB_Registered].c_str());
        ExpirationData->Caption = SafeAnsiString(a->Fields[AC_DB_RegUntil].c_str());
        EngineType->Caption = SafeAnsiString(a->Fields[AC_DB_Engines].c_str());
        AirType->Caption = SafeAnsiString(a->Fields[AC_DB_ICAOAircraftType].c_str());

        printf("Aircraft metadata updated in UI\n");
    }
    else
    {
        // Clear metadata when no data available
        SerialNum->Caption = "N/A";
        Manufacturer->Caption = "N/A";
        Model->Caption = "N/A";
        MFRYear->Caption = "N/A";
        CeritificatedInfo->Caption = "N/A";
        ExpirationData->Caption = "N/A";
        EngineType->Caption = "N/A";
        AirType->Caption = "N/A";

        printf("No aircraft metadata available\n");
    }

    // Update ROUTE DESCRIPTION
    UpdateRouteInfo(Data);
}

void __fastcall TForm1::UpdateRouteInfo(TADS_B_Aircraft *Data)
{
    if (!Data)
    {
        // Clear all route info when no data
        ClearRouteInfo();
        return;
    }
    return;

    // Get aircraft data which contains route information
    const TAircraftData *a = (TAircraftData *)ght_get(AircraftDBHashTable, sizeof(Data->ICAO), &Data->ICAO);

    printf("=== Route Information ===\n");

    if (a && a->airport_size > 0)
    {
        // Departure Airport (첫 번째 공항)
        DepartureAirportName->Caption = SafeAnsiString(a->airport_name[0].c_str());
        DepartureAirportICAO->Caption = SafeAnsiString(a->airport_icao[0].c_str());
        DepartureAirportLocation->Caption = SafeAnsiString(a->airport_location[0].c_str());
        Label36->Caption = SafeAnsiString(a->airport_countryiso2[0].c_str()); // Departure Country

        printf("Departure: %s (%s) - %s, %s\n",
               a->airport_name[0].c_str(),
               a->airport_icao[0].c_str(),
               a->airport_location[0].c_str(),
               a->airport_countryiso2[0].c_str());

        // Destination Airport (마지막 공항, airport_size > 1인 경우)
        if (a->airport_size > 1)
        {
            uint32_t lastIndex = a->airport_size - 1;
            DestinationAirportName->Caption = SafeAnsiString(a->airport_name[lastIndex].c_str());
            DestinationAirportICAO->Caption = SafeAnsiString(a->airport_icao[lastIndex].c_str());
            DestinationAirportLocation->Caption = SafeAnsiString(a->airport_location[lastIndex].c_str());
            DestinationCounty->Caption = SafeAnsiString(a->airport_countryiso2[lastIndex].c_str()); // Destination Country

            printf("Destination: %s (%s) - %s, %s\n",
                   a->airport_name[lastIndex].c_str(),
                   a->airport_icao[lastIndex].c_str(),
                   a->airport_location[lastIndex].c_str(),
                   a->airport_countryiso2[lastIndex].c_str());
        }
        else
        {
            // 목적지가 없는 경우 (단일 공항)
            DestinationAirportName->Caption = "N/A";
            DestinationAirportICAO->Caption = "N/A";
            DestinationAirportLocation->Caption = "N/A";
            DestinationCounty->Caption = "N/A";
        }

        // Transit Airport 1 (두 번째 공항, airport_size > 2인 경우)
        if (a->airport_size > 2)
        {
            TransitAirport1Name->Caption = SafeAnsiString(a->airport_name[1].c_str());
            TransitAirport1ICAO->Caption = SafeAnsiString(a->airport_icao[1].c_str());
            TransitAirport1Location->Caption = SafeAnsiString(a->airport_location[1].c_str());
            TransitAirport1Country->Caption = SafeAnsiString(a->airport_countryiso2[1].c_str());

            printf("Transit1: %s (%s) - %s, %s\n",
                   a->airport_name[1].c_str(),
                   a->airport_icao[1].c_str(),
                   a->airport_location[1].c_str(),
                   a->airport_countryiso2[1].c_str());
        }
        else
        {
            TransitAirport1Name->Caption = "N/A";
            TransitAirport1ICAO->Caption = "N/A";
            TransitAirport1Location->Caption = "N/A";
            TransitAirport1Country->Caption = "N/A";
        }

        // Transit Airport 2 (세 번째 공항, airport_size > 3인 경우)
        if (a->airport_size > 3)
        {
            TransitAirport2Name->Caption = SafeAnsiString(a->airport_name[2].c_str());
            TransitAirport2ICAO->Caption = SafeAnsiString(a->airport_icao[2].c_str());
            TransitAirport2Location->Caption = SafeAnsiString(a->airport_location[2].c_str());
            TransitAirport2Country->Caption = SafeAnsiString(a->airport_countryiso2[2].c_str());

            printf("Transit2: %s (%s) - %s, %s\n",
                   a->airport_name[2].c_str(),
                   a->airport_icao[2].c_str(),
                   a->airport_location[2].c_str(),
                   a->airport_countryiso2[2].c_str());
        }
        else
        {
            TransitAirport2Name->Caption = "N/A";
            TransitAirport2ICAO->Caption = "N/A";
            TransitAirport2Location->Caption = "N/A";
            TransitAirport2Country->Caption = "N/A";
        }
    }
    else
    {
        // 루트 정보가 없는 경우 모든 필드를 N/A로 설정
        ClearRouteInfo();
        printf("No route information available\n");
    }
    printf("========================\n");
}

void __fastcall TForm1::ClearAircraftInfo()
{
    // Clear aircraft metadata
    SerialNum->Caption = "N/A";
    Manufacturer->Caption = "N/A";
    Model->Caption = "N/A";
    MFRYear->Caption = "N/A";
    CeritificatedInfo->Caption = "N/A";
    ExpirationData->Caption = "N/A";
    EngineType->Caption = "N/A";
    AirType->Caption = "N/A";

    // Also clear route information
    ClearRouteInfo();

    printf("Aircraft metadata and route info cleared\n");
}

void __fastcall TForm1::ClearRouteInfo()
{
    // Clear Departure Airport
    DepartureAirportName->Caption = "N/A";
    DepartureAirportICAO->Caption = "N/A";
    DepartureAirportLocation->Caption = "N/A";
    Label36->Caption = "N/A"; // Departure Country

    // Clear Destination Airport
    DestinationAirportName->Caption = "N/A";
    DestinationAirportICAO->Caption = "N/A";
    DestinationAirportLocation->Caption = "N/A";
    DestinationCounty->Caption = "N/A"; // Destination Country

    // Clear Transit Airport 1
    TransitAirport1Name->Caption = "N/A";
    TransitAirport1ICAO->Caption = "N/A";
    TransitAirport1Location->Caption = "N/A";
    TransitAirport1Country->Caption = "N/A";

    // Clear Transit Airport 2
    TransitAirport2Name->Caption = "N/A";
    TransitAirport2ICAO->Caption = "N/A";
    TransitAirport2Location->Caption = "N/A";
    TransitAirport2Country->Caption = "N/A";

    printf("Route information cleared\n");
}
void __fastcall TForm1::PanelTitle1Click(TObject *Sender)
{
    TogglePanels();
}

void __fastcall TForm1::TogglePanels()
{
    panelsVisible = !panelsVisible;

    // Panel들의 Visible 속성 토글
    Panel1->Visible = panelsVisible;
    Panel2->Visible = panelsVisible;
    Panel3->Visible = panelsVisible;

    if (panelsVisible)
    {
        // 확장 모드
        Panel7->Align = alBottom;
        Panel7->Height = 415; // 원래 높이

        Panel4->Align = alBottom;
        Panel4->Height = 390;

        PanelTitle1->Caption = "Control Menu ▲";
        PanelTitle1->Color = clTeal;
        PanelTitle1->Hint = "Click to hide Control Menu";
    }
    else
    {
        // 축소 모드
        Panel7->Align = alClient;
        Panel4->Align = alClient;
        // Panel4 스크롤을 맨 위로 리셋
        Panel4->VertScrollBar->Position = 0;

        PanelTitle1->Caption = "Control Menu ▶ (Click to expand)";
        PanelTitle1->Color = clLtGray;
        PanelTitle1->Hint = "Click to show Control Menu";
    }

    // 레이아웃 업데이트
    RightPanel->Realign();
    Panel7->Realign();
    Application->ProcessMessages();

    printf("Panels %s, Panel4 %s\n",
           panelsVisible ? "expanded" : "collapsed",
           panelsVisible ? "bottom-aligned" : "client-aligned");
}
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Constructor for the connection thread class
__fastcall TConnectionThread::TConnectionThread(AnsiString host, int port, bool isSBS) : TThread(true)
{
    Host = host;
    Port = port;
    IsSBS = isSBS;
    FreeOnTerminate = true;
}
//---------------------------------------------------------------------------
// Destructor for the connection thread class
__fastcall TConnectionThread::~TConnectionThread()
{
    // Clean up resources if needed
}
//---------------------------------------------------------------------------
// Execute method for connection thread
void __fastcall TConnectionThread::Execute(void)
{
    try
    {
        if (IsSBS)
        {
            Form1->IdTCPClientSBS->Host = Host;
            Form1->IdTCPClientSBS->Port = Port;
            Form1->IdTCPClientSBS->ConnectTimeout = 5000;
            Form1->IdTCPClientSBS->ReadTimeout = 10000;
            Form1->IdTCPClientSBS->Connect();
        }
        else
        {
            Form1->IdTCPClientRaw->Host = Host;
            Form1->IdTCPClientRaw->Port = Port;
            Form1->IdTCPClientRaw->ConnectTimeout = 5000;
            Form1->IdTCPClientRaw->ReadTimeout = 10000;
            Form1->IdTCPClientRaw->Connect();
        }

        // Connection successful, update UI on main thread
        TThread::Synchronize(OnConnectionComplete);
    }
    catch (const Exception &e)
    {
        // Store error message and restore UI on main thread
        ErrorMessage = e.Message;
        TThread::Synchronize(OnConnectionFailed);
    }
}
//---------------------------------------------------------------------------
// UI update method called on main thread
void __fastcall TConnectionThread::OnConnectionComplete(void)
{
    if (IsSBS)
    {
        Form1->TCPClientSBSHandleThread = new TTCPClientSBSHandleThread(true);
        Form1->TCPClientSBSHandleThread->UseFileInsteadOfNetwork = false;
        Form1->TCPClientSBSHandleThread->FreeOnTerminate = TRUE;
        Form1->TCPClientSBSHandleThread->Resume();
        Form1->SBSConnectButton->Caption = "SBS Disconnect";
        Form1->SBSConnectButton->Enabled = true;
		Form1->SBSConnectButton->Color = clMoneyGreen;

        // Add to IP history
        Form1->AddToIpHistory(Form1->SBSIpAddress->Text, true);
    }
    else
    {
        Form1->TCPClientRawHandleThread = new TTCPClientRawHandleThread(true);
        Form1->TCPClientRawHandleThread->UseFileInsteadOfNetwork = false;
        Form1->TCPClientRawHandleThread->FreeOnTerminate = TRUE;
        Form1->TCPClientRawHandleThread->Resume();
        Form1->RawConnectButton->Caption = "Raw Disconnect";
        Form1->RawConnectButton->Enabled = true;
		Form1->RawConnectButton->Color = clMoneyGreen;

        // Add to IP history
        Form1->AddToIpHistory(Form1->RawIpAddress->Text, false);
    }
}
//---------------------------------------------------------------------------
// UI update method called on main thread when connection fails
void __fastcall TConnectionThread::OnConnectionFailed(void)
{
    if (IsSBS)
    {
        Form1->SBSConnectButton->Caption = "SBS Connect";
        Form1->SBSConnectButton->Enabled = true;
		Form1->SBSConnectButton->Color = clMoneyGreen;
    }
    else
    {
        Form1->RawConnectButton->Caption = "Raw Connect";
        Form1->RawConnectButton->Enabled = true;
		Form1->RawConnectButton->Color = clMoneyGreen;
    }
    ShowMessage("Connection failed: " + ErrorMessage);
}

//---------------------------------------------------------------------------
// IP 히스토리 로드
void __fastcall TForm1::LoadIpHistory()
{
    SBSIpHistory = new TStringList();
    RawIpHistory = new TStringList();

    AnsiString configPath = ExtractFilePath(Application->ExeName) + "ip_history.ini";

    if (FileExists(configPath))
    {
        TIniFile *ini = new TIniFile(configPath);
        try
        {
            // SBS IP 히스토리 로드
            int sbsCount = ini->ReadInteger("SBS", "Count", 0);
            for (int i = 0; i < sbsCount && i < MAX_IP_HISTORY; i++)
            {
                AnsiString ip = ini->ReadString("SBS", "IP" + IntToStr(i), "");
                if (ip != "")
                {
                    SBSIpHistory->Add(ip);
                }
            }

            // Raw IP 히스토리 로드
            int rawCount = ini->ReadInteger("Raw", "Count", 0);
            for (int i = 0; i < rawCount && i < MAX_IP_HISTORY; i++)
            {
                AnsiString ip = ini->ReadString("Raw", "IP" + IntToStr(i), "");
                if (ip != "")
                {
                    RawIpHistory->Add(ip);
                }
            }
        }
        __finally
        {
            delete ini;
        }
    }

    // 기본값 추가 (히스토리가 비어있는 경우)
    if (SBSIpHistory->Count == 0)
    {
        SBSIpHistory->Add("data.adsbhub.org");
        SBSIpHistory->Add("128.237.96.41");
    }

    if (RawIpHistory->Count == 0)
    {
        RawIpHistory->Add("raspberrypi");
        RawIpHistory->Add("127.0.0.1");
        RawIpHistory->Add("192.168.1.100");
    }

    LoadIpHistoryToComboBox();
}
//---------------------------------------------------------------------------
// IP 히스토리 저장
void __fastcall TForm1::SaveIpHistory()
{
    AnsiString configPath = ExtractFilePath(Application->ExeName) + "ip_history.ini";
    TIniFile *ini = new TIniFile(configPath);

    try
    {
        // SBS IP 히스토리 저장
        ini->WriteInteger("SBS", "Count", SBSIpHistory->Count);
        for (int i = 0; i < SBSIpHistory->Count; i++)
        {
            ini->WriteString("SBS", "IP" + IntToStr(i), SBSIpHistory->Strings[i]);
        }

        // Raw IP 히스토리 저장
        ini->WriteInteger("Raw", "Count", RawIpHistory->Count);
        for (int i = 0; i < RawIpHistory->Count; i++)
        {
            ini->WriteString("Raw", "IP" + IntToStr(i), RawIpHistory->Strings[i]);
        }
    }
    __finally
    {
        delete ini;
    }
}
//---------------------------------------------------------------------------
// IP 히스토리에 추가
void __fastcall TForm1::AddToIpHistory(AnsiString ip, bool isSBS)
{
    TStringList *history = isSBS ? SBSIpHistory : RawIpHistory;

    // 이미 존재하는지 확인
    int existingIndex = history->IndexOf(ip);
    if (existingIndex >= 0)
    {
        // 이미 존재하면 맨 위로 이동
        history->Move(existingIndex, 0);
    }
    else
    {
        // 새로 추가
        history->Insert(0, ip);

        // 최대 개수 제한
        if (history->Count > MAX_IP_HISTORY)
        {
            history->Delete(MAX_IP_HISTORY);
        }
    }

    // 히스토리 저장
    SaveIpHistory();
    LoadIpHistoryToComboBox();
}
//---------------------------------------------------------------------------
// ComboBox에 히스토리 로드
void __fastcall TForm1::LoadIpHistoryToComboBox()
{
    // SBS ComboBox 업데이트
    SBSIpAddress->Items->Clear();
    for (int i = 0; i < SBSIpHistory->Count; i++)
    {
        SBSIpAddress->Items->Add(SBSIpHistory->Strings[i]);
    }
    if (SBSIpAddress->Items->Count > 0)
    {
        SBSIpAddress->Text = SBSIpHistory->Strings[0];
    }

    // Raw ComboBox 업데이트
    RawIpAddress->Items->Clear();
    for (int i = 0; i < RawIpHistory->Count; i++)
    {
        RawIpAddress->Items->Add(RawIpHistory->Strings[i]);
    }
    if (RawIpAddress->Items->Count > 0)
    {
        RawIpAddress->Text = RawIpHistory->Strings[0];
    }
}

void TForm1::LoadWaypointsFromHttp(const std::string &callsign)
{
    //std::vector<std::pair<double, double>> waypoints = plannedRouteManager->GetWaypoints(callsign);
    plannedRouteManager->GetWaypoints(callsign);
}

// feature AreaFilter
bool __fastcall TForm1::IsAircraftInSelectedAreas(TADS_B_Aircraft *aircraft)
{
    // 1. aircraft 유효성 체크
    if (!aircraft)
    {
        return false;
    }

    // 2. 위치 데이터 유효성 체크
    if (!aircraft->HaveLatLon)
    {
        return false;
    }

    // 3. 필터가 비활성화되어 있거나 선택된 Area가 없으면 모든 유효한 항공기 표시
    if (!areaFilterEnabled || selectedFilterAreas->Count == 0)
    {
        return true;
    }

    // 4. 항공기 위치를 pfVec3 형태로 변환
    pfVec3 aircraftPoint;
    aircraftPoint[0] = aircraft->Longitude;
    aircraftPoint[1] = aircraft->Latitude;
    aircraftPoint[2] = 0.0;

    // 5. 선택된 Area들 중 하나라도 포함되면 true (OR 조건)
    for (int i = 0; i < selectedFilterAreas->Count; i++)
    {
        TArea *area = (TArea *)selectedFilterAreas->Items[i];
        if (area && PointInPolygon(area->Points, area->NumPoints, aircraftPoint))
        {
            return true; // 하나의 Area에라도 포함되면 표시
        }
    }

    return false; // 어떤 Area에도 포함되지 않으면 숨김
}

void __fastcall TForm1::AddAreaToFilter(TArea *area)
{
    if (!area || IsAreaInFilter(area))
    {
        return; // NULL이거나 이미 필터에 있으면 무시
    }

    selectedFilterAreas->Add(area);
    areaFilterEnabled = true;

    printf("Area added to filter: %s (Total: %lld areas)\n",
           area->Name.c_str(), selectedFilterAreas->Count);

    ObjectDisplay->Repaint();
}

void __fastcall TForm1::RemoveAreaFromFilter(TArea *area)
{
    if (!area)
    {
        return;
    }

    int index = selectedFilterAreas->IndexOf(area);
    if (index >= 0)
    {
        selectedFilterAreas->Delete(index);
        printf("Area removed from filter: %s (Remaining: %lld areas)\n",
               area->Name.c_str(), selectedFilterAreas->Count);

        // 필터에 Area가 없으면 필터 비활성화
        if (selectedFilterAreas->Count == 0)
        {
            areaFilterEnabled = false;
        }

        ObjectDisplay->Repaint();
    }
}

void __fastcall TForm1::ClearAreaFilter()
{
    selectedFilterAreas->Clear();
    areaFilterEnabled = false;
    printf("All area filters cleared\n");
    ObjectDisplay->Repaint();
}

void __fastcall TForm1::ToggleAreaInFilter(TArea *area)
{
    if (!area)
    {
        return;
    }

    if (IsAreaInFilter(area))
    {
        RemoveAreaFromFilter(area);
    }
    else
    {
        AddAreaToFilter(area);
    }
}

bool __fastcall TForm1::IsAreaInFilter(TArea *area)
{
    if (!area)
    {
        return false;
    }

    return selectedFilterAreas->IndexOf(area) >= 0;
}

int __fastcall TForm1::GetFilteredAreaCount()
{
    return selectedFilterAreas->Count;
}

void __fastcall TForm1::FormKeyDown(TObject *Sender, WORD &Key, TShiftState Shift)
{
    // ComboBox에 포커스가 있을 때는 Key로 맵 이동 하지 않음.
    if (ActiveControl && 
        (ActiveControl == MapComboBox || 
         ActiveControl == AircraftTypeFilterComboBox ||
         ActiveControl == AltitudeFilterComboBox ||
         ActiveControl == SpeedFilterComboBox ||
         ActiveControl == SBSIpAddress ||
         ActiveControl == RawIpAddress ||
         ActiveControl->ClassName() == "TComboBox"))
    {
        return; // Key는 각 Compbo Box에서 처리됨.
    }

    // 키 반복 방지 - static 변수로 이전 키 추적
    static WORD lastKey = 0;
    static DWORD lastKeyTime = 0;
    DWORD currentTime = GetTickCount();
    
    // 같은 키가 50ms 이내에 다시 들어오면 반복으로 간주
    if (Key == lastKey && (currentTime - lastKeyTime) < 50) {
        return; // 반복 키는 무시
    }
    
    lastKey = Key;
    lastKeyTime = currentTime;
    
    printf("Key pressed: %d (0x%X)\n", Key, Key);
    
    // g_EarthView가 초기화되지 않았으면 스킵
    if (!g_EarthView)
        return;
    
    switch (Key)
    {
    case VK_UP: // 위쪽 화살표
        lastArrowDir = DIR_VERT;
        if (MapVScrollBar->Visible) {
            MapVScrollBar->Position -= MapVScrollBar->SmallChange;
            int pos = MapVScrollBar->Position;
            MapVScrollBar->OnScroll(MapVScrollBar, scLineUp, pos);
        }
        break;
    case VK_DOWN: // 아래쪽 화살표
        lastArrowDir = DIR_VERT;
        if (MapVScrollBar->Visible) {
            MapVScrollBar->Position += MapVScrollBar->SmallChange;
            int pos = MapVScrollBar->Position;
            MapVScrollBar->OnScroll(MapVScrollBar, scLineDown, pos);
        }
        break;
    case VK_LEFT: // 왼쪽 화살표
        lastArrowDir = DIR_HORZ;
        if (MapHScrollBar->Visible) {
            MapHScrollBar->Position -= MapHScrollBar->SmallChange;
            int pos = MapHScrollBar->Position;
            MapHScrollBar->OnScroll(MapHScrollBar, scLineUp, pos);
        }
        break;
    case VK_RIGHT: // 오른쪽 화살표
        lastArrowDir = DIR_HORZ;
        if (MapHScrollBar->Visible) {
            MapHScrollBar->Position += MapHScrollBar->SmallChange;
            int pos = MapHScrollBar->Position;
            MapHScrollBar->OnScroll(MapHScrollBar, scLineDown, pos);
        }
        break;
    case VK_PRIOR: // Page Up
        if (lastArrowDir == DIR_HORZ) {
            if (MapHScrollBar->Visible) {
                MapHScrollBar->Position -= MapHScrollBar->LargeChange;
                int pos = MapHScrollBar->Position;
                MapHScrollBar->OnScroll(MapHScrollBar, scPageUp, pos);
            }
        } else {
            if (MapVScrollBar->Visible) {
                MapVScrollBar->Position -= MapVScrollBar->LargeChange;
                int pos = MapVScrollBar->Position;
                MapVScrollBar->OnScroll(MapVScrollBar, scPageUp, pos);
            }
        }
        break;
    case VK_NEXT: // Page Down
        if (lastArrowDir == DIR_HORZ) {
            if (MapHScrollBar->Visible) {
                MapHScrollBar->Position += MapHScrollBar->LargeChange;
                int pos = MapHScrollBar->Position;
                MapHScrollBar->OnScroll(MapHScrollBar, scPageDown, pos);
            }
        } else {
            if (MapVScrollBar->Visible) {
                MapVScrollBar->Position += MapVScrollBar->LargeChange;
                int pos = MapVScrollBar->Position;
                MapVScrollBar->OnScroll(MapVScrollBar, scPageDown, pos);
            }
        }
        break;
    case VK_HOME: // Home 키: 피츠버그로 이동
        MapCenterLat = 40.4406;   // 피츠버그 위도
        MapCenterLon = -79.9959;  // 피츠버그 경도
        SetMapCenter(g_EarthView->m_Eye.x, g_EarthView->m_Eye.y);
        UpdateScrollBarRanges();
        UpdateScrollBarPositions();
        ObjectDisplay->Repaint();
        break;
    case VK_END: // End 키: 비활성화 (아무 동작 안 함)
        break;
    case VK_F1:
        printf("F1 Key Pressed - User Manual\n");
        ShellExecute(0, L"open", L"https://github.com/lgswa-25t2/ADS-B-Display/blob/main/docs/usermanual/User_Guide_EN.md", NULL, NULL, SW_SHOWNORMAL);
        break;
    case VK_F2:
        printf("========================\n");
        printf("F2 Key Pressed - Debug Info:\n");
        printf("MapCenterLat: %.6f\n", MapCenterLat);
        printf("MapCenterLon: %.6f\n", MapCenterLon);
        printf("g_EarthView->m_Eye.x: %.6f\n", g_EarthView->m_Eye.x);
        printf("g_EarthView->m_Eye.y: %.6f\n", g_EarthView->m_Eye.y);
        printf("g_EarthView->m_Eye.h: %.6f\n", g_EarthView->m_Eye.h);
        printf("========================\n");
        break;
    }
}

//---------------------------------------------------------------------------
void __fastcall TForm1::FormKeyUp(TObject *Sender, WORD &Key, TShiftState Shift)
{
    // 키를 떼면 lastKey 초기화
    static WORD lastKey = 0;
    if (Key == lastKey) {
        lastKey = 0;
        printf("Key released: %d (0x%X)\n", Key, Key);
    }
}

void __fastcall TForm1::DrawAllAirports()
{
    // Display Airports Info (DisplayAirportCheckBox is checked)
    if (!DisplayAirportCheckBox->Checked || !airportManager)
    {
        return; // Early return if conditions not met
    }

    // Calculate current screen bounds
    double minLat, maxLat, minLon, maxLon;
    int screenWidth = ObjectDisplay->Width;
    int screenHeight = ObjectDisplay->Height;

    // Convert screen corner coordinates to latitude and longitude
    XY2LatLon2(0, 0, minLat, maxLon);
    XY2LatLon2(screenWidth, screenHeight, maxLat, minLon);

    // Current zoom level
    int zoomLevel = (int)(log(g_EarthView->m_Eye.h) / log(1.3));

    // Get airports to display on the screen
    auto visibleAirports = airportManager->getVisibleAirports(
        minLat, maxLat, minLon, maxLon, zoomLevel);

    // Set airport color
    glColor4f(0.8, 0.0, 0.0, 1.0); // red

    // Draw airports on the screen
    for (const auto &airport : visibleAirports)
    {
        double airportX, airportY;
        LatLon2XY(airport.latitude, airport.longitude, airportX, airportY);

        // Get icon size based on zoom level
        float size = airportManager->getAirportIconSize(airport, zoomLevel);

        // 1. 내부 채우기
        glColor4f(1.0, 0.0, 0.0, 1.0);
        // glColor4f(1.0, 1.0, 0.6, 1.0);
        glBegin(GL_QUADS);
        glVertex2f(airportX - size, airportY - size * 0.6f); // bottom left
        glVertex2f(airportX + size, airportY - size * 0.6f); // bottom right
        glVertex2f(airportX + size, airportY + size * 0.2f); // top right
        glVertex2f(airportX - size, airportY + size * 0.2f); // top left
        glEnd();

        // Draw base building (rectangle)
        glColor4f(1.0, 1.0, 0.6, 1.0);
        glBegin(GL_LINE_LOOP);
        glVertex2f(airportX - size, airportY - size * 0.6f); // bottom left
        glVertex2f(airportX + size, airportY - size * 0.6f); // bottom right
        glVertex2f(airportX + size, airportY + size * 0.2f); // top right
        glVertex2f(airportX - size, airportY + size * 0.2f); // top left
        glEnd();

        // === Control Tower ===
        // 1. 내부 채우기
        glColor4f(1.0, 0.0, 0.0, 1.0);
        // glColor4f(1.0, 1.0, 0.6, 1.0);
        glBegin(GL_QUADS);
        glVertex2f(airportX - size * 0.5f, airportY + size * 0.2f); // bottom left
        glVertex2f(airportX + size * 0.5f, airportY + size * 0.2f); // bottom right
        glVertex2f(airportX + size * 0.5f, airportY + size);        // top right
        glVertex2f(airportX - size * 0.5f, airportY + size);        // top left
        glEnd();

        // Draw control tower (smaller rectangle on top)
        glColor4f(1.0, 1.0, 0.6, 1.0);
        glBegin(GL_LINE_LOOP);
        glVertex2f(airportX - size * 0.5f, airportY + size * 0.2f); // bottom left
        glVertex2f(airportX + size * 0.5f, airportY + size * 0.2f); // bottom right
        glVertex2f(airportX + size * 0.5f, airportY + size);        // top right
        glVertex2f(airportX - size * 0.5f, airportY + size);        // top left
        glEnd();

        // Draw antenna on top
        glBegin(GL_LINES);
        glVertex2f(airportX, airportY + size);        // tower top
        glVertex2f(airportX, airportY + size * 1.3f); // antenna top
        glEnd();
    }
}

// BigQuery related methods
//---------------------------------------------------------------------------
void __fastcall TForm1::InitializeBigQuery(void)
{
    try {
        bigquery_client_ = std::make_unique<BigQueryClient>();

        if (bigquery_client_->Initialize(bigquery_credentials_path_.c_str())) {
            bigquery_initialized_ = true;
            printf("BigQuery client initialized successfully\n");
        } else {
            printf("Failed to initialize BigQuery client: %s\n",
                   bigquery_client_->GetLastError().c_str());
            bigquery_initialized_ = false;
        }
    }
    catch (const std::exception& e) {
        printf("Exception during BigQuery initialization: %s\n", e.what());
        bigquery_initialized_ = false;
    }
}

void __fastcall TForm1::AircraftTypeFilterComboBoxCloseUp(TObject *Sender)
{
    // 실제로 선택이 바뀌었을 때만 처리
    if (SelectedAircraftTypeFilter != AircraftTypeFilterComboBox->ItemIndex)
    {
        SelectedAircraftTypeFilter = AircraftTypeFilterComboBox->ItemIndex;
        ObjectDisplay->Repaint();
    }
}
void __fastcall TForm1::GetTimeToGoLineColor(double speed, float &r, float &g, float &b, float &alpha)
{
    alpha = 1.0f; // 기본 투명도

    if (speed >= 600)
    {
        // 초고속 (600+ knots) - 빨간색 (위험/고속)
        r = 1.0f;
        g = 0.0f;
        b = 0.0f;
    }
    else if (speed >= 400)
    {
        // 고속 (400-600 knots) - 주황색
        r = 1.0f;
        g = 0.5f;
        b = 0.0f;
    }
    else if (speed >= 250)
    {
        // 중고속 (250-400 knots) - 노란색 (현재 기본색)
        r = 1.0f;
        g = 1.0f;
        b = 0.0f;
    }
    else
    {
        // 중속 (150-250 knots) - 연두색
        r = 0.5f;
        g = 1.0f;
        b = 0.0f;
    }
}

void __fastcall TForm1::LiveMapCheckboxClick(TObject *Sender)
{
    // 현재 맵 타입이 온라인을 지원하는지 확인
    int currentMapIndex = MapComboBox->ItemIndex;
    bool supportsOnline = (currentMapIndex == 0 || currentMapIndex == 4);

    if (!supportsOnline && LiveMapCheckbox->Checked)
    {
        // 지원하지 않는 맵에서 체크하려고 하면 막기
        LiveMapCheckbox->Checked = false;
        ShowMessage("This map type does not support online mode");
        return;
    }

    // 상태가 바뀌었으면 현재 맵 다시 로드
    if (LoadMapFromInternet != LiveMapCheckbox->Checked)
    {
        // 기존 방식과 동일하게 처리
        if (SelectedMapIndex != MapComboBox->ItemIndex)
        {
            // 이미 변경 중이면 처리하지 않음
            return;
        }

        // 수동으로 SelectedMapIndex를 다른 값으로 만들어서 CloseUp 로직 실행
        int currentIndex = SelectedMapIndex;
        SelectedMapIndex = -1;           // 강제로 다르게 만들기
        MapComboBoxCloseUp(Sender);      // 기존 로직 재사용
        SelectedMapIndex = currentIndex; // 복원
    }
}
//---------------------------------------------------------------------------
void __fastcall TForm1::MapScrollBoxScroll(TObject *Sender, TScrollBarKind ScrollBarKind, int ScrollCode, int &ScrollPos)
{
    // 스크롤바를 통한 지도 네비게이션 처리
    if (ScrollCode == SB_THUMBPOSITION || ScrollCode == SB_THUMBTRACK)
    {
        double deltaX = 0, deltaY = 0;

        if (ScrollBarKind == sbHorizontal)
        {
            // 수평 스크롤 - 경도 변경
            // 스크롤바 위치를 경도로 변환 (0~36000 -> -180~180)
            double newLon = (ScrollPos / 100.0) - 180.0;
            deltaX = newLon - MapCenterLon;
        }
        else if (ScrollBarKind == sbVertical)
        {
            // 수직 스크롤 - 위도 변경
            // 스크롤바 위치를 위도로 변환 (0~17000 -> -85~85)
            double newLat = (ScrollPos / 100.0) - 85.0;
            deltaY = newLat - MapCenterLat;
        }

        // 지도 중심점 업데이트
        if (deltaX != 0 || deltaY != 0)
        {
            MapCenterLon += deltaX;
            MapCenterLat += deltaY;

            // 경계값 체크
            if (MapCenterLat > 85.0)
                MapCenterLat = 85.0;
            if (MapCenterLat < -85.0)
                MapCenterLat = -85.0;
            if (MapCenterLon > 180.0)
                MapCenterLon = 180.0;
            if (MapCenterLon < -180.0)
                MapCenterLon = -180.0;

            // 지도 중심점 설정
            SetMapCenter(g_EarthView->m_Eye.x, g_EarthView->m_Eye.y);
            ObjectDisplay->Repaint();
        }
    }
}

//---------------------------------------------------------------------------
void __fastcall TForm1::MapScrollBoxVScroll(TObject *Sender, TScrollCode ScrollCode, int &ScrollPos)
{
    // 수직 스크롤바를 통한 지도 네비게이션 처리
    if (ScrollCode == scTrack || ScrollCode == scPosition)
    {
        // PageSize를 고려한 실제 이동 가능한 범위로 ScrollPos를 clamp
        int actualMaxVert = MapVScrollBar->Max - MapVScrollBar->PageSize;
        if (actualMaxVert < MapVScrollBar->Min)
            actualMaxVert = MapVScrollBar->Min;
        if (ScrollPos > actualMaxVert)
            ScrollPos = actualMaxVert;
        if (ScrollPos < MapVScrollBar->Min)
            ScrollPos = MapVScrollBar->Min;

        // 스크롤바 위치를 위도로 변환 (정확한 매핑)
        double scrollableLatRange = actualMaxVert - MapVScrollBar->Min;
        double latRatio = (scrollableLatRange > 0) ? (double)(ScrollPos - MapVScrollBar->Min) / scrollableLatRange : 0.0;
        double newLat = 85.0 - (latRatio * 170.0); // 85 ~ -85 (위에서 아래로)

        // 검은색 영역을 건너뛰고 바로 지도 영역으로 이동
        // 현재 줌 레벨에 따른 지도 높이 계산
        double zoomLevel = g_EarthView->m_Eye.h;
        double mapHeight = 170.0 * zoomLevel;

        // 지도가 화면을 완전히 채우는 경우의 경계값 계산
        double maxLat = 85.0 - mapHeight / 2.0;
        double minLat = -85.0 + mapHeight / 2.0;

        // 경계값으로 조정
        if (newLat > maxLat)
            newLat = maxLat;
        if (newLat < minLat)
            newLat = minLat;

        double deltaY = newLat - MapCenterLat;

        // 지도 중심점 업데이트
        if (deltaY != 0)
        {
            MapCenterLat = newLat;

            // 지도 중심점 설정
            SetMapCenter(g_EarthView->m_Eye.x, g_EarthView->m_Eye.y);
            ObjectDisplay->Repaint();
        }
    }
}

//---------------------------------------------------------------------------
void __fastcall TForm1::MapScrollBoxHScroll(TObject *Sender, TScrollCode ScrollCode, int &ScrollPos)
{
    // 수평 스크롤바를 통한 지도 네비게이션 처리
    if (ScrollCode == scTrack || ScrollCode == scPosition)
    {
        // PageSize를 고려한 실제 이동 가능한 범위로 ScrollPos를 clamp
        int actualMaxHorz = MapHScrollBar->Max - MapHScrollBar->PageSize;
        if (actualMaxHorz < MapHScrollBar->Min)
            actualMaxHorz = MapHScrollBar->Min;
        if (ScrollPos > actualMaxHorz)
            ScrollPos = actualMaxHorz;
        if (ScrollPos < MapHScrollBar->Min)
            ScrollPos = MapHScrollBar->Min;
        // 스크롤바 위치를 경도로 변환 (정확한 매핑)
        double scrollableLonRange = actualMaxHorz - MapHScrollBar->Min;
        double lonRatio = (scrollableLonRange > 0) ? (double)(ScrollPos - MapHScrollBar->Min) / scrollableLonRange : 0.0;
        double newLon = (lonRatio * 360.0) - 180.0;

        // 경계값 체크 (전체 경도 범위)
        if (newLon > 180.0)
            newLon = 180.0;
        if (newLon < -180.0)
            newLon = -180.0;

        if (MapCenterLon != newLon)
        {
            MapCenterLon = newLon;
            SetMapCenter(g_EarthView->m_Eye.x, g_EarthView->m_Eye.y);
            ObjectDisplay->Repaint();
        }
    }
    else if (ScrollCode == scLineUp || ScrollCode == scLineDown)
    {
        // 화살표 키 처리 (수평 스크롤바에서는 좌우 이동)
        double deltaX = 0;
        if (ScrollCode == scLineUp)
        {
            deltaX = -MapHScrollBar->SmallChange / 100.0; // 왼쪽으로
        }
        else
        {
            deltaX = MapHScrollBar->SmallChange / 100.0; // 오른쪽으로
        }

        MapCenterLon += deltaX;

        // 경계값 체크 (전체 경도 범위 - 화살표 키는 끝까지 이동 가능)
        if (MapCenterLon > 180.0) MapCenterLon = 180.0;
        if (MapCenterLon < -180.0) MapCenterLon = -180.0;

        // 지도 중심점 설정
        SetMapCenter(g_EarthView->m_Eye.x, g_EarthView->m_Eye.y);
        ObjectDisplay->Repaint();
    }
    else if (ScrollCode == scPageUp || ScrollCode == scPageDown)
    {
        // Page 키 처리 (수평 스크롤바에서는 좌우 이동)
        double deltaX = 0;
        if (ScrollCode == scPageUp)
        {
            deltaX = -MapHScrollBar->LargeChange / 100.0; // 왼쪽으로
        }
        else
        {
            deltaX = MapHScrollBar->LargeChange / 100.0; // 오른쪽으로
        }

        MapCenterLon += deltaX;

        // 경계값 체크 (검은색 영역 제외)
        double zoomLevel = g_EarthView->m_Eye.h;
        double mapWidth = 360.0 * zoomLevel;
        double maxLon = 180.0 - mapWidth / 2.0;
        double minLon = -180.0 + mapWidth / 2.0;

        if (MapCenterLon > maxLon)
            MapCenterLon = maxLon;
        if (MapCenterLon < minLon)
            MapCenterLon = minLon;

        // 지도 중심점 설정
        SetMapCenter(g_EarthView->m_Eye.x, g_EarthView->m_Eye.y);
        ObjectDisplay->Repaint();
    }
}

//---------------------------------------------------------------------------
void __fastcall TForm1::MapVScrollBarScroll(TObject *Sender, TScrollCode ScrollCode, int &ScrollPos)
{
    // 수직 스크롤바를 통한 지도 네비게이션 처리
    if (ScrollCode == scTrack || ScrollCode == scPosition)
    {
        // PageSize를 고려한 실제 이동 가능한 범위로 ScrollPos를 clamp
        int actualMaxVert = MapVScrollBar->Max - MapVScrollBar->PageSize;
        if (actualMaxVert < MapVScrollBar->Min)
            actualMaxVert = MapVScrollBar->Min;
        if (ScrollPos > actualMaxVert)
            ScrollPos = actualMaxVert;
        if (ScrollPos < MapVScrollBar->Min)
            ScrollPos = MapVScrollBar->Min;

        // 스크롤바 위치를 위도로 변환 (정확한 매핑)
        double scrollableLatRange = actualMaxVert - MapVScrollBar->Min;
        double latRatio = (scrollableLatRange > 0) ? (double)(ScrollPos - MapVScrollBar->Min) / scrollableLatRange : 0.0;
        double newLat = 85.0 - (latRatio * 170.0); // 85 ~ -85 (위에서 아래로)

        // 경계값 체크 (전체 위도 범위)
        if (newLat > 85.0)
            newLat = 85.0;
        if (newLat < -85.0)
            newLat = -85.0;

        if (MapCenterLat != newLat)
        {
            MapCenterLat = newLat;
            SetMapCenter(g_EarthView->m_Eye.x, g_EarthView->m_Eye.y);
            ObjectDisplay->Repaint();
        }
    }
    else if (ScrollCode == scLineUp || ScrollCode == scLineDown)
    {
        // 화살표 키 처리 (수직 스크롤바에서는 상하 이동)
        double deltaY = 0;
        if (ScrollCode == scLineUp)
        {
            deltaY = MapVScrollBar->SmallChange / 100.0; // 위로: 위도 증가(북쪽)
        }
        else
        {
            deltaY = -MapVScrollBar->SmallChange / 100.0; // 아래로: 위도 감소(남쪽)
        }

        MapCenterLat += deltaY;

        // 경계값 체크 (전체 위도 범위 - 화살표 키는 끝까지 이동 가능)
        if (MapCenterLat > 85.0) MapCenterLat = 85.0;
        if (MapCenterLat < -85.0) MapCenterLat = -85.0;

        // 지도 중심점 설정
        SetMapCenter(g_EarthView->m_Eye.x, g_EarthView->m_Eye.y);
        ObjectDisplay->Repaint();
    }
    else if (ScrollCode == scPageUp || ScrollCode == scPageDown)
    {
        // Page 키 처리 (수직 스크롤바에서는 상하 이동)
        double deltaY = 0;
        if (ScrollCode == scPageUp)
        {
            deltaY = MapVScrollBar->LargeChange / 100.0; // 위로: 위도 증가(북쪽)
        }
        else
        {
            deltaY = -MapVScrollBar->LargeChange / 100.0; // 아래로: 위도 감소(남쪽)
        }

        MapCenterLat += deltaY;

        // 경계값 체크 (전체 위도 범위 - Page 키도 끝까지 이동 가능)
        if (MapCenterLat > 85.0) MapCenterLat = 85.0;
        if (MapCenterLat < -85.0) MapCenterLat = -85.0;

        // 지도 중심점 설정
        SetMapCenter(g_EarthView->m_Eye.x, g_EarthView->m_Eye.y);
        ObjectDisplay->Repaint();
    }
}

//---------------------------------------------------------------------------
void __fastcall TForm1::UpdateScrollBarRanges()
{
    if (!MapVScrollBar || !MapHScrollBar)
        return;

    // 현재 줌 레벨과 화면 크기에 따라 스크롤바 표시 여부 결정
    double zoomLevel = g_EarthView->m_Eye.h;
    int screenWidth = ObjectDisplay->Width;
    int screenHeight = ObjectDisplay->Height;

    // 스크롤바는 항상 표시하되, 크기와 범위를 동적으로 조정
    bool showScrollbars = true;
    bool showVertical = true;
    bool showHorizontal = true;

    // 지도가 화면 밖으로 나가서 검은색 영역이 보이는지 확인
    bool mapOutOfBounds = false;

    // 지도의 실제 표시 영역 계산
    double mapWidth = 360.0 * zoomLevel;  // 지도 경도 범위
    double mapHeight = 170.0 * zoomLevel; // 지도 위도 범위

    // 지도 중심점에서 지도 영역의 경계 계산
    double mapLeft = MapCenterLon - mapWidth / 2.0;
    double mapRight = MapCenterLon + mapWidth / 2.0;
    double mapTop = MapCenterLat + mapHeight / 2.0;
    double mapBottom = MapCenterLat - mapHeight / 2.0;

    // 지도가 화면 밖으로 나가는지 확인 (디버그용)
    if (mapLeft < -180.0 || mapRight > 180.0 || mapTop > 85.0 || mapBottom < -85.0)
    {
        mapOutOfBounds = true;
    }

    // 화면 크기가 너무 작으면 스크롤바 숨김
    if (screenWidth < 200 || screenHeight < 200)
    {
        showScrollbars = false;
        showVertical = false;
        showHorizontal = false;
    }

    // 스크롤바 크기를 화면 크기에 맞게 조정
    int scrollBarSize = 17; // 기본 크기
    if (screenWidth > 800 && screenHeight > 600)
    {
        scrollBarSize = 20; // 큰 화면에서는 더 큰 스크롤바
    }
    else if (screenWidth < 400 || screenHeight < 300)
    {
        scrollBarSize = 14; // 작은 화면에서는 더 작은 스크롤바
    }

    // 스크롤바 크기 설정
    MapVScrollBar->Width = scrollBarSize;
    MapHScrollBar->Height = scrollBarSize;

    // 줌 레벨에 따라 스크롤바 범위와 PageSize 동적 조정
    // 검은색 빈공간도 감안하여 PageSize 계산

    // 전체 지도 범위 (고정)
    const int TOTAL_LON_RANGE = 36000; // -180도 ~ 180도 * 100
    const int TOTAL_LAT_RANGE = 17000; // -85도 ~ 85도 * 100

    // 줌 레벨에 따른 현재 보이는 영역 계산
    // 줌이 확대될수록 (zoomLevel이 작을수록) 보이는 영역이 작아짐
    double visibleLonRange = TOTAL_LON_RANGE * zoomLevel;
    double visibleLatRange = TOTAL_LAT_RANGE * zoomLevel;

    // 검은색 빈공간을 감안한 PageSize 계산
    // 지도가 화면을 완전히 채우지 않을 때는 PageSize를 더 크게 설정
    double effectiveLonRange = visibleLonRange;
    double effectiveLatRange = visibleLatRange;

    // 지도가 화면 밖으로 나가는 경우 PageSize 조정
    if (mapLeft < -180.0)
    {
        effectiveLonRange += (-180.0 - mapLeft) * 100; // 왼쪽 검은색 영역
    }
    if (mapRight > 180.0)
    {
        effectiveLonRange += (mapRight - 180.0) * 100; // 오른쪽 검은색 영역
    }
    if (mapTop > 85.0)
    {
        effectiveLatRange += (mapTop - 85.0) * 100; // 위쪽 검은색 영역
    }
    if (mapBottom < -85.0)
    {
        effectiveLatRange += (-85.0 - mapBottom) * 100; // 아래쪽 검은색 영역
    }

    // 최소/최대 PageSize 제한 (너무 작거나 크지 않도록)
    int minPageSize = 50;                  // 최소 50
    int maxPageSize = TOTAL_LON_RANGE / 2; // 최대 전체의 절반

    // PageSize 계산 (검은색 빈공간 감안)
    int horzPageSize = (int)(effectiveLonRange);
    int vertPageSize = (int)(effectiveLatRange);

    // PageSize 제한 적용
    if (horzPageSize < minPageSize)
        horzPageSize = minPageSize;
    if (horzPageSize > maxPageSize)
        horzPageSize = maxPageSize;
    if (vertPageSize < minPageSize)
        vertPageSize = minPageSize;
    if (vertPageSize > maxPageSize)
        vertPageSize = maxPageSize;

    // 수평 스크롤바 설정 (경도)
    MapHScrollBar->Min = 0;
    MapHScrollBar->Max = TOTAL_LON_RANGE; // 전체 범위로 설정
    MapHScrollBar->PageSize = horzPageSize;

    // SmallChange와 LargeChange를 더 크게 설정하여 확대 시에도 움직이도록 함
    int minSmallChange = 1000; // 최소 10도 (1000/100)
    int minLargeChange = 5000; // 최소 50도 (5000/100)

    MapHScrollBar->LargeChange = max(horzPageSize / 4, minLargeChange);
    MapHScrollBar->SmallChange = max(horzPageSize / 20, minSmallChange);

    // 수직 스크롤바 설정 (위도)
    MapVScrollBar->Min = 0;
    MapVScrollBar->Max = TOTAL_LAT_RANGE; // 전체 범위로 설정
    MapVScrollBar->PageSize = vertPageSize;
    MapVScrollBar->LargeChange = max(vertPageSize / 4, minLargeChange);
    MapVScrollBar->SmallChange = max(vertPageSize / 20, minSmallChange);

    // 확대 완전히 했을 때도 스크롤바가 움직이도록 최소값 보장 (더 큰 값으로 설정)
    if (MapHScrollBar->SmallChange < minSmallChange)
        MapHScrollBar->SmallChange = minSmallChange;
    if (MapVScrollBar->SmallChange < minSmallChange)
        MapVScrollBar->SmallChange = minSmallChange;
    if (MapHScrollBar->LargeChange < minLargeChange)
        MapHScrollBar->LargeChange = minLargeChange;
    if (MapVScrollBar->LargeChange < minLargeChange)
        MapVScrollBar->LargeChange = minLargeChange;

    // 스크롤바 표시/숨김 설정
    MapVScrollBar->Visible = showVertical;
    MapHScrollBar->Visible = showHorizontal;

    // 디버그 정보 출력 (선택사항)
    if (showScrollbars)
    {
        // printf("Scrollbars: zoom=%.3f, H_Page=%d/36000 (%.1f%%), V_Page=%d/17000 (%.1f%%)\n",
        //     zoomLevel, horzPageSize, (float)horzPageSize/TOTAL_LON_RANGE*100,
        //     vertPageSize, (float)vertPageSize/TOTAL_LAT_RANGE*100);
        // printf("H_Small=%d, H_Large=%d, V_Small=%d, V_Large=%d\n",
        //     MapHScrollBar->SmallChange, MapHScrollBar->LargeChange,
        //     MapVScrollBar->SmallChange, MapVScrollBar->LargeChange);

        if (mapOutOfBounds)
        {
            printf("Map out of bounds: Lat=%.2f, Lon=%.2f\n", MapCenterLat, MapCenterLon);
        }
    }

    // SmallChange를 보이는 영역의 1/10로 동적으로 설정
    int horzSmallChange = std::max(1, int((effectiveLonRange / 10.0)));
    int vertSmallChange = std::max(1, int((effectiveLatRange / 10.0)));
    MapHScrollBar->SmallChange = horzSmallChange;
    MapVScrollBar->SmallChange = vertSmallChange;

    // LargeChange를 보이는 영역의 1/2로 동적으로 설정
    int horzLargeChange = std::max(1, int(effectiveLonRange));
    int vertLargeChange = std::max(1, int(effectiveLatRange));
    MapHScrollBar->LargeChange = horzLargeChange;
    MapVScrollBar->LargeChange = vertLargeChange;
}

//---------------------------------------------------------------------------
void __fastcall TForm1::UpdateScrollBarPositions()
{
    if (!MapVScrollBar || !MapHScrollBar)
        return;

    // ObjectDisplay의 위치와 크기에 따라 스크롤바 위치 설정
    int objLeft = ObjectDisplay->Left;
    int objTop = ObjectDisplay->Top;
    int objWidth = ObjectDisplay->Width;
    int objHeight = ObjectDisplay->Height;

    // 스크롤바가 보이는 경우에만 위치 조정
    if (MapVScrollBar->Visible && MapHScrollBar->Visible)
    {
        // 수직 스크롤바 위치 설정 (ObjectDisplay 오른쪽)
        MapVScrollBar->Left = objLeft + objWidth - MapVScrollBar->Width;
        MapVScrollBar->Top = objTop;
        MapVScrollBar->Height = objHeight - MapHScrollBar->Height; // 수평 스크롤바 공간 제외

        // 수평 스크롤바 위치 설정 (ObjectDisplay 아래쪽)
        MapHScrollBar->Left = objLeft;
        MapHScrollBar->Top = objTop + objHeight - MapHScrollBar->Height;
        MapHScrollBar->Width = objWidth - MapVScrollBar->Width; // 수직 스크롤바 공간 제외
    }
    else if (MapVScrollBar->Visible)
    {
        // 수직 스크롤바만 보이는 경우
        MapVScrollBar->Left = objLeft + objWidth - MapVScrollBar->Width;
        MapVScrollBar->Top = objTop;
        MapVScrollBar->Height = objHeight;
    }
    else if (MapHScrollBar->Visible)
    {
        // 수평 스크롤바만 보이는 경우
        MapHScrollBar->Left = objLeft;
        MapHScrollBar->Top = objTop + objHeight - MapHScrollBar->Height;
        MapHScrollBar->Width = objWidth;
    }

    // 현재 지도 중심점에 따라 스크롤바 위치 업데이트
    // 스크롤바 범위를 고려한 정확한 매핑
    // 스크롤바 범위: 0 ~ (Max)
    // 경도 범위: -180 ~ 180
    // PageSize를 고려하여 실제 이동 가능한 범위 계산

    // 수평 스크롤바 위치 계산
    double scrollableLonRange = MapHScrollBar->Max - MapHScrollBar->Min;
    double lonRatio = (MapCenterLon + 180.0) / 360.0; // 0.0 ~ 1.0
    int horzPos = MapHScrollBar->Min + (int)(lonRatio * scrollableLonRange);

    // 경계값 체크
    if (horzPos > MapHScrollBar->Max)
        horzPos = MapHScrollBar->Max;
    if (horzPos < MapHScrollBar->Min)
        horzPos = MapHScrollBar->Min;
    MapHScrollBar->Position = horzPos;

    // 수직 스크롤바 위치 계산
    int actualMaxVert = MapVScrollBar->Max - MapVScrollBar->PageSize;
    if (actualMaxVert < MapVScrollBar->Min)
        actualMaxVert = MapVScrollBar->Min;
    double latRatio = (85.0 - MapCenterLat) / 170.0; // 0.0 ~ 1.0 (위에서 아래로)
    int vertPos = MapVScrollBar->Min + (int)(latRatio * (actualMaxVert - MapVScrollBar->Min));
    if (vertPos > actualMaxVert)
        vertPos = actualMaxVert;
    if (vertPos < MapVScrollBar->Min)
        vertPos = MapVScrollBar->Min;
    MapVScrollBar->Position = vertPos;

    // 디버그 출력
    // printf("Scroll Debug: Lon=%.2f, Lat=%.2f, H_Pos=%d/%d, V_Pos=%d/%d\n",
    //     MapCenterLon, MapCenterLat, horzPos, MapHScrollBar->Max, vertPos, MapVScrollBar->Max);
}

//---------------------------------------------------------------------------
void __fastcall TForm1::MapHScrollBarScroll(TObject *Sender, TScrollCode ScrollCode, int &ScrollPos)
{
    // 수평 스크롤바를 통한 지도 네비게이션 처리
    if (ScrollCode == scTrack || ScrollCode == scPosition)
    {
        // PageSize를 고려한 실제 이동 가능한 범위로 ScrollPos를 clamp
        int actualMaxHorz = MapHScrollBar->Max - MapHScrollBar->PageSize;
        if (actualMaxHorz < MapHScrollBar->Min)
            actualMaxHorz = MapHScrollBar->Min;
        if (ScrollPos > actualMaxHorz)
            ScrollPos = actualMaxHorz;
        if (ScrollPos < MapHScrollBar->Min)
            ScrollPos = MapHScrollBar->Min;
        // 스크롤바 위치를 경도로 변환 (정확한 매핑)
        double scrollableLonRange = actualMaxHorz - MapHScrollBar->Min;
        double lonRatio = (scrollableLonRange > 0) ? (double)(ScrollPos - MapHScrollBar->Min) / scrollableLonRange : 0.0;
        double newLon = (lonRatio * 360.0) - 180.0;

        // 경계값 체크 (전체 경도 범위)
        if (newLon > 180.0)
            newLon = 180.0;
        if (newLon < -180.0)
            newLon = -180.0;

        if (MapCenterLon != newLon)
        {
            MapCenterLon = newLon;
            SetMapCenter(g_EarthView->m_Eye.x, g_EarthView->m_Eye.y);
            ObjectDisplay->Repaint();
        }
    }
    else if (ScrollCode == scLineUp || ScrollCode == scLineDown)
    {
        // 화살표 키 처리 (수평 스크롤바에서는 좌우 이동)
        double deltaX = 0;
        if (ScrollCode == scLineUp)
        {
            deltaX = -MapHScrollBar->SmallChange / 100.0; // 왼쪽으로
        }
        else
        {
            deltaX = MapHScrollBar->SmallChange / 100.0; // 오른쪽으로
        }

        MapCenterLon += deltaX;

        // 경계값 체크 (전체 경도 범위 - 화살표 키는 끝까지 이동 가능)
        if (MapCenterLon > 180.0) MapCenterLon = 180.0;
        if (MapCenterLon < -180.0) MapCenterLon = -180.0;

        // 지도 중심점 설정
        SetMapCenter(g_EarthView->m_Eye.x, g_EarthView->m_Eye.y);
        ObjectDisplay->Repaint();
    }
    else if (ScrollCode == scPageUp || ScrollCode == scPageDown)
    {
        // Page 키 처리 (수평 스크롤바에서는 좌우 이동)
        double deltaX = 0;
        if (ScrollCode == scPageUp)
        {
            deltaX = -MapHScrollBar->LargeChange / 100.0; // 왼쪽으로
        }
        else
        {
            deltaX = MapHScrollBar->LargeChange / 100.0; // 오른쪽으로
        }

        MapCenterLon += deltaX;

        // 경계값 체크 (검은색 영역 제외)
        double zoomLevel = g_EarthView->m_Eye.h;
        double mapWidth = 360.0 * zoomLevel;
        double maxLon = 180.0 - mapWidth / 2.0;
        double minLon = -180.0 + mapWidth / 2.0;

        if (MapCenterLon > maxLon)
            MapCenterLon = maxLon;
        if (MapCenterLon < minLon)
            MapCenterLon = minLon;

        // 지도 중심점 설정
        SetMapCenter(g_EarthView->m_Eye.x, g_EarthView->m_Eye.y);
        ObjectDisplay->Repaint();
    }
}

// 항공기-공항 거리 계산 스레드 구현
__fastcall TAircraftAirportDistanceThread::TAircraftAirportDistanceThread(AircraftAirportDistanceResult *result, int interval)
    : TThread(true), distanceResult(result), updateIntervalMs(interval)
{
    FreeOnTerminate = false;
}

__fastcall TAircraftAirportDistanceThread::~TAircraftAirportDistanceThread()
{
}

void __fastcall TAircraftAirportDistanceThread::Execute()
{
    while (!Terminated)
    {
        if (distanceResult && Form1->airportManager)
        {
            distanceResult->isUpdating = true;

            // 기존 결과 클리어
            distanceResult->nearAirportAircraft.clear();

            // 현재 화면 범위 계산
            double minLat, maxLat, minLon, maxLon;
            int screenWidth = Form1->ObjectDisplay->Width;
            int screenHeight = Form1->ObjectDisplay->Height;

            Form1->XY2LatLon2(0, 0, minLat, maxLon);
            Form1->XY2LatLon2(screenWidth, screenHeight, maxLat, minLon);

            // 공항 목록 가져오기
            auto visibleAirports = Form1->airportManager->getVisibleAirports(
                minLat, maxLat, minLon, maxLon, 0);

            // 모든 항공기에 대해 거리 계산
            uint32_t *Key;
            ght_iterator_t iterator;
            TADS_B_Aircraft *Data;

            for (Data = (TADS_B_Aircraft *)ght_first(Form1->HashTable, &iterator, (const void **)&Key);
                 Data; Data = (TADS_B_Aircraft *)ght_next(Form1->HashTable, &iterator, (const void **)&Key))
            {

                if (Data->HaveLatLon)
                {
                    double aircraftX, aircraftY;
                    Form1->LatLon2XY(Data->Latitude, Data->Longitude, aircraftX, aircraftY);

                    // 화면 밖 항공기는 제외
                    if (!(0 <= aircraftX && aircraftX < screenWidth && 0 <= aircraftY && aircraftY < screenHeight))
                    {
                        continue;
                    }

                    double minDistanceSquare = 999999.0;

                    // 모든 공항과의 거리 계산
                    for (const auto &airport : visibleAirports)
                    {
                        double airportX, airportY;
                        Form1->LatLon2XY(airport.latitude, airport.longitude, airportX, airportY);

                        // 화면 밖 공항은 제외
                        if (!(0 <= airportX && airportX < screenWidth && 0 <= airportY && airportY < screenHeight))
                        {
                            continue;
                        }

                        // 간단한 근사 거리 계산
                        double dlat = Data->Latitude - airport.latitude;
                        double dlon = Data->Longitude - airport.longitude;
                        double latDist = dlat * 60.0;
                        double lonDist = dlon * 60.0 * cos(Data->Latitude * DEG_TO_RAD);
                        double distanceSquare = latDist * latDist + lonDist * lonDist;

                        if (distanceSquare < minDistanceSquare)
                        {
                            minDistanceSquare = distanceSquare;
                        }
                    }

                    // 5해리 이내인 경우 결과에 추가
                    if (minDistanceSquare <= 25.0)
                    {
                        distanceResult->nearAirportAircraft.insert(Data->ICAO);
                    }
                }
            }

            distanceResult->lastUpdate = std::chrono::system_clock::now();
            distanceResult->isUpdating = false;
        }

        // 지정된 간격만큼 대기
        Sleep(updateIntervalMs);
    }
}

// 항공기가 공항 근처에 있는지 확인하는 함수
bool TForm1::isAircraftNearAirport(uint32_t aircraftICAO)
{
    if (!aircraftAirportDistanceResult)
    {
        return false;
    }

    return aircraftAirportDistanceResult->nearAirportAircraft.find(aircraftICAO) !=
           aircraftAirportDistanceResult->nearAirportAircraft.end();
}

// 거리 계산 스레드 시작
void TForm1::startDistanceCalculationThread()
{
    if (!distanceCalculationThread)
    {
        aircraftAirportDistanceResult = new AircraftAirportDistanceResult();
        aircraftAirportDistanceResult->isUpdating = false;

        distanceCalculationThread = new TAircraftAirportDistanceThread(aircraftAirportDistanceResult, 2000); // 2초마다 업데이트
        distanceCalculationThread->Start();
    }
}

// 거리 계산 스레드 중지
void TForm1::stopDistanceCalculationThread()
{
    if (distanceCalculationThread)
    {
        distanceCalculationThread->Terminate();
        distanceCalculationThread->WaitFor();
        delete distanceCalculationThread;
        distanceCalculationThread = nullptr;
    }

    if (aircraftAirportDistanceResult)
    {
        delete aircraftAirportDistanceResult;
        aircraftAirportDistanceResult = nullptr;
    }
}

void __fastcall TForm1::GetAltitudeLineColor(double altitude, float &r, float &g, float &b, float &alpha)
{
    // 고도에 따른 색상 매핑
    if (altitude >= 35000)
    {
        // 35,000ft 이상: 핫핑크 (고고도)
        r = 1.0f;
        g = 0.0f;
        b = 0.8f;
        alpha = 1.0f;
    }
    else if (altitude >= 25000)
    {
        // 25,000-35,000ft: 파란색 (순항고도)
        r = 0.2f;
        g = 0.4f;
        b = 1.0f;
        alpha = 1.0f;
    }
    else if (altitude >= 15000)
    {
        // 15,000-25,000ft: 청록색 (중고도)
        r = 0.0f;
        g = 0.8f;
        b = 0.8f;
        alpha = 1.0f;
    }
    else if (altitude >= 8000)
    {
        // 8,000-15,000ft: 초록색 (중저고도)
        r = 0.2f;
        g = 1.0f;
        b = 0.2f;
        alpha = 1.0f;
    }
    else if (altitude >= 3000)
    {
        // 3,000-8,000ft: 노란색 (저고도)
        r = 1.0f;
        g = 1.0f;
        b = 0.2f;
        alpha = 1.0f;
    }
    else if (altitude >= 1000)
    {
        // 1,000-3,000ft: 주황색 (접근/이륙)
        r = 1.0f;
        g = 0.6f;
        b = 0.2f;
        alpha = 1.0f;
    }
    else
    {
        // 1,000ft 미만: 빨간색 (지상/매우 낮음)
        r = 1.0f;
        g = 0.2f;
        b = 0.2f;
        alpha = 1.0f;
    }
}

//---------------------------------------------------------------------------
// Raw 연결 끊김 다이얼로그 표시
void __fastcall TForm1::ShowRawConnectionLostDialog()
{
    AnsiString ipAddress = RawIpAddress->Text;

    int result = MessageDlg(
        "Connection to Pi device (" + ipAddress + ") has been lost.\n\n"
                                                  "Would you like to reconnect automatically?",
        mtConfirmation,
        TMsgDlgButtons() << mbYes << mbNo,
        0);

    switch (result)
    {
    case mrYes:
        ReconnectToRawDevice();
        break;
    case mrNo:
    default:
        // 사용자가 취소한 경우, 연결은 그대로 두고 팝업만 닫음
        // 다시 보지 않도록 플래그는 그대로 유지
        printf("User chose to keep Raw connection state\n");
        break;
    }
}

//---------------------------------------------------------------------------
// SBS 연결 끊김 다이얼로그 표시
void __fastcall TForm1::ShowSBSConnectionLostDialog()
{
    AnsiString ipAddress = SBSIpAddress->Text;

    int result = MessageDlg(
        "Connection to SBS Hub (" + ipAddress + ") has been lost.\n\n"
                                                "Would you like to reconnect automatically?",
        mtConfirmation,
        TMsgDlgButtons() << mbYes << mbNo,
        0);

    switch (result)
    {
    case mrYes:
        ReconnectToSBSDevice();
        break;
    case mrNo:
    default:
        // 사용자가 취소한 경우, 연결은 그대로 두고 팝업만 닫음
        // 다시 보지 않도록 플래그는 그대로 유지
        printf("User chose to keep SBS connection state\n");
        break;
    }
}

//---------------------------------------------------------------------------
// Raw 디바이스에 재연결
void __fastcall TForm1::ReconnectToRawDevice()
{
    // Raw 연결 끊기
	// 안전한 스레드 종료
	if (TCPClientRawHandleThread && TCPClientRawHandleThread->Handle)
	{
		try {
			TCPClientRawHandleThread->Terminate();
			TCPClientRawHandleThread->WaitFor();
		}
		catch (...) {
			printf("Error: Raw thread termination failed\n");
		}
	}
    if (IdTCPClientRaw->Connected())
    {
        IdTCPClientRaw->Disconnect();
    }
    IdTCPClientRaw->IOHandler->InputBuffer->Clear();

    // 재연결 시도
    RawConnectButton->Caption = "Connecting...";
    RawPlaybackButton->Enabled = false;
	RawConnectButton->Color = clCream;
    RawConnectionLostShown = false;

    printf("Attempting to reconnect to Pi device (%s)...\n", AnsiString(RawIpAddress->Text).c_str()); // "AnsiString" is for Removing Warning
    TConnectionThread *connectionThread = new TConnectionThread(RawIpAddress->Text, 30002, false);
    connectionThread->Resume();
}

//---------------------------------------------------------------------------
// SBS 디바이스에 재연결
void __fastcall TForm1::ReconnectToSBSDevice()
{
	// SBS 연결 끊기
	// 안전한 스레드 종료
	if (TCPClientSBSHandleThread && TCPClientSBSHandleThread->Handle)
	{
		try {
			TCPClientSBSHandleThread->Terminate();
			TCPClientSBSHandleThread->WaitFor();
		}
		catch (...) {
			printf("Error: SBS thread termination failed\n");
		}
	}
    if (IdTCPClientSBS->Connected())
    {
        IdTCPClientSBS->Disconnect();
    }
    IdTCPClientSBS->IOHandler->InputBuffer->Clear();

    // 재연결 시도
    SBSConnectButton->Caption = "Connecting...";
    SBSPlaybackButton->Enabled = false;
	SBSConnectButton->Color = clCream;
    SBSConnectionLostShown = false;

    printf("Attempting to reconnect to SBS Hub (%s)...\n", AnsiString(SBSIpAddress->Text).c_str()); // "AnsiString" is for Removing Warning
    TConnectionThread *connectionThread = new TConnectionThread(SBSIpAddress->Text, 5002, true);
    connectionThread->Resume();
}

//---------------------------------------------------------------------------
// Raw 데이터 타임아웃 다이얼로그 표시
void __fastcall TForm1::ShowRawTimeoutDialog()
{
    AnsiString ipAddress = RawIpAddress->Text;

    int result = MessageDlg(
        "Data reception timeout from Pi device (" + ipAddress + ").\n"
                                                                "No data received for 30 seconds.\n\n"
                                                                "Would you like to reconnect automatically?",
        mtConfirmation,
        TMsgDlgButtons() << mbYes << mbNo,
        0);

    switch (result)
    {
    case mrYes:
        ReconnectToRawDevice();
        break;
    case mrNo:
    default:
        // 사용자가 취소한 경우, 연결은 그대로 두고 팝업만 닫음
        printf("User chose to keep Raw timeout state\n");
        break;
    }
}

//---------------------------------------------------------------------------
// SBS 데이터 타임아웃 다이얼로그 표시
void __fastcall TForm1::ShowSBSTimeoutDialog()
{
    AnsiString ipAddress = SBSIpAddress->Text;

    int result = MessageDlg(
        "Data reception timeout from SBS Hub (" + ipAddress + ").\n"
															  "No data received for 30 seconds.\n\n"
                                                              "Would you like to reconnect automatically?",
		mtConfirmation,
        TMsgDlgButtons() << mbYes << mbNo,
        0);

    switch (result)
    {
    case mrYes:
        ReconnectToSBSDevice();
        break;
    case mrNo:
    default:
        // 사용자가 취소한 경우, 연결은 그대로 두고 팝업만 닫음
        printf("User chose to keep SBS timeout state\n");
        break;
    }
}

//---------------------------------------------------------------------------
// Progress Bar 초기화 메서드 구현
void __fastcall TForm1::InitializePlaybackProgress()
{
    PlaybackProgressTrackBar->Min = 0;
    PlaybackProgressTrackBar->Max = 1000;
    PlaybackProgressTrackBar->Position = 0;
    PlaybackProgressPanel->Visible = false;
    // 변수 초기화
    PlaybackStartTime = 0;
    PlaybackEndTime = 0;
    PlaybackCurrentTime = 0;
    PlaybackPaused = false;
    PlaybackSeeking = false;
    ProgrammaticProgressUpdate = false; // 플래그 초기화

    if (!PlaybackFileIndex)
    {
        PlaybackFileIndex = new TStringList();
    }
}

// Progress Bar 변경 이벤트 핸들러
void __fastcall TForm1::PlaybackProgressBarChange(TObject *Sender)
{
    // 프로그램적 업데이트인 경우 무시
    if (ProgrammaticProgressUpdate)
    {
        return;
    }

    printf("PlaybackProgressBarChange\n");
    if (PlaybackSeeking || !PlayBackSBSStream || !TCPClientSBSHandleThread)
        return;

    PlaybackSeeking = true;

    // Progress Bar 위치를 시간으로 변환
    float progress = (float)PlaybackProgressTrackBar->Position / 1000.0f;
    __int64 targetTime = PlaybackStartTime +
                         (__int64)((PlaybackEndTime - PlaybackStartTime) * progress);

    printf("PlaybackProgressBarChange 1\n");
    SeekToPosition(targetTime);
    printf("PlaybackProgressBarChange 2\n");

    // 스레드에 Seek 요청
    TCPClientSBSHandleThread->RequestSeek(targetTime);

    PlaybackSeeking = false;
}

// Play/Pause 버튼 이벤트 핸들러
void __fastcall TForm1::PlayPauseButtonClick(TObject *Sender)
{
    PlaybackPaused = !PlaybackPaused;
}

// Progress 업데이트 메서드
void __fastcall TForm1::UpdatePlaybackProgress()
{
#ifdef PROGRESSBAR_DEBUGGING
    printf("UpdatePlaybackProgress\n");
#endif
    if (PlaybackSeeking)
        return;

#ifdef PROGRESSBAR_DEBUGGING
    printf("PlaybackCurrentTime: %lld, PlaybackEndTime: %lld, PlaybackStartTime: %lld\n", PlaybackCurrentTime, PlaybackEndTime, PlaybackStartTime);
#endif
    if (PlaybackEndTime > PlaybackStartTime)
    {
        float progress = (float)(PlaybackCurrentTime - PlaybackStartTime) /
                         (float)(PlaybackEndTime - PlaybackStartTime);

        // 프로그램적 업데이트 플래그 설정
        ProgrammaticProgressUpdate = true;
        PlaybackProgressTrackBar->Position = (int)(progress * 1000);
        ProgrammaticProgressUpdate = false;

        // 진행 상황을 printf로 출력
        AnsiString currentTimeStr = FormatPlaybackTime(PlaybackCurrentTime - PlaybackStartTime);
        AnsiString totalTimeStr = FormatPlaybackTime(PlaybackEndTime - PlaybackStartTime);
        float progressPercent = progress * 100.0f;
#ifdef PROGRESSBAR_DEBUGGING
        printf("[SBS Playback] Progress: %.1f%% (%s / %s) Speed: x%d %s\n",
               progressPercent,
               currentTimeStr.c_str(),
               totalTimeStr.c_str(),
               globalTrackbarValue,
               PlaybackPaused ? "[PAUSED]" : "[PLAYING]");
#endif
        PlayTimeLabel->Caption = currentTimeStr;
    }
}
void __fastcall TForm1::SeekToPosition(__int64 targetTime)
{
    // 타겟 시간이 범위를 벗어나면 처음으로 이동
    if (!PlayBackSBSStream || !PlaybackFileIndex || PlaybackFileIndex->Count == 0)
        return;

    // 가장 가까운 인덱스 포인트 찾기
    __int64 bestLineNumber = 0;
    __int64 minTimeDiff = MAXINT64;

    for (int i = 0; i < PlaybackFileIndex->Count; i++)
    {
        AnsiString entry = PlaybackFileIndex->Strings[i];
        int pos = entry.Pos("=");
        if (pos > 0)
        {
            __int64 lineNum = StrToInt64(entry.SubString(1, pos - 1));
            __int64 timestamp = StrToInt64(entry.SubString(pos + 1, entry.Length()));

            __int64 timeDiff = abs(timestamp - targetTime);
            if (timeDiff < minTimeDiff)
            {
                minTimeDiff = timeDiff;
                bestLineNumber = lineNum;
            }
        }
    }

    // bestLineNumber가 짝수이면 홀수로 조정 (타임스탬프 라인은 홀수 라인)
    if (bestLineNumber % 2 == 1)
    {
        bestLineNumber--; // 이전 타임스탬프 라인으로 이동
    }

    // 해당 위치로 이동 (근사치)
    try
    {
        PlayBackSBSStream->BaseStream->Position = 0;
        PlayBackSBSStream->DiscardBufferedData();

        // bestLineNumber까지 라인 건너뛰기
        for (__int64 i = 0; i < bestLineNumber && !PlayBackSBSStream->EndOfStream; i++)
        {
            PlayBackSBSStream->ReadLine();
        }
#ifdef PROGRESSBAR_DEBUGGING
        printf("SeekToPosition: Moved to line %lld\n", bestLineNumber);
#endif

        // 정확한 타임스탬프 위치 찾기 - 순차적으로 targetTime에 가까운 위치 찾기
        while (!PlayBackSBSStream->EndOfStream)
        {
            __int64 currentPos = PlayBackSBSStream->BaseStream->Position;
            AnsiString timeStr = PlayBackSBSStream->ReadLine();
            try
            {
                __int64 currentTime = StrToInt64(timeStr);
#ifdef PROGRESSBAR_DEBUGGING
                printf("SeekToPosition: Found timestamp %lld (target: %lld)\n", currentTime, targetTime);
#endif
                if (currentTime >= targetTime)
                {
                    // 타임스탬프 라인 시작으로 되돌리기
                    PlayBackSBSStream->BaseStream->Position = currentPos;
                    PlayBackSBSStream->DiscardBufferedData();
                    PlaybackCurrentTime = currentTime;
#ifdef PROGRESSBAR_DEBUGGING
                    printf("SeekToPosition: Positioned at timestamp %lld\n", currentTime);
#endif
                    return;
                }
                // SBS 메시지 라인 건너뛰기
                if (!PlayBackSBSStream->EndOfStream)
                {
                    PlayBackSBSStream->ReadLine();
                }
            }
            catch (...)
            {
                printf("SeekToPosition: Failed to parse timestamp: %s\n", timeStr.c_str());
                // 타임스탬프가 아닌 라인을 만났으면, 다음 타임스탬프 라인을 찾기 위해 계속
                continue;
            }
        }
    }
    catch (...)
    {
        // 오류 발생시 처음으로 이동
        printf("SeekToPosition Exception\n");
        PlayBackSBSStream->BaseStream->Position = 0;
        PlayBackSBSStream->DiscardBufferedData();
        PlaybackCurrentTime = PlaybackStartTime;
    }
}

// BuildFileIndex 함수 구현
void __fastcall TForm1::BuildFileIndex()
{
    if (!PlayBackSBSStream)
        return;

    // PlaybackFileIndex 초기화
    if (!PlaybackFileIndex)
    {
        PlaybackFileIndex = new TStringList();
    }
    PlaybackFileIndex->Clear();

    // 현재 위치 저장
    __int64 currentPos = PlayBackSBSStream->BaseStream->Position;

    // 파일 처음부터 스캔
    PlayBackSBSStream->BaseStream->Position = 0;
    PlayBackSBSStream->DiscardBufferedData();

    __int64 lineNumber = 0;

    try
    {
        while (!PlayBackSBSStream->EndOfStream)
        {
            AnsiString timeStr = PlayBackSBSStream->ReadLine();

            // 짝수 라인(0, 2, 4, ...)은 타임스탬프
            if (lineNumber % 2 == 0)
            {
                try
                {
                    __int64 timestamp = StrToInt64(timeStr);

                    if (lineNumber == 0)
                    {
                        PlaybackStartTime = timestamp;
                    }
                    PlaybackEndTime = timestamp;

                    // 매 100번째 라인마다 인덱스 저장 (성능 최적화)
                    if (lineNumber % 100 == 0)
                    {
                        AnsiString indexEntry = IntToStr(lineNumber) + "=" + IntToStr(timestamp);
                        PlaybackFileIndex->Add(indexEntry);
#ifdef PROGRESSBAR_DEBUGGING
                        printf("Index: Line %lld = Timestamp %lld\n", lineNumber, timestamp);
#endif
                    }
                }
                catch (...)
                {
                    printf("BuildFileIndex: Failed to parse timestamp at line %lld: %s\n", lineNumber, timeStr.c_str());
                }
            }
            // 홀수 라인(1, 3, 5, ...)은 SBS 메시지 - 그냥 건너뛰기
            lineNumber++;
        }

        // UI 업데이트
        if (TotalTimeLabel)
        {
            TotalTimeLabel->Caption = FormatPlaybackTime(PlaybackEndTime - PlaybackStartTime);
        }
#ifdef PROGRESSBAR_DEBUGGING
        printf("File index built: %d entries, Duration: %s\n", PlaybackFileIndex->Count, FormatPlaybackTime(PlaybackEndTime - PlaybackStartTime).c_str());
#endif
    }
    catch (...)
    {
        ShowMessage("Failed to build file index");
    }

    // 원래 위치로 복원
    PlayBackSBSStream->BaseStream->Position = currentPos;
    PlayBackSBSStream->DiscardBufferedData();
}

// FormatPlaybackTime 함수 구현
AnsiString __fastcall TForm1::FormatPlaybackTime(__int64 timeMs)
{
    // 음수 처리
    if (timeMs < 0)
    {
        return "00:00:00";
    }

    // 밀리초를 시, 분, 초로 변환
    int totalSeconds = timeMs / 1000;
    int hours = totalSeconds / 3600;
    int minutes = (totalSeconds % 3600) / 60;
    int seconds = totalSeconds % 60;

    // HH:MM:SS 형식으로 포맷팅
    AnsiString result;
    result.sprintf("%02d:%02d:%02d", hours, minutes, seconds);

    return result;
}
void __fastcall TForm1::UpdateRawConnectionStatus(const AnsiString &status)
{
    TColor color;
    if (RawStatusLabel)
    {
        AnsiString icon;
        RawStatusLabel->Caption = "●";
        if (status == "Connecting...")
        {
            color = clYellow;
        }
        else if (status == "Raw Disconnect")
        {
            if (RawConnectionLostShown || RawTimeoutPopupShown)
            {
                color = clRed;
            }
            else
            {
                color = clGreen;
            }
        }
        else
        {
            color = clBlack;
            RawStatusLabel->Caption = "";
        }

        RawStatusLabel->Font->Color = color;
    }
}
void __fastcall TForm1::UpdateSBSConnectionStatus(const AnsiString &status)
{
    TColor color;
    if (SBSStatusLabel)
    {
        AnsiString icon;
        SBSStatusLabel->Caption = "●";
        if (status == "Connecting...")
        {
            color = clYellow;
        }
        else if (status == "SBS Disconnect")
        {
            if (SBSConnectionLostShown || SBSTimeoutPopupShown)
            {
                color = clRed;
            }
            else
            {
                color = clGreen;
            }
        }
        else
        {
            color = clBlack;
            SBSStatusLabel->Caption = "";
        }

        SBSStatusLabel->Font->Color = color;
    }
}

// 항공기 간 거리 계산 스레드 구현
__fastcall TAircraftAircraftDistanceThread::TAircraftAircraftDistanceThread(AircraftAircraftDistanceResult *result, int interval)
    : TThread(true), distanceResult(result), updateIntervalMs(interval)
{
    FreeOnTerminate = false;
}

__fastcall TAircraftAircraftDistanceThread::~TAircraftAircraftDistanceThread()
{
}

void __fastcall TAircraftAircraftDistanceThread::Execute()
{
    while (!Terminated)
    {
        if (distanceResult)
        {
            distanceResult->isUpdating = true;

            // 기존 결과 클리어
            distanceResult->closeAircraftPairs.clear();

            // 모든 항공기에 대해 거리 계산
            uint32_t *Key1, *Key2;
            ght_iterator_t iterator1, iterator2;
            TADS_B_Aircraft *Data1, *Data2;

            for (Data1 = (TADS_B_Aircraft *)ght_first(Form1->HashTable, &iterator1, (const void **)&Key1);
                 Data1; Data1 = (TADS_B_Aircraft *)ght_next(Form1->HashTable, &iterator1, (const void **)&Key1))
            {

                if (!Data1->HaveLatLon)
                    continue;

                // 고도 정보가 없거나 N/A이거나 0인 경우 제외
                if (!Data1->HaveAltitude || Data1->Altitude <= 0)
                    continue;

                // 헬리콥터나 군용기는 제외
                if (aircraft_is_helicopter(Data1->ICAO, NULL) || IsAircraftMilitary(Data1->ICAO))
                    continue;

                // 두 번째 항공기와의 거리 계산
                for (Data2 = (TADS_B_Aircraft *)ght_first(Form1->HashTable, &iterator2, (const void **)&Key2);
                     Data2; Data2 = (TADS_B_Aircraft *)ght_next(Form1->HashTable, &iterator2, (const void **)&Key2))
                {

                    if (!Data2->HaveLatLon)
                        continue;

                    // 고도 정보가 없거나 N/A이거나 0인 경우 제외
                    if (!Data2->HaveAltitude || Data2->Altitude <= 0)
                        continue;

                    // 같은 항공기는 제외
                    if (Data1->ICAO == Data2->ICAO)
                        continue;

                    // 헬리콥터나 군용기는 제외
                    if (aircraft_is_helicopter(Data2->ICAO, NULL) || IsAircraftMilitary(Data2->ICAO))
                        continue;

                    // 1. 평면 거리 계산 (해리 단위)
                    double dlat = Data1->Latitude - Data2->Latitude;
                    double dlon = Data1->Longitude - Data2->Longitude;
                    double latDist = dlat * 60.0;
                    double lonDist = dlon * 60.0 * cos(Data1->Latitude * DEG_TO_RAD);
                    double horizontalDistanceSquare = latDist * latDist + lonDist * lonDist;

                    // 2. 고도 차이 계산 (feet를 해리로 변환)
                    // 1 NM = 6076.12 feet
                    double altitudeDiff = abs(Data1->Altitude - Data2->Altitude);
                    double verticalDistance = altitudeDiff / 6076.12; // feet to NM

                    // 3. 3차원 거리 계산 (피타고라스 정리)
                    double distance3DSquare = horizontalDistanceSquare + verticalDistance * verticalDistance;

                    // 1해리 이내인 경우 결과에 추가
                    if (distance3DSquare <= 1.0)
                    {
                        // 중복 방지를 위해 작은 ICAO가 앞에 오도록 정렬
                        uint32_t icao1 = (Data1->ICAO < Data2->ICAO) ? Data1->ICAO : Data2->ICAO;
                        uint32_t icao2 = (Data1->ICAO < Data2->ICAO) ? Data2->ICAO : Data1->ICAO;
                        std::pair<uint32_t, uint32_t> pair = std::make_pair(icao1, icao2);

                        // 중복 체크
                        bool alreadyExists = false;
                        for (const auto &existingPair : distanceResult->closeAircraftPairs)
                        {
                            if (existingPair == pair)
                            {
                                alreadyExists = true;
                                break;
                            }
                        }

                        if (!alreadyExists)
                        {
                            distanceResult->closeAircraftPairs.push_back(pair);

                            // 콘솔에 로그 출력
                            //printf("CLOSE AIRCRAFT PAIR: ICAO1=%06X, ICAO2=%06X, Distance=%.2f NM\n",
                            //       icao1, icao2, sqrt(distance3DSquare));
                            
                            //printf("CLOSE AIRCRAFT PAIR: ICAO1=%06X, ICAO2=%06X, Distance=%.2f NM, v=%.2f, h=%.2f\n",
                            //       icao1, icao2, sqrt(distance3DSquare), verticalDistance, sqrt(horizontalDistanceSquare));
                            //printf("    ALT      ICAO1=%.1f, ICAO2=%.1f\n",
                            //       Data1->Altitude, Data2->Altitude);
                            //printf("    LAT      ICAO1=%.5f, ICAO2=%.5f\n",
                            //       Data1->Latitude, Data2->Latitude);
                            //printf("    LON      ICAO1=%.5f, ICAO2=%.5f\n\n",
                            //       Data1->Longitude, Data2->Longitude);
                        }
                    }
                }
            }

            distanceResult->lastUpdate = std::chrono::system_clock::now();
            distanceResult->isUpdating = false;
        }

        // 지정된 간격만큼 대기
        Sleep(updateIntervalMs);
    }
}

// 항공기 간 거리 계산 스레드 시작
void TForm1::startAircraftDistanceCalculationThread()
{
    if (!aircraftDistanceCalculationThread)
    {
        aircraftAircraftDistanceResult = new AircraftAircraftDistanceResult();
        aircraftAircraftDistanceResult->isUpdating = false;

        aircraftDistanceCalculationThread = new TAircraftAircraftDistanceThread(aircraftAircraftDistanceResult, 5000); // 5초마다 업데이트
        aircraftDistanceCalculationThread->Start();
    }
}

// 항공기 간 거리 계산 스레드 중지
void TForm1::stopAircraftDistanceCalculationThread()
{
    if (aircraftDistanceCalculationThread)
    {
        aircraftDistanceCalculationThread->Terminate();
        aircraftDistanceCalculationThread->WaitFor();
        delete aircraftDistanceCalculationThread;
        aircraftDistanceCalculationThread = nullptr;
    }

    if (aircraftAircraftDistanceResult)
    {
        delete aircraftAircraftDistanceResult;
        aircraftAircraftDistanceResult = nullptr;
    }
}

// 마우스 다운 효과
void __fastcall TForm1::PanelButtonMouseDown(TObject *Sender, TMouseButton Button,
    TShiftState Shift, int X, int Y)
{
    TPanel* panel = dynamic_cast<TPanel*>(Sender);
    if (panel) {
        panel->BevelOuter = bvLowered;  // 눌린 효과
    }
}

// 마우스 업 효과
void __fastcall TForm1::PanelButtonMouseUp(TObject *Sender, TMouseButton Button,
    TShiftState Shift, int X, int Y)
{
    TPanel* panel = dynamic_cast<TPanel*>(Sender);
    if (panel) {
        panel->BevelOuter = bvRaised;   // 원래대로
    }
}

AnsiString __fastcall TForm1::SafeAnsiString(AnsiString input)
{
    AnsiString trimmed = input.Trim();
    
    // 빈 문자열이거나 "?"이면 "N/A" 반환
    if (trimmed.IsEmpty() || trimmed == "?" || trimmed == "???" || trimmed == "????")
        return "N/A";
    
    return trimmed;
}

// 고도 필터링 함수
bool __fastcall TForm1::IsAircraftInAltitudeFilter(TADS_B_Aircraft *aircraft)
{
    if (SelectedAltitudeFilter == afAll)
        return true;
        
    if (!aircraft->HaveAltitude)
        return false;
        
    double altitude = aircraft->Altitude;
    
    switch (SelectedAltitudeFilter)
    {
        case af35000Plus:
            return altitude >= 35000;
        case af25000To35000:
            return altitude >= 25000 && altitude < 35000;
        case af15000To25000:
            return altitude >= 15000 && altitude < 25000;
        case af8000To15000:
            return altitude >= 8000 && altitude < 15000;
        case af3000To8000:
            return altitude >= 3000 && altitude < 8000;
        case af1000To3000:
            return altitude >= 1000 && altitude < 3000;
        case afBelow1000:
            return altitude < 1000;
        default:
            return true;
    }
}

// 속도 필터링 함수
bool __fastcall TForm1::IsAircraftInSpeedFilter(TADS_B_Aircraft *aircraft)
{
    if (SelectedSpeedFilter == sfAll)
        return true;
        
    if (!aircraft->Speed)
        return false;
        
    double speed = aircraft->Speed;
    
    switch (SelectedSpeedFilter)
    {
        case sf500Plus:
            return speed >= 500;
        case sf400To500:
            return speed >= 400 && speed < 500;
        case sf300To400:
            return speed >= 300 && speed < 400;
        case sf200To300:
            return speed >= 200 && speed < 300;
        case sfBelow200:
            return speed < 200;
        default:
            return true;
    }
}

// 고도 필터 ComboBox 이벤트
void __fastcall TForm1::AltitudeFilterComboBoxCloseUp(TObject *Sender)
{
    SelectedAltitudeFilter = AltitudeFilterComboBox->ItemIndex;
}

// 속도 필터 ComboBox 이벤트
void __fastcall TForm1::SpeedFilterComboBoxCloseUp(TObject *Sender)
{
    SelectedSpeedFilter = SpeedFilterComboBox->ItemIndex;
}

void __fastcall TForm1::FormActivate(TObject *Sender)
{
    if (this->Visible && this->Enabled)
        this->SetFocus();
    this->ActiveControl = nullptr;
    printf("Form activated, focus set to Form\n");
}
