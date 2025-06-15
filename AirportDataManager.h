#ifndef AIRPORTDATAMANAGER_H
#define AIRPORTDATAMANAGER_H

#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include "AirportDB.h"      //for TAirportData

using namespace std;

// Forward declaration
//struct TAirportData;

class AirportDataManager {
private:
    struct Airport {
        string code;
        string name;
        string icao;
        string iata;
        string location;
        string countryISO2;
        double latitude;
        double longitude;
        double altitudeFeet;
        int importance;  // 계산된 중요도 캐싱
    };

    // 공항 데이터 저장
    vector<Airport> airports;
    unordered_map<string, size_t> icaoIndex;  // 빠른 검색을 위한 인덱스

    // 주요 공항 ICAO 코드
    const unordered_set<string> MAJOR_AIRPORTS;

    // 공항 중요도 계산 (이전 AirportImportanceCalculator 기능)
    int calculateImportance(const Airport& airport);
    
    // 표시 여부 결정 (이전 AirportDisplayManager 기능)
    bool shouldDisplayAirport(const Airport& airport, int zoomLevel);
    
    // TAirportData를 Airport로 변환
    Airport convertFromTAirportData(const TAirportData& data);

public:
    AirportDataManager();
    ~AirportDataManager();
    
    // 공항 데이터 로드 (기존)
    void loadAirports(const vector<Airport>& airportData);
    
    // 공항 데이터 로드 (TAirportData 벡터에서)
    void loadAirportsFromHashTable(const vector<TAirportData>& airportData);

    // 화면에 표시할 공항 필터링
    vector<Airport> getVisibleAirports(
        double viewMinLat, double viewMaxLat,
        double viewMinLon, double viewMaxLon,
        int zoomLevel
    );

    // 공항 검색 기능
    Airport* findAirport(const string& icao);

    // 아이콘 크기 결정
    int getAirportIconSize(const Airport& airport, int zoomLevel);
};

#endif // AIRPORTDATAMANAGER_H 