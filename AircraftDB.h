//---------------------------------------------------------------------------

#ifndef AircraftDBH
#define AircraftDBH
#include <vcl.h>
#include "ght_hash_table.h"

#define  AC_DB_NUM_FIELDS          27
#define  AC_DB_ICAO                 0
#define  AC_DB_Registration         1
#define  AC_DB_ManufacturerICAO     2
#define  AC_DB_ManufacturerName     3
#define  AC_DB_Model                4
#define  AC_DB_TypeCode             5
#define  AC_DB_SerialNumber         6
#define  AC_DB_LineNumber           7
#define  AC_DB_ICAOAircraftType     8
#define  AC_DB_Operator             9
#define  AC_DB_OperatorCallSign    10
#define  AC_DB_OperatorICAO        11
#define  AC_DB_OperatorIATA        12
#define  AC_DB_Owner               13
#define  AC_DB_TestReg             14
#define  AC_DB_Registered          15
#define  AC_DB_RegUntil            16
#define  AC_DB_Status              17
#define  AC_DB_Built               18
#define  AC_DB_FirstFlightDate     19
#define  AC_DB_Seatconfiguration   20
#define  AC_DB_Engines             21
#define  AC_DB_Modes               22
#define  AC_DB_ADSB                23
#define  AC_DB_ACARS               24
#define  AC_DB_Notes               25
#define  AC_DB_CategoryDescription 26

typedef struct
{
  uint32_t    ICAO24;
  AnsiString  Fields[AC_DB_NUM_FIELDS];
  std::string callSign;
  std::string number;
  std::string airline_code;
  std::string airport_codes;
  std::string airport_codes_iata;
  uint32_t    airport_size;
  std::string airport_name[10];
  std::string airport_icao[10];
  std::string airport_iata[10];
  std::string airport_location[10];
  std::string airport_countryiso2[10];
  double airport_lat[10];
  double airport_lon[10];
  double airport_alt_feet[10];
  double airport_alt_meters[10];

  // Planned route information
  uint32_t    route_size;
  
  double route_latitude[10];
  double route_longitude[10];  
} TAircraftData;

bool InitAircraftDB(AnsiString FileName);
const char * GetAircraftDBInfo(uint32_t addr);
bool aircraft_is_helicopter(uint32_t addr, const char **type_ptr);
bool aircraft_is_military(uint32_t addr, const char **type_ptr);
bool IsAircraftMilitary(uint32_t icao_addr);
static char *stristr(const char *String, const char *Pattern);

// Planned route functions
void LoadPlannedRouteData();
void ParseRouteJson(const std::string& jsonContent);
void StoreRoutePoint(const std::string& icao, double lat, double lon, int index);

//---------------------------------------------------------------------------
#endif
