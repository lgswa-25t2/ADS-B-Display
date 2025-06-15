#include <vcl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#pragma hdrstop

#include "AirportDB.h"
#include "csv.h"

// global hashtable
ght_hash_table_t *AirportDBHashTable = NULL;

// CSV field index
#define AIRPORT_CODE_FIELD        0
#define AIRPORT_NAME_FIELD      1
#define AIRPORT_ICAO_FIELD     2
#define AIRPORT_IATA_FIELD      3
#define AIRPORT_LOCATION_FIELD 4
#define AIRPORT_COUNTRY_FIELD   5
#define AIRPORT_LAT_FIELD       6
#define AIRPORT_LON_FIELD       7
#define AIRPORT_ALTITUDE_FIELD  8
#define AIRPORT_TYPE_FIELD      9
#define AIRPORT_REGION_FIELD    10

static int CSV_callback_Airports(struct CSV_context *ctx, const char *value)
{
    int rc = 1;
    static bool IsFirstRow = true;
    static TAirportData Record;
    static bool RecordInit = false;

    // initialize
    if (!RecordInit) {
        Record.Code = "";
        Record.ICAO = "";
        Record.IATA = "";
        Record.Type = "";
        Record.Name = "";
        Record.Latitude = 0.0;
        Record.Longitude = 0.0;
        Record.AltitudeFeet = 0;
        Record.Country = "";
        Record.Location = "";
        Record.Visible = true;
        RecordInit = true;
    }

    // skip header
    if (IsFirstRow) {
        if (ctx->field_num == (ctx->num_fields - 1)) {
            IsFirstRow = false;
        }
        return rc;
    }


    switch (ctx->field_num) {
        case AIRPORT_CODE_FIELD:
            if (strlen(value) > 0) {
                Record.Code = atoi(value);
            }
            break;
            
        case AIRPORT_ICAO_FIELD:
            Record.ICAO = AnsiString(value);
            break;

        case AIRPORT_IATA_FIELD:
            Record.IATA = AnsiString(value);
            break;


        case AIRPORT_TYPE_FIELD:
            Record.Type = AnsiString(value);
            break;
            
        case AIRPORT_NAME_FIELD:
            Record.Name = AnsiString(value);
            break;
            
        case AIRPORT_LAT_FIELD:
            if (strlen(value) > 0) {
                Record.Latitude = atof(value);
            }
            break;
            
        case AIRPORT_LON_FIELD:
            if (strlen(value) > 0) {
                Record.Longitude = atof(value);
            }
            break;
            
        case AIRPORT_ALTITUDE_FIELD:
            if (strlen(value) > 0) {
                Record.AltitudeFeet = atoi(value);
            }
            break;
            
        case AIRPORT_COUNTRY_FIELD:
            Record.Country = AnsiString(value);
            break;

        case AIRPORT_LOCATION_FIELD:
            Record.Country = AnsiString(value);
            break;
    }

    // Store the record in the hash table when processing the last field.
    if (ctx->field_num == (ctx->num_fields - 1)) {
        // Store only valid data (when both ICAO code and coordinates are present).
        if (!Record.ICAO.IsEmpty() && 
            Record.Latitude != 0.0 && 
            Record.Longitude != 0.0) {
            /*
            // Filter by airport type (if necessary).
            bool shouldStore = false;
            if (Record.Type == "large_airport" || 
                Record.Type == "medium_airport" ||
                Record.Type == "small_airport") {
                shouldStore = true;
            }
            */
            bool shouldStore = false;
            if (!Record.IATA.IsEmpty()) {
                 shouldStore = true;
            }

            if (shouldStore) {
                TAirportData *Data = new TAirportData;
                *Data = Record; 
                
                // Store in the hash table using the ICAO code as the key.
                AnsiString key = Record.ICAO;
                if (ght_insert(AirportDBHashTable, Data, 
                              key.Length(), key.c_str()) < 0) {
                    printf("Airport DB: Hash insert error for %s\n", 
                           Record.ICAO.c_str());
                    delete Data;
                } else {
//                     printf("Airport loaded: %s - %s type:%s, IATA:%s\n",
//                            Record.ICAO.c_str(), Record.Name.c_str(), Record.Type.c_str(), Record.IATA.c_str());
                }
            }
        }
        
        // Initialize for the next record.
        Record.Code = 0;
        Record.ICAO = "";
        Record.Type = "";
        Record.Name = "";
        Record.Latitude = 0.0;
        Record.Longitude = 0.0;
        Record.AltitudeFeet = 0;
        Record.Country = "";
        Record.Visible = true;
    }
    
    return rc;
}

// Initialize airport database.
bool InitAirportDB(AnsiString FileName)
{
    CSV_context csv_ctx;
    
    // create hashtable (expected number of airports: 50000개)
    AirportDBHashTable = ght_create(50000);
    if (!AirportDBHashTable) {
        printf("Airport DB: Failed to create hash table\n");
        return false;
    }
    
    ght_set_rehash(AirportDBHashTable, TRUE);
    
    // configure CSV parsing context
    memset(&csv_ctx, 0, sizeof(csv_ctx));
    csv_ctx.file_name = FileName.c_str();
    csv_ctx.delimiter = ',';
    csv_ctx.callback = CSV_callback_Airports;
    csv_ctx.line_size = 2000;  // Allocate sufficient size for long airport names
    
    printf("Loading Airport Database from %s...\n", FileName.c_str());
    
    // parsing CSV file
    if (!CSV_open_and_parse_file(&csv_ctx)) {
        printf("Airport DB: Parsing failed - %s\n", strerror(errno));
        ght_finalize(AirportDBHashTable);
        AirportDBHashTable = NULL;
        return false;
    }
    
    printf("Airport Database loaded successfully. Total airports: %d\n", 
           (int)ght_size(AirportDBHashTable));

    return true;
}

void CleanupAirportDB(void)
{
    if (AirportDBHashTable) {
        ght_iterator_t iterator;
        uint32_t *Key;
        TAirportData *Data;
        
        // delete airport data
        for (Data = (TAirportData *)ght_first(AirportDBHashTable, &iterator, 
                                             (const void **)&Key);
             Data; 
             Data = (TAirportData *)ght_next(AirportDBHashTable, &iterator, 
                                            (const void **)&Key)) {
            delete Data;
        }
        
        ght_finalize(AirportDBHashTable);
        AirportDBHashTable = NULL;
        printf("Airport Database cleaned up\n");
    }
}

// search with ICAO code
TAirportData* GetAirportByICAO(AnsiString ICAO)
{
    if (!AirportDBHashTable || ICAO.IsEmpty()) {
        return NULL;
    }
    
    return (TAirportData *)ght_get(AirportDBHashTable, 
                                   ICAO.Length(), 
                                   ICAO.c_str());
}

// Return all airport data from the hash table as a vector.
std::vector<TAirportData> GetAllAirportsFromHashTable()
{
    std::vector<TAirportData> airports;
    
    if (!AirportDBHashTable) {
        printf("Airport DB: Hash table not initialized\n");
        return airports;
    }
    
    ght_iterator_t iterator;
    uint32_t *Key;
    TAirportData *Data;
    
    for (Data = (TAirportData *)ght_first(AirportDBHashTable, &iterator, 
                                         (const void **)&Key);
         Data; 
         Data = (TAirportData *)ght_next(AirportDBHashTable, &iterator, 
                                        (const void **)&Key)) {
        airports.push_back(*Data);
    }
    
    printf("Extracted %d airports from hash table\n", (int)airports.size());
    return airports;
}