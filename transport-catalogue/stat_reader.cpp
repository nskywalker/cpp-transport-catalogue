#include "stat_reader.h"

using namespace ctg::catalogue;

void ctg::statistics::stat_reader(TransportCatalogue &catalogue, std::istream &in, std::ostream &os) {
    int number;
    in >> number;
    std::string line;
    for (int i = 0; i < number; ++i) {
        std::getline(in, line);
        if (line.empty()) {
            i -= 1;
            continue;
        }
        std::string_view str = line;
        if (str.substr(0, 3) == "Bus") {
            str.remove_prefix(4);
            catalogue.GetBusInfo(str, os);
        }
        else {
            str.remove_prefix(5);
            catalogue.GetStopInfo(str, os);
        }
    }
}