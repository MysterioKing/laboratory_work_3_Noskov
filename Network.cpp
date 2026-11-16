#include "Network.h"

#include <functional>
#include <queue>

bool GasNetwork::connect(int pipeId, int inputStationId,
                         int outputStationId) {
    if (pipeId <= 0 || inputStationId <= 0 || outputStationId <= 0 ||
        inputStationId == outputStationId || isPipeUsed(pipeId)) {
        return false;
    }

    connections.emplace(pipeId,
        Connection{pipeId, inputStationId, outputStationId});
    return true;
}

bool GasNetwork::disconnect(int pipeId) {
    return connections.erase(pipeId) > 0;
}

bool GasNetwork::isPipeUsed(int pipeId) const {
    return connections.find(pipeId) != connections.end();
}

bool GasNetwork::isStationUsed(int stationId) const {
    for (const auto& item : connections) {
        const Connection& connection = item.second;
        if (connection.inputStationId == stationId ||
            connection.outputStationId == stationId) {
            return true;
        }
    }
    return false;
}

const std::unordered_map<int, Connection>&
GasNetwork::getConnections() const {
    return connections;
}

void GasNetwork::print(
    const std::unordered_map<int, Pipe>& pipes,
    const std::unordered_map<int, CompressorStation>& stations) const {
    if (connections.empty()) {
        std::cout << "The network has no connections.\n";
        return;
    }

    std::cout << "\nGas transportation network:\n";
    for (const auto& item : connections) {
        const Connection& connection = item.second;
        const Pipe& pipe = pipes.at(connection.pipeId);
        const CompressorStation& input = stations.at(connection.inputStationId);
        const CompressorStation& output = stations.at(connection.outputStationId);

        std::cout << "Station " << input.getId() << " (" << input.getName()
                  << ") -- pipe " << pipe.getId()
                  << ", diameter " << pipe.getDiameter()
                  << " mm --> station " << output.getId()
                  << " (" << output.getName() << ")\n";
    }
}

std::vector<int> GasNetwork::topologicalSort(
    const std::unordered_map<int, CompressorStation>& stations,
    bool& hasCycle) const {
    std::unordered_map<int, int> indegree;
    std::unordered_map<int, std::vector<int>> graph;

    for (const auto& station : stations)
        indegree[station.first] = 0;

    for (const auto& item : connections) {
        const Connection& connection = item.second;
        graph[connection.inputStationId].push_back(connection.outputStationId);
        indegree[connection.outputStationId]++;
    }

    std::priority_queue<int, std::vector<int>, std::greater<int>> ready;
    for (const auto& item : indegree)
        if (item.second == 0) ready.push(item.first);

    std::vector<int> result;
    while (!ready.empty()) {
        int stationId = ready.top();
        ready.pop();
        result.push_back(stationId);

        for (int nextStationId : graph[stationId]) {
            indegree[nextStationId]--;
            if (indegree[nextStationId] == 0)
                ready.push(nextStationId);
        }
    }

    hasCycle = result.size() != stations.size();
    return result;
}

std::ostream& operator<<(std::ostream& out, const GasNetwork& network) {
    out << network.connections.size() << '\n';
    for (const auto& item : network.connections) {
        const Connection& connection = item.second;
        out << connection.pipeId << ' '
            << connection.inputStationId << ' '
            << connection.outputStationId << '\n';
    }
    return out;
}

std::istream& operator>>(std::istream& in, GasNetwork& network) {
    int count;
    if (!(in >> count)) return in;
    if (count < 0) {
        in.setstate(std::ios::failbit);
        return in;
    }

    GasNetwork loadedNetwork;
    for (int i = 0; i < count; i++) {
        int pipeId, inputStationId, outputStationId;
        if (!(in >> pipeId >> inputStationId >> outputStationId) ||
            !loadedNetwork.connect(pipeId, inputStationId, outputStationId)) {
            in.setstate(std::ios::failbit);
            return in;
        }
    }

    network = loadedNetwork;
    return in;
}
