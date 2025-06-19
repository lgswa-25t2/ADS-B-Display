//---------------------------------------------------------------------------

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

#define AIRCRAFT_DATABASE_URL   "https://opensky-network.org/datasets/metadata/aircraftDatabase.zip"
#define AIRCRAFT_DATABASE_FILE   "aircraftDatabase.csv"
#define ARTCC_BOUNDARY_FILE      "Ground_Level_ARTCC_Boundary_Data_2025-05-15.csv"

#define MAP_CENTER_LAT  40.73612;
#define MAP_CENTER_LON -80.33158;

#define BIG_QUERY_UPLOAD_COUNT 50000
#define BIG_QUERY_RUN_FILENAME  "SimpleCSVtoBigQuery.py"
#define   LEFT_MOUSE_DOWN   1
#define   RIGHT_MOUSE_DOWN  2
#define   MIDDLE_MOUSE_DOWN 4

#define BG_INTENSITY   0.37
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
 static void RunPythonScript(AnsiString scriptPath,AnsiString args);
 static bool DeleteFilesWithExtension(AnsiString dirPath, AnsiString extension);
 static int FinshARTCCBoundary(void);
 //---------------------------------------------------------------------------

static char *stristr(const char *String, const char *Pattern);
static const char * strnistr(const char * pszSource, DWORD dwLength, const char * pszFind) ;
int globalTrackbarValue = 1;
extern ght_hash_table_t *AircraftDBHashTable;

//---------------------------------------------------------------------------
uint32_t createRGB(uint8_t r, uint8_t g, uint8_t b)
{
	return ((uint32_t)r << 16) | ((uint32_t)g << 8) | b;
}
//---------------------------------------------------------------------------
uint32_t PopularColors[] = {
	createRGB(255, 0, 0),	  // Red
	createRGB(0, 255, 0),	  // Green
	createRGB(0, 0, 255),	  // Blue
	createRGB(255, 255, 0),	  // Yellow
	createRGB(255, 165, 0),	  // Orange
	createRGB(255, 192, 203), // Pink
	createRGB(0, 255, 255),	  // Cyan
	createRGB(255, 0, 255),	  // Magenta
	createRGB(255, 255, 255), // White
	// createRGB(0, 0, 0),        // Black
	createRGB(128, 128, 128), // Gray
	createRGB(165, 42, 42)	  // Brown
};

int NumColors = sizeof(PopularColors) / sizeof(PopularColors[0]);
unsigned int CurrentColor = 0;
__int64 LastHeartbeatTime = 0;
bool RawTimeoutPopupShown = true;
__int64 LastSBSDataReceiveTime = 0;
bool SBSTimeoutPopupShown = true;

//---------------------------------------------------------------------------
typedef struct
{
   union{ 
     struct{ 
	 System::Byte Red;
	 System::Byte Green;
	 System::Byte Blue;
	 System::Byte Alpha;
     }; 
     struct{ 
     TColor Cl; 
     }; 
     struct{ 
     COLORREF Rgb; 
     }; 
   };

}TMultiColor;


//---------------------------------------------------------------------------
static const char * strnistr(const char * pszSource, DWORD dwLength, const char * pszFind)
{
	DWORD        dwIndex   = 0;
	DWORD        dwStrLen  = 0;
	const char * pszSubStr = NULL;

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

		dwIndex ++;
	}

	return pszSubStr;
}
//---------------------------------------------------------------------------
static char *stristr(const char *String, const char *Pattern)
{
  char *pptr, *sptr, *start;
  size_t  slen, plen;

  for (start = (char *)String,pptr  = (char *)Pattern,slen  = strlen(String),plen  = strlen(Pattern);
       slen >= plen;start++, slen--)
      {
            /* find start of pattern in string */
            while (toupper(*start) != toupper(*Pattern))
            {
                  start++;
                  slen--;

                  /* if pattern longer than string */

                  if (slen < plen)
                        return(NULL);
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
   return(NULL);
}
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
__fastcall TForm1::TForm1(TComponent* Owner)
	: TForm(Owner)
{
	// Initialize IP history
	LoadIpHistory();
	
	// Initialize cache cleanup time
	lastCleanupTime = std::chrono::system_clock::now();
	
	AircraftDBPathFileName=ExtractFilePath(ExtractFileDir(Application->ExeName)) +AnsiString("..\\AircraftDB\\")+AIRCRAFT_DATABASE_FILE;
  ARTCCBoundaryDataPathFileName=ExtractFilePath(ExtractFileDir(Application->ExeName)) +AnsiString("..\\ARTCC_Boundary_Data\\")+ARTCC_BOUNDARY_FILE;
  BigQueryPath=ExtractFilePath(ExtractFileDir(Application->ExeName)) +AnsiString("..\\BigQuery\\");
  BigQueryPythonScript= BigQueryPath+ AnsiString(BIG_QUERY_RUN_FILENAME);
  DeleteFilesWithExtension(BigQueryPath, "csv");
  BigQueryLogFileName=BigQueryPath+"BigQuery.log";
  DeleteFileA(BigQueryLogFileName.c_str());
  CurrentSpriteImage=0;
  InitDecodeRawADS_B();
  RecordRawStream=NULL;
  PlayBackRawStream=NULL;
  TrackHook.Valid_CC=false;
  TrackHook.Valid_CPA=false;

  HashTable = ght_create(50000);

  if ( !HashTable)
	{
	  throw Sysutils::Exception("Create Hash Failed");
	}
  ght_set_rehash(HashTable, TRUE);

  AreaTemp=NULL;
  Areas= new TList;

 MouseDown=false;

 MapCenterLat=MAP_CENTER_LAT;
 MapCenterLon=MAP_CENTER_LON;

 LoadMapFromInternet=false;
 MapComboBox->ItemIndex=GoogleMaps;
 SelectedMapIndex=GoogleMaps;  
 
 panelsVisible = true;

 Panel4->VertScrollBar->Position = 0;
 Panel4->HorzScrollBar->Position= 0;

 this->ShowHint = true;
	// PanelTitle1에 tooltip 설정
 PanelTitle1->ShowHint = true;
 PanelTitle1->Hint = "Click to hide Control Menu";
 PanelTitle1->Cursor = crHandPoint;

 // Hint 표시 시간 조정 (선택사항)
 Application->HintPause = 500;    // 0.5초 후 표시
 Application->HintHidePause = 5000; // 5초 후 숨김

 LoadMap(MapComboBox->ItemIndex);

 g_EarthView->m_Eye.h /= pow(1.3,18);//pow(1.3,43);
 SetMapCenter(g_EarthView->m_Eye.x, g_EarthView->m_Eye.y);
 TimeToGoTrackBar->Position=120;
 BigQueryCSV=NULL;
 BigQueryRowCount=0;
 BigQueryFileCount=0;
 InitAircraftDB(AircraftDBPathFileName);
 printf("init complete\n");
 //init airport db
 AnsiString AirportDBFile = ExtractFilePath(ExtractFileDir(Application->ExeName)) +AnsiString("..\\AirportDB\\")+"airports.csv";
 if (FileExists(AirportDBFile)) {
     if (!InitAirportDB(AirportDBFile)) {
         ShowMessage("Failed to load Airport Database");
     }
 } else {
     ShowMessage("Airport Database file not found: " + AirportDBFile);
 }
 
 //init AirportDataManager
 airportManager = new AirportDataManager();
 
 //load AirportData from hashtable
 //printf("=== Airport Data Loading Debug ===\n");
 auto airportData = GetAllAirportsFromHashTable();
 //printf("GetAllAirportsFromHashTable returned %d airports\n", (int)airportData.size());
 
 if (!airportData.empty()) {
     // print airport info for first 5 airports for debugging
     /*
     for (int i = 0; i < min(5, (int)airportData.size()); i++) {
         printf("Airport %d: ICAO=%s, Name=%s, Lat=%.6f, Lon=%.6f\n",
                i, airportData[i].ICAO.c_str(), airportData[i].Name.c_str(),
                airportData[i].Latitude, airportData[i].Longitude);
     } */
     
     airportManager->loadAirportsFromHashTable(airportData);
     printf("Successfully loaded %d airports into AirportDataManager\n", (int)airportData.size());
 } else {
     printf("No airport data found in hash table\n");
 }
 //printf("=== End Airport Data Loading Debug ===\n");

  // Set to initial state
 	ClearAircraftInfo();

  lastCleanupTime = std::chrono::system_clock::now();
 // Initial Trackbar Value
 PlaybackSpeedTrackBar->Visible = false;

}
//---------------------------------------------------------------------------
__fastcall TForm1::~TForm1()
{
	// Save IP history before closing
	SaveIpHistory();
	
	// Clean up IP history
	if (SBSIpHistory) {
		delete SBSIpHistory;
		SBSIpHistory = NULL;
	}
	if (RawIpHistory) {
		delete RawIpHistory;
		RawIpHistory = NULL;
	}
	
	Timer1->Enabled=false;
 Timer2->Enabled=false;
 delete g_EarthView;
 if (g_GETileManager) delete g_GETileManager;
 delete g_MasterLayer;
 delete g_Storage;
 if (LoadMapFromInternet)
 {
   if (g_Keyhole) delete g_Keyhole;
 }
  //cleanup AirportDB
 CleanupAirportDB();
 
 // delete AirportDataManager
 if (airportManager) {
     delete airportManager;
     airportManager = nullptr;
 }
}
//---------------------------------------------------------------------------
void __fastcall  TForm1::SetMapCenter(double &x, double &y)
{
  double siny;
  x=(MapCenterLon+0.0)/360.0;
  siny=sin((MapCenterLat * M_PI) / 180.0);
  siny = fmin(fmax(siny, -0.9999), 0.9999);
  y=(log((1 + siny) / (1 - siny)) / (4 * M_PI));
}
//---------------------------------------------------------------------------
void __fastcall TForm1::ObjectDisplayInit(TObject *Sender)
{
	glViewport(0,0,(GLsizei)ObjectDisplay->Width,(GLsizei)ObjectDisplay->Height);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glEnable (GL_LINE_STIPPLE);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    NumSpriteImages=MakeAirplaneImages();
	MakeAirTrackFriend();
	MakeAirTrackHostile();
	MakeAirTrackUnknown();
	MakePoint();
	MakeTrackHook();
	g_EarthView->Resize(ObjectDisplay->Width,ObjectDisplay->Height);
	glPushAttrib (GL_LINE_BIT);
	glPopAttrib ();
    printf("OpenGL Version %s\n",glGetString(GL_VERSION));
}
//---------------------------------------------------------------------------

void __fastcall TForm1::ObjectDisplayResize(TObject *Sender)
{
	 double Value;
	//ObjectDisplay->Width=ObjectDisplay->Height;
	glViewport(0,0,(GLsizei)ObjectDisplay->Width,(GLsizei)ObjectDisplay->Height);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glEnable (GL_LINE_STIPPLE);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	g_EarthView->Resize(ObjectDisplay->Width,ObjectDisplay->Height);
}
//---------------------------------------------------------------------------
void __fastcall TForm1::ObjectDisplayPaint(TObject *Sender)
{

 if (DrawMap->Checked)
   glClearColor(0.0,0.0,0.0,0.0);
 else
   glClearColor(0.94, 0.94, 0.96, 1.0); //background color

 glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

 g_EarthView->Animate();
 g_EarthView->Render(DrawMap->Checked);
 g_GETileManager->Cleanup();
 Mw1 = Map_w[1].x-Map_w[0].x;
 Mw2 = Map_v[1].x-Map_v[0].x;
 Mh1 = Map_w[1].y-Map_w[0].y;
 Mh2 = Map_v[3].y-Map_v[0].y;

 xf=Mw1/Mw2;
 yf=Mh1/Mh2;

 DrawObjects();
}
//---------------------------------------------------------------------------
void __fastcall TForm1::Timer1Timer(TObject *Sender)
{
 __int64 CurrentTime;

 CurrentTime=GetCurrentTimeInMsec();
 SystemTime->Caption=TimeToChar(CurrentTime);

 // monitoring cache status (per 5 secs)
 static __int64 lastCacheCheck = 0;
 if (CurrentTime - lastCacheCheck > 5000) {
     if (g_GETileManager) {
         int currentTextures = g_GETileManager->GetTextureCount();
         int maxTextures = g_GETileManager->GetMaxTextures();
         printf("Cache Status: %d/%d textures (%.1f%% full)\n", 
                currentTextures, maxTextures, 
                (float)currentTextures / maxTextures * 100.0f);
     }
     lastCacheCheck = CurrentTime;
 }
 
   // Check RawData Timeout
  __int64 now = GetCurrentTimeInMsec();
  if (RawConnectButton->Caption=="Raw Disconnect") {
	if (!RawTimeoutPopupShown && (now - LastHeartbeatTime > 10000)) // 10 Sec
	{
		RawTimeoutPopupShown = true;
		if (Form1->IdTCPClientRaw->Connected()) {
			TCPClientRawHandleThread->Terminate();
			IdTCPClientRaw->Disconnect();
			IdTCPClientRaw->IOHandler->InputBuffer->Clear();
		}
		RawConnectButton->Caption="Raw Connect";
		RawPlaybackButton->Enabled=true;
		//ShowMessage("Raw data heartbeat timeout: No heartbeat received from PI for 10 seconds.");
		ShowMessage("SBS Hub connection timeout: No data received from PI for 10 seconds.");
	}
  }

	 // Check SBSData Timeout
  if (SBSConnectButton->Caption=="SBS Disconnect") {
	if (!SBSTimeoutPopupShown && (now - LastSBSDataReceiveTime > 10000)) // 10 Sec
	{
		SBSTimeoutPopupShown = true;
		if (Form1->IdTCPClientSBS->Connected()) {
			TCPClientSBSHandleThread->Terminate();
			IdTCPClientSBS->Disconnect();
			IdTCPClientSBS->IOHandler->InputBuffer->Clear();
		}
		SBSConnectButton->Caption="SBS Connect";
		SBSPlaybackButton->Enabled=true;
		ShowMessage("SBS Hub connection timeout: No data received from SBS Hub for 10 seconds.");
	}
  }

 ObjectDisplay->Repaint();
}
//---------------------------------------------------------------------------
void __fastcall TForm1::DrawObjects(void)
{
    // Cache remove
    cleanupOldCache();
    
    double ScrX, ScrY;
    int    ViewableAircraft=0;

  glEnable( GL_LINE_SMOOTH );
  glEnable( GL_POINT_SMOOTH );
  glEnable (GL_BLEND);
  glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glHint (GL_LINE_SMOOTH_HINT, GL_NICEST);
  glHint (GL_POINT_SMOOTH_HINT, GL_NICEST);
  glLineWidth(3.0);
  glPointSize(4.0);

  LatLon2XY(MapCenterLat,MapCenterLon, ScrX, ScrY);

  // display Airports Info (DisplayAirportCheckBox is checked)
  if (DisplayAirportCheckBox->Checked && airportManager) {
      /*
      printf("=== Airport Display Debug ===\n");
      printf("DisplayAirportCheckBox is checked: %s\n", DisplayAirportCheckBox->Checked ? "true" : "false");
      printf("airportManager exists: %s\n", airportManager ? "true" : "false");
      */
      // calculate current screen
      double minLat, maxLat, minLon, maxLon;
      int screenWidth = ObjectDisplay->Width;
      int screenHeight = ObjectDisplay->Height;
      
      //printf("Screen size: %dx%d\n", screenWidth, screenHeight);
      
      // Convert screen corner coordinates to latitude and longitude
      XY2LatLon2(0, 0, minLat, maxLon);
      XY2LatLon2(screenWidth, screenHeight, maxLat, minLon);
      /*
      printf("Screen bounds: Lat[%.6f, %.6f], Lon[%.6f, %.6f]\n",
             minLat, maxLat, minLon, maxLon);
      */
      // current zoon level (not used at the moment)
      int zoomLevel = (int)(log(g_EarthView->m_Eye.h) / log(1.3));
      //printf("Current zoom level: %d (Eye.h = %.6f)\n", zoomLevel, g_EarthView->m_Eye.h);
      
      // for test: Conversion using known coordinates.
	    /*
      double testLat = 37.5665;  // Seoul Lat
      double testLon = 126.9780; // Seoul Long
      double testX, testY;
      LatLon2XY(testLat, testLon, testX, testY);
      printf("Test coordinate: Seoul(%.6f,%.6f) -> Screen(%.2f,%.2f)\n", 
             testLat, testLon, testX, testY);  */
      
      // get airports to display on the screen.
      auto visibleAirports = airportManager->getVisibleAirports(
          minLat, maxLat, minLon, maxLon, zoomLevel);
      
      //printf("Found %d visible airports\n", (int)visibleAirports.size());
      
      // Draw airports on the screen (in the same way as aircraft).
      glColor4f(0.8, 0.0, 0.0, 1.0);  // red
      
      int drawnCount = 0;
      for (const auto& airport : visibleAirports) {
          double airportX, airportY;
          LatLon2XY(airport.latitude, airport.longitude, airportX, airportY);
          /*
          if (drawnCount < 10) {  // Detailed log for the first 10 items only.
              printf("Airport %s: lat=%.6f,lon=%.6f -> screen=(%.2f,%.2f)\n",
                     airport.icao.c_str(), airport.latitude, airport.longitude, 
                     airportX, airportY);
          }
          */
          // Draw airports as triangles icon.
          //float size = 8.0f;  // size of triangle
          float size = airportManager->getAirportIconSize(airport, zoomLevel);
          glBegin(GL_TRIANGLES);
          glVertex2f(airportX, airportY - size);        // Top vertex
          glVertex2f(airportX - size, airportY + size); // left vertex
          glVertex2f(airportX + size, airportY + size); // right vertex
          glEnd();
          
          drawnCount++;
      }
      /*
      printf("Actually drew %d airports\n", drawnCount);
      printf("=== End Airport Debug ===\n");
      */
  } else {
      /*
      printf("Airport display skipped - CheckBox:%s, Manager:%s\n",
             DisplayAirportCheckBox->Checked ? "checked" : "unchecked",
             airportManager ? "exists" : "null");
      */
  }


  if (DrawMap->Checked)
    glColor4f(1, 1, 1, 1.0);  // white
  else
    glColor4f(0.3, 0.3, 0.3, 0.8);  // dark gray

  glBegin(GL_LINE_STRIP);
  glVertex2f(ScrX-20.0,ScrY);
  glVertex2f(ScrX+20.0,ScrY);
  glEnd();

  glBegin(GL_LINE_STRIP);
  glVertex2f(ScrX,ScrY-20.0);
  glVertex2f(ScrX,ScrY+20.0);
  glEnd();


  uint32_t *Key;
  ght_iterator_t iterator;
  TADS_B_Aircraft* Data,*DataCPA;
  TAircraftData *a = NULL;

  DWORD i,j,Count;
  int cell[10][10] = {0, };
  int cellWidth = ObjectDisplay->Width / 10 - 1;
  int cellHeight = ObjectDisplay->Height / 10 - 1;
  int cellSize = std::min(cellWidth, cellHeight) /2;
  int cellMin = cellSize/5;
  int cellMax = cellSize;
  double cellDrawZoomRate = 0.00005;

  if (AreaTemp)
  {
   glPointSize(3.0);
	for (DWORD i = 0; i <AreaTemp->NumPoints ; i++)
		LatLon2XY(AreaTemp->Points[i][1],AreaTemp->Points[i][0],
				  AreaTemp->PointsAdj[i][0],AreaTemp->PointsAdj[i][1]);

   glBegin(GL_POINTS);
   for (DWORD i = 0; i <AreaTemp->NumPoints ; i++)
	{
	glVertex2f(AreaTemp->PointsAdj[i][0],
			   AreaTemp->PointsAdj[i][1]);
	}
	glEnd();
   glBegin(GL_LINE_STRIP);
   for (DWORD i = 0; i <AreaTemp->NumPoints ; i++)
	{
	glVertex2f(AreaTemp->PointsAdj[i][0],
			   AreaTemp->PointsAdj[i][1]);
	}
	glEnd();
  }
	Count=Areas->Count;
	for (i = 0; i < Count; i++)
	 {
	   TArea *Area = (TArea *)Areas->Items[i];
	   TMultiColor MC;

	   MC.Rgb=ColorToRGB(Area->Color);
	   if (Area->Selected)
	   {
		  glLineWidth(4.0);
		  glPushAttrib (GL_LINE_BIT);
		  glLineStipple (3, 0xAAAA);
	   }


	   glColor4f(MC.Red/255.0, MC.Green/255.0, MC.Blue/255.0, 1.0);
	   glBegin(GL_LINE_LOOP);
	   for (j = 0; j <Area->NumPoints; j++)
	   {
		LatLon2XY(Area->Points[j][1],Area->Points[j][0], ScrX, ScrY);
		glVertex2f(ScrX,ScrY);
	   }
	  glEnd();
	   if (Area->Selected)
	   {
		glPopAttrib ();
		glLineWidth(2.0);
	   }

	   glColor4f(MC.Red/255.0, MC.Green/255.0, MC.Blue/255.0, 0.4);

	   for (j = 0; j <Area->NumPoints; j++)
	   {
		LatLon2XY(Area->Points[j][1],Area->Points[j][0],
				  Area->PointsAdj[j][0],Area->PointsAdj[j][1]);
	   }
	  TTriangles *Tri=Area->Triangles;

	  while(Tri)
	  {
		glBegin(GL_TRIANGLES);
		glVertex2f(Area->PointsAdj[Tri->indexList[0]][0],
				   Area->PointsAdj[Tri->indexList[0]][1]);
		glVertex2f(Area->PointsAdj[Tri->indexList[1]][0],
				   Area->PointsAdj[Tri->indexList[1]][1]);
		glVertex2f(Area->PointsAdj[Tri->indexList[2]][0],
				   Area->PointsAdj[Tri->indexList[2]][1]);
		glEnd();
		Tri=Tri->next;
	  }
	 }

    AircraftCountLabel->Caption=IntToStr((int)ght_size(HashTable));
	for(Data = (TADS_B_Aircraft *)ght_first(HashTable, &iterator,(const void **) &Key);
			  Data; Data = (TADS_B_Aircraft *)ght_next(HashTable, &iterator, (const void **)&Key))
	{
	  if (Data->HaveLatLon)
	  {
		  ViewableAircraft++;
		  double aircraftX, aircraftY;
		  // calculate distance between aircraft and airport
		  bool isNearAirport = false;
		  if (airportManager) {
			double minDistance = 999999.0;

			// calculate current screen
			int screenWidth = ObjectDisplay->Width;
			int screenHeight = ObjectDisplay->Height;

			// Convert screen corner coordinates to latitude and longitude
			LatLon2XY(Data->Latitude, Data->Longitude, aircraftX, aircraftY);

			// outside display Airplane not Calculate 
			if (!(0 <= aircraftX && aircraftX < screenWidth && 0 <= aircraftY && aircraftY < screenHeight))
			{
				continue;
			}

			auto visibleAirports = airportManager->getVisibleAirports(
				0, 0, 0, 0, 0);

			double airportX, airportY;
			for (const auto& airport : visibleAirports) {
			  LatLon2XY(airport.latitude, airport.longitude, airportX, airportY);
			  // outside display Airport not Calculate 
			  if (!(0 <= airportX && airportX < screenWidth && 0 <= airportY && airportY < screenHeight))
			  {
				continue;
			  }

			  // Use Cached distance
			  double distance = getCachedDistance(Data->ICAO, airport.icao,
											   Data->Latitude, Data->Longitude,
											   airport.latitude, airport.longitude);
					
			  if (distance < minDistance) {
				minDistance = distance;
			  }
			}
			isNearAirport = (minDistance <= 5.0); // 5 nautical miles
		  }

			UpdateAircraftHistory(Data);

	   LatLon2XY(Data->Latitude,Data->Longitude, ScrX, ScrY);
	   
	   if(ScrX >=0 && ScrX <= ObjectDisplay->Width &&
        	ScrY >=0 && ScrY <= ObjectDisplay->Height) {

            int y = ScrY/cellHeight;
            int x = ScrX/cellWidth;
            if (x>=0 && x<10 && y>=0 && y<10) {
       			cell[y][x] += 1;
            }
        }
        else {
            continue;
        }
		
	   //DrawPoint(ScrX,ScrY);

      // Set aircraft color based on type (map-independent)
      if (aircraft_is_helicopter(Data->ICAO, NULL)) {
          glColor4f(1.0f, 0.65f, 0.0f, 1.0f);  // Orange for helicopters
      }
      else if (IsAircraftMilitary(Data->ICAO)) {
          glColor4f(0.0f, 1.0f, 0.0f, 1.0f);   // Fluorescent green for military
      }
      else {
          const TAircraftData *a = (TAircraftData *)ght_get(AircraftDBHashTable, sizeof(Data->ICAO), &Data->ICAO);
          if (!a) {
              glColor4f(0.0f, 0.75f, 1.0f, 1.0f);    //  for unknown
          }
          else {
              glColor4f(1.0, 0.0, 1.0, 1.0); //magenta for known civilian aircraft
          }
      }
	  if (airportManager && isNearAirport) {
      	glColor4f(1.0, 1.0, 0.0, 1.0); // yellow
      }

      // If aircraft has no speed/heading data, make it semi-transparent
      if (!Data->HaveSpeedAndHeading) {
          Data->Heading = 0.0;
          // Make unknown heading aircraft semi-transparent
          float currentColor[4];
          glGetFloatv(GL_CURRENT_COLOR, currentColor);
          glColor4f(currentColor[0], currentColor[1], currentColor[2], 0.6f);
      }


       if ( xf < cellDrawZoomRate) {
		   DrawAirplaneImage(ScrX,ScrY,0.8,Data->Heading,Data->SpriteImage);

	       //text color
		   if (DrawMap->Checked){
	         switch(SelectedMapIndex){
	           case 0: // GoogleMaps
		         glColor4f(0.92, 0.92, 0.96, 1.0);
	             break;
	           case 1:
	           case 2:
	           case 3:
	             glColor4f(0.0, 0.0, 0.0, 1.0);
	             break;
	           default:
	             glColor4f(0.92, 0.92, 0.96, 1.0);
	         }
	       }
		   else
		     glColor4f(0.0, 0.0, 0.0, 1.0);

		   glRasterPos2i(ScrX+15,ScrY-10);
		   ObjectDisplay->Draw2DText(Data->HexAddr);
       }

	   if ((Data->HaveSpeedAndHeading) && (TimeToGoCheckBox->State==cbChecked) && xf < cellDrawZoomRate)
	   {
		double lat,lon,az;
		if (VDirect(Data->Latitude,Data->Longitude,
					Data->Heading,Data->Speed/3060.0*TimeToGoTrackBar->Position ,&lat,&lon,&az)==OKNOERROR)
		  {
			 double ScrX2, ScrY2;
			 LatLon2XY(lat,lon, ScrX2, ScrY2);

	       if (DrawMap->Checked){
	       	glColor4f(1.0, 1.0, 0.0, 1.0);  //yellow
	       }
	       else{
	        glColor4f(1.0, 0.58, 0.0, 0.8);  //orange
	       }
			 glBegin(GL_LINE_STRIP);
			 glVertex2f(ScrX,ScrY);
			 glVertex2f(ScrX2,ScrY2);
			 glEnd();
		  }
	   }
	 }
	}
 ViewableAircraftCountLabel->Caption=ViewableAircraft;
 if (TrackHook.Valid_CC)
 {
		Data= (TADS_B_Aircraft *)ght_get(HashTable, sizeof(TrackHook.ICAO_CC), (void *)&TrackHook.ICAO_CC);
		if (Data)
		{
		a = (TAircraftData *)ght_get(AircraftDBHashTable,sizeof(Data->ICAO),&Data->ICAO);

		ICAOLabel->Caption=Data->HexAddr;
        if (Data->HaveFlightNum)
          FlightNumLabel->Caption=Data->FlightNum;
		else FlightNumLabel->Caption="N/A";
        if (Data->HaveLatLon)
		{
		 CLatLabel->Caption=DMS::DegreesMinutesSecondsLat(Data->Latitude).c_str();
		 CLonLabel->Caption=DMS::DegreesMinutesSecondsLon(Data->Longitude).c_str();
        }
        else
        {
         CLatLabel->Caption="N/A";
		 CLonLabel->Caption="N/A";
        }
        if (Data->HaveSpeedAndHeading)
        {
		 SpdLabel->Caption=FloatToStrF(Data->Speed, ffFixed,12,2)+" KTS  VRATE:"+FloatToStrF(Data->VerticalRate,ffFixed,12,2);
		 HdgLabel->Caption=FloatToStrF(Data->Heading, ffFixed,12,2)+" DEG";
        }
        else
        {
 		 SpdLabel->Caption="N/A";
		 HdgLabel->Caption="N/A";
        }
        if (Data->Altitude)
		 AltLabel->Caption= FloatToStrF(Data->Altitude, ffFixed,12,2)+" FT";
		else AltLabel->Caption="N/A";

		MsgCntLabel->Caption="Raw: "+IntToStr((int)Data->NumMessagesRaw)+" SBS: "+IntToStr((int)Data->NumMessagesSBS);
		TrkLastUpdateTimeLabel->Caption=TimeToChar(Data->LastSeen);

        glColor4f(1.0, 0.0, 0.0, 1.0);
        LatLon2XY(Data->Latitude,Data->Longitude, ScrX, ScrY);
        DrawTrackHook(ScrX, ScrY);

		// Display Tracking history
		if (Data && Data->HistoryCount > 0 && Data->HistoryIndex >= 0 && Data->HistoryIndex < FLIGHT_TRACK_HISTORY_COUNT)
		{
			//printf("[Data] %s HistoryCount=%d HistoryIndex=%d\n", Data->HexAddr, Data->HistoryCount, Data->HistoryIndex);
			glBegin(GL_LINE_STRIP);
			for (int i = 0; i < Data->HistoryCount && i < FLIGHT_TRACK_HISTORY_COUNT; i++)
			{
				int idx = (Data->HistoryIndex - i - 1 + FLIGHT_TRACK_HISTORY_COUNT) % FLIGHT_TRACK_HISTORY_COUNT;
				
				// Error Handling
				if (idx < 0 || idx >= FLIGHT_TRACK_HISTORY_COUNT) {
					continue;
				}

				// Error Handling - -90 < LAT < 90, -180 < LON < 180
				if (fabs(Data->PrevLatitude[idx]) > 90.0 || fabs(Data->PrevLongitude[idx]) > 180.0) {
					//printf("[NG] Invalid history: %s idx=%d lat=%.6f lon=%.6f\n",	Data->HexAddr, idx, Data->PrevLatitude[idx], Data->PrevLongitude[idx]);
					continue;
				}

				// Error Handling - 0.000000
				if (fabs(Data->PrevLatitude[idx]) < 0.01 || fabs(Data->PrevLongitude[idx]) < 0.01){
					//printf("[NG] Invalid history: 0.000000 %s idx=%d %f %f\n", Data->HexAddr, idx, Data->PrevLatitude[idx], Data->PrevLongitude[idx]);
					continue;
				}

				// Error Handling - Check difference with previous value
				if (i > 0) {
					int prevIdx = (Data->HistoryIndex - i + FLIGHT_TRACK_HISTORY_COUNT) % FLIGHT_TRACK_HISTORY_COUNT;
					double latDiff = fabs(Data->PrevLatitude[idx] - Data->PrevLatitude[prevIdx]);
					double lonDiff = fabs(Data->PrevLongitude[idx] - Data->PrevLongitude[prevIdx]);
					if (latDiff > 0.8 || lonDiff > 0.8) {
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
		TrackHook.Valid_CC=false;
		ICAOLabel->Caption="N/A";
		FlightNumLabel->Caption="N/A";
		CLatLabel->Caption="N/A";
		CLonLabel->Caption="N/A";
		SpdLabel->Caption="N/A";
		HdgLabel->Caption="N/A";
		AltLabel->Caption="N/A";
		MsgCntLabel->Caption="N/A";
		TrkLastUpdateTimeLabel->Caption="N/A";
	}
 }
 if (TrackHook.Valid_CPA)
 {
  bool CpaDataIsValid=false;
  DataCPA= (TADS_B_Aircraft *)ght_get(HashTable, sizeof(TrackHook.ICAO_CPA), (void *)&TrackHook.ICAO_CPA);
  if ((DataCPA) && (TrackHook.Valid_CC))
	{

	  double tcpa,cpa_distance_nm, vertical_cpa;
	  double lat1, lon1,lat2, lon2, junk;
	  if (computeCPA(Data->Latitude, Data->Longitude, Data->Altitude,
					 Data->Speed,Data->Heading,
					 DataCPA->Latitude, DataCPA->Longitude, DataCPA->Altitude,
					 DataCPA->Speed,DataCPA->Heading,
					 tcpa,cpa_distance_nm, vertical_cpa))
	  {
		if (VDirect(Data->Latitude,Data->Longitude,
					Data->Heading,Data->Speed/3600.0*tcpa,&lat1,&lon1,&junk)==OKNOERROR)
		{
		  if (VDirect(DataCPA->Latitude,DataCPA->Longitude,
					  DataCPA->Heading,DataCPA->Speed/3600.0*tcpa,&lat2,&lon2,&junk)==OKNOERROR)
		   {
			 glColor4f(0.0, 1.0, 0.0, 1.0);
			 glBegin(GL_LINE_STRIP);
			 LatLon2XY(Data->Latitude,Data->Longitude, ScrX, ScrY);
			 glVertex2f(ScrX, ScrY);
			 LatLon2XY(lat1,lon1, ScrX, ScrY);
			 glVertex2f(ScrX, ScrY);
			 glEnd();
			 glBegin(GL_LINE_STRIP);
			 LatLon2XY(DataCPA->Latitude,DataCPA->Longitude, ScrX, ScrY);
			 glVertex2f(ScrX, ScrY);
			 LatLon2XY(lat2,lon2, ScrX, ScrY);
			 glVertex2f(ScrX, ScrY);
			 glEnd();
			 glColor4f(1.0, 0.0, 0.0, 1.0);
			 glBegin(GL_LINE_STRIP);
			 LatLon2XY(lat1,lon1, ScrX, ScrY);
			 glVertex2f(ScrX, ScrY);
			 LatLon2XY(lat2,lon2, ScrX, ScrY);
			 glVertex2f(ScrX, ScrY);
			 glEnd();
			 CpaTimeValue->Caption=TimeToChar(tcpa*1000);
			 CpaDistanceValue->Caption= FloatToStrF(cpa_distance_nm, ffFixed,10,2)+" NM VDIST: "+IntToStr((int)vertical_cpa)+" FT";
			 CpaDataIsValid=true;
		   }
		}
	  }
	}
   if (!CpaDataIsValid)
   {
	TrackHook.Valid_CPA=false;
	CpaTimeValue->Caption="None";
	CpaDistanceValue->Caption="None";
   }
 }

 if (a != NULL && a->airport_size > 0) {
	for (i = 0; i < a->airport_size; i++) {
		DrawAirportIcon(a->airport_lat[i], a->airport_lon[i], (i == 0) ? true : false);
		DrawAirportInfo(a->airport_lat[i], a->airport_lon[i], a->airport_iata[i].c_str(), (i == 0) ? true : false);
	}

	// Draw connecting line between airports if we have both departure and arrival
	if (a->airport_size >= 2) {
		for (i = 0; i <= a->airport_size - 2; i++) {
			double ScrX1, ScrY1, ScrX2, ScrY2;
			LatLon2XY(a->airport_lat[i], a->airport_lon[i], ScrX1, ScrY1);
			LatLon2XY(a->airport_lat[i+1], a->airport_lon[i+1], ScrX2, ScrY2);

			// Draw red line connecting airports
			glColor4f(1.0, 0.0, 0.0, 1.0);  // Red color
			glLineWidth(4.0);  // Make the line thicker
			glBegin(GL_LINES);
			glVertex2f(ScrX1, ScrY1);
			glVertex2f(ScrX2, ScrY2);
			glEnd();	
		}
		
	}
 }
 
	// Draw Cells(white bubbles) instead of whole aircrafts for the performance and usability, 
	// when zoomRate(xf) >= cellDrawZoomRate
    int s = 0;
 	if (xf >= cellDrawZoomRate) {
        for (j = 0; j < 10; j++) {
            for (i=0; i < 10; i++) {

                if (cell[j][i] > 0) {

                    s = cell[j][i];

                    if(cell[j][i] < cellMin ) {
                    	s = cellMin;
                    } else if (cell[j][i] > cellMax) {
                        s = cellMax;
                    }

                    DrawCircleWithNumber(
                    	(float)(cellWidth*(i+1)-cellWidth/2),
                        (float)(cellHeight*(j+1)-cellHeight/2),
                        s,
                        cell[j][i]
                    );
                }
            }
        }
    }
}

int __fastcall TForm1::getAirplaneType(uint32_t addr)
{
	int rtn = 0;
	if (aircraft_is_helicopter(addr, NULL)) {
		rtn = 46;  // Orange for helicopters
	}
	else if (IsAircraftMilitary(addr)) {
		rtn = 7;   // Fluorescent green for military
	}
	else{
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
    for(int i = 0; i <= 360; i += 10) {
        float angle = i * M_PI / 180.0f;
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
    glRasterPos2i(textX,textY);
	ObjectDisplay->Draw2DText(numStr);
}

//---------------------------------------------------------------------------
void __fastcall TForm1::ObjectDisplayMouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y)
{
	if (Button==mbLeft)
	{
		if (Shift.Contains(ssCtrl))
		{

		}
		else
		{
			g_MouseLeftDownX = X;
			g_MouseLeftDownY = Y;
			g_MouseDownMask |= LEFT_MOUSE_DOWN ;
			g_EarthView->StartDrag(X, Y, NAV_DRAG_PAN);
		}
	}
	else if (Button==mbRight)
	{
		if (AreaTemp)
		{
			if (AreaTemp->NumPoints<MAX_AREA_POINTS)
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
				HookTrack(X,Y,true);
			}
			else
			{
				HookTrack(X,Y,false);
			}
		}
	}
 	else if (Button==mbMiddle)
	{
		ResetXYOffset();
	}
}
//---------------------------------------------------------------------------
void __fastcall TForm1::UpdateAircraftHistory(TADS_B_Aircraft *aircraft)
{
    if (!aircraft->HaveLatLon)
        return;

    // Check index range
    if (aircraft->HistoryIndex < 0 || aircraft->HistoryIndex >= FLIGHT_TRACK_HISTORY_COUNT) {
        aircraft->HistoryIndex = 0;
        aircraft->HistoryCount = 0;
    }

    // Compare previous loc and current loc
    bool shouldUpdate = true;
    if (aircraft->HistoryCount > 0) {
        int prevIdx = (aircraft->HistoryIndex - 1 + FLIGHT_TRACK_HISTORY_COUNT) % FLIGHT_TRACK_HISTORY_COUNT;
        // Same data, not save
        if (aircraft->PrevLatitude[prevIdx] == aircraft->Latitude &&
            aircraft->PrevLongitude[prevIdx] == aircraft->Longitude) {
            shouldUpdate = false;
        }
    }

    if (shouldUpdate) {
        int idx = aircraft->HistoryIndex;

        // Save current position
        aircraft->PrevLatitude[idx] = aircraft->Latitude;
        aircraft->PrevLongitude[idx] = aircraft->Longitude;
        aircraft->PrevAltitude[idx] = aircraft->Altitude;
        aircraft->PrevTimestamp[idx] = aircraft->LastSeen;

        // Circular buffer index update
        aircraft->HistoryIndex = (aircraft->HistoryIndex + 1) % FLIGHT_TRACK_HISTORY_COUNT;
        if (aircraft->HistoryCount < FLIGHT_TRACK_HISTORY_COUNT)
        {
            aircraft->HistoryCount++;
        }
    }
}

//---------------------------------------------------------------------------
void __fastcall TForm1::PurgeOldHistory(TADS_B_Aircraft *aircraft, __int64 currentTime)
{
	//printf("PurgeOldHistory\n");
	if (aircraft->HistoryCount == 0)
		return;

	int validCount = 0;
	int newIndex = 0;

	for (int i = 0; i < aircraft->HistoryCount; i++)
	{
		int idx = (aircraft->HistoryIndex - i - 1 + 100) % 100;
		if ((currentTime - aircraft->PrevTimestamp[idx]) <= 30000)
		{
			validCount++;
		}
		else
		{
			break;
		}
	}

	aircraft->HistoryCount = validCount;
}

//---------------------------------------------------------------------------

void __fastcall TForm1::ObjectDisplayMouseUp(TObject *Sender,
											 TMouseButton Button, TShiftState Shift, int X, int Y)
{
  if (Button == mbLeft) g_MouseDownMask &= ~LEFT_MOUSE_DOWN;
}
//---------------------------------------------------------------------------
void __fastcall TForm1::ObjectDisplayMouseMove(TObject *Sender, TShiftState Shift, int X, int Y)
{
 int X1,Y1;
 double VLat,VLon;
 int i;
 Y1=(ObjectDisplay->Height-1)-Y;
 X1=X;
 if  ((X1>=Map_v[0].x) && (X1<=Map_v[1].x) &&
	  (Y1>=Map_v[0].y) && (Y1<=Map_v[3].y))

  {
   pfVec3 Point;
   VLat=atan(sinh(M_PI * (2 * (Map_w[1].y-(yf*(Map_v[3].y-Y1))))))*(180.0 / M_PI);
   VLon=(Map_w[1].x-(xf*(Map_v[1].x-X1)))*360.0;
   Lat->Caption=DMS::DegreesMinutesSecondsLat(VLat).c_str();
   Lon->Caption=DMS::DegreesMinutesSecondsLon(VLon).c_str();
   Point[0]=VLon;
   Point[1]=VLat;
   Point[2]=0.0;

   for (i = 0; i < Areas->Count; i++)
	 {
	   TArea *Area = (TArea *)Areas->Items[i];
	   if (PointInPolygon(Area->Points,Area->NumPoints,Point))
	   {
#if 0
		  MsgLog->Lines->Add("In Polygon "+ Area->Name);
#endif
       }
	 }
  }

  if (g_MouseDownMask & LEFT_MOUSE_DOWN)
  {
   g_EarthView->Drag(g_MouseLeftDownX, g_MouseLeftDownY, X,Y, NAV_DRAG_PAN);
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
  double Lat,Lon;

 if (XY2LatLon2(X,Y,Lat,Lon)==0)
 {

	AreaTemp->Points[AreaTemp->NumPoints][1]=Lat;
	AreaTemp->Points[AreaTemp->NumPoints][0]=Lon;
	AreaTemp->Points[AreaTemp->NumPoints][2]=0.0;
	AreaTemp->NumPoints++;
	ObjectDisplay->Repaint();
 }
 }
//---------------------------------------------------------------------------
 void __fastcall TForm1::HookTrack(int X, int Y,bool CPA_Hook)
 {
  double VLat,VLon, dlat,dlon,Range;
  int X1,Y1;
   uint32_t *Key;

   uint32_t Current_ICAO;
   double MinRange;
  ght_iterator_t iterator;
  TADS_B_Aircraft* Data;

  Y1=(ObjectDisplay->Height-1)-Y;
  X1=X;

  if  ((X1<Map_v[0].x) || (X1>Map_v[1].x) ||
	   (Y1<Map_v[0].y) || (Y1>Map_v[3].y)) return;

  VLat=atan(sinh(M_PI * (2 * (Map_w[1].y-(yf*(Map_v[3].y-Y1))))))*(180.0 / M_PI);
  VLon=(Map_w[1].x-(xf*(Map_v[1].x-X1)))*360.0;

  MinRange=16.0;

  for(Data = (TADS_B_Aircraft *)ght_first(HashTable, &iterator,(const void **) &Key);
			  Data; Data = (TADS_B_Aircraft *)ght_next(HashTable, &iterator, (const void **)&Key))
	{
	  if (Data->HaveLatLon)
	  {
	   dlat= VLat-Data->Latitude;
	   dlon= VLon-Data->Longitude;
	   Range=sqrt(dlat*dlat+dlon*dlon);
	   if (Range<MinRange)
	   {
		Current_ICAO=Data->ICAO;
		MinRange=Range;
	   }
	  }
	}
	if (MinRange< 0.2)
	{
	  TADS_B_Aircraft * ADS_B_Aircraft =(TADS_B_Aircraft *)
			ght_get(HashTable,sizeof(Current_ICAO),
					&Current_ICAO);
	  if (ADS_B_Aircraft)
	  {
		if (!CPA_Hook)
		{
		 TrackHook.Valid_CC=true;
		 TrackHook.ICAO_CC=ADS_B_Aircraft->ICAO;
		 
		 // Get local aircraft info
		 const char* info = GetAircraftDBInfo(ADS_B_Aircraft->ICAO);

		 printf("info: %s\n", info);
		 if(ADS_B_Aircraft->HaveFlightNum) {
			bool isExist = false;
			const char* additionalInfo = GetAircraftAPIInfo(ADS_B_Aircraft->ICAO, ADS_B_Aircraft->FlightNum, &isExist);
			printf("additionalInfo: %s\n", additionalInfo);

			if (isExist) {
                 ObjectDisplay->Repaint();
			}
		 }
    	//Update aircraft information panel
      	UpdateAircraftInfo(ADS_B_Aircraft);
		}
		else
		{
		 TrackHook.Valid_CPA=true;
		 TrackHook.ICAO_CPA=ADS_B_Aircraft->ICAO;
        }
	  }

	}
	else
		{
		 if (!CPA_Hook)
		  {
		   TrackHook.Valid_CC=false;
           ICAOLabel->Caption="N/A";
		   FlightNumLabel->Caption="N/A";
		   CLatLabel->Caption="N/A";
		   CLonLabel->Caption="N/A";
		   SpdLabel->Caption="N/A";
		   HdgLabel->Caption="N/A";
		   AltLabel->Caption="N/A";
		   MsgCntLabel->Caption="N/A";
		   TrkLastUpdateTimeLabel->Caption="N/A";
		  }
		 else
		   {
			TrackHook.Valid_CPA=false;
			CpaTimeValue->Caption="None";
	        CpaDistanceValue->Caption="None";
           }
		}

 }
//---------------------------------------------------------------------------
void __fastcall TForm1::LatLon2XY(double lat,double lon, double &x, double &y)
{
 x=(Map_v[1].x-((Map_w[1].x-(lon/360.0))/xf));
 y= Map_v[3].y- (Map_w[1].y/yf)+ (asinh(tan(lat*M_PI/180.0))/(2*M_PI*yf));
}
//---------------------------------------------------------------------------
int __fastcall TForm1::XY2LatLon2(int x, int y,double &lat,double &lon )
{
  double Lat,Lon, dlat,dlon,Range;
  int X1,Y1;

  Y1=(ObjectDisplay->Height-1)-y;
  X1=x;

  if  ((X1<Map_v[0].x) || (X1>Map_v[1].x) ||
	   (Y1<Map_v[0].y) || (Y1>Map_v[3].y)) return -1;

  lat=atan(sinh(M_PI * (2 * (Map_w[1].y-(yf*(Map_v[3].y-Y1))))))*(180.0 / M_PI);
  lon=(Map_w[1].x-(xf*(Map_v[1].x-X1)))*360.0;

  return 0;
}
//---------------------------------------------------------------------------
void __fastcall TForm1::ZoomInClick(TObject *Sender)
{
  g_EarthView->SingleMovement(NAV_ZOOM_IN);
  ObjectDisplay->Repaint();
}
//---------------------------------------------------------------------------

void __fastcall TForm1::ZoomOutClick(TObject *Sender)
{
 g_EarthView->SingleMovement(NAV_ZOOM_OUT);

 ObjectDisplay->Repaint();
}
//---------------------------------------------------------------------------
void __fastcall TForm1::Purge(void)
{
  uint32_t *Key;
  ght_iterator_t iterator;
  TADS_B_Aircraft* Data;
  void *p;
  __int64 CurrentTime=GetCurrentTimeInMsec();
  __int64  StaleTimeInMs=CSpinStaleTime->Value*1000;

  if (PurgeStale->Checked==false) return;

  for(Data = (TADS_B_Aircraft *)ght_first(HashTable, &iterator,(const void **) &Key);
			  Data; Data = (TADS_B_Aircraft *)ght_next(HashTable, &iterator, (const void **)&Key))
	{
		if ((CurrentTime - Data->LastSeen) >= StaleTimeInMs)
		{
			p = ght_remove(HashTable, sizeof(*Key), Key);
			if (!p)
				ShowMessage("Removing the current iterated entry failed! This is a BUG\n");

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
  TADS_B_Aircraft* Data;
  void *p;

  for(Data = (TADS_B_Aircraft *)ght_first(HashTable, &iterator,(const void **) &Key);
			  Data; Data = (TADS_B_Aircraft *)ght_next(HashTable, &iterator, (const void **)&Key))
	{

	  p = ght_remove(HashTable,sizeof(*Key), Key);
	  if (!p)
		ShowMessage("Removing the current iterated entry failed! This is a BUG\n");

	  delete Data;

	}
}
//---------------------------------------------------------------------------
void __fastcall TForm1::InsertClick(TObject *Sender)
{
 Insert->Enabled=false;
 LoadARTCCBoundaries1->Enabled=false;
 Complete->Enabled=true;
 Cancel->Enabled=true;
 //Delete->Enabled=false;

 AreaTemp= new TArea;
 AreaTemp->NumPoints=0;
 AreaTemp->Name="";
 AreaTemp->Selected=false;
 AreaTemp->Triangles=NULL;

}
//---------------------------------------------------------------------------
void __fastcall TForm1::CancelClick(TObject *Sender)
{
 TArea *Temp;
 Temp= AreaTemp;
 AreaTemp=NULL;
 delete  Temp;
 Insert->Enabled=true;
 Complete->Enabled=false;
 Cancel->Enabled=false;
 LoadARTCCBoundaries1->Enabled=true;
 //if (Areas->Count>0)  Delete->Enabled=true;
 //else   Delete->Enabled=false;

}
//---------------------------------------------------------------------------
void __fastcall TForm1::CompleteClick(TObject *Sender)
{

  int or1=orientation2D_Polygon( AreaTemp->Points,AreaTemp->NumPoints);
  if (or1==0)
   {
	ShowMessage("Degenerate Polygon");
    CancelClick(NULL);
	return;
   }
  if (or1==CLOCKWISE)
  {
	DWORD i;

	memcpy(AreaTemp->PointsAdj,AreaTemp->Points,sizeof(AreaTemp->Points));
	for (i = 0; i <AreaTemp->NumPoints; i++)
	 {
	   memcpy(AreaTemp->Points[i],
			 AreaTemp->PointsAdj[AreaTemp->NumPoints-1-i],sizeof( pfVec3));
	 }
  }
  if (checkComplex( AreaTemp->Points,AreaTemp->NumPoints))
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
   DWORD Count;
   TArea *AreaS=(TArea *)Item->Data;
   bool HaveSelected=false;
	Count=Areas->Count;
	for (unsigned int i = 0; i < Count; i++)
	 {
	   TArea *Area = (TArea *)Areas->Items[i];
	   if (Area==AreaS)
	   {
		if (Item->Selected)
		{
		 Area->Selected=true;
		 HaveSelected=true;
		}
		else
		 Area->Selected=false;
	   }
	   else
		 Area->Selected=false;

	 }
	if (HaveSelected)  Delete->Enabled=true;
	else Delete->Enabled=false;
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

	 Area=(TArea *)AreaListView->Items->Item[i]->Data;
	 for (Index = 0; Index < Areas->Count; Index++)
	 {
	  if (Area==Areas->Items[Index])
	  {
	   Areas->Delete(Index);
	   AreaListView->Items->Item[i]->Delete();
	   TTriangles *Tri=Area->Triangles;
	   while(Tri)
	   {
		TTriangles *temp=Tri;
		Tri=Tri->next;
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
 //if (Areas->Count>0)  Delete->Enabled=true;
 //else   Delete->Enabled=false;

 ObjectDisplay->Repaint();
}
//---------------------------------------------------------------------------
void __fastcall TForm1::AreaListViewCustomDrawItem(TCustomListView *Sender,
	  TListItem *Item, TCustomDrawState State, bool &DefaultDraw)
{
   TRect   R;
   int Left;
  AreaListView->Canvas->Brush->Color = AreaListView->Color;
  AreaListView->Canvas->Font->Color = AreaListView->Font->Color;
  R=Item->DisplayRect(drBounds);
  AreaListView->Canvas->FillRect(R);

  AreaListView->Canvas->TextWidth(Item->Caption);

 AreaListView->Canvas->TextOut(2, R.Top, Item->Caption );

 Left = AreaListView->Column[0]->Width;

  for(int   i=0   ;i<Item->SubItems->Count;i++)
	 {
	  R=Item->DisplayRect(drBounds);
	  R.Left=R.Left+Left;
	   TArea *Area=(TArea *)Item->Data;
	  AreaListView->Canvas->Brush->Color=Area->Color;
	  AreaListView->Canvas->FillRect(R);
	 }

  if (Item->Selected)
	 {
	  R=Item->DisplayRect(drBounds);
	  AreaListView->Canvas->DrawFocusRect(R);
	 }
   DefaultDraw=false;
}
//---------------------------------------------------------------------------
void __fastcall TForm1::DeleteAllAreas(void)
{
 int i = 0;

 while (AreaListView->Items->Count)
  {

	 TArea *Area;
	 int Index;

	 Area=(TArea *)AreaListView->Items->Item[i]->Data;
	 for (Index = 0; Index < Areas->Count; Index++)
	 {
	  if (Area==Areas->Items[Index])
	  {
	   Areas->Delete(Index);
	   AreaListView->Items->Item[i]->Delete();
	   TTriangles *Tri=Area->Triangles;
	   while(Tri)
	   {
		TTriangles *temp=Tri;
		Tri=Tri->next;
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
 if (WheelDelta>0)
	  g_EarthView->SingleMovement(NAV_ZOOM_IN);
 else g_EarthView->SingleMovement(NAV_ZOOM_OUT);
  ObjectDisplay->Repaint();
}                                  
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
void __fastcall TTCPClientRawHandleThread::HandleInput(void)
{
  modeS_message mm;
  TDecodeStatus Status;
  __int64 CurrentTime;
  CurrentTime=GetCurrentTimeInMsec();

 // Form1->MsgLog->Lines->Add(StringMsgBuffer);
 
  if (Form1->RecordRawStream)
  {
   //__int64 CurrentTime;
   //CurrentTime=GetCurrentTimeInMsec();
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
			if (!ADS_B_Aircraft) {
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
			memset(ADS_B_Aircraft->PrevLatitude, 0, sizeof(ADS_B_Aircraft->PrevLatitude));
			memset(ADS_B_Aircraft->PrevLongitude, 0, sizeof(ADS_B_Aircraft->PrevLongitude));
			memset(ADS_B_Aircraft->PrevAltitude, 0, sizeof(ADS_B_Aircraft->PrevAltitude));
			memset(ADS_B_Aircraft->PrevTimestamp, 0, sizeof(ADS_B_Aircraft->PrevTimestamp));

			if (Form1->CycleImages->Checked)
				Form1->CurrentSpriteImage = (Form1->CurrentSpriteImage + 1) % Form1->NumSpriteImages;
			if (ght_insert(Form1->HashTable, ADS_B_Aircraft, sizeof(addr), &addr) < 0)
			{
				printf("ght_insert Error - Should Not Happen\n");
			}
		}

	  RawToAircraft(&mm,ADS_B_Aircraft);
	  LastHeartbeatTime = GetCurrentTimeInMsec();
	  RawTimeoutPopupShown = false;
  }
  else if (Status == MsgHeartBeat) {
	  LastHeartbeatTime = GetCurrentTimeInMsec();
	  RawTimeoutPopupShown = false;
  }
  else
  {
	 //Not User Scene
	 //ShowMessage("Error while connecting: E%.2d"+Status);
  }
  
  printf("[%lld]PI Raw Decode code:%d\n",CurrentTime, Status);

}
//---------------------------------------------------------------------------
void __fastcall TForm1::RawConnectButtonClick(TObject *Sender)
{
 if ((RawConnectButton->Caption=="Raw Connect") && (Sender!=NULL))
 {
  // Disable button to prevent multiple clicks
  RawConnectButton->Enabled = false;
  RawConnectButton->Caption = "Connecting...";
  
  // Start connection in separate thread to keep UI responsive
  TConnectionThread* connectionThread = new TConnectionThread(RawIpAddress->Text, 30002, false);
  connectionThread->Resume();
 }
 else
  {
	TCPClientRawHandleThread->Terminate();
	IdTCPClientRaw->Disconnect();
	IdTCPClientRaw->IOHandler->InputBuffer->Clear();
	RawConnectButton->Caption="Raw Connect";
	RawPlaybackButton->Enabled=true;
  }
 }
//---------------------------------------------------------------------------
void __fastcall TForm1::IdTCPClientRawConnected(TObject *Sender)
{
   //SetKeepAliveValues(const AEnabled: Boolean; const ATimeMS, AInterval: Integer);
   IdTCPClientRaw->Socket->Binding->SetKeepAliveValues(true,60*1000,15*1000);
   RawConnectButton->Caption="Raw Disconnect";
   RawPlaybackButton->Enabled=false;
   RawTimeoutPopupShown = false;
   LastHeartbeatTime = GetCurrentTimeInMsec();
}
//---------------------------------------------------------------------------
void __fastcall TForm1::IdTCPClientRawDisconnected(TObject *Sender)
{
  TCPClientRawHandleThread->Terminate();
  RawTimeoutPopupShown = true;
}
//---------------------------------------------------------------------------
void __fastcall TForm1::RawRecordButtonClick(TObject *Sender)
{
 if (RawRecordButton->Caption=="Raw Record")
 {
  if (RecordRawSaveDialog->Execute())
   {
	// First, check if the file exists.
	if (FileExists(RecordRawSaveDialog->FileName))
	  ShowMessage("File "+RecordRawSaveDialog->FileName+"already exists. Cannot overwrite.");
	else
	{
		// Open a file for writing. Creates the file if it doesn't exist, or overwrites it if it does.
	RecordRawStream= new TStreamWriter(RecordRawSaveDialog->FileName, false);
	if (RecordRawStream==NULL)
	  {
		ShowMessage("Cannot Open File "+RecordRawSaveDialog->FileName);
	  }
	 else RawRecordButton->Caption="Stop Raw Recording";
	}
  }
 }
 else
 {
   delete RecordRawStream;
   RecordRawStream=NULL;
   RawRecordButton->Caption="Raw Record";
 }
}
//---------------------------------------------------------------------------
void __fastcall TForm1::RawPlaybackButtonClick(TObject *Sender)
{
  if ((RawPlaybackButton->Caption=="Raw Playback") && (Sender!=NULL))
 {
  if (PlaybackRawDialog->Execute())
   {
	// First, check if the file exists.
	if (!FileExists(PlaybackRawDialog->FileName))
	  ShowMessage("File "+PlaybackRawDialog->FileName+" does not exist");
	else
	{
		// Open a file for writing. Creates the file if it doesn't exist, or overwrites it if it does.
	PlayBackRawStream= new TStreamReader(PlaybackRawDialog->FileName);
	if (PlayBackRawStream==NULL)
	  {
		ShowMessage("Cannot Open File "+PlaybackRawDialog->FileName);
	  }
	 else {
		   TCPClientRawHandleThread = new TTCPClientRawHandleThread(true);
		   TCPClientRawHandleThread->UseFileInsteadOfNetwork=true;
		   TCPClientRawHandleThread->First=true;
		   TCPClientRawHandleThread->FreeOnTerminate=TRUE;
		   TCPClientRawHandleThread->Resume();
		   RawPlaybackButton->Caption="Stop Raw Playback";
           RawConnectButton->Enabled=false;
		  }
	}
  }
 }
 else
 {
   TCPClientRawHandleThread->Terminate();
   delete PlayBackRawStream;
   PlayBackRawStream=NULL;
   RawPlaybackButton->Caption="Raw Playback";
   RawConnectButton->Enabled=true;
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
  __int64 Time,SleepTime;
  while (!Terminated)
  {
	if (!UseFileInsteadOfNetwork)
	 {
	  try {
		   if (!Form1->IdTCPClientRaw->Connected()) {
			   Terminate();
			   break;
		   }
		   
		   // Check if data is available before reading
		   if (Form1->IdTCPClientRaw->IOHandler->InputBuffer->Size > 0) {
			   StringMsgBuffer = Form1->IdTCPClientRaw->IOHandler->ReadLn();
		   } else {
			   // No data available, sleep briefly to prevent busy waiting
			   Sleep(10);
			   continue;
		   }
		  }
       catch (const EIdReadTimeout& e)
		{
		 // Handle read timeout specifically
		 printf("Raw Read timeout: %s\n", AnsiString(e.Message).c_str());
		 TThread::Synchronize(StopTCPClient);
		 break;
		}
       catch (const EIdException& e)
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
		 StringMsgBuffer= Form1->PlayBackRawStream->ReadLine();
         Time=StrToInt64(StringMsgBuffer);
		 if (First)
	      {
		   First=false;
		   LastTime=Time;
		  }
		 SleepTime=Time-LastTime;
		 LastTime=Time;
		 if (SleepTime>0) Sleep(SleepTime);
         if (Form1->PlayBackRawStream->EndOfStream)
           {
            printf("End Raw Playback 2\n");
            TThread::Synchronize(StopPlayback);
            break;
           }
		 StringMsgBuffer= Form1->PlayBackRawStream->ReadLine();
		}
        catch (...)
		{
         printf("Raw Playback Exception\n");
		 TThread::Synchronize(StopPlayback);
		 break;
		}
	   }
	   
	 // Only process if we have data
	 if (StringMsgBuffer.Length() > 0) {
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
 CurrentSpriteImage=0;
}
//---------------------------------------------------------------------------
void __fastcall TForm1::SBSConnectButtonClick(TObject *Sender)
{
 if ((SBSConnectButton->Caption=="SBS Connect") && (Sender!=NULL))
 {
  // Disable button to prevent multiple clicks
  SBSConnectButton->Enabled = false;
  SBSConnectButton->Caption = "Connecting...";
  
  // Start connection in separate thread to keep UI responsive
  TConnectionThread* connectionThread = new TConnectionThread(SBSIpAddress->Text, 5002, true);
  connectionThread->Resume();
 }
 else
  {
	TCPClientSBSHandleThread->Terminate();
	IdTCPClientSBS->Disconnect();
    IdTCPClientSBS->IOHandler->InputBuffer->Clear();
	SBSConnectButton->Caption="SBS Connect";
	SBSPlaybackButton->Enabled=true;
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
   CurrentTime=GetCurrentTimeInMsec();
   Form1->RecordSBSStream->WriteLine(IntToStr(CurrentTime));
   Form1->RecordSBSStream->WriteLine(StringMsgBuffer);
  }

  if (Form1->BigQueryCSV)
  {
    Form1->BigQueryCSV->WriteLine(StringMsgBuffer);
    Form1->BigQueryRowCount++;
	if (Form1->BigQueryRowCount>=BIG_QUERY_UPLOAD_COUNT)
	{
	 Form1->CloseBigQueryCSV();
	 //printf("string is:%s\n", Form1->BigQueryPythonScript.c_str());
	 RunPythonScript(Form1->BigQueryPythonScript,Form1->BigQueryPath+" "+Form1->BigQueryCSVFileName);
	 Form1->CreateBigQueryCSV();
	}
  }

	// Detect SBS Message Timeout
  if (StringMsgBuffer.Length() > 0) {
	  LastSBSDataReceiveTime = GetCurrentTimeInMsec();
	  SBSTimeoutPopupShown = false;
  }
  
  // Process SBS message - this should be fast and not block
  try {
    SBS_Message_Decode( StringMsgBuffer.c_str());
  } catch (...) {
    // Log error but don't crash the thread
    printf("Error in SBS_Message_Decode\n");
  }
}
//---------------------------------------------------------------------------
// Constructor for the thread class
__fastcall TTCPClientSBSHandleThread::TTCPClientSBSHandleThread(bool value) : TThread(value)
{
	FreeOnTerminate = true; // Automatically free the thread object after execution
}
//---------------------------------------------------------------------------
// Destructor for the thread class
__fastcall TTCPClientSBSHandleThread::~TTCPClientSBSHandleThread()
{
	// Clean up resources if needed
}
//---------------------------------------------------------------------------
// Execute method where the thread's logic resides
void __fastcall TTCPClientSBSHandleThread::Execute(void)
{
	__int64 Time,SleepTime;
	while (!Terminated)
	{
		if (!UseFileInsteadOfNetwork)
		{
			try 
			{
				if (!Form1->IdTCPClientSBS->Connected()) {
					Terminate();
					break;
				}
				
				// Check if data is available before reading
				if (Form1->IdTCPClientSBS->IOHandler->InputBuffer->Size > 0) {
					StringMsgBuffer = Form1->IdTCPClientSBS->IOHandler->ReadLn();
				} else {
					// No data available, sleep briefly to prevent busy waiting
					Sleep(10);
					continue;
				}
			}
			catch (const EIdReadTimeout& e)
			{
				// Handle read timeout specifically
				printf("SBS Read timeout: %s\n", AnsiString(e.Message).c_str());
				TThread::Synchronize(StopTCPClient);
				break;
			}
			catch (const EIdException& e)
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
					printf("End SBS Playback 1\n");
					TThread::Synchronize(StopPlayback);
					break;
				}
				StringMsgBuffer = Form1->PlayBackSBSStream->ReadLine();
				Time = StrToInt64(StringMsgBuffer);
				if (First)
				{
					First=false;
					LastTime=Time;
				}
				int SpeedFactor = globalTrackbarValue;
				//printf("SpeedFactor: %d\n", SpeedFactor);
				if (SpeedFactor < 1) SpeedFactor = 1;
				SleepTime = (Time - LastTime) / SpeedFactor;
				LastTime=Time;
				if (SleepTime>0) {
					Sleep(SleepTime);
				}
				if (Form1->PlayBackSBSStream->EndOfStream)
				{
					printf("End SBS Playback 2\n");
					TThread::Synchronize(StopPlayback);
					break;
				}
				StringMsgBuffer= Form1->PlayBackSBSStream->ReadLine();
			}
			catch (...)
			{
				printf("SBS Playback Exception\n");
				TThread::Synchronize(StopPlayback);
				break;
			}
		}
		
		// Only process if we have data
		if (StringMsgBuffer.Length() > 0) {
			try
			{
				// Synchronize method to safely access UI components
				TThread::Synchronize(HandleInput);
			}
			catch (...)
			{
				ShowMessage("TTCPClientSBSHandleThread::Execute Exception 3");
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
 if (SBSRecordButton->Caption=="SBS Record")
 {
  if (RecordSBSSaveDialog->Execute())
   {
	// First, check if the file exists.
	if (FileExists(RecordSBSSaveDialog->FileName))
	  ShowMessage("File "+RecordSBSSaveDialog->FileName+"already exists. Cannot overwrite.");
	else
	{
		// Open a file for writing. Creates the file if it doesn't exist, or overwrites it if it does.
	RecordSBSStream= new TStreamWriter(RecordSBSSaveDialog->FileName, false);
	if (RecordSBSStream==NULL)
	  {
		ShowMessage("Cannot Open File "+RecordSBSSaveDialog->FileName);
	  }
	 else SBSRecordButton->Caption="Stop SBS Recording";
	}
  }
 }
 else
 {
   delete RecordSBSStream;
   RecordSBSStream=NULL;
   SBSRecordButton->Caption="SBS Record";
 }

}
//---------------------------------------------------------------------------
void __fastcall TForm1::SBSPlaybackButtonClick(TObject *Sender)
{
  if ((SBSPlaybackButton->Caption=="SBS Playback") && (Sender!=NULL))
 {
  if (PlaybackSBSDialog->Execute())
   {
	// First, check if the file exists.
	if (!FileExists(PlaybackSBSDialog->FileName))
	  ShowMessage("File "+PlaybackSBSDialog->FileName+" does not exist");
	else
	{
		// Open a file for writing. Creates the file if it doesn't exist, or overwrites it if it does.
	PlayBackSBSStream= new TStreamReader(PlaybackSBSDialog->FileName);
	if (PlayBackSBSStream==NULL)
	  {
		ShowMessage("Cannot Open File "+PlaybackSBSDialog->FileName);
	  }
	 else {
		   TCPClientSBSHandleThread = new TTCPClientSBSHandleThread(true);
		   TCPClientSBSHandleThread->UseFileInsteadOfNetwork=true;
		   TCPClientSBSHandleThread->First=true;
		   TCPClientSBSHandleThread->FreeOnTerminate=TRUE;
		   TCPClientSBSHandleThread->Resume();
		   SBSPlaybackButton->Caption="Stop SBS Playback";
           SBSConnectButton->Enabled=false;
 		   PlaybackSpeedTrackBar->Visible = true;
		  }
	}
  }
 }
 else
 {
   TCPClientSBSHandleThread->Terminate();
   delete PlayBackSBSStream;
   PlayBackSBSStream=NULL;
   SBSPlaybackButton->Caption="SBS Playback";
   SBSConnectButton->Enabled=true;
   PlaybackSpeedTrackBar->Visible = false;
 }

}
//---------------------------------------------------------------------------

void __fastcall TForm1::IdTCPClientSBSConnected(TObject *Sender)
{
   //SetKeepAliveValues(const AEnabled: Boolean; const ATimeMS, AInterval: Integer);
   IdTCPClientSBS->Socket->Binding->SetKeepAliveValues(true,60*1000,15*1000);
   SBSConnectButton->Caption="SBS Disconnect";
   SBSPlaybackButton->Enabled=false;
   SBSTimeoutPopupShown = false;
   LastSBSDataReceiveTime = GetCurrentTimeInMsec();
}
//---------------------------------------------------------------------------
void __fastcall TForm1::IdTCPClientSBSDisconnected(TObject *Sender)
{
  TCPClientSBSHandleThread->Terminate();
  SBSTimeoutPopupShown = true;
}
//---------------------------------------------------------------------------

void __fastcall TForm1::TimeToGoTrackBarChange(TObject *Sender)
{
  _int64 hmsm;
  hmsm=TimeToGoTrackBar->Position*1000;
  TimeToGoText->Caption=TimeToChar(hmsm);
}
//---------------------------------------------------------------------------
void __fastcall TForm1::LoadMap(int Type)
{
   info("%s: Loading map type %d\n", __func__, Type);
   AnsiString  HomeDir = ExtractFilePath(ExtractFileDir(Application->ExeName));
    if (Type==GoogleMaps)
   {
     HomeDir+= "..\\GoogleMap";
     if (LoadMapFromInternet) HomeDir+= "_Live\\";
     else  HomeDir+= "\\";
     std::string cachedir;
     cachedir=HomeDir.c_str();

     if (mkdir(cachedir.c_str()) != 0 && errno != EEXIST)
	    throw Sysutils::Exception("Can not create cache directory");

     g_Storage = new FilesystemStorage(cachedir,true);
     if (LoadMapFromInternet)
       {
	    g_Keyhole = new KeyholeConnection(GoogleMaps);
        g_Keyhole->SetSaveStorage(g_Storage);
	    g_Storage->SetNextLoadStorage(g_Keyhole);
	   }
    }
  else if (Type==SkyVector_VFR)
   {
     HomeDir+= "..\\VFR_Map";
     if (LoadMapFromInternet) HomeDir+= "_Live\\";
     else  HomeDir+= "\\";
     std::string cachedir;
     cachedir=HomeDir.c_str();

     if (mkdir(cachedir.c_str()) != 0 && errno != EEXIST)
	    throw Sysutils::Exception("Can not create cache directory");

     g_Storage = new FilesystemStorage(cachedir,true);
     if (LoadMapFromInternet)
       {
	    g_Keyhole = new KeyholeConnection(SkyVector_VFR);
        g_Keyhole->SetSaveStorage(g_Storage);
	    g_Storage->SetNextLoadStorage(g_Keyhole);
	   }
    }
  else if (Type==SkyVector_IFR_Low)
   {
     HomeDir+= "..\\IFR_Low_Map";
     if (LoadMapFromInternet) HomeDir+= "_Live\\";
     else  HomeDir+= "\\";
     std::string cachedir;
     cachedir=HomeDir.c_str();

     if (mkdir(cachedir.c_str()) != 0 && errno != EEXIST)
	    throw Sysutils::Exception("Can not create cache directory");

     g_Storage = new FilesystemStorage(cachedir,true);
     if (LoadMapFromInternet)
       {
	    g_Keyhole = new KeyholeConnection(SkyVector_IFR_Low);
        g_Keyhole->SetSaveStorage(g_Storage);
	    g_Storage->SetNextLoadStorage(g_Keyhole);
	   }
    }
  else if (Type==SkyVector_IFR_High)
   {
     HomeDir+= "..\\IFR_High_Map";
     if (LoadMapFromInternet) HomeDir+= "_Live\\";
     else  HomeDir+= "\\";
     std::string cachedir;
     cachedir=HomeDir.c_str();

     if (mkdir(cachedir.c_str()) != 0 && errno != EEXIST)
	    throw Sysutils::Exception("Can not create cache directory");

     g_Storage = new FilesystemStorage(cachedir,true);
     if (LoadMapFromInternet)
       {
	    g_Keyhole = new KeyholeConnection(SkyVector_IFR_High);
        g_Keyhole->SetSaveStorage(g_Storage);
	    g_Storage->SetNextLoadStorage(g_Keyhole);
	   }
    }
	else if (Type==OpenStreetMaps)
	{
	    HomeDir= "D:\\OpenstreetMap";
	    if (LoadMapFromInternet) HomeDir+= "_Live\\";
	    else  HomeDir+= "\\";
	    std::string cachedir;
	    cachedir=HomeDir.c_str();

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
   if (GlobalMemoryStatusEx(&memInfo)) {
       DWORDLONG totalPhysMB = memInfo.ullTotalPhys / (1024 * 1024);
       int maxTextures = 500; // default
       
       if (totalPhysMB >= 16384) {        // 16GB 
           maxTextures = 2000;
       } else if (totalPhysMB >= 8192) {  // 8GB 
           maxTextures = 1000;
       } else if (totalPhysMB >= 4096) {  // 4GB 
           maxTextures = 500;
       } else {                           // 4GB 
           maxTextures = 200;
       }
       
       g_GETileManager->SetMaxTextures(maxTextures);
       printf("System RAM: %lld MB, Max Textures: %d\n", totalPhysMB, maxTextures);
   }
   if (Type == OpenStreetMaps) {
       g_MasterLayer = new OsmLayer(g_GETileManager);
   } else {
       g_MasterLayer = new GoogleLayer(g_GETileManager);
   }

   g_EarthView = new FlatEarthView(g_MasterLayer);
   g_EarthView->Resize(ObjectDisplay->Width,ObjectDisplay->Height);
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
    double    m_Eyeh= g_EarthView->m_Eye.h;
    double    m_Eyex= g_EarthView->m_Eye.x;
    double    m_Eyey= g_EarthView->m_Eye.y;

    Timer1->Enabled=false;
    Timer2->Enabled=false;
    delete g_EarthView;
    if (g_GETileManager) delete g_GETileManager;
    delete g_MasterLayer;
    delete g_Storage;
    if (LoadMapFromInternet)
    {
     if (g_Keyhole) delete g_Keyhole;
    }

    // update map index that is really selected
    SelectedMapIndex = MapComboBox->ItemIndex;
    
    if (MapComboBox->ItemIndex==GoogleMaps)   LoadMap(GoogleMaps);
    else if (MapComboBox->ItemIndex==SkyVector_VFR)  LoadMap(SkyVector_VFR);
    else if (MapComboBox->ItemIndex==SkyVector_IFR_Low)  LoadMap(SkyVector_IFR_Low);
    else if (MapComboBox->ItemIndex==SkyVector_IFR_High)   LoadMap(SkyVector_IFR_High);
    else if (MapComboBox->ItemIndex==OpenStreetMaps)   LoadMap(OpenStreetMaps);

     g_EarthView->m_Eye.h =m_Eyeh;
     g_EarthView->m_Eye.x=m_Eyex;
     g_EarthView->m_Eye.y=m_Eyey;
     Timer1->Enabled=true;
     Timer2->Enabled=true;
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
			nullptr,		  // No module name (use command line)
			cmdLineCharArray, // Command line
			nullptr,		  // Process handle not inheritable
			nullptr,		  // Thread handle not inheritable
#if LOG_PYTHON
			TRUE,
#else
			FALSE, // Set handle inheritance to FALSE
#endif
			CREATE_NO_WINDOW, // Don't create a console window
			nullptr,		  // Use parent's environment block
			nullptr,		  // Use parent's starting directory
			&si,			  // Pointer to STARTUPINFO structure
			&pi))			  // Pointer to PROCESS_INFORMATION structure
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

    if (DisplayAirportCheckBox->Checked && airportManager) {
        printf("Airport display should be enabled\n");
    } else {
        printf("Airport display is disabled\n");
    }

}
//---------------------------------------------------------------------------

void __fastcall TForm1::DrawAirportIcon(double lat, double lon, bool isDeparture)
{
    double ScrX, ScrY;
    LatLon2XY(lat, lon, ScrX, ScrY);

    // Set color based on departure/arrival
    if (isDeparture) {
        glColor4f(0.0, 1.0, 0.0, 1.0);  // Green for departure
    } else {
        glColor4f(1.0, 0.0, 0.0, 1.0);  // Red for arrival
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
    for (int i = 0; i < 360; i += 10) {
		double angle = i * M_PI / 180.0;
        glVertex2f(ScrX + 15 * cos(angle), ScrY + 15 * sin(angle));
    }
    glEnd();
}

void __fastcall TForm1::DrawAirportInfo(double lat, double lon, const char* name, bool isDeparture)
{
    double ScrX, ScrY;
    LatLon2XY(lat, lon, ScrX, ScrY);

	// Draw airport name
	glColor4f(1.0, 1.0, 0.0, 1.0);
    glRasterPos2i(ScrX + 20, ScrY + 20);
    ObjectDisplay->Draw2DText(name);
}

// ĳ�õ� �Ÿ� ���� �Լ� ����
double TForm1::getCachedDistance(uint32_t aircraftICAO, const std::string& airportICAO,
                               double aircraftLat, double aircraftLon,
                               double airportLat, double airportLon) {
    auto now = std::chrono::system_clock::now();
    auto key = std::make_pair(aircraftICAO, airportICAO);
    
    // ĳ�ÿ��� �Ÿ� ã��
    auto it = distanceCache.find(key);
    if (it != distanceCache.end()) {
        // ĳ�ð� �������� �ʾҴ��� Ȯ��
        auto age = std::chrono::duration_cast<std::chrono::milliseconds>(
            now - it->second.timestamp).count();
        if (age < CACHE_EXPIRY_MS) {
            return it->second.distance;
        }
    }
    
    // ĳ�ÿ� ���ų� ������ ���� ���� ����
    double dlat = aircraftLat - airportLat;
    double dlon = aircraftLon - airportLon;
    double latDist = dlat * 60.0;
    double lonDist = dlon * 60.0 * cos(aircraftLat * M_PI/180.0);
    double distance = sqrt(latDist * latDist + lonDist * lonDist);
    
    // ������ ĳ�ÿ� ����
    DistanceCache cache;
    cache.distance = distance;
    cache.timestamp = now;
    distanceCache[key] = cache;
    
    return distance;
}

// ĳ�� ���� �Լ� ����
void TForm1::cleanupOldCache() {
    auto now = std::chrono::system_clock::now();
    
    // ������ ���� ���� ���� �ð��� �������� Ȯ��
    auto timeSinceLastCleanup = std::chrono::duration_cast<std::chrono::milliseconds>(
        now - lastCleanupTime).count();
    if (timeSinceLastCleanup < CACHE_CLEANUP_INTERVAL_MS) {
        return;
    }
    
    // ������ ĳ�� �׸� ����
    for (auto it = distanceCache.begin(); it != distanceCache.end();) {
        auto age = std::chrono::duration_cast<std::chrono::milliseconds>(
            now - it->second.timestamp).count();
        if (age > CACHE_MAX_AGE_MS) {
            it = distanceCache.erase(it);
        } else {
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
	ShowMessage("Team : SW Architect #2 Challenger\n");
}
//---------------------------------------------------------------------------

void __fastcall TForm1::UserManual1Click(TObject *Sender)
{
   	printf("User Manual Clicked\n");
	ShellExecute(0, L"open", L"https://www.naver.com", NULL, NULL, SW_SHOWNORMAL);
}

void __fastcall TForm1::UpdateAircraftInfo(TADS_B_Aircraft* Data)
{
    if (!Data) {
        return;
    }

    // Get aircraft information from database
    const TAircraftData *a = (TAircraftData *)ght_get(AircraftDBHashTable, sizeof(Data->ICAO), &Data->ICAO);

    if (a) {
        // Update aircraft metadata in right panel
        SerialNum->Caption = AnsiString(a->Fields[AC_DB_SerialNumber].c_str());
        Manufacturer->Caption = AnsiString(a->Fields[AC_DB_ManufacturerName].c_str());
        Model->Caption = AnsiString(a->Fields[AC_DB_Model].c_str());
        MFRYear->Caption = AnsiString(a->Fields[AC_DB_Built].c_str());
        CeritificatedInfo->Caption = AnsiString(a->Fields[AC_DB_Registered].c_str());
        ExpirationData->Caption = AnsiString(a->Fields[AC_DB_RegUntil].c_str());
        EngineType->Caption = AnsiString(a->Fields[AC_DB_Engines].c_str());
        AirType->Caption = AnsiString(a->Fields[AC_DB_ICAOAircraftType].c_str());

        printf("Aircraft metadata updated in UI\n");
    } else {
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


void __fastcall TForm1::UpdateRouteInfo(TADS_B_Aircraft* Data)
{
    if (!Data) {
        // Clear all route info when no data
        ClearRouteInfo();
        return;
    }

    // Get aircraft data which contains route information
    const TAircraftData *a = (TAircraftData *)ght_get(AircraftDBHashTable, sizeof(Data->ICAO), &Data->ICAO);

    printf("=== Route Information ===\n");

    if (a && a->airport_size > 0) {
        // Departure Airport (첫 번째 공항)
        DepartureAirportName->Caption = AnsiString(a->airport_name[0].c_str());
        DepartureAirportICAO->Caption = AnsiString(a->airport_icao[0].c_str());
        DepartureAirportLocation->Caption = AnsiString(a->airport_location[0].c_str());
        Label36->Caption = AnsiString(a->airport_countryiso2[0].c_str()); // Departure Country

        printf("Departure: %s (%s) - %s, %s\n",
               a->airport_name[0].c_str(),
               a->airport_icao[0].c_str(),
               a->airport_location[0].c_str(),
               a->airport_countryiso2[0].c_str());

        // Destination Airport (마지막 공항, airport_size > 1인 경우)
        if (a->airport_size > 1) {
            uint32_t lastIndex = a->airport_size - 1;
            DestinationAirportName->Caption = AnsiString(a->airport_name[lastIndex].c_str());
            DestinationAirportICAO->Caption = AnsiString(a->airport_icao[lastIndex].c_str());
            DestinationAirportLocation->Caption = AnsiString(a->airport_location[lastIndex].c_str());
            Label42->Caption = AnsiString(a->airport_countryiso2[lastIndex].c_str()); // Destination Country

            printf("Destination: %s (%s) - %s, %s\n",
                   a->airport_name[lastIndex].c_str(),
                   a->airport_icao[lastIndex].c_str(),
                   a->airport_location[lastIndex].c_str(),
                   a->airport_countryiso2[lastIndex].c_str());
        } else {
            // 목적지가 없는 경우 (단일 공항)
            DestinationAirportName->Caption = "N/A";
            DestinationAirportICAO->Caption = "N/A";
            DestinationAirportLocation->Caption = "N/A";
            Label42->Caption = "N/A";
        }

        // Transit Airport 1 (두 번째 공항, airport_size > 2인 경우)
        if (a->airport_size > 2) {
            TransitAirport1Name->Caption = AnsiString(a->airport_name[1].c_str());
            TransitAirport1ICAO->Caption = AnsiString(a->airport_icao[1].c_str());
            TransitAirport1Location->Caption = AnsiString(a->airport_location[1].c_str());
            TransitAirport1Country->Caption = AnsiString(a->airport_countryiso2[1].c_str());

            printf("Transit1: %s (%s) - %s, %s\n",
                   a->airport_name[1].c_str(),
                   a->airport_icao[1].c_str(),
                   a->airport_location[1].c_str(),
                   a->airport_countryiso2[1].c_str());
        } else {
            TransitAirport1Name->Caption = "N/A";
            TransitAirport1ICAO->Caption = "N/A";
            TransitAirport1Location->Caption = "N/A";
            TransitAirport1Country->Caption = "N/A";
        }

        // Transit Airport 2 (세 번째 공항, airport_size > 3인 경우)
        if (a->airport_size > 3) {
            TransitAirport2Name->Caption = AnsiString(a->airport_name[2].c_str());
            TransitAirport2ICAO->Caption = AnsiString(a->airport_icao[2].c_str());
            TransitAirport2Location->Caption = AnsiString(a->airport_location[2].c_str());
            TransitAirport2Country->Caption = AnsiString(a->airport_countryiso2[2].c_str());

            printf("Transit2: %s (%s) - %s, %s\n",
                   a->airport_name[2].c_str(),
                   a->airport_icao[2].c_str(),
                   a->airport_location[2].c_str(),
                   a->airport_countryiso2[2].c_str());
        } else {
            TransitAirport2Name->Caption = "N/A";
            TransitAirport2ICAO->Caption = "N/A";
            TransitAirport2Location->Caption = "N/A";
            TransitAirport2Country->Caption = "N/A";
        }

    } else {
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
    Label42->Caption = "N/A"; // Destination Country

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

    if (panelsVisible) {
        // 확장 모드
        Panel7->Align = alBottom;
        Panel7->Height = 465;  // 원래 높이

        Panel5->Align = alClient;
        Panel4->Align = alBottom;
        Panel4->Height = 350;

        PanelTitle1->Caption = "Control Menu ▼";
        PanelTitle1->Color = clSkyBlue;
        PanelTitle1->Hint = "Click to hide Control Menu";
    } else {
        // 축소 모드
        Panel7->Align = alClient;

        Panel5->Align = alTop;
        Panel5->Height = 95;

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
	try {
		if (IsSBS) {
			Form1->IdTCPClientSBS->Host = Host;
			Form1->IdTCPClientSBS->Port = Port;
			Form1->IdTCPClientSBS->ConnectTimeout = 5000;
			Form1->IdTCPClientSBS->ReadTimeout = 10000;
			Form1->IdTCPClientSBS->Connect();
		} else {
			Form1->IdTCPClientRaw->Host = Host;
			Form1->IdTCPClientRaw->Port = Port;
			Form1->IdTCPClientRaw->ConnectTimeout = 5000;
			Form1->IdTCPClientRaw->ReadTimeout = 10000;
			Form1->IdTCPClientRaw->Connect();
		}
		
		// Connection successful, update UI on main thread
		TThread::Synchronize(OnConnectionComplete);
	}
	catch (const Exception& e) {
		// Store error message and restore UI on main thread
		ErrorMessage = e.Message;
		TThread::Synchronize(OnConnectionFailed);
	}
}
//---------------------------------------------------------------------------
// UI update method called on main thread
void __fastcall TConnectionThread::OnConnectionComplete(void)
{
	if (IsSBS) {
		Form1->TCPClientSBSHandleThread = new TTCPClientSBSHandleThread(true);
		Form1->TCPClientSBSHandleThread->UseFileInsteadOfNetwork = false;
		Form1->TCPClientSBSHandleThread->FreeOnTerminate = TRUE;
		Form1->TCPClientSBSHandleThread->Resume();
		Form1->SBSConnectButton->Caption = "SBS Disconnect";
		Form1->SBSConnectButton->Enabled = true;
		
		// Add to IP history
		Form1->AddToIpHistory(Form1->SBSIpAddress->Text, true);
	} else {
		Form1->TCPClientRawHandleThread = new TTCPClientRawHandleThread(true);
		Form1->TCPClientRawHandleThread->UseFileInsteadOfNetwork = false;
		Form1->TCPClientRawHandleThread->FreeOnTerminate = TRUE;
		Form1->TCPClientRawHandleThread->Resume();
		Form1->RawConnectButton->Caption = "Raw Disconnect";
		Form1->RawConnectButton->Enabled = true;
		
		// Add to IP history
		Form1->AddToIpHistory(Form1->RawIpAddress->Text, false);
	}
}
//---------------------------------------------------------------------------
// UI update method called on main thread when connection fails
void __fastcall TConnectionThread::OnConnectionFailed(void)
{
	if (IsSBS) {
		Form1->SBSConnectButton->Caption = "SBS Connect";
		Form1->SBSConnectButton->Enabled = true;
	} else {
		Form1->RawConnectButton->Caption = "Raw Connect";
		Form1->RawConnectButton->Enabled = true;
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
	
	if (FileExists(configPath)) {
		TIniFile* ini = new TIniFile(configPath);
		try {
			// SBS IP 히스토리 로드
			int sbsCount = ini->ReadInteger("SBS", "Count", 0);
			for (int i = 0; i < sbsCount && i < MAX_IP_HISTORY; i++) {
				AnsiString ip = ini->ReadString("SBS", "IP" + IntToStr(i), "");
				if (ip != "") {
					SBSIpHistory->Add(ip);
				}
			}
			
			// Raw IP 히스토리 로드
			int rawCount = ini->ReadInteger("Raw", "Count", 0);
			for (int i = 0; i < rawCount && i < MAX_IP_HISTORY; i++) {
				AnsiString ip = ini->ReadString("Raw", "IP" + IntToStr(i), "");
				if (ip != "") {
					RawIpHistory->Add(ip);
				}
			}
		}
		__finally {
			delete ini;
		}
	}
	
	// 기본값 추가 (히스토리가 비어있는 경우)
	if (SBSIpHistory->Count == 0) {
		SBSIpHistory->Add("data.adsbhub.org");
		SBSIpHistory->Add("128.237.96.41");
	}
	
	if (RawIpHistory->Count == 0) {
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
	TIniFile* ini = new TIniFile(configPath);
	
	try {
		// SBS IP 히스토리 저장
		ini->WriteInteger("SBS", "Count", SBSIpHistory->Count);
		for (int i = 0; i < SBSIpHistory->Count; i++) {
			ini->WriteString("SBS", "IP" + IntToStr(i), SBSIpHistory->Strings[i]);
		}
		
		// Raw IP 히스토리 저장
		ini->WriteInteger("Raw", "Count", RawIpHistory->Count);
		for (int i = 0; i < RawIpHistory->Count; i++) {
			ini->WriteString("Raw", "IP" + IntToStr(i), RawIpHistory->Strings[i]);
		}
	}
	__finally {
		delete ini;
	}
}
//---------------------------------------------------------------------------
// IP 히스토리에 추가
void __fastcall TForm1::AddToIpHistory(AnsiString ip, bool isSBS)
{
	TStringList* history = isSBS ? SBSIpHistory : RawIpHistory;
	
	// 이미 존재하는지 확인
	int existingIndex = history->IndexOf(ip);
	if (existingIndex >= 0) {
		// 이미 존재하면 맨 위로 이동
		history->Move(existingIndex, 0);
	} else {
		// 새로 추가
		history->Insert(0, ip);
		
		// 최대 개수 제한
		if (history->Count > MAX_IP_HISTORY) {
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
	for (int i = 0; i < SBSIpHistory->Count; i++) {
		SBSIpAddress->Items->Add(SBSIpHistory->Strings[i]);
	}
	if (SBSIpAddress->Items->Count > 0) {
		SBSIpAddress->Text = SBSIpHistory->Strings[0];
	}
	
	// Raw ComboBox 업데이트
	RawIpAddress->Items->Clear();
	for (int i = 0; i < RawIpHistory->Count; i++) {
		RawIpAddress->Items->Add(RawIpHistory->Strings[i]);
	}
	if (RawIpAddress->Items->Count > 0) {
		RawIpAddress->Text = RawIpHistory->Strings[0];
	}
}
//---------------------------------------------------------------------------

