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
#include <unordered_set>
#include "domain.h"

namespace ctg::catalogue {

    class TransportCatalogue {
    public:
        void AddStop(const std::string &stop, const std::optional<ctg::geo::Coordinates> &stop_coords);

        void AddBus(const std::string &bus);

        void AddStopToLastBus(const std::string &stop);

        std::optional<BusInfo> GetBusInfo(std::string_view bus);

        const std::vector<Stop *> &FindLastRoute() const;

        const Stop *FindStop(std::string_view stop) const;

        void SetDistStops(std::string_view from, std::string_view to, double dist);

        const std::set<std::string_view> * GetStopInBuses(std::string_view stop) const;

        const std::unordered_map<std::string_view, std::vector<Stop *>>& GetAllRoutes() const;

        void SetRoundTripLastRoute(bool is_roundtrip);

        bool GetRoundTripRoute(std::string_view bus) const;

        const std::unordered_map<std::string_view, Stop *>& GetAllStops() const;

        const std::unordered_map<std::pair<Stop *, Stop *>, double, HashStopPair>& GetAllDist() const;

        double FindStopsDist(std::string_view from, std::string_view to) const;

        double FindStopsDistInBus(size_t from, size_t to, std::string_view bus) const;

    private:
        std::deque<Stop> stops;
        std::deque<std::string> buses;
        std::unordered_map<std::string_view, Stop *> stopname_to_stop;
        std::unordered_map<std::string_view, std::vector<Stop *>> routes;
        std::unordered_map<std::string_view, double> bus_dist;
        std::unordered_map<std::string_view, double> forward_bus_dist;
        std::unordered_map<Stop *, std::set<std::string_view>> stop_to_bus;
        std::unordered_map<std::pair<Stop *, Stop *>, double, HashStopPair> stops_to_dist;
        std::unordered_map<std::string_view, bool> bus_roundtrip;
    };

}