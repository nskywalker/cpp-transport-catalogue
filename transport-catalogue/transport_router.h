#pragma once

#include "transport_catalogue.h"
#include "graph.h"
#include "router.h"
#include "json_builder.h"
#include <memory>

class TransportRoute {
    const ctg::catalogue::TransportCatalogue& catalogue;
    const json::Dict &settings;
    double bus_wait_time;
    std::unordered_map<graph::VertexId, std::string_view> num_to_stops;
    std::unordered_map<std::string_view, graph::VertexId> stops_to_num;
    std::unique_ptr<graph::DirectedWeightedGraph<ctg::catalogue::EdgeInfo>> graph;
    std::unique_ptr<graph::Router<ctg::catalogue::EdgeInfo>> route;
protected:
    void WriteRouteToItems(json::Array &items, std::string_view bus, int count, double time) const;
    void AddStopToBusWait(std::string_view stop, json::Array &items) const;
    static json::Dict ErrorMessage(const json::Node& id);
    static json::Dict GetAnswer(double time, const json::Node& id, const json::Array& items);
    graph::DirectedWeightedGraph<ctg::catalogue::EdgeInfo> BuildGraph();
public:
    TransportRoute(const ctg::catalogue::TransportCatalogue& catalogue_, const json::Dict &settings_);
    json::Dict GetShortRoute(std::string_view from, std::string_view to, const json::Node &id) const;
};

