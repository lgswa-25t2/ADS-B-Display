#ifndef _AIRCRAFT_PLANNED_ROUTE_H
#define _AIRCRAFT_PLANNED_ROUTE_H

#include <string>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include "HttpClient.h"

class AircraftPlannedRoute {
public:
    AircraftPlannedRoute();
    ~AircraftPlannedRoute();

    const std::vector<std::pair<double, double>>& GetWaypoints(const std::string &callsign);
private:
    HttpClient httpClient_;
    // Map to store waypoints for each Callsign
    std::unordered_map<std::string, std::vector<std::pair<double, double>>> waypoints_;
    std::unordered_set<std::string> failedAircraft_;

    void init();
    void cleanup();
    void loadWaypointsFromSource(const std::string &callsign);
    void parseLatLonTable(const std::string &callsign, const std::string& html);
};

#endif // _AIRCRAFT_PLANNED_ROUTE_H