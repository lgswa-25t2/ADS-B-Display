#ifndef BIGQUERY_CLIENT_H
#define BIGQUERY_CLIENT_H

#include <string>
#include <vector>
#include <memory>
#include <functional>

#if 0
// Forward declarations for Google Cloud libraries
namespace google {
namespace cloud {
namespace bigquery {
class BigQueryClient;
class Table;
}  // namespace bigquery
}  // namespace cloud
}  // namespace google
#endif

// Forward declarations
class OAuth2Helper;

struct AircraftData {
    std::string icao24;
    std::string callsign;
    double latitude;
    double longitude;
    
    std::string timestamp;
    std::string departure_airport;
    std::string arrival_airport;
    std::string airline;
    std::string aircraft_type;
    std::string date_msg_generated;  // BigQuery DATE type stored as string (YYYY-MM-DD format)
    std::string time_msg_generated;  // BigQuery TIME type stored as string (HH:MM:SS format)
    
    // BigQuery double type columns
    double bigquery_latitude;    // BigQuery Latitude column (double)
    double bigquery_longitude;   // BigQuery Longitude column (double)
    double bigquery_altitude;    // BigQuery Altitude column (double)
    double bigquery_ground_speed; // BigQuery GroundSpeed column (double)
    
    // BigQuery boolean type columns
    bool is_on_ground;           // BigQuery IsOnGround column (boolean)
};

class BigQueryClient {
public:
    BigQueryClient();
    ~BigQueryClient();

    // Initialize the client with credentials
    bool Initialize(const std::string& credentials_path);

    // Load aircraft data from BigQuery
    bool LoadAircraftData(const std::string& query, std::vector<AircraftData>& results);

    // Load data for a specific time range
    bool LoadAircraftDataByTimeRange(const std::string& start_time,
                                   const std::string& end_time,
                                   std::vector<AircraftData>& results);

    // Load data for a specific aircraft
    bool LoadAircraftDataByICAO(const std::string& icao24,
                               std::vector<AircraftData>& results);

    // Load recent data (last N minutes)
    bool LoadRecentAircraftData(int minutes, std::vector<AircraftData>& results);

    // Load planned route data from 2025-06-01 onwards
    bool LoadPlannedRouteData(std::vector<AircraftData>& results);

    // Execute custom query
    bool ExecuteQuery(const std::string& query, std::vector<AircraftData>& results);

    // Check if client is initialized
    bool IsInitialized() const { return initialized_; }

    // Get last error message
	std::string GetLastError() const { return last_error_; }

	std::string GetAccessToken();
	
	// Get project ID
	std::string GetProjectId() const { return project_id_; }

private:
	//std::unique_ptr<google::cloud::bigquery::BigQueryClient> client_;
    std::unique_ptr<OAuth2Helper> oauth2_helper_;
    bool initialized_;
    std::string last_error_;
    std::string project_id_;
    std::string dataset_id_;
    std::string table_id_;

    // Helper methods
    void SetLastError(const std::string& error);
    std::string BuildQuery(const std::string& conditions = "");
	bool ParseQueryResults(const std::string& json_response, std::vector<AircraftData>& results);

};

#endif // BIGQUERY_CLIENT_H
