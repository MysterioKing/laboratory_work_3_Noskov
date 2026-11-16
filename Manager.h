#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "CompressorStation.h"
#include "Network.h"
#include "Pipe.h"

class Manager {
private:
    std::unordered_map<int, Pipe> pipes;
    std::unordered_map<int, CompressorStation> stations;
    GasNetwork network;
    int nextPipeId = 1;
    int nextStationId = 1;

    void printMenu() const;
    int readDiameter() const;
    int createPipe(int fixedDiameter = 0);
    void addPipe();
    void addStation();
    void showAll() const;

    void workWithPipes();
    void workWithStations();
    std::vector<int> findPipes() const;
    std::vector<int> findStations() const;
    std::vector<int> selectObjects(const std::vector<int>& foundIds,
                                   const std::string& objectName) const;
    void processFoundPipes(const std::vector<int>& foundIds);
    void processFoundStations(const std::vector<int>& foundIds);

    void connectStations();
    void disconnectPipe();
    void showNetwork() const;
    void showTopologicalSort() const;

    void save() const;
    void load();

public:
    void run();
};
