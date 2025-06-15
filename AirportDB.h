#ifndef AirportDBH
#define AirportDBH

#include <vcl.h>
#include "ght_hash_table.h"
#include <vector>
#include <string>

typedef struct {
    AnsiString Code;                // Airport code
		AnsiString Name;            // Airport name
    AnsiString ICAO;            // ICAO code
		AnsiString IATA;            // IATA
    AnsiString Type;            // type (large_airport, medium_airport 등)

    double Latitude;
    double Longitude;
    int AltitudeFeet;
    AnsiString Country;
    AnsiString Location;
    bool Visible;
} TAirportData;

extern ght_hash_table_t *AirportDBHashTable;

bool InitAirportDB(AnsiString FileName);
void CleanupAirportDB(void);
TAirportData* GetAirportByICAO(AnsiString ICAO);

// Function for integration with AirportDataManager.
std::vector<TAirportData> GetAllAirportsFromHashTable();

#endif