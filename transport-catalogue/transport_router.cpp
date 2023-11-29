#include "transport_router.h"

using namespace graph;

using namespace ctg::catalogue;

graph::DirectedWeightedGraph<EdgeInfo> TransportRoute::BuildGraph() {
    graph::DirectedWeightedGraph<EdgeInfo> result(stops_to_num.size());
    const auto speed = settings.at(tor::bus_velocity).AsDouble() / 3.6;
    for (const auto& [bus, stops] : catalogue.GetAllRoutes()) {
        auto end = catalogue.GetRoundTripRoute(bus) ? stops.end() : stops.begin() + static_cast<int>(stops.size() / 2) + 1;
        for (auto stop = stops.begin(); stop != end; ++stop) {
            for (auto cur_stop = catalogue.GetRoundTripRoute(bus) ? stop + 1 : stops.begin(); cur_stop != end; ++cur_stop) {
                auto edge_time = (
                        catalogue.FindStopsDistInBus(stop - stops.begin(), cur_stop - stops.begin(), bus) / speed / 60
                        + bus_wait_time);
                EdgeInfo info = {static_cast<int>(abs(stop - cur_stop)), edge_time, bus};
                Edge<EdgeInfo> edge = {stops_to_num[(*stop)->name], stops_to_num[(*cur_stop)->name], info};
                result.AddEdge(edge);
            }
        }
    }
    return result;
}

TransportRoute::TransportRoute(const ctg::catalogue::TransportCatalogue &catalogue_, const json::Dict &settings_)
: catalogue(catalogue_), settings(settings_), bus_wait_time(settings_.at(tor::bus_wait_time).AsDouble()) {
    VertexId num = 0;
    for (const auto&[stop_name, stop_struct] : catalogue.GetAllStops()) {
        num_to_stops[num] = stop_name;
        stops_to_num[stop_name] = num++;
    }
    graph = std::make_unique<DirectedWeightedGraph<EdgeInfo>>(BuildGraph());
    route = std::make_unique<Router<EdgeInfo>>(*graph);
}

json::Dict TransportRoute::GetShortRoute(std::string_view from, std::string_view to, const json::Node &id) const {
    if (stops_to_num.count(from) == 0 or stops_to_num.count(to) == 0) {
        return ErrorMessage(id);
    }
    auto short_route = route->BuildRoute(stops_to_num.at(from), stops_to_num.at(to));
    if (!short_route) {
        return ErrorMessage(id);
    }
    json::Array items;
    for (const auto& edge_id : short_route->edges) {
        auto cur_edge = graph->GetEdge(edge_id);
        AddStopToBusWait(num_to_stops.at(cur_edge.from), items);
        WriteRouteToItems(items, cur_edge.weight.bus, cur_edge.weight.stops_count, cur_edge.weight.time - bus_wait_time);
    }
    return GetAnswer(short_route->weight.time, id, items);
}

void TransportRoute::WriteRouteToItems(json::Array &items, std::string_view bus, int count, double time) const {
    items.emplace_back(json::Builder{}.StartDict().Key(tor::bus).Value(std::string{bus})
                               .Key("span_count").Value(count)
                               .Key("time").Value(time).EndDict().Build().AsDict());
}

void TransportRoute::AddStopToBusWait(std::string_view stop, json::Array &items) const {
    items.emplace_back(json::Builder{}.StartDict().Key("stop_name").Value(std::string{stop})
            .Key("time").Value(settings.at(tor::bus_wait_time)).Key("type").Value("Wait")
            .EndDict().Build().AsDict());
}

json::Dict TransportRoute::ErrorMessage(const json::Node &id) {
    return json::Builder{}.StartDict().Key(tor::request_id).Value(id)
            .Key("error_message").Value("not found").EndDict().Build().AsDict();
}

json::Dict TransportRoute::GetAnswer(double time, const json::Node &id, const json::Array &items) {
    return json::Builder{}.StartDict().Key("total_time").Value(time)
            .Key(tor::request_id).Value(id).Key("items").Value(items).EndDict().Build().AsDict();
}
