#include "stat_reader.h"

using namespace ctg::catalogue;

void ctg::statistics::Calculate_statistics(ctg::catalogue::TransportCatalogue &catalogue, std::istream &in, std::ostream &os) {
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
            Print_route_data(os, str, catalogue);
        }
        else {
            Print_stop_data(os, str, catalogue);
        }
    }
}

void ctg::statistics::Print_route_data(std::ostream &os, std::string_view str,
                                       ctg::catalogue::TransportCatalogue &catalogue) {
    str.remove_prefix(4);
    os << catalogue.GetBusInfo(str);
}

void ctg::statistics::Print_stop_data(std::ostream &os, std::string_view str, ctg::catalogue::TransportCatalogue &catalogue) {
    str.remove_prefix(5);
    os << catalogue.GetStopInfo(str);
}