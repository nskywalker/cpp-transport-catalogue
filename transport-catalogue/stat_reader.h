#pragma once
#include "transport_catalogue.h"
#include <iostream>

namespace ctg::statistics {
    void ReadRequestsForOutput(ctg::catalogue::TransportCatalogue &catalogue, std::istream &in, std::ostream &os);

    inline void PrintRouteData(std::ostream &os, std::string_view str, ctg::catalogue::TransportCatalogue &catalogue);

    inline void PrintStopData(std::ostream &os, std::string_view str, ctg::catalogue::TransportCatalogue &catalogue);
}