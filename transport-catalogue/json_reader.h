#pragma once
#include <iostream>
#include "transport_catalogue.h"
#include "json.h"
#include <memory>
#include "map_renderer.h"

/*
 * Здесь можно разместить код наполнения транспортного справочника данными из JSON,
 * а также код обработки запросов к базе и формирование массива ответов в формате JSON
 */



class JsonReader final {
    ctg::catalogue::TransportCatalogue& db_;
    std::istream& input;
    std::ostream& out;
    std::unique_ptr<json::Document> requests;
    std::unique_ptr<renderer::MapRenderer> rend;
    json::VisitNode visitNode;
protected:
    void FillCatalogue(const json::Array &base_requests_array);
    void FormingOutput(const json::Array &stats);
    void ReadStatRequests();
    void ReadBaseRequests();
    void Print(const std::vector<json::Node>& answer);
public:
    explicit JsonReader(ctg::catalogue::TransportCatalogue &db, std::istream &in, std::ostream& out_);
    ~JsonReader() = default;
    void ProcessingRequest();
};