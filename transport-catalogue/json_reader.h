#pragma once
#include <iostream>
#include "transport_catalogue.h"
#include "json.h"
#include <memory>

/*
 * Здесь можно разместить код наполнения транспортного справочника данными из JSON,
 * а также код обработки запросов к базе и формирование массива ответов в формате JSON
 */



class JsonReader final {
    ctg::catalogue::TransportCatalogue& db;
    std::istream& input;
    std::unique_ptr<json::Document> requests;
    json::VisitNode visitNode;
protected:
//    void FillCatalogue(const json::Array &base_requests_array);
//    void FormingOutput(const json::Array &stats);

public:
    const json::Dict & GetRenderSettings() const;
//    void ReadStatRequests();
//    void ReadBaseRequests();
    explicit JsonReader(ctg::catalogue::TransportCatalogue &db_, std::istream &in);
    ~JsonReader() = default;
    void ProcessingRequest();
    const std::map<std::string, json::Node>& GetAllRequests() const;
};