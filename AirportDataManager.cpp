#include "AirportDataManager.h"

AirportDataManager::AirportDataManager() : MAJOR_AIRPORTS({
    "RKSI", "RKSS", // Korea major aiports
    "6PN6", // Pittsburgh airports
    // ... etc major airports
}) {

}

AirportDataManager::~AirportDataManager() {

}

// convert from TAirportData to Airport
AirportDataManager::Airport AirportDataManager::convertFromTAirportData(const TAirportData& data) {
    Airport airport;
    
    airport.code = data.Code.c_str();
    airport.name = data.Name.c_str();
    airport.icao = data.ICAO.c_str();
    airport.iata = data.IATA.c_str();
    airport.location = data.Location.c_str();
    airport.countryISO2 = data.Country.c_str();
    airport.latitude = data.Latitude;
    airport.longitude = data.Longitude;
    airport.altitudeFeet = data.AltitudeFeet;
    airport.importance = 0; //to be calculated later
    
    return airport;
}

void AirportDataManager::loadAirportsFromHashTable(const vector<TAirportData>& airportData) {
    airports.clear();
    icaoIndex.clear();
    
    // convert from TAirportData to Airport
    for (const auto& data : airportData) {
        Airport airport = convertFromTAirportData(data);
        airports.push_back(airport);
    }


    for (size_t i = 0; i < airports.size(); i++) {
        icaoIndex[airports[i].icao] = i;
        //calculate importans (to be updated)
        airports[i].importance = calculateImportance(airports[i]);
        //printf("Airport:%s, importance:%d", airports[i].icao.c_str(), airports[i].importance);
    }
    
    printf("AirportDataManager: Loaded %d airports from hash table\n", (int)airports.size());
}

// filter aiports for display
vector<AirportDataManager::Airport> AirportDataManager::getVisibleAirports(
    double viewMinLat, double viewMaxLat,
    double viewMinLon, double viewMaxLon,
    int zoomLevel
) {
    vector<Airport> visible;
    
    for (const auto& airport : airports) {

    		visible.push_back(airport);
        /* currently below logic is not working.. to be updated by using better logic.
        // check if airport is within viewport
        if (airport.latitude >= viewMinLat && airport.latitude <= viewMaxLat &&
            airport.longitude >= viewMinLon && airport.longitude <= viewMaxLon) {
            
            // filtering by zoomlevel
            if (shouldDisplayAirport(airport, zoomLevel)) {
                printf("shouldDisplayAirport: %s\n", airport.icao.c_str());
                visible.push_back(airport);
            }
            else{
                printf("NOT shouldDisplayAirport: %s\n", airport.icao.c_str());
            }
        }    */
    }
    
    return visible;
}

// calculate Importance (it is calulated, but we do not use this value for Now.)
int AirportDataManager::calculateImportance(const Airport& airport) {
    int score = 0;
    
    // IATA cod existence (currently all airports loaded have IATA code)
    if (!airport.iata.empty()) {
        score += 3;
    }
    
    // Major airport (configuration)
    if (MAJOR_AIRPORTS.find(airport.icao) != MAJOR_AIRPORTS.end()) {
        score += 3;
    }
    
    // by altitudeFeet
    if (airport.altitudeFeet > 5000) {
        score += 1;
    }
    
    return score;
}

// we do not use this value for Now
bool AirportDataManager::shouldDisplayAirport(const Airport& airport, int zoomLevel) {
    if (zoomLevel < 5) {
        return airport.importance >= 5;
    } else if (zoomLevel < 8) {
        return airport.importance >= 3;
    } else if (zoomLevel < 12) {
        return airport.importance >= 2;
    }
    return true;
}

// search airport by ICAO
AirportDataManager::Airport* AirportDataManager::findAirport(const string& icao) {
    auto it = icaoIndex.find(icao);
    if (it != icaoIndex.end()) {
        return &airports[it->second];
    }
    return nullptr;
}

// for Refrence. icon size may be variable like this.
int AirportDataManager::getAirportIconSize(const Airport& airport, int zoomLevel) {
    int baseSize = 8;


    if (airport.importance >= 5) {
        baseSize += 4;
    }

    if (zoomLevel <= -30) {
      baseSize += 10;
    }
    else if(zoomLevel <= -20) {
			baseSize += 6;
    }
    else if(zoomLevel <= -10) {
      baseSize += 2;
    }
    else if(zoomLevel <= -8) {
      baseSize += 0;
    }
    else{
      baseSize -= 6;
    }
    
    return baseSize;
} 