#pragma once
#include <string>
#include <vector>
#include <deque>
#include <unordered_map>
#include <set>
#include <iostream>
#include "geo.h"
#include <iomanip>
#include <optional>
#include <sstream>

namespace ctg::catalogue {

    struct Stop {
        std::string name;
        std::optional<ctg::coord::Coordinates> coords;
    };

    struct BusInfo {
        size_t number_of_stops;
        size_t number_of_unique_stops;
        double route_length;
        double curvature;
    };

    struct HashStopPair {
        size_t operator()(const std::pair<Stop *, Stop *> &stop_pair) const {
            return hasher(stop_pair.first) * 37 * 37 + hasher(stop_pair.second);
        }

    private:
        std::hash<const void *> hasher;
    };

    class TransportCatalogue {
    public:
        void AddStop(const std::string &stop, const std::optional<ctg::coord::Coordinates> &stop_coords);

        void AddBus(const std::string &bus);

        void AddStopToLastBus(const std::string &stop);

        std::optional<BusInfo> GetBusInfo(std::string_view bus);

        const std::vector<Stop *> &FindLastRoute() const;

        const Stop *FindStop(std::string_view stop) const;

        void SetDistStops(std::string_view from, std::string_view to, double dist);

        const std::set<std::string_view> * GetStopInBuses(std::string_view stop) const;

    private:
        std::deque<Stop> stops;
        std::deque<std::string> buses;
        std::unordered_map<std::string_view, Stop *> stopname_to_stop;
        std::unordered_map<std::string_view, std::vector<Stop *>> routes;
        std::unordered_map<std::string_view, double> bus_dist;
        std::unordered_map<std::string_view, double> forward_bus_dist;
        std::unordered_map<Stop *, std::set<std::string_view>> stop_to_bus;
        std::unordered_map<std::pair<Stop *, Stop *>, double, HashStopPair> stops_to_dist;
    };

}