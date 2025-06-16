//---------------------------------------------------------------------------

#pragma hdrstop

#include "AircraftAPI.h"
#include "AircraftDB.h"
#include "HttpClient.h"
#include "csv.h"
#include <fstream>
#include <sstream>
#include <map>

HttpClient http;

extern ght_hash_table_t *AircraftDBHashTable;

bool HttpClientInit() {
  static bool http_initialized = false;

  if (!http_initialized) {
    if (!http.init()) {
      printf("Failed to initialize HTTP client: %s\n", http.getLastError().c_str());
      return false;
    }
    http_initialized = true;
  }

  return http_initialized;
}
  
const char * GetAircraftAPIInfo(uint32_t addr, char *flightNum) {
  static char          buf [2048];
  TAircraftData *a;
  a =(TAircraftData *) ght_get(AircraftDBHashTable,sizeof(addr),&addr);

  if (a) {
	if (!HttpClientInit()) {
		return NULL;
	}

	char routes_url[256];
	json routes_response;

	for(int i = 0; i < strlen(flightNum); ++i) {
		if (flightNum[i] == ' ')
			flightNum[i] = '\0';
	}

	snprintf(routes_url, sizeof(routes_url),
		"https://vrs-standing-data.adsb.lol/routes/%c%c/%s.json",
		flightNum[0], flightNum[1], flightNum);

	routes_response = http.getJson(routes_url);

	if (!routes_response.is_null() && routes_response.contains("callsign")) {
		a->callSign = routes_response["callsign"].get<std::string>();
		a->number = routes_response["number"].get<std::string>();
		a->airline_code = routes_response["airline_code"].get<std::string>();
		a->airport_codes = routes_response["airport_codes"].get<std::string>();
		a->airport_codes_iata = routes_response["_airport_codes_iata"].get<std::string>();
		a->airport_size = routes_response["_airports"].size();

		if (routes_response.contains("_airports") && routes_response["_airports"].is_array()) {
			const auto& airports = routes_response["_airports"];

			for(int i = 0; i < a->airport_size; i++) {
				a->airport_name[i] = airports[i]["name"].get<std::string>();
				a->airport_icao[i] = airports[i]["icao"].get<std::string>();
				a->airport_iata[i] = airports[i]["iata"].get<std::string>();
				a->airport_location[i] = airports[i]["location"].get<std::string>();
				a->airport_countryiso2[i] = airports[i]["countryiso2"].get<std::string>();
				a->airport_lat[i] = airports[i]["lat"].get<double>();
				a->airport_lon[i] = airports[i]["lon"].get<double>();
				a->airport_alt_feet[i] = airports[i]["alt_feet"].get<double>();
				a->airport_alt_meters[i] = airports[i]["alt_meters"].get<double>();
			}
		}

		snprintf (buf,sizeof(buf), "CallSign: %s, Number: %s, Airline Code: %s, Airport Codes: %s, Airport Codes IATA: %s, Airport Size: %d\n",
			a->callSign.c_str(), a->number.c_str(), a->airline_code.c_str(), a->airport_codes.c_str(), a->airport_codes_iata.c_str(), a->airport_size);

		for(int i = 0; i < a->airport_size; i++) {
			snprintf (buf + strlen(buf), sizeof(buf) - strlen(buf), "Airport Name: %s, Airport ICAO: %s, Airport IATA: %s, Airport Location: %s, Airport Country ISO2: %s, Airport Lat: %lf, Airport Lon: %lf, Airport Alt Feet: %lf, Airport Alt Meters: %lf\n",
				a->airport_name[i].c_str(), a->airport_icao[i].c_str(), a->airport_iata[i].c_str(), a->airport_location[i].c_str(), a->airport_countryiso2[i].c_str(), a->airport_lat[i], a->airport_lon[i], a->airport_alt_feet[i], a->airport_alt_meters[i]);
		}
	}
	else {
		snprintf (buf,sizeof(buf), "There is no route data for this flight\n");
	}
  }
  else {
		snprintf (buf,sizeof(buf), "There is no route data for this flight\n");
  }


  return buf;
}
//---------------------------------------------------------------------------
