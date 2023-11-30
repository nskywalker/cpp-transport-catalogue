#include "domain.h"

/*
 * В этом файле вы можете разместить классы/структуры, которые являются частью предметной области
 * (domain) вашего приложения и не зависят от транспортного справочника. Например Автобусные
 * маршруты и Остановки.
 *
 * Их можно было бы разместить и в transport_catalogue.h, однако вынесение их в отдельный
 * заголовочный файл может оказаться полезным, когда дело дойдёт до визуализации карты маршрутов:
 * визуализатор карты (map_renderer) можно будет сделать независящим от транспортного справочника.
 *
 * Если структура вашего приложения не позволяет так сделать, просто оставьте этот файл пустым.
 *
 */
size_t ctg::catalogue::HashStopPair::operator()(const std::pair<Stop *, Stop *> &stop_pair) const {
    return hasher(stop_pair.first) * 37 * 37 + hasher(stop_pair.second);
}

bool ctg::catalogue::operator<(const EdgeInfo &lhs, const EdgeInfo &rhs) {
    return lhs.time < rhs.time;
}

bool ctg::catalogue::operator>(const EdgeInfo &lhs, const EdgeInfo &rhs) {
    return lhs.time > rhs.time;
}

ctg::catalogue::EdgeInfo ctg::catalogue::operator+(const EdgeInfo &lhs, const EdgeInfo &rhs) {
    return {lhs.stops_count + rhs.stops_count,lhs.time + rhs.time, {}};
}
