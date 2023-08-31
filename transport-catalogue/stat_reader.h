#pragma once
#include "transport_catalogue.h"
#include <iostream>

namespace ctg::statistics {
    void stat_reader(ctg::catalogue::TransportCatalogue &catalogue, std::istream &in, std::ostream &os);
}