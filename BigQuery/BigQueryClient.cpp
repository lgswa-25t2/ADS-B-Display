#include "BigQueryClient.h"
#include "OAuth2Helper.h"
#include "../json.hpp"
#include "../HttpClient.h"
#include <fstream>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <ctime>

using json = nlohmann::json;

BigQueryClient::BigQueryClient()
	: initialized_(false), project_id_("scs-lg-arch-2"), dataset_id_("SBS_Data"), table_id_("backup") {
    oauth2_helper_ = std::make_unique<OAuth2Helper>();
}

BigQueryClient::~BigQueryClient() {
}

bool BigQueryClient::Initialize(const std::string& credentials_path) {
    try {
        // Initialize OAuth2 helper
        if (!oauth2_helper_->Initialize(credentials_path)) {
            SetLastError("Failed to initialize OAuth2 helper: " + oauth2_helper_->GetLastError());
            return false;
        }

        // Read credentials file to get project ID
        std::ifstream cred_file(credentials_path);
        if (!cred_file.is_open()) {
            SetLastError("Failed to open credentials file: " + credentials_path);
            return false;
        }

        json credentials;
        cred_file >> credentials;
        cred_file.close();

        // Extract project ID from credentials
        if (credentials.contains("project_id")) {
            project_id_ = credentials["project_id"].get<std::string>();
        }

        initialized_ = true;
        return true;
    }
    catch (const std::exception& e) {
        SetLastError("Failed to initialize BigQuery client: " + std::string(e.what()));
        return false;
    }
}

bool BigQueryClient::LoadAircraftData(const std::string& query, std::vector<AircraftData>& results) {
    if (!initialized_) {
        SetLastError("BigQuery client not initialized");
        return false;
    }

    return ExecuteQuery(query, results);
}

bool BigQueryClient::LoadAircraftDataByTimeRange(const std::string& start_time,
                                               const std::string& end_time,
                                               std::vector<AircraftData>& results) {
    std::string query = BuildQuery("WHERE timestamp BETWEEN '" + start_time + "' AND '" + end_time + "'");
    return ExecuteQuery(query, results);
}

bool BigQueryClient::LoadAircraftDataByICAO(const std::string& icao24,
                                           std::vector<AircraftData>& results) {
	std::string query = BuildQuery("WHERE HexIdent = '" + icao24 + "'");
    return ExecuteQuery(query, results);
}

bool BigQueryClient::LoadRecentAircraftData(int minutes, std::vector<AircraftData>& results) {
    // Get current time and subtract minutes
    auto now = std::chrono::system_clock::now();
    auto time_minus_minutes = now - std::chrono::minutes(minutes);

    std::time_t start_time_t = std::chrono::system_clock::to_time_t(time_minus_minutes);
    std::time_t end_time_t = std::chrono::system_clock::to_time_t(now);

    std::stringstream start_ss, end_ss;
    start_ss << std::put_time(std::gmtime(&start_time_t), "%Y-%m-%d %H:%M:%S");
    end_ss << std::put_time(std::gmtime(&end_time_t), "%Y-%m-%d %H:%M:%S");

    return LoadAircraftDataByTimeRange(start_ss.str(), end_ss.str(), results);
}

bool BigQueryClient::LoadPlannedRouteData(std::vector<AircraftData>& results) {
    // Query for data from 2025-06-01 onwards with valid coordinates
    std::string query = BuildQuery("WHERE Date_MSG_Generated >= '2025-06-01' "
                                  "AND Latitude IS NOT NULL "
                                  "AND Longitude IS NOT NULL "
                                  "AND Latitude != 0 "
                                  "AND Longitude != 0 "
                                  "ORDER BY HexIdent, Date_MSG_Generated, Time_MSG_Generated");
    return ExecuteQuery(query, results);
}

bool BigQueryClient::ExecuteQuery(const std::string& query, std::vector<AircraftData>& results) {
    if (!initialized_) {
        SetLastError("BigQuery client not initialized");
        return false;
    }

    try {
        // Create HTTP client
        HttpClient http_client;

        // Initialize HTTP client
        if (!http_client.init()) {
            SetLastError("Failed to initialize HTTP client: " + http_client.getLastError());
            return false;
        }
		// BigQuery REST API endpoint
		std::string url = "https://bigquery.googleapis.com/bigquery/v2/projects/" + project_id_ + "/queries";

        // Prepare request headers
		std::map<std::string, std::string> headers;
        
        std::string access_token = GetAccessToken();
        if (access_token.empty()) {
            SetLastError("Failed to get access token");
            printf("BigQueryClient::ExecuteQuery - ERROR: Access token is empty!\n");
            return false;
        }
        
		headers["Authorization"] = "Bearer " + access_token;
        headers["Content-Type"] = "application/json";
        headers["Accept"] = "application/json";

        // Prepare request body
        json request_body;
        request_body["query"] = query;
        request_body["useLegacySql"] = false;

        std::string request_json = request_body.dump();

		// Make HTTP POST request
        std::string response;
        int status_code = http_client.post(url, request_json, response, headers);

        if (status_code != 200) {
            SetLastError("HTTP request failed with status code: " + std::to_string(status_code));
            return false;
        }

        // Parse response
        return ParseQueryResults(response, results);
    }
    catch (const std::exception& e) {
        SetLastError("Failed to execute query: " + std::string(e.what()));
        return false;
    }
}

std::string BigQueryClient::BuildQuery(const std::string& conditions) {
    std::string query = "SELECT ";
	query += "SessionID, AircraftID, ";
	query += "HexIdent, FlightID, Callsign, Date_MSG_Generated, Time_MSG_Generated, ";
	query += "Latitude, Longitude, Altitude, GroundSpeed, IsOnGround ";
    query += "FROM `" + project_id_ + "." + dataset_id_ + "." + table_id_ + "`";

    if (!conditions.empty()) {
        query += " " + conditions;
    }

	query += " LIMIT 1000";

    return query;
}

bool BigQueryClient::ParseQueryResults(const std::string& json_response, std::vector<AircraftData>& results) {
    try {
        printf("BigQueryClient::ParseQueryResults - Starting to parse JSON response (length: %zu)\n", json_response.length());
        
        json response_json = json::parse(json_response);

		if (!response_json.contains("rows")) {
            SetLastError("No rows found in response");
            printf("BigQueryClient::ParseQueryResults - No 'rows' field found in response\n");
            return true; // No data is not an error
        }

        // Get schema information
        std::vector<std::string> column_names;
        if (response_json.contains("schema") && response_json["schema"].contains("fields")) {
            for (const auto& field : response_json["schema"]["fields"]) {
                if (field.contains("name")) {
                    column_names.push_back(field["name"].get<std::string>());
                }
            }
        }

        printf("BigQueryClient::ParseQueryResults - Column names: ");
        for (const auto& col : column_names) {
            printf("%s ", col.c_str());
        }
        printf("\n");

        results.clear();

		for (const auto& row : response_json["rows"]) {
			AircraftData data;

			if (row.contains("f")) {
				const auto& fields = row["f"];

				// Parse fields based on their position in the SELECT statement
				if (fields.size() >= 11) { // SELECT SessionID, AircraftID, HexIdent, FlightID, Callsign, Date_MSG_Generated, Time_MSG_Generated, Latitude, Longitude, Altitude, GroundSpeed
					// Map fields to AircraftData structure based on column names
					for (size_t i = 0; i < fields.size() && i < column_names.size(); i++) {
						std::string value;
						std::string column_name = column_names[i];
						
						// Safely handle null values
						if (fields[i]["v"].is_null()) {
							value = "";
						} else {
							try {
								value = fields[i]["v"].get<std::string>();
							} catch (const std::exception& e) {
								printf("  Column %zu (%s): Error getting string value: %s\n", i, column_name.c_str(), e.what());
								value = "";
							}
						}

						if (column_name == "HexIdent") {
							data.icao24 = value;
						} else if (column_name == "Callsign") {
							data.callsign = value;
						} else if (column_name == "Date_MSG_Generated") {
							data.date_msg_generated = value;
						} else if (column_name == "Time_MSG_Generated") {
							data.time_msg_generated = value;
						} else if (column_name == "Latitude") {
							if(value.empty()) {
								data.bigquery_latitude = 0;
							} else {
								try {
									data.bigquery_latitude = std::stod(value);
								} catch (const std::exception& e) {
									printf("  Error converting Latitude '%s' to double: %s\n", value.c_str(), e.what());
									data.bigquery_latitude = 0;
								}
							}
						} else if (column_name == "Longitude") {
							if(value.empty()) {
								data.bigquery_longitude = 0;
							} else {
								try {
									data.bigquery_longitude = std::stod(value);
								} catch (const std::exception& e) {
									printf("  Error converting Longitude '%s' to double: %s\n", value.c_str(), e.what());
									data.bigquery_longitude = 0;
								}
							}
						} else if (column_name == "Altitude") {
							if(value.empty()) {
								data.bigquery_altitude = 0;
							} else {
								try {
									data.bigquery_altitude = std::stod(value);
								} catch (const std::exception& e) {
									printf("  Error converting Altitude '%s' to double: %s\n", value.c_str(), e.what());
									data.bigquery_altitude = 0;
								}
							}
						} else if (column_name == "GroundSpeed") {
							if(value.empty()) {
								data.bigquery_ground_speed = 0;
							} else {
								try {
									data.bigquery_ground_speed = std::stod(value);
								} catch (const std::exception& e) {
									printf("  Error converting GroundSpeed '%s' to double: %s\n", value.c_str(), e.what());
									data.bigquery_ground_speed = 0;
								}
							}
						} else if (column_name == "IsOnGround") {
							if(value.empty()) {
								data.is_on_ground = false;
							} else {
								// Convert string to boolean (assuming "true"/"false" or "1"/"0")
								data.is_on_ground = (value == "true" || value == "1" || value == "TRUE");
							}
						} else if (column_name == "SessionID") {
							// Store in a temporary field or ignore
						} else if (column_name == "AircraftID") {
							// Store in a temporary field or ignore
						} else if (column_name == "FlightID") {
							// Store in a temporary field or ignore
						}
					}

					results.push_back(data);
                } else {
                    printf("BigQueryClient::ParseQueryResults - Warning: Row has only %zu fields, expected at least 11\n", fields.size());
                }
			}
		}

		printf("BigQueryClient::ParseQueryResults - Successfully parsed %zu aircraft records\n", results.size());
		return true;
	}
	catch (const json::exception& e) {
		SetLastError("JSON parsing error: " + std::string(e.what()));
		printf("BigQueryClient::ParseQueryResults - JSON parsing error: %s\n", e.what());
		return false;
	}
	catch (const std::exception& e) {
		SetLastError("Failed to parse query results: " + std::string(e.what()));
		printf("BigQueryClient::ParseQueryResults - General parsing error: %s\n", e.what());
		return false;
	}
}

std::string BigQueryClient::GetAccessToken() {
	if (!oauth2_helper_) {
        SetLastError("OAuth2 helper is not initialized");
        return "";
    }
    
    try {
        std::string token = oauth2_helper_->GetAccessToken();
        if (token.empty()) {
            SetLastError("Failed to get access token: " + oauth2_helper_->GetLastError());
            return "";
        }
        
        printf("Successfully obtained BigQuery access token (length: %d)\n", (int)token.length());
        return token;
    }
    catch (const std::exception& e) {
        SetLastError("Exception while getting access token: " + std::string(e.what()));
        return "";
    }
}

void BigQueryClient::SetLastError(const std::string& error) {
    last_error_ = error;
}
