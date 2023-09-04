#include <queue>
#include "input_reader.h"
#include <optional>

using namespace ctg::catalogue;

namespace ctg::input {

    void AddBus(TransportCatalogue &catalogue, std::string_view str, char divisor, const std::string &bus) {
        size_t pos = str.find(divisor);
        for (; pos != std::string_view::npos; pos = str.find(divisor)) {
            catalogue.AddStopToBus(bus, std::string{str.substr(0, pos - 1)});
            str.remove_prefix(pos + 2);
        }
        catalogue.AddStopToBus(bus, std::string{str});
        if (divisor == '-') {
            for (int i = static_cast<int>(catalogue.FindRoute(bus).size() - 2); i >= 0; --i) {
                catalogue.AddStopToBus(bus, catalogue.FindRoute(bus)[i]->name);
            }
        }

    }

    void Fill_catalogue(ctg::catalogue::TransportCatalogue &catalogue, std::istream &in) {
        int number = 0;
        in >> number;
        std::string line;
        std::queue<std::string> queue, queue_stop;
        for (int i = 0; i < number; ++i) {
            std::getline(in, line);
            if (line.empty()) {
                --i;
                continue;
            }
            if (line.find(" to ") != std::string::npos) {
                queue_stop.push(line);
                continue;
            }
            if (line.substr(0, 4) == "Stop") {
                std::string_view str = line;
                str.remove_prefix(5);
                size_t pos1 = str.find(':');
                std::string_view needed_stop = str.substr(0, pos1);
                str.remove_prefix(pos1 + 1);
                pos1 = str.find(',');
                double latitude = std::stod(std::string{str.substr(0, pos1)});
                str.remove_prefix(pos1 + 1);
                double longitude = std::stod(std::string{str});
                catalogue.AddStop(std::string{needed_stop},
                                  std::make_optional<ctg::coord::Coordinates>({latitude, longitude}));
            } else {
                queue.push(line);
            }
        }
        while (!queue_stop.empty()) {
            std::string_view str = queue_stop.front();
            str.remove_prefix(5);
            size_t pos1 = str.find(':');
            std::string_view needed_stop = str.substr(0, pos1);
            str.remove_prefix(pos1 + 1);
            pos1 = str.find(',');
            double latitude = std::stod(std::string{str.substr(0, pos1)});
            str.remove_prefix(pos1 + 1);
            pos1 = str.find(',');
            double longitude = std::stod(std::string{str.substr(0, pos1)});
            str.remove_prefix(pos1 + 2);
            catalogue.AddStop(std::string{needed_stop},
                              std::make_optional<ctg::coord::Coordinates>({latitude, longitude}));
            CalcStopDist(catalogue, str, catalogue.FindStop(needed_stop)->name);
            queue_stop.pop();
        }
        while (!queue.empty()) {
            std::string_view str = queue.front();
            str.remove_prefix(4);
            size_t pos1 = str.find(':');
            std::string_view needed_bus = str.substr(0, pos1);
            str.remove_prefix(pos1 + 2);
            pos1 = str.find('-');
            auto &cut_bus = catalogue.AddAndGetBus(std::string{needed_bus});
            if (pos1 != std::string_view::npos) {
                catalogue.AddStopToBus(cut_bus, std::string{str.substr(0, pos1 - 1)});
                str.remove_prefix(pos1 + 2);
                AddBus(catalogue, str, '-', cut_bus);
            } else {
                AddBus(catalogue, str, '>', cut_bus);
            }
            queue.pop();
        }
    }

    void CalcStopDist(TransportCatalogue &catalogue, std::string_view str, const std::string &stop) {
        size_t pos = str.find('m');
        for (; pos != std::string_view::npos; pos = str.find('m')) {
            double dist = std::stod(std::string{str.substr(0, pos)});
            str.remove_prefix(pos + 5);
            pos = str.find(',');
            std::string_view need_stop = str.substr(0, pos);
            catalogue.AddStop(std::string{need_stop}, std::nullopt);
            catalogue.SetDistStops(stop, need_stop, dist);
            if (str.find(',') == std::string_view::npos) {
                break;
            }
            str.remove_prefix(pos + 2);
        }
    }

}