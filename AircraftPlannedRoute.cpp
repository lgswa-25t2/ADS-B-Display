#include "AircraftPlannedRoute.h"
#include <regex>
#include <set>
#include <ctime>
#include <chrono>

AircraftPlannedRoute::AircraftPlannedRoute() {
    init();
}

AircraftPlannedRoute::~AircraftPlannedRoute() {
    cleanup();
}

void AircraftPlannedRoute::init() {
    // Initialize the waypoints map or any other necessary setup
    httpClient_.init();
}

void AircraftPlannedRoute::cleanup() {
    // Cleanup the waypoints map or any other necessary teardown
    httpClient_.cleanup();
}

void AircraftPlannedRoute::loadWaypointsFromSource(const std::string &callsign) {
    if (callsign.empty() || waypoints_.count(callsign)) {
        return; // No Callsign or already loaded
    }

    std::string url = "https://www.flightaware.com/live/flight/" + callsign;
    std::string html = httpClient_.get(url.c_str());
    if (html.empty()) {
        return; // Failed to fetch main page
    }

    // Get local time now
    time_t now = time(nullptr);
    tm local_tm;
#if defined(_WIN32)
    localtime_s(&local_tm, &now);
#else
    localtime_r(&now, &local_tm);
#endif

    std::string latest_url;

    // Try last 7 days, from yesterday backwards
    for (int i = 1; i <= 7; ++i) {
        tm temp_tm = local_tm;
        temp_tm.tm_mday -= i;
        mktime(&temp_tm); // Normalize date

        char date_buf[16];
        snprintf(date_buf, sizeof(date_buf), "%04d%02d%02d",
                 temp_tm.tm_year + 1900, temp_tm.tm_mon + 1, temp_tm.tm_mday);

        std::string base_search = "/live/flight/" + callsign + "/history/" + date_buf + "/";

        size_t pos = html.find(base_search);
        if (pos == std::string::npos) {
            continue; // Not found for this date, try previous day
        }

        // Extract the full URL until the next double quote after pos
        size_t end_quote = html.find('"', pos);
        if (end_quote == std::string::npos) {
            continue; // Malformed, skip
        }

        std::string found_url = html.substr(pos, end_quote - pos);

        latest_url = "https://www.flightaware.com" + found_url + "/route";
        //printf("%s: Route history URL found for date %s: %s\n", callsign.c_str(), date_buf, latest_url.c_str());
        break; // Stop after first found
    }

    if (latest_url.empty()) {
        printf("%s: No route history URL found in last 7 days.\n", callsign.c_str());
        return;
    }

    std::string routeHtml = httpClient_.get(latest_url.c_str());
    if (routeHtml.empty()) {
        printf("%s: Failed to fetch route HTML.\n", callsign.c_str());
        return;
    }

    parseLatLonTable(callsign, routeHtml);
    if (waypoints_[callsign].empty()) {
        printf("%s: No lat/lon data found.\n", callsign.c_str());
    }
}

void AircraftPlannedRoute::parseLatLonTable(const std::string &callsign, const std::string& html) {
    std::regex rowRegex(
        R"(<tr[^>]*>[\s\S]*?<td[^>]*>[\s\S]*?</td>[\s\S]*?<td[^>]*>\s*(-?\d+\.\d+)\s*</td>[\s\S]*?<td[^>]*>\s*(-?\d+\.\d+)\s*</td>)",
        std::regex_constants::icase
    );
    
    std::smatch match;

    auto begin = html.cbegin();
    auto end = html.cend();
    waypoints_[callsign].clear(); // Clear existing waypoints before parsing new data

    while (std::regex_search(begin, end, match, rowRegex)) {
        try {
            if (match.size() < 3) {
                continue; // Skip if not enough groups matched
            }
            double lat = std::stod(match[1].str());
            double lon = std::stod(match[2].str());
            waypoints_[callsign].emplace_back(lat, lon);
            //printf("%s: Parsed waypoint: (%f, %f)\n", callsign.c_str(), lat, lon);
        } catch (...) {
            // Ignore malformed rows
        }
        begin = match.suffix().first;
    }
}

const std::vector<std::pair<double, double>>& AircraftPlannedRoute::GetWaypoints(const std::string &callsign) {
    static const std::vector<std::pair<double, double>> empty;
    if (callsign.empty()) {
        return empty; // No Callsign provided
    }
    // Check if the Callsign is in the failed aircraft set
    if (failedAircraft_.find(callsign) != failedAircraft_.end()) {
        //printf("%s: Waypoints already failed to load previously.\n", callsign.c_str());
        return empty; // Waypoints for this Callsign have already failed to load
    }
    // If the Callsign does not exist, try fetching it from an external source
    if (waypoints_.find(callsign) != waypoints_.end()) {
        return waypoints_[callsign];
    }
 
    loadWaypointsFromSource(callsign);
 
    if (waypoints_.find(callsign) != waypoints_.end()) {
        return waypoints_[callsign];
    }

    // update the failed aircraft set if waypoints are still not found
    if (waypoints_.find(callsign) == waypoints_.end()) {
        failedAircraft_.insert(callsign);
        printf("%s: Failed to load waypoints.\n", callsign.c_str());
    }
    return empty;
}