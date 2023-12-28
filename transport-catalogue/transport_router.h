#pragma once

#include "transport_catalogue.h"
#include "graph.h"
#include "router.h"
#include "json_builder.h"
#include <memory>

class TransportRoute {
    const ctg::catalogue::TransportCatalogue& catalogue;
    double bus_wait_time;
    double speed;
    std::unordered_map<graph::VertexId, std::string_view> num_to_stops;
    std::unordered_map<std::string_view, graph::VertexId> stops_to_num;
    std::unique_ptr<graph::DirectedWeightedGraph<ctg::catalogue::EdgeInfo>> graph;
    std::unique_ptr<graph::Router<ctg::catalogue::EdgeInfo>> route;
protected:
    graph::DirectedWeightedGraph<ctg::catalogue::EdgeInfo> BuildGraph();
public:
    TransportRoute(const ctg::catalogue::TransportCatalogue &catalogue_, double bus_wait_time_, double speed_,
                   std::unordered_map<graph::VertexId, std::string_view> num_to_stops_,
                   std::unordered_map<std::string_view, graph::VertexId> stops_to_num_,
                   std::unique_ptr<graph::DirectedWeightedGraph<ctg::catalogue::EdgeInfo>> graph_,
                   graph::Router<ctg::catalogue::EdgeInfo>::RoutesInternalData routes_internal_data);
    TransportRoute(const ctg::catalogue::TransportCatalogue &catalogue_, double bus_wait_time_, double speed_);
    std::optional<ctg::catalogue::RouteInfo> GetShortRoute(std::string_view from, std::string_view to) const;
    const graph::DirectedWeightedGraph<ctg::catalogue::EdgeInfo>& GetGraph() const;
    const graph::Router<ctg::catalogue::EdgeInfo>& GetRoute() const;
    const std::unordered_map<graph::VertexId, std::string_view>& GetNumToStops() const;
    const std::unordered_map<std::string_view, graph::VertexId>& GetStopsToNum() const;
};

