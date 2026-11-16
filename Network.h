#pragma once

#include <iostream>
#include <unordered_map>
#include <vector>

#include "CompressorStation.h"
#include "Pipe.h"

struct Connection {
    int pipeId;
    int inputStationId;
    int outputStationId;
};

class GasNetwork {
private:
    std::unordered_map<int, Connection> connections;

public:
    bool connect(int pipeId, int inputStationId, int outputStationId);
    bool disconnect(int pipeId);
    bool isPipeUsed(int pipeId) const;
    bool isStationUsed(int stationId) const;
    const std::unordered_map<int, Connection>& getConnections() const;

    void print(const std::unordered_map<int, Pipe>& pipes,
               const std::unordered_map<int, CompressorStation>& stations) const;

    std::vector<int> topologicalSort(
        const std::unordered_map<int, CompressorStation>& stations,
        bool& hasCycle) const;

    friend std::ostream& operator<<(std::ostream& out,
                                    const GasNetwork& network);
    friend std::istream& operator>>(std::istream& in,
                                    GasNetwork& network);
};
