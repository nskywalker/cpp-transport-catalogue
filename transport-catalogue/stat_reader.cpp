#include "stat_reader.h"

using namespace ctg::catalogue;

void ctg::statistics::ReadRequestsForOutput(ctg::catalogue::TransportCatalogue &catalogue, std::istream &in, std::ostream &os) {
    int number;
    in >> number;
    std::string line;
    for (int i = 0; i < number; ++i) {
        std::getline(in, line);
        if (line.empty()) {
            --i;
            continue;
        }
        std::string_view str = line;
        if (str.substr(0, 3) == "Bus") {
            PrintRouteData(os, str, catalogue);
        }
        else {
            PrintStopData(os, str, catalogue);
        }
    }
}

void ctg::statistics::PrintRouteData(std::ostream &os, std::string_view str,
                                     ctg::catalogue::TransportCatalogue &catalogue) {
    str.remove_prefix(4);
    auto bus_info = catalogue.GetBusInfo(str);
    if (!bus_info.has_value()) {
        os << "Bus " << str << ": not found\n";
        return;
    }
    os << std::setprecision(6);
    os << "Bus " << str << ": " << bus_info->number_of_stops << " stops on route, "
       << bus_info->number_of_unique_stops << " unique stops, "
       << bus_info->route_length << " route length, " << bus_info->curvature << " curvature\n";
}

void ctg::statistics::PrintStopData(std::ostream &os, std::string_view str, ctg::catalogue::TransportCatalogue &catalogue) {
    str.remove_prefix(5);
    if (!catalogue.FindStop(str)) {
        os << "Stop " << str << ": not found\n";
        return;
    }
    auto stop_in_buses = catalogue.GetStopInBuses(str);
    if (!stop_in_buses) {
        os << "Stop " << str << ": no buses\n";
    }
    else {
        os << "Stop " << str << ": buses ";
        for (const auto& cur_bus : *stop_in_buses) {
            os << cur_bus << ' ';
        }
        os << '\n';
    }
}