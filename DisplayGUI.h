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
#include <map>
#include <unordered_map>
#include <chrono>
// Forward declarations
class AirportDataManager;

typedef float T_GL_Color[4];

typedef struct
{
	bool Valid_CC;
	bool Valid_CPA;
	uint32_t ICAO_CC;
	uint32_t ICAO_CPA;
}TTrackHook;

typedef struct
{
	double lat;
	double lon;
	double hae;
}TPolyLine;


#define MAX_AREA_POINTS 500
typedef struct
{
	AnsiString  Name;
	TColor      Color;
	DWORD       NumPoints;
	pfVec3      Points[MAX_AREA_POINTS];
	pfVec3      PointsAdj[MAX_AREA_POINTS];
	TTriangles *Triangles;
	bool        Selected;
}TArea;

// 거리 계산 결과를 저장할 구조체
struct DistanceCache {
    double distance;
    std::chrono::system_clock::time_point timestamp;
};

// 캐시 만료 시간 (밀리초)
const int CACHE_EXPIRY_MS = 3000; // 3 Sec
const int CACHE_CLEANUP_INTERVAL_MS = 5000; // 5초마다 캐시 정리
const int CACHE_MAX_AGE_MS = 30000; // 30초 이상 된 캐시 제거

//---------------------------------------------------------------------------
class  TTCPClientRawHandleThread : public TThread
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
class  TTCPClientSBSHandleThread : public TThread
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
	__fastcall TTCPClientSBSHandleThread(bool value);
	~TTCPClientSBSHandleThread();
};
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
class  TConnectionThread : public TThread
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
__published:	// IDE-managed Components
	TMainMenu *MainMenu1;
	TPanel *RightPanel;
	TMenuItem *File1;
	TMenuItem *Exit1;
	TTimer *Timer1;
	TOpenGLPanel *ObjectDisplay;
	TPanel *Panel1;
	TPanel *Panel3;
	TButton *ZoomIn;
	TButton *ZoomOut;
	TCheckBox *DrawMap;
	TCheckBox *PurgeStale;
	TTimer *Timer2;
	TCSpinEdit *CSpinStaleTime;
	TButton *PurgeButton;
	TListView *AreaListView;
	TButton *Insert;
	TButton *Delete;
	TButton *Complete;
	TButton *Cancel;
	TButton *RawConnectButton;
	//TLabel *Label16;
	//TLabel *Label17;
	TComboBox *RawIpAddress;
	TIdTCPClient *IdTCPClientRaw;
	TSaveDialog *RecordRawSaveDialog;
	TOpenDialog *PlaybackRawDialog;
	TCheckBox *CycleImages;
	TScrollBox *Panel4;
	TLabel *CLatLabel;
	TLabel *CLonLabel;
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
	TLabel *Label7;
	TLabel *Label6;
	TLabel *Label18;
	TLabel *FlightNumLabel;
	TLabel *ICAOLabel;
	TLabel *Label5;
	//TLabel *Label4;
	TPanel *Panel5;
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
	TButton *RawPlaybackButton;
	TButton *RawRecordButton;
	TIdTCPClient *IdTCPClientSBS;
	TButton *SBSConnectButton;
	TComboBox *SBSIpAddress;
	TButton *SBSRecordButton;
	TButton *SBSPlaybackButton;
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
	TLabel *Label16;
	TCheckBox *DisplayAirportCheckBox;
	TTrackBar *PlaybackSpeedTrackBar;
	TMenuItem *Help1;
	TMenuItem *AboutADSBDisplay1;
	TMenuItem *UserManual1;
	TPanel *PanelTitle1;
	TPanel *Panel7;
	TLabel *PanelTitle2;
	TLabel *Label20;
	TLabel *Label21;
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
	TLabel *EngineType;
	TLabel *Label4;
	TLabel *Label28;
	TLabel *Label29;
	TLabel *CeritificatedInfo;
	TLabel *Label30;
	TLabel *ExpirationData;
	TLabel *Label31;
	TLabel *DepartureAirportName;
	TLabel *Label32;
	TLabel *Label33;
	TLabel *DepartureAirportICAO;
	TLabel *Label35;
	TLabel *DepartureAirportLocation;
	TLabel *Label34;
	TLabel *Label36;
	TLabel *Label37;
	TLabel *Label38;
	TLabel *DestinationAirportName;
	TLabel *Label39;
	TLabel *DestinationAirportICAO;
	TLabel *Label40;
	TLabel *DestinationAirportLocation;
	TLabel *Label41;
	TLabel *Label42;
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
private:	// User declarations
	// 항공기-공항 거리 캐시
	std::map<std::pair<uint32_t, std::string>, DistanceCache> distanceCache;
	
	// 캐시된 거리 계산 함수
	double getCachedDistance(uint32_t aircraftICAO, const std::string& airportICAO, 
						   double aircraftLat, double aircraftLon,
						   double airportLat, double airportLon);

	// 캐시 정리 함수
	void cleanupOldCache();
	
	// 마지막 캐시 정리 시간
	std::chrono::system_clock::time_point lastCleanupTime;
	
	// Panel Menu Visible Toggle
	bool panelsVisible;

	// IP 주소 히스토리 관리
	TStringList* SBSIpHistory;
	TStringList* RawIpHistory;
	const int MAX_IP_HISTORY = 10; // 최대 10개까지 저장
	
	// IP 히스토리 관리 함수들
	void LoadIpHistory();
	void SaveIpHistory();
	void LoadIpHistoryToComboBox();
	
	// Area Filter
	TList *selectedFilterAreas;  // selected filtering Area  s
    bool areaFilterEnabled;     // enable Area filter

public:		// User declarations
	__fastcall TForm1(TComponent* Owner);
	__fastcall ~TForm1();
	void __fastcall LatLon2XY(double lat,double lon, double &x, double &y);
	int __fastcall  XY2LatLon2(int x, int y,double &lat,double &lon );
	void __fastcall HookTrack(int X, int Y,bool CPA_Hook);
	void __fastcall DrawObjects(void);
	void __fastcall DeleteAllAreas(void);
	void __fastcall Purge(void);
	void __fastcall SendCotMessage(AnsiString IP_address, unsigned short Port,char *Buffer,DWORD Length);
	void __fastcall RegisterWithCoTRouter(void);
    void __fastcall SetMapCenter(double &x, double &y);
    void __fastcall LoadMap(int Type);
    void __fastcall CreateBigQueryCSV(void);
    void __fastcall CloseBigQueryCSV(void);
    bool __fastcall LoadARTCCBoundaries(AnsiString FileName);
	void __fastcall UpdateAircraftHistory(TADS_B_Aircraft* aircraft);
	void __fastcall PurgeOldHistory(TADS_B_Aircraft* aircraft, __int64 currentTime);
  	void __fastcall TogglePanels();  // Panel Menu Visible
	//Area Filter functions
    bool __fastcall IsAircraftInSelectedAreas(TADS_B_Aircraft* aircraft);
    void __fastcall AddAreaToFilter(TArea* area);
    void __fastcall RemoveAreaFromFilter(TArea* area);
    void __fastcall ClearAreaFilter();
    void __fastcall ToggleAreaInFilter(TArea* area);
    bool __fastcall IsAreaInFilter(TArea* area);
    int __fastcall GetFilteredAreaCount();
    void __fastcall FormKeyDown(TObject *Sender, WORD &Key, TShiftState Shift);

	int                        MouseDownX,MouseDownY;
	bool                       MouseDown;
	TTrackHook                 TrackHook;
	Vector3d                   Map_v[4],Map_p[4];
	Vector2d                   Map_w[2];
	double                     Mw1,Mw2,Mh1,Mh2,xf,yf;
	KeyholeConnection	      *g_Keyhole;
	FilesystemStorage	      *g_Storage;
	MasterLayer	      	      *g_MasterLayer;
	TileManager		          *g_GETileManager;
	EarthView		          *g_EarthView;
	double                     MapCenterLat,MapCenterLon;
	int			               g_MouseLeftDownX;
	int			               g_MouseLeftDownY;
	int			               g_MouseDownMask ;
	bool                       LoadMapFromInternet;
	TList                     *Areas;
	TArea                     *AreaTemp;
	ght_hash_table_t          *HashTable;
	TTCPClientRawHandleThread *TCPClientRawHandleThread;
    TTCPClientSBSHandleThread *TCPClientSBSHandleThread;
	TStreamWriter              *RecordRawStream;
	TStreamReader              *PlayBackRawStream;
    TStreamWriter              *RecordSBSStream;
	TStreamReader              *PlayBackSBSStream;
	TStreamWriter              *BigQueryCSV;
    AnsiString                 BigQueryCSVFileName;
	unsigned int               BigQueryRowCount;
	unsigned int               BigQueryFileCount;
    AnsiString                 BigQueryPythonScript;
	AnsiString                 BigQueryPath;
	AnsiString                 BigQueryLogFileName;
	int                        NumSpriteImages;
	int                        CurrentSpriteImage;
	AnsiString                 AircraftDBPathFileName;
	AnsiString                 ARTCCBoundaryDataPathFileName;
	int                        SelectedMapIndex;

    void DrawAirportIcon(double lat, double lon, bool isDeparture);
	void DrawAirportInfo(double lat, double lon, const char* name, bool isDeparture);
	int __fastcall getAirplaneType(uint32_t addr);
  	void UpdateAircraftInfo(TADS_B_Aircraft* Data);
  	void UpdateRouteInfo(TADS_B_Aircraft* Data);
  	void ClearAircraftInfo();
  	void ClearRouteInfo();

	// Airport management
	AirportDataManager        *airportManager;

	// IP 히스토리에 추가 (public으로 이동)
	void AddToIpHistory(AnsiString ip, bool isSBS);
};
//---------------------------------------------------------------------------
extern PACKAGE TForm1 *Form1;
//---------------------------------------------------------------------------

#endif
