// напишите решение с нуля
// код сохраните в свой git-репозиторий

#include <algorithm>
#include "transport_catalogue.h"
#include "unordered_set"

using namespace ctg::catalogue;

void TransportCatalogue::AddStop(const std::string &stop, std::optional<double> latitude, std::optional<double> longitude) {
    if (stopname_to_stop.find(stop) == stopname_to_stop.end()) {
        stops.push_back({stop, latitude, longitude});
        stopname_to_stop[stops.back().name] = &stops.back();
        return;
    }
    if (longitude.has_value() and latitude.has_value()) {
        stopname_to_stop[stop]->latitude = latitude;
        stopname_to_stop[stop]->longitude = longitude;
    }
}

const std::string& TransportCatalogue::AddBus(const std::string &bus) {
    buses.push_back(bus);
    return buses.back();
}

void TransportCatalogue::AddStopToBus(const std::string &bus, const std::string &stop) {
    if (!routes[bus].empty()) {
        if (stops_to_dist.find({routes[bus].back(), stopname_to_stop[stop]}) != stops_to_dist.end()) {
            bus_dist[bus] += stops_to_dist[{routes[bus].back(), stopname_to_stop[stop]}];
            forward_bus_dist[bus] += coord::ComputeDistance({stopname_to_stop[stop]->latitude.value(), stopname_to_stop[stop]->longitude.value()},
                                                     {routes[bus].back()->latitude.value(), routes[bus].back()->longitude.value()});
        }
        else if (stops_to_dist.find({stopname_to_stop[stop], routes[bus].back()}) != stops_to_dist.end()) {
            bus_dist[bus] += stops_to_dist[{stopname_to_stop[stop], routes[bus].back()}];
            forward_bus_dist[bus] += coord::ComputeDistance({stopname_to_stop[stop]->latitude.value(), stopname_to_stop[stop]->longitude.value()},
                                                     {routes[bus].back()->latitude.value(), routes[bus].back()->longitude.value()});
        }
        else {
            bus_dist[bus] += coord::ComputeDistance({stopname_to_stop[stop]->latitude.value(), stopname_to_stop[stop]->longitude.value()},
                                             {routes[bus].back()->latitude.value(), routes[bus].back()->longitude.value()});
            forward_bus_dist[bus] += bus_dist[bus];
        }
    }
    routes[bus].push_back(stopname_to_stop[stop]);
    stop_to_bus[stopname_to_stop[stop]].insert(bus);
}

void TransportCatalogue::GetBusInfo(std::string_view bus, std::ostream &os) {
    if (routes.count(bus) == 0) {
        os << "Bus " << bus << ": not found\n";
        return;
    }
    os << std::setprecision(6);
    os << "Bus " << bus << ": " << routes[bus].size() << " stops on route, "
    << std::unordered_set<Stop*>(routes[bus].begin(), routes[bus].end()).size() << " unique stops, "
    << bus_dist[bus] << " route length, " << bus_dist[bus] / forward_bus_dist[bus] << " curvature\n";
}

const std::vector<Stop *>& TransportCatalogue::FindBus(std::string_view bus) const {
    return routes.at(bus);
}

const Stop *TransportCatalogue::FindStop(std::string_view stop) const {
    if (stopname_to_stop.find(stop) != stopname_to_stop.end()) {
        return stopname_to_stop.at(stop);
    }
    return nullptr;
}

void TransportCatalogue::GetStopInfo(std::string_view stop, std::ostream &os) const {
    if (stopname_to_stop.find(stop) == stopname_to_stop.end()) {
        os << "Stop " << stop << ": not found\n";
    }
    else if (stop_to_bus.find(stopname_to_stop.at(stop)) == stop_to_bus.end()) {
        os << "Stop " << stop << ": no buses\n";
    }
    else {
        os << "Stop " << stop << ": buses ";
        for (const auto& cur_bus : stop_to_bus.at(stopname_to_stop.at(stop))) {
            os << cur_bus << ' ';
        }
        os << '\n';
    }
}

void TransportCatalogue::SetDistStops(std::string_view from, std::string_view to, double dist) {
    stops_to_dist[{stopname_to_stop[from], stopname_to_stop[to]}] = dist;
}
