#pragma once
#include "transport_catalogue.h"
#include <iostream>
#include <sstream>
#include <algorithm>

namespace ctg::input {
    void AddStopsToLastBus(std::string_view str, ctg::catalogue::TransportCatalogue &catalogue);

    void AddRoute(ctg::catalogue::TransportCatalogue &catalogue, std::string_view str, char divisor);

    void FillCatalogue(ctg::catalogue::TransportCatalogue &catalogue, std::istream &in);

    void CalcStopDist(ctg::catalogue::TransportCatalogue &catalogue, std::string_view str, const std::string &stop);
}

