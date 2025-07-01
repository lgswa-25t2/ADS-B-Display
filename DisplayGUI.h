//---------------------------------------------------------------------------

#ifndef DisplayGUIH
#define DisplayGUIH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include "Components\OpenGLv0.5BDS2006\Component\OpenGLPanel.h"
#include <ComCtrls.hpp>
#include <ExtCtrls.hpp>
#include <Menus.hpp>
#include <IdBaseComponent.hpp>
#include <IdComponent.hpp>
#include <Graphics.hpp>
#include "FilesystemStorage.h"
#include "KeyholeConnection.h"
#include "GoogleLayer.h"
#include "OsmLayer.h"
#include "FlatEarthView.h"
#include "ght_hash_table.h"
#include "TriangulatPoly.h"
#include <Dialogs.hpp>
#include <IdTCPClient.hpp>
#include <IdTCPConnection.hpp>
#include "cspin.h"
#include "Aircraft.h"
#include <Vcl.Grids.hpp>
#include <Vcl.Outline.hpp>
#include <map>
#include <unordered_map>
#include <chrono>
#include <unordered_set>
#include <atomic>
#include <vector>
#include "BigQuery/BigQueryClient.h"
#include "AircraftPlannedRoute.h"
// Forward declarations
class AirportDataManager;

typedef float T_GL_Color[4];

typedef struct
{
    bool     Valid_CC;
    bool     Valid_CPA;
    uint32_t ICAO_CC;
    uint32_t ICAO_CPA;
} TTrackHook;

typedef struct
{
    double lat;
    double lon;
    double hae;
} TPolyLine;

#define MAX_AREA_POINTS 500
typedef struct
{
    AnsiString Name;
    TColor     Color;
    DWORD      NumPoints;
    pfVec3     Points[MAX_AREA_POINTS];
    pfVec3     PointsAdj[MAX_AREA_POINTS];
    TTriangles *Triangles;
    bool       Selected;
} TArea;

// 거리 계산 결과를 저장할 구조체
struct DistanceCache
{
    double                                distance;
    std::chrono::system_clock::time_point timestamp;
};

enum TAircraftTypeFilter
{
    atfAll = 0,
    atfHelicopters = 1,
    atfMilitary = 2,
    atfKnownCivilian = 3,
    atfUnknown = 4
};

// 고도 필터 enum 추가
enum TAltitudeFilter
{
    afAll = 0,
    af35000Plus = 1,        // 35000+ feet
    af25000To35000 = 2,     // 25000-35000 feet
    af15000To25000 = 3,     // 15000-25000 feet
    af8000To15000 = 4,      // 8000-15000 feet
    af3000To8000 = 5,       // 3000-8000 feet
    af1000To3000 = 6,       // 1000-3000 feet
    afBelow1000 = 7         // 1000 미만 feet
};

// 속도 필터 enum 추가
enum TSpeedFilter
{
    sfAll = 0,
    sf500Plus = 1,          // 500+ knots
    sf400To500 = 2,         // 400-500 knots
    sf300To400 = 3,         // 300-400 knots
    sf200To300 = 4,         // 200-300 knots
    sfBelow200 = 5          // 200 미만 knots
};

// 캐시 만료 시간 (밀리초)
const int CACHE_EXPIRY_MS = 3000;           // 3 Sec
const int CACHE_CLEANUP_INTERVAL_MS = 5000; // 5초마다 캐시 정리
const int CACHE_MAX_AGE_MS = 30000;         // 30초 이상 된 캐시 제거

// 항공기-공항 거리 계산 결과를 저장할 구조체
struct AircraftAirportDistanceResult
{
    std::unordered_set<uint32_t>          nearAirportAircraft; // 공항 근처 항공기 ICAO 주소들
    std::chrono::system_clock::time_point lastUpdate;
    std::atomic<bool>                     isUpdating;
};

// 항공기 간 거리 계산 결과를 저장할 구조체
struct AircraftAircraftDistanceResult
{
    std::vector<std::pair<uint32_t, uint32_t>> closeAircraftPairs; // 1해리 이내 항공기 쌍들
    std::chrono::system_clock::time_point      lastUpdate;
    std::atomic<bool>                          isUpdating;
};

// 항공기-공항 거리 계산 스레드 클래스
class TAircraftAirportDistanceThread : public TThread
{
private:
    AircraftAirportDistanceResult *distanceResult;
    int                           updateIntervalMs; // 업데이트 간격 (밀리초)

public:
    __fastcall TAircraftAirportDistanceThread(AircraftAirportDistanceResult *result, int interval = 1000);
    __fastcall ~TAircraftAirportDistanceThread();

protected:
    virtual void __fastcall Execute();
};

// 항공기 간 거리 계산 스레드 클래스
class TAircraftAircraftDistanceThread : public TThread
{
private:
    AircraftAircraftDistanceResult *distanceResult;
    int                            updateIntervalMs; // 업데이트 간격 (밀리초)

public:
    __fastcall TAircraftAircraftDistanceThread(AircraftAircraftDistanceResult *result, int interval = 1000);
    __fastcall ~TAircraftAircraftDistanceThread();

protected:
    virtual void __fastcall Execute();
};

//---------------------------------------------------------------------------
class TTCPClientRawHandleThread : public TThread
{
private:
    AnsiString StringMsgBuffer;
    void __fastcall HandleInput(void);
    void __fastcall StopPlayback(void);
    void __fastcall StopTCPClient(void);

protected:
    void __fastcall Execute(void);

public:
    bool UseFileInsteadOfNetwork;
    bool First;
    __int64 LastTime;
    __fastcall TTCPClientRawHandleThread(bool value);
    ~TTCPClientRawHandleThread();
};
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
class TTCPClientSBSHandleThread : public TThread
{
private:
    AnsiString StringMsgBuffer;
    void __fastcall HandleInput(void);
    void __fastcall StopPlayback(void);
    void __fastcall StopTCPClient(void);

protected:
    void __fastcall Execute(void);

public:
    bool UseFileInsteadOfNetwork;
    bool First;
    __int64 LastTime;
    bool SeekRequested;     // Seek 요청 플래그 추가
    __int64 SeekTargetTime; // Seek 대상 시간 추가

    __fastcall TTCPClientSBSHandleThread(bool value);
    ~TTCPClientSBSHandleThread();
    void RequestSeek(__int64 targetTime); // Seek 요청 메서드 추가
};
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
class TConnectionThread : public TThread
{
private:
    AnsiString Host;
    int Port;
    bool IsSBS;
    AnsiString ErrorMessage;
    void __fastcall OnConnectionComplete(void);
    void __fastcall OnConnectionFailed(void);

protected:
    void __fastcall Execute(void);

public:
    __fastcall TConnectionThread(AnsiString host, int port, bool isSBS);
    ~TConnectionThread();
};
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
class TForm1 : public TForm
{
    __published : // IDE-managed Components
                  TMainMenu *MainMenu1;
    TPanel *RightPanel;
    TMenuItem *File1;
    TMenuItem *Exit1;
    TTimer *Timer1;
    TPanel *Panel1;
    TPanel *Panel3;
	TPanel *ZoomIn;
	TPanel *ZoomOut;
    TCheckBox *DrawMap;
    TCheckBox *PurgeStale;
    TTimer *Timer2;
    TCSpinEdit *CSpinStaleTime;
    TListView *AreaListView;
	TPanel *Insert;
	TPanel *Delete;
	TPanel *Complete;
	TPanel *Cancel;
    TLabel *RawStatusLabel; // Raw 연결 상태 표시 라벨
    TLabel *SBSStatusLabel; // SBS 연결 상태 표시 라벨
    // TLabel *Label16;
    // TLabel *Label17;
    TComboBox *RawIpAddress;
    TIdTCPClient *IdTCPClientRaw;
    TSaveDialog *RecordRawSaveDialog;
    TOpenDialog *PlaybackRawDialog;
    TCheckBox *CycleImages;
    TScrollBox *Panel4;
    TLabel *SpdLabel;
    TLabel *HdgLabel;
    TLabel *AltLabel;
    TLabel *MsgCntLabel;
    TLabel *TrkLastUpdateTimeLabel;
    TLabel *Label14;
    TLabel *Label13;
    TLabel *Label10;
    TLabel *Label9;
    TLabel *Label8;
    TLabel *Label18;
    TLabel *FlightNumLabel;
    TLabel *ICAOLabel;
    TLabel *Label5;
    TLabel *Lon;
    TLabel *Label3;
    TLabel *Lat;
    TLabel *Label2;
    TStaticText *SystemTime;
    TLabel *SystemTimeLabel;
    TLabel *ViewableAircraftCountLabel;
    TLabel *AircraftCountLabel;
    TLabel *Label11;
    TLabel *Label1;
    TPanel *RawConnectButton;
	TPanel *RawPlaybackButton;
	TPanel *RawRecordButton;
    TIdTCPClient *IdTCPClientSBS;
	TPanel *SBSConnectButton;
    TComboBox *SBSIpAddress;
	TPanel *SBSRecordButton;
	TPanel *SBSPlaybackButton;
    TSaveDialog *RecordSBSSaveDialog;
    TOpenDialog *PlaybackSBSDialog;
    TTrackBar *TimeToGoTrackBar;
    TCheckBox *TimeToGoCheckBox;
    TStaticText *TimeToGoText;
    TLabel *Label12;
    TLabel *Label19;
    TLabel *CpaTimeValue;
    TLabel *CpaDistanceValue;
    TPanel *Panel2;
    TComboBox *MapComboBox;
    TCheckBox *BigQueryCheckBox;
    TMenuItem *UseSBSLocal;
    TMenuItem *UseSBSRemote;
    TMenuItem *LoadARTCCBoundaries1;
    TCheckBox *DisplayAirportCheckBox;
    TTrackBar *PlaybackSpeedTrackBar;
    TMenuItem *Help1;
    TMenuItem *AboutADSBDisplay1;
    TMenuItem *UserManual1;
    TPanel *PanelTitle1;
    TPanel *Panel7;
    TLabel *PanelTitle2;
    TLabel *Label22;
    TLabel *SerialNum;
    TLabel *Label23;
    TLabel *Manufacturer;
    TLabel *Label24;
    TLabel *Model;
    TLabel *Label25;
    TLabel *MFRYear;
    TLabel *Label26;
    TLabel *Label27;
	TLabel *AirType;
	TLabel *Airline;
    TLabel *Label29;
    TLabel *AircraftCountry;
    TLabel *Label30;
    TLabel *ExpirationData;
	TLabel *SubTitleDeparture;
    TLabel *DepartureAirportName;
    TLabel *Label32;
    TLabel *Label33;
    TLabel *DepartureAirportICAO;
    TLabel *Label35;
    TLabel *DepartureAirportLocation;
    TLabel *Label34;
    TLabel *Label36;
	TLabel *subTitleDestination;
    TLabel *Label38;
    TLabel *DestinationAirportName;
    TLabel *Label39;
    TLabel *DestinationAirportICAO;
    TLabel *Label40;
    TLabel *DestinationAirportLocation;
    TLabel *Label41;
	TLabel *DestinationCounty;
    TLabel *Label43;
    TLabel *Label44;
    TLabel *TransitAirport1Name;
    TLabel *Label46;
    TLabel *TransitAirport1ICAO;
    TLabel *Label48;
    TLabel *TransitAirport1Location;
    TLabel *Label50;
    TLabel *TransitAirport1Country;
    TLabel *Label45;
    TLabel *Label47;
    TLabel *TransitAirport2Name;
    TLabel *Label51;
    TLabel *TransitAirport2ICAO;
    TLabel *Label53;
    TLabel *TransitAirport2Location;
    TLabel *Label55;
    TLabel *TransitAirport2Country;
    TPanel *LegendPanel;
    TLabel *LegendTitle;
    TShape *HelicopterColorBox;
    TLabel *HelicopterLabel;
    TShape *MilitaryColorBox;
    TLabel *MilitaryLabel;
    TShape *CivilianColorBox;
    TLabel *CivilianLabel;
    TShape *UnknownColorBox;
    TLabel *UnknownLabel;
    TShape *NearAirportColorBox;
    TLabel *NearAirportLabel;
    TShape *SelectedColorBox;
    TLabel *SelectedLabel;
    TShape *TrackHistoryColorBox;
    TLabel *TrackHistoryLabel;
    TComboBox *AircraftTypeFilterComboBox;
    TPanel *TimeToGoLegendPanel;
    TLabel *TimeToGoLegendTitle;
    TShape *HighAltitudeShape;
    TLabel *HighAltitudeLabel;
    TShape *MedHighAltitudeShape;
    TLabel *MedHighAltitudeLabel;
    TShape *MedAltitudeShape;
    TLabel *MedAltitudeLabel;
    TShape *LowMedAltitudeShape;
    TLabel *LowMedAltitudeLabel;
    TCheckBox *LiveMapCheckbox;
    TOpenGLPanel *ObjectDisplay;
    TLabel *x1;
    TLabel *x5;
    TLabel *x10;
    TPanel *PlaybackSpeedPanel;
    TPanel *PlaybackControlTitle;
    TLabel *LowAltitudeLabel;
    TShape *LowAltitudeShape;
    TLabel *VeryLowAltitudeLabel;
    TShape *VeryLowAltitudeShape;
    TShape *LowestAltitudeShape;
    TLabel *LowestAltitudeLabel;
    TPanel *PlaybackProgressPanel;
    TTrackBar *PlaybackProgressTrackBar;
    TLabel *TotalTimeLabel;
    TLabel *PlayTimeLabel;
	TLabel *Label16;
	TLabel *Label20;
	TLabel *Label21;
	TLabel *Label28;
	TLabel *Label4;
	TPanel *InnePanel;
	TPanel *InnerPannel2;
	TOutline *Outline1;
	TLabel *LabelLat;
	TLabel *LabelLong;
	TLabel *CLatiLabel;
	TLabel *CLongtLabel;
	TShape *Shape1;
    TComboBox *AltitudeFilterComboBox;
	TComboBox *SpeedFilterComboBox;
    TLabel *Label6;
	TShape *Shape2;
	TShape *Shape3;
	TShape *Shape4;
	TShape *Shape5;
	TPanel *PanelRouteLegend;
  	TPaintBox *PaintBoxRouteLegend;
    void __fastcall ObjectDisplayInit(TObject *Sender);
    void __fastcall ObjectDisplayResize(TObject *Sender);
    void __fastcall ObjectDisplayPaint(TObject *Sender);
    void __fastcall Timer1Timer(TObject *Sender);
    void __fastcall ResetXYOffset(void);
    void __fastcall ObjectDisplayMouseDown(TObject *Sender, TMouseButton Button,
                                           TShiftState Shift, int X, int Y);
    void __fastcall ObjectDisplayMouseMove(TObject *Sender, TShiftState Shift,
                                           int X, int Y);
    void __fastcall AddPoint(int X, int Y);
    void __fastcall ObjectDisplayMouseUp(TObject *Sender, TMouseButton Button,
                                         TShiftState Shift, int X, int Y);
    void __fastcall Exit1Click(TObject *Sender);
    void __fastcall ZoomInClick(TObject *Sender);
    void __fastcall ZoomOutClick(TObject *Sender);
    void __fastcall Timer2Timer(TObject *Sender);
    void __fastcall PurgeButtonClick(TObject *Sender);
    void __fastcall InsertClick(TObject *Sender);
    void __fastcall CancelClick(TObject *Sender);
    void __fastcall CompleteClick(TObject *Sender);
    void __fastcall AreaListViewSelectItem(TObject *Sender, TListItem *Item,
                                           bool Selected);
    void __fastcall DeleteClick(TObject *Sender);
    void __fastcall AreaListViewCustomDrawItem(TCustomListView *Sender,
                                               TListItem *Item, TCustomDrawState State, bool &DefaultDraw);
    void __fastcall FormMouseWheel(TObject *Sender, TShiftState Shift,
                                   int WheelDelta, TPoint &MousePos, bool &Handled);
    void __fastcall RawConnectButtonClick(TObject *Sender);
    void __fastcall IdTCPClientRawConnected(TObject *Sender);
    void __fastcall RawRecordButtonClick(TObject *Sender);
    void __fastcall RawPlaybackButtonClick(TObject *Sender);
    void __fastcall IdTCPClientRawDisconnected(TObject *Sender);
    void __fastcall CycleImagesClick(TObject *Sender);
    void __fastcall SBSConnectButtonClick(TObject *Sender);
    void __fastcall SBSRecordButtonClick(TObject *Sender);
    void __fastcall SBSPlaybackButtonClick(TObject *Sender);
    void __fastcall IdTCPClientSBSConnected(TObject *Sender);
    void __fastcall IdTCPClientSBSDisconnected(TObject *Sender);
    void __fastcall TimeToGoTrackBarChange(TObject *Sender);
    void __fastcall MapComboBoxChange(TObject *Sender);
    void __fastcall MapComboBoxCloseUp(TObject *Sender);
    void __fastcall LiveMapCheckboxClick(TObject *Sender);
    void __fastcall BigQueryCheckBoxClick(TObject *Sender);
    void __fastcall UseSBSRemoteClick(TObject *Sender);
    void __fastcall UseSBSLocalClick(TObject *Sender);
    void __fastcall LoadARTCCBoundaries1Click(TObject *Sender);
    void __fastcall DisplayAirportCheckBoxClick(TObject *Sender);
    void __fastcall PlaybackSpeedTrackBarChanged(TObject *Sender);
    void __fastcall DrawCircleWithNumber(float x, float y, float radius, int number);
    void __fastcall AboutADSBDisplay1Click(TObject *Sender);
    void __fastcall UserManual1Click(TObject *Sender);
    void __fastcall PanelTitle1Click(TObject *Sender);
    void __fastcall AircraftTypeFilterComboBoxCloseUp(TObject *Sender);
    void __fastcall AltitudeFilterComboBoxCloseUp(TObject *Sender);
    void __fastcall SpeedFilterComboBoxCloseUp(TObject *Sender); 
    // Scrollbar
    void __fastcall MapScrollBoxScroll(TObject *Sender, TScrollBarKind ScrollBarKind, int ScrollCode, int &ScrollPos);
    void __fastcall MapScrollBoxVScroll(TObject *Sender, TScrollCode ScrollCode, int &ScrollPos);
    void __fastcall MapScrollBoxHScroll(TObject *Sender, TScrollCode ScrollCode, int &ScrollPos);
    void __fastcall MapVScrollBarScroll(TObject *Sender, TScrollCode ScrollCode, int &ScrollPos);
    void __fastcall MapHScrollBarScroll(TObject *Sender, TScrollCode ScrollCode, int &ScrollPos);
    void __fastcall UpdateScrollBarRanges();
    void __fastcall UpdateScrollBarPositions();
    // Playback Progress Bar
    void __fastcall PlaybackProgressBarChange(TObject *Sender);
    void __fastcall PlayPauseButtonClick(TObject *Sender);
    void __fastcall UpdatePlaybackProgress();
    void __fastcall InitializePlaybackProgress();
    void __fastcall SeekToPosition(__int64 targetTime);
    void __fastcall BuildFileIndex();
    AnsiString __fastcall FormatPlaybackTime(__int64 timeMs);
    void __fastcall PanelButtonMouseDown(TObject *Sender, TMouseButton Button,
    TShiftState Shift, int X, int Y);
    void __fastcall PanelButtonMouseUp(TObject *Sender, TMouseButton Button,
    TShiftState Shift, int X, int Y);
    void __fastcall FormKeyDown(TObject *Sender, WORD &Key, TShiftState Shift);
    void __fastcall FormKeyUp(TObject *Sender, WORD &Key, TShiftState Shift);
    void __fastcall FormActivate(TObject *Sender);
    // 항공기가 공항 근처에 있는지 확인하는 함수
    bool isAircraftNearAirport(uint32_t aircraftICAO);
	void __fastcall PaintBoxRouteLegendPaint(TObject *Sender);
private: // User declarations
    bool __fastcall CheckCellClickAndZoom(int X, int Y);
    void __fastcall DrawDeadReckoningStatusBar(void);
    void __fastcall DrawCenterCross(void);

    // Map scrollbars
    TScrollBar *MapVScrollBar;
    TScrollBar *MapHScrollBar;

    OpenGLFont2D *Font2DSmall;

    // 항공기-공항 거리 캐시
    std::map<std::pair<uint32_t, std::string>, DistanceCache> distanceCache;

    // 캐시된 거리 계산 함수
    double getCachedDistance(uint32_t aircraftICAO, const std::string &airportICAO,
                             double aircraftLat, double aircraftLon,
                             double airportLat, double airportLon);

    // 캐시 정리 함수
    void cleanupOldCache();

    void __fastcall GetTimeToGoLineColor(double speed, float &r, float &g, float &b, float &alpha);

    // 마지막 캐시 정리 시간
    std::chrono::system_clock::time_point lastCleanupTime;

    // Panel Menu Visible Toggle
    bool panelsVisible;

    // IP 주소 히스토리 관리
    TStringList *SBSIpHistory;
    TStringList *RawIpHistory;
    const int MAX_IP_HISTORY = 10; // 최대 10개까지 저장

    // IP 히스토리 관리 함수들
    void LoadIpHistory();
    void SaveIpHistory();
    void LoadIpHistoryToComboBox();
    void LoadWaypointsFromHttp(const std::string &callsign);
    AircraftPlannedRoute *plannedRouteManager;

    // Area Filter
    TList *selectedFilterAreas; // selected filtering Area  s
    bool areaFilterEnabled;     // enable Area filter

	// BigQuery related members
    std::unique_ptr<BigQueryClient> bigquery_client_;
	
	std::vector<AircraftData> bigquery_aircraft_data_;
    bool bigquery_initialized_;
	AnsiString bigquery_credentials_path_;
	
	// BigQuery periodic update settings
	bool bigquery_auto_update_enabled_;
	int bigquery_update_interval_seconds_;  // Update interval in seconds
	AnsiString last_tracked_icao_;  // Last tracked aircraft ICAO

	// Planned Route related members
	std::vector<AircraftData> planned_route_data_;
	bool planned_route_enabled_;
	AnsiString planned_route_icao_;  // Currently displayed planned route ICAO
	
	// BigQuery load methods
    void __fastcall InitializeBigQuery(void);
    void __fastcall LoadBigQueryData(void);
    void __fastcall LoadBigQueryDataByTimeRange(AnsiString start_time, AnsiString end_time);
    void __fastcall LoadBigQueryDataByICAO(AnsiString icao24);
    void __fastcall LoadRecentBigQueryData(int minutes);
    void __fastcall DisplayBigQueryData(void);
    void __fastcall DrawBigQueryTrackHistory(void);  // Draw BigQuery track history as red lines

    // Planned Route methods
    void __fastcall LoadPlannedRouteData(void);
    void __fastcall LoadPlannedRouteDataByICAO(AnsiString icao24);
    void __fastcall DisplayPlannedRouteData(void);
    void __fastcall DrawPlannedRoute(void);  // Draw planned route as blue lines
    void __fastcall DrawPlannedRouteLegend(void);  // Draw planned route legend
    void __fastcall TogglePlannedRouteDisplay(void);
    void __fastcall ClearPlannedRouteData(void);
	
    // 항공기-공항 거리 계산 결과
    AircraftAirportDistanceResult *aircraftAirportDistanceResult;

    // 항공기-공항 거리 계산 스레드
    TAircraftAirportDistanceThread *distanceCalculationThread;

    // 거리 계산 스레드 시작/중지
    void startDistanceCalculationThread();
    void stopDistanceCalculationThread();

    // 항공기 간 거리 계산 결과
    AircraftAircraftDistanceResult *aircraftAircraftDistanceResult;

    // 항공기 간 거리 계산 스레드
    TAircraftAircraftDistanceThread *aircraftDistanceCalculationThread;

    // 항공기 간 거리 계산 스레드 시작/중지
    void startAircraftDistanceCalculationThread();
    void stopAircraftDistanceCalculationThread();

    // Raw 연결 관련 함수들
    void __fastcall ShowRawConnectionLostDialog();
    void __fastcall ReconnectToRawDevice();

    // SBS 연결 관련 함수들
    void __fastcall ShowSBSConnectionLostDialog();
    void __fastcall ReconnectToSBSDevice();

    // 타임아웃 관련 함수들
    void __fastcall ShowRawTimeoutDialog();
    void __fastcall ShowSBSTimeoutDialog();

    // 연결 상태 표시 함수들
    void __fastcall UpdateRawConnectionStatus(const AnsiString &status);
    void __fastcall UpdateSBSConnectionStatus(const AnsiString &status);

    // 연결 재시도 관련 변수들
    __int64 LastRawConnectionCheck; // 마지막 Raw 연결 체크 시간
    __int64 LastSBSConnectionCheck; // 마지막 SBS 연결 체크 시간

    // 빠른 클릭 감지를 위한 변수들
    int rapidClickCount;         // 빠른 클릭 횟수
    __int64 firstClickTime;      // 첫 번째 클릭 시간
    static const int RAPID_CLICK_THRESHOLD = 10;    // 클릭 임계값 (10회)
    static const int RAPID_CLICK_TIME_WINDOW = 2000; // 시간 윈도우 (2초, 밀리초)

    // 숨겨진 기능 다이얼로그 표시
    void __fastcall ShowSecretDialog();

    bool PlaybackSeeking;
    __int64 PlaybackStartTime;
    __int64 PlaybackEndTime;

    bool PlaybackPaused;
    TStringList *PlaybackFileIndex;

    enum { DIR_NONE, DIR_VERT, DIR_HORZ } lastArrowDir = DIR_NONE;

public: // User declarations
    __fastcall TForm1(TComponent *Owner);
    __fastcall ~TForm1();
    void __fastcall LatLon2XY(double lat, double lon, double &x, double &y);
    int __fastcall XY2LatLon2(int x, int y, double &lat, double &lon);
    void __fastcall HookTrack(int X, int Y, bool CPA_Hook);
    void __fastcall DrawObjects(void);
    void __fastcall DeleteAllAreas(void);
    void __fastcall Purge(void);
    void __fastcall SendCotMessage(AnsiString IP_address, unsigned short Port, char *Buffer, DWORD Length);
    void __fastcall RegisterWithCoTRouter(void);
    void __fastcall SetMapCenter(double &x, double &y);
    void __fastcall LoadMap(int Type);
    void __fastcall CreateBigQueryCSV(void);
    void __fastcall CloseBigQueryCSV(void);
    bool __fastcall LoadARTCCBoundaries(AnsiString FileName);
    void __fastcall UpdateAircraftHistory(TADS_B_Aircraft *aircraft);
    void __fastcall PurgeOldHistory(TADS_B_Aircraft *aircraft, __int64 currentTime);
    void __fastcall TogglePanels(); // Panel Menu Visible
    // Area Filter functions
    bool __fastcall IsAircraftInSelectedAreas(TADS_B_Aircraft *aircraft);
    void __fastcall AddAreaToFilter(TArea *area);
    void __fastcall RemoveAreaFromFilter(TArea *area);
    void __fastcall ClearAreaFilter();
    void __fastcall ToggleAreaInFilter(TArea *area);
    bool __fastcall IsAreaInFilter(TArea *area);
    int __fastcall GetFilteredAreaCount();
    int MouseDownX, MouseDownY;
    bool MouseDown;
    TTrackHook TrackHook;
    Vector3d Map_v[4], Map_p[4];
    Vector2d Map_w[2];
    double Mw1, Mw2, Mh1, Mh2, xf, yf;
    KeyholeConnection *g_Keyhole;
    FilesystemStorage *g_Storage;
    MasterLayer *g_MasterLayer;
    TileManager *g_GETileManager;
    EarthView *g_EarthView;
    double MapCenterLat, MapCenterLon;
    int g_MouseLeftDownX;
    int g_MouseLeftDownY;
    int g_MouseDownMask;
    bool LoadMapFromInternet;
    TList *Areas;
    TArea *AreaTemp;
    ght_hash_table_t *HashTable;
    TTCPClientRawHandleThread *TCPClientRawHandleThread;
    TTCPClientSBSHandleThread *TCPClientSBSHandleThread;
    TStreamWriter *RecordRawStream;
    TStreamReader *PlayBackRawStream;
    TStreamWriter *RecordSBSStream;
    TStreamReader *PlayBackSBSStream;
    TStreamWriter *BigQueryCSV;
    AnsiString BigQueryCSVFileName;
    unsigned int BigQueryRowCount;
    unsigned int BigQueryFileCount;
    AnsiString BigQueryPythonScript;
    AnsiString BigQueryPath;
    AnsiString BigQueryLogFileName;
    int NumSpriteImages;
    int CurrentSpriteImage;
    AnsiString AircraftDBPathFileName;
    AnsiString ARTCCBoundaryDataPathFileName;
    int SelectedMapIndex;
    int SelectedAircraftTypeFilter;
    int SelectedAltitudeFilter;
    int SelectedSpeedFilter;
    int gCell[10][10] = {
        0,
    };
    __int64 PlaybackCurrentTime;
    bool ProgrammaticProgressUpdate;

	void DrawAirportIcon(double lat, double lon, bool isDeparture, bool isAlreadyOccupy);
    void DrawAirportInfo(double lat, double lon, const char *name, bool isDeparture);
    int __fastcall getAirplaneType(uint32_t addr);
    void UpdateAircraftInfo(TADS_B_Aircraft *Data);
    void UpdateRouteInfo(TADS_B_Aircraft *Data);
    void ClearAircraftInfo();
    void ClearRouteInfo();

    // Airport management
    AirportDataManager *airportManager;

    // IP 히스토리에 추가 (public으로 이동)
    void AddToIpHistory(AnsiString ip, bool isSBS);
    // Draw all airport in the map by checkbox
    void __fastcall DrawAllAirports();
    void __fastcall GetAltitudeLineColor(double altitude, float &r, float &g, float &b, float &alpha);
    AnsiString __fastcall SafeAnsiString(AnsiString input);
    // aircraft altitude filter
    bool __fastcall IsAircraftInAltitudeFilter(TADS_B_Aircraft *aircraft);  
    // aircraft speed filter
    bool __fastcall IsAircraftInSpeedFilter(TADS_B_Aircraft *aircraft);  
    // 대원호 경로 그리기 함수 추가
    void __fastcall DrawGreatCircleRoute(double depLat, double depLon, double arrLat, double arrLon, const char* depICAO, const char* arrICAO);
    void __fastcall CalculateGreatCirclePoints(double lat1, double lon1, double lat2, double lon2, std::vector<double>& lats, std::vector<double>& lons, int numPoints = 100);
    void __fastcall DrawAirportIconOnPanel(TCanvas* Canvas, int x, int y, bool isDeparture, int size = 10);
    void __fastcall PanelRouteLegendPaint(TObject *Sender);
};
//---------------------------------------------------------------------------
extern PACKAGE TForm1 *Form1;
//---------------------------------------------------------------------------

// 수학 상수들 (성능 최적화를 위해 미리 계산)
#define DEG_TO_RAD 0.0174532925199433 // M_PI / 180.0
#define RAD_TO_DEG 57.2957795131      // 180.0 / M_PI

#endif
