#include "transport_router.h"

using namespace graph;

using namespace ctg::catalogue;

graph::DirectedWeightedGraph<EdgeInfo> TransportRoute::BuildGraph() {
    graph::DirectedWeightedGraph<EdgeInfo> result(stops_to_num.size());
//    const auto speed = settings.at(tor::bus_velocity).AsDouble() / 3.6;
    for (const auto& [bus, stops] : catalogue.GetAllRoutes()) {
        auto end = catalogue.GetRoundTripRoute(bus) ? stops.end() : stops.begin() + static_cast<int>(stops.size() / 2) + 1;
        for (auto stop = stops.begin(); stop != end; ++stop) {
            for (auto cur_stop = catalogue.GetRoundTripRoute(bus) ? stop + 1 : stops.begin(); cur_stop != end; ++cur_stop) {
                auto edge_time = (
                        catalogue.FindStopsDistInBus(stop - stops.begin(), cur_stop - stops.begin(), bus) / (speed / 3.6) / 60
                        + bus_wait_time);
                EdgeInfo info = {static_cast<int>(abs(stop - cur_stop)), edge_time, bus};
                Edge<EdgeInfo> edge = {stops_to_num[(*stop)->name], stops_to_num[(*cur_stop)->name], info};
                result.AddEdge(edge);
            }
        }
    }
    return result;
}

TransportRoute::TransportRoute(const ctg::catalogue::TransportCatalogue &catalogue_, double bus_wait_time_, double speed_)
: catalogue(catalogue_), bus_wait_time(bus_wait_time_), speed(speed_) {
    VertexId num = 0;
    for (const auto&[stop_name, stop_struct] : catalogue.GetAllStops()) {
        num_to_stops[num] = stop_name;
        stops_to_num[stop_name] = num++;
    }
    graph = std::make_unique<DirectedWeightedGraph<EdgeInfo>>(BuildGraph());
    route = std::make_unique<Router<EdgeInfo>>(*graph);
}

std::optional<ctg::catalogue::RouteInfo> TransportRoute::GetShortRoute(std::string_view from, std::string_view to) const {
    if (stops_to_num.count(from) == 0 or stops_to_num.count(to) == 0) {
        return std::nullopt;
    }
    auto short_route = route->BuildRoute(stops_to_num.at(from), stops_to_num.at(to));
    if (!short_route) {
        return std::nullopt;
    }
    ctg::catalogue::RouteInfo result;
    result.bus_wait_time = bus_wait_time;
    result.total_time = short_route->weight.time;
    result.drive_info.reserve(short_route->edges.size());
    for (const auto& id : short_route->edges) {
        auto edge = graph->GetEdge(id);
        auto& temp = result.drive_info.emplace_back();
        temp.wait_stop = num_to_stops.at(edge.from);
        temp.bus = edge.weight.bus;
        temp.stops_count = edge.weight.stops_count;
        temp.time_driving = edge.weight.time - bus_wait_time;
    }
    return result;
}

const graph::DirectedWeightedGraph<ctg::catalogue::EdgeInfo> &TransportRoute::GetGraph() const {
    return *graph;
}

const graph::Router<ctg::catalogue::EdgeInfo> &TransportRoute::GetRoute() const {
    return *route;
}

const std::unordered_map<graph::VertexId, std::string_view> &TransportRoute::GetNumToStops() const {
    return num_to_stops;
}

const std::unordered_map<std::string_view, graph::VertexId> &TransportRoute::GetStopsToNum() const {
    return stops_to_num;
}

TransportRoute::TransportRoute(const ctg::catalogue::TransportCatalogue &catalogue_, double bus_wait_time_, double speed_,
                               std::unordered_map<graph::VertexId, std::string_view> num_to_stops_,
                               std::unordered_map<std::string_view, graph::VertexId> stops_to_num_,
                               std::unique_ptr<graph::DirectedWeightedGraph<ctg::catalogue::EdgeInfo>> graph_,
                               graph::Router<ctg::catalogue::EdgeInfo>::RoutesInternalData routes_internal_data) : catalogue(catalogue_),
                                                                                                                   bus_wait_time(bus_wait_time_), speed(speed_), num_to_stops(std::move(num_to_stops_)),
                                                                                                                   stops_to_num(std::move(stops_to_num_)), graph(std::move(graph_)), route(std::make_unique<Router<ctg::catalogue::EdgeInfo>>(*graph, std::move(routes_internal_data))){

}

