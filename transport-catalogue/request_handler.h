#pragma once

#include <unordered_set>
#include "svg.h"
#include "transport_catalogue.h"
#include "map_renderer.h"

/*
 * Здесь можно было бы разместить код обработчика запросов к базе, содержащего логику, которую не
 * хотелось бы помещать ни в transport_catalogue, ни в json reader.
 *
 * В качестве источника для идей предлагаем взглянуть на нашу версию обработчика запросов.
 * Вы можете реализовать обработку запросов способом, который удобнее вам.
 *
 * Если вы затрудняетесь выбрать, что можно было бы поместить в этот файл,
 * можете оставить его пустым.
 */

// Класс RequestHandler играет роль Фасада, упрощающего взаимодействие JSON reader-а
// с другими подсистемами приложения.
// См. паттерн проектирования Фасад: https://ru.wikipedia.org/wiki/Фасад_(шаблон_проектирования)

class RequestHandler {
public:
    // MapRenderer понадобится в следующей части итогового проекта
    RequestHandler(ctg::catalogue::TransportCatalogue &db, renderer::MapRenderer &renderer,
                   const std::map<std::string, json::Node> &req_, std::ostream &out_);

    // Возвращает информацию о маршруте (запрос Bus)
//    std::optional<BusStat> GetBusStat(const std::string_view& bus_name) const;

    // Возвращает маршруты, проходящие через
//    const std::unordered_set<BusPtr>* GetBusesByStop(const std::string_view& stop_name) const;

    // Этот метод будет нужен в следующей части итогового проекта
    svg::Document RenderMap();
    void ProcessingRequests();
protected:
    std::vector<ctg::geo::Coordinates> GetAllStopsCoordsInBuses() const;
    std::map<std::string_view, ctg::catalogue::Stop*> GetSortedStops() const;
    void ProcessingBaseRequests();
    void ProcessingStatRequests();
    void FillCatalogue(const json::Array &base_requests_array);
    void FormingOutput(const json::Array &stats);
    void Print(const std::vector<json::Node>& answer);
private:
    // RequestHandler использует агрегацию объектов "Транспортный Справочник" и "Визуализатор Карты"
    ctg::catalogue::TransportCatalogue& db_;
    renderer::MapRenderer& renderer_;
    const std::map<std::string, json::Node>& requests;
    std::ostream& out;
    json::VisitNode visitNode;
};
