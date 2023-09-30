// напишите решение с нуля
// код сохраните в свой git-репозиторий

#include <algorithm>
#include "transport_catalogue.h"
#include "unordered_set"

using namespace ctg::catalogue;

void TransportCatalogue::AddStop(const std::string &stop, const std::optional<ctg::geo::Coordinates> &stop_coords) {
    if (stopname_to_stop.find(stop) == stopname_to_stop.end()) {
        stops.push_back({stop, stop_coords});
        stopname_to_stop[stops.back().name] = &stops.back();
        return;
    }
    if (stop_coords.has_value()) {
        stopname_to_stop[stop]->coords = stop_coords;
    }
}

void TransportCatalogue::AddBus(const std::string &bus) {
    buses.push_back(bus);
}

void TransportCatalogue::AddStopToLastBus(const std::string &stop) {
    if (!routes[buses.back()].empty()) {
        if (stops_to_dist.find({routes[buses.back()].back(), stopname_to_stop[stop]}) != stops_to_dist.end()) {
            bus_dist[buses.back()] += stops_to_dist[{routes[buses.back()].back(), stopname_to_stop[stop]}];
            forward_bus_dist[buses.back()] += geo::ComputeDistance(stopname_to_stop[stop]->coords.value(),
                                                                   routes[buses.back()].back()->coords.value());
        }
        else if (stops_to_dist.find({stopname_to_stop[stop], routes[buses.back()].back()}) != stops_to_dist.end()) {
            bus_dist[buses.back()] += stops_to_dist[{stopname_to_stop[stop], routes[buses.back()].back()}];
            forward_bus_dist[buses.back()] += geo::ComputeDistance(stopname_to_stop[stop]->coords.value(),
                                                                   routes[buses.back()].back()->coords.value());
        }
        else {
            bus_dist[buses.back()] += geo::ComputeDistance(stopname_to_stop[stop]->coords.value(),
                                                           routes[buses.back()].back()->coords.value());
            forward_bus_dist[buses.back()] += bus_dist[buses.back()];
        }
    }
    routes[buses.back()].push_back(stopname_to_stop[stop]);
    stop_to_bus[stopname_to_stop[stop]].insert(buses.back());
}

std::optional<BusInfo> TransportCatalogue::GetBusInfo(std::string_view bus) {
    if (routes.count(bus) == 0) {
        return std::nullopt;
    }
    return std::make_optional<BusInfo>({routes[bus].size(),
                                        std::unordered_set<Stop *>(routes[bus].begin(), routes[bus].end()).size(),
                                        bus_dist[bus], bus_dist[bus] / forward_bus_dist[bus]});
}

const std::vector<Stop *>& TransportCatalogue::FindLastRoute() const {
    return routes.at(buses.back());
}

const Stop *TransportCatalogue::FindStop(std::string_view stop) const {
    if (stopname_to_stop.find(stop) != stopname_to_stop.end()) {
        return stopname_to_stop.at(stop);
    }
    return nullptr;
}


void TransportCatalogue::SetDistStops(std::string_view from, std::string_view to, double dist) {
    stops_to_dist[{stopname_to_stop[from], stopname_to_stop[to]}] = dist;
}

const std::set<std::string_view> * TransportCatalogue::GetStopInBuses(std::string_view stop) const {
    if (stop_to_bus.find(stopname_to_stop.at(stop)) == stop_to_bus.end()) {
        return nullptr;
    }
    return &stop_to_bus.at(stopname_to_stop.at(stop));
}

const std::unordered_map<std::string_view, std::vector<Stop *>> &TransportCatalogue::GetAllRoutes() const {
    return routes;
}

void TransportCatalogue::SetRoundTripLastRoute(bool is_roundtrip) {
    bus_roundtrip[buses.back()] = is_roundtrip;
}

bool TransportCatalogue::GetRoundTripRoute(std::string_view bus) const {
    return bus_roundtrip.at(bus);
}

const std::unordered_map<std::string_view, Stop *> &TransportCatalogue::GetAllStops() const {
    return stopname_to_stop;
}
