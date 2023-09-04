#pragma once
#include "transport_catalogue.h"
#include <iostream>
#include <sstream>
#include <algorithm>

namespace ctg::input {
    void AddBus(ctg::catalogue::TransportCatalogue &catalogue, std::string_view str, char divisor, const std::string &bus);

    void Fill_catalogue(ctg::catalogue::TransportCatalogue &catalogue, std::istream &in);

    void CalcStopDist(ctg::catalogue::TransportCatalogue &catalogue, std::string_view str, const std::string &stop);
}

