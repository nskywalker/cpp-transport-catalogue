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
    graph::DirectedWeightedGraph<ctg::catalogue::EdgeInfo> BuildGraph();
public:
    TransportRoute(const ctg::catalogue::TransportCatalogue& catalogue_, const json::Dict &settings_);
    std::optional<graph::Router<ctg::catalogue::EdgeInfo>::RouteInfo> GetShortRoute(std::string_view from, std::string_view to) const;
    graph::Edge<ctg::catalogue::EdgeInfo> GetEdge(graph::EdgeId id) const;
    std::string_view GetStopNameThroughNum(size_t num) const;
};

