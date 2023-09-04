#pragma once
#include "transport_catalogue.h"
#include <iostream>

namespace ctg::statistics {
    void Calculate_statistics(ctg::catalogue::TransportCatalogue &catalogue, std::istream &in, std::ostream &os);

    inline void Print_route_data(std::ostream &os, std::string_view str, ctg::catalogue::TransportCatalogue &catalogue);

    inline void Print_stop_data(std::ostream &os, std::string_view str, ctg::catalogue::TransportCatalogue &catalogue);
}