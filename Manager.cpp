#include "Manager.h"

#include "Logger.h"
#include "Utils.h"

#include <cmath>
#include <fstream>
#include <iostream>

namespace {
bool containsId(const std::vector<int>& ids, int id) {
    for (int value : ids)
        if (value == id) return true;
    return false;
}

bool isAllowedDiameter(int diameter) {
    return diameter == 500 || diameter == 700 ||
           diameter == 1000 || diameter == 1400;
}
}

void Manager::printMenu() const {
    std::cout << "\n1. Add pipe\n"
              << "2. Add station\n"
              << "3. View all objects\n"
              << "4. Find and manage pipes\n"
              << "5. Find and manage stations\n"
              << "6. Connect stations\n"
              << "7. Disconnect pipe\n"
              << "8. View network\n"
              << "9. Topological sort\n"
              << "10. Save\n"
              << "11. Load\n"
              << "0. Exit\n";
}

int Manager::readDiameter() const {
    while (true) {
        int diameter = readValue<int>(
            "Diameter (500, 700, 1000 or 1400 mm): ", 500, 1400);

        if (isAllowedDiameter(diameter)) return diameter;
        std::cout << "Allowed diameters: 500, 700, 1000 and 1400 mm.\n";
    }
}

int Manager::createPipe(int fixedDiameter) {
    std::string name = readLine("Pipe name: ");
    double length = readValue<double>("Length (km): ", 0.01, 1000000.0);
    int diameter = fixedDiameter == 0 ? readDiameter() : fixedDiameter;
    bool repairing = false;

    if (fixedDiameter == 0)
        repairing = readValue<int>("Under repair? (0-no, 1-yes): ", 0, 1);

    int id = nextPipeId++;
    pipes.emplace(id, Pipe(id, name, length, diameter, repairing));
    std::cout << "Pipe added. ID: " << id << '\n';
    Logger::getInstance().write("Added pipe ID " + std::to_string(id));
    return id;
}

void Manager::addPipe() {
    createPipe();
}

void Manager::addStation() {
    std::string name = readLine("Station name: ");
    int workshops = readValue<int>("Number of workshops: ", 1, 10000);
    int working = readValue<int>("Working workshops: ", 0, workshops);
    int stationClass = readValue<int>("Station class: ", 0, 10000);

    int id = nextStationId++;
    stations.emplace(id, CompressorStation(
        id, name, workshops, working, stationClass));
    std::cout << "Station added. ID: " << id << '\n';
    Logger::getInstance().write("Added station ID " + std::to_string(id));
}

void Manager::showAll() const {
    if (pipes.empty()) std::cout << "\nNo pipes.\n";
    for (const auto& item : pipes) item.second.print();

    if (stations.empty()) std::cout << "\nNo stations.\n";
    for (const auto& item : stations) item.second.print();

    Logger::getInstance().write("Viewed all objects");
}

std::vector<int> Manager::findPipes() const {
    std::vector<int> foundIds;
    int filter = readValue<int>(
        "1. Search by name\n2. Search by repair status\nChoice: ", 1, 2);

    if (filter == 1) {
        std::string text = readLine("Part of name: ");
        for (const auto& item : pipes)
            if (item.second.getName().find(text) != std::string::npos)
                foundIds.push_back(item.first);
        Logger::getInstance().write("Searched pipes by name: " + text);
    } else {
        bool repairing = readValue<int>(
            "Under repair? (0-no, 1-yes): ", 0, 1);
        for (const auto& item : pipes)
            if (item.second.isRepairing() == repairing)
                foundIds.push_back(item.first);
        Logger::getInstance().write("Searched pipes by repair status");
    }

    return foundIds;
}

std::vector<int> Manager::findStations() const {
    std::vector<int> foundIds;
    int filter = readValue<int>(
        "1. Search by name\n2. Search by inactive workshop percent\nChoice: ",
        1, 2);

    if (filter == 1) {
        std::string text = readLine("Part of name: ");
        for (const auto& item : stations)
            if (item.second.getName().find(text) != std::string::npos)
                foundIds.push_back(item.first);
        Logger::getInstance().write("Searched stations by name: " + text);
    } else {
        double percent = readValue<double>(
            "Inactive workshops percent: ", 0.0, 100.0);
        for (const auto& item : stations)
            if (std::fabs(item.second.getInactivePercent() - percent) < 0.01)
                foundIds.push_back(item.first);
        Logger::getInstance().write("Searched stations by inactive percent");
    }

    return foundIds;
}

std::vector<int> Manager::selectObjects(
    const std::vector<int>& foundIds, const std::string& objectName) const {
    int mode = readValue<int>(
        "1. Select all found\n2. Select by ID\nChoice: ", 1, 2);
    if (mode == 1) return foundIds;

    std::vector<int> selectedIds;
    int count = readValue<int>(
        "Number of objects: ", 1, static_cast<int>(foundIds.size()));

    while (static_cast<int>(selectedIds.size()) < count) {
        int id = readValue<int>(objectName + " ID: ", 1, 1000000000);

        if (!containsId(foundIds, id))
            std::cout << "This ID is not in the search result.\n";
        else if (containsId(selectedIds, id))
            std::cout << "This ID is already selected.\n";
        else
            selectedIds.push_back(id);
    }

    return selectedIds;
}

void Manager::processFoundPipes(const std::vector<int>& foundIds) {
    int action = readValue<int>(
        "1. Edit\n2. Delete\n0. Return to menu\nChoice: ", 0, 2);
    if (action == 0) return;

    std::vector<int> selectedIds = selectObjects(foundIds, "Pipe");

    if (action == 1) {
        bool repairing = readValue<int>(
            "New repair status (0-no, 1-yes): ", 0, 1);
        int changed = 0;
        for (int id : selectedIds) {
            if (network.isPipeUsed(id) && repairing) {
                std::cout << "Pipe " << id
                          << " is connected and cannot be sent for repair.\n";
            } else {
                pipes.at(id).setRepairing(repairing);
                changed++;
            }
        }
        std::cout << "Changed pipes: " << changed << '\n';
        Logger::getInstance().write(
            "Batch edited " + std::to_string(changed) + " pipes");
    } else {
        int deleted = 0;
        for (int id : selectedIds) {
            if (network.isPipeUsed(id)) {
                std::cout << "Pipe " << id
                          << " is connected and cannot be deleted.\n";
            } else {
                pipes.erase(id);
                deleted++;
            }
        }
        std::cout << "Deleted pipes: " << deleted << '\n';
        Logger::getInstance().write(
            "Batch deleted " + std::to_string(deleted) + " pipes");
    }
}

void Manager::processFoundStations(const std::vector<int>& foundIds) {
    int action = readValue<int>(
        "1. Edit\n2. Delete\n0. Return to menu\nChoice: ", 0, 2);
    if (action == 0) return;

    std::vector<int> selectedIds = selectObjects(foundIds, "Station");

    if (action == 1) {
        int workshopAction = readValue<int>(
            "1. Start workshop\n2. Stop workshop\nChoice: ", 1, 2);
        int changed = 0;

        for (int id : selectedIds) {
            bool success = workshopAction == 1
                ? stations.at(id).startWorkshop()
                : stations.at(id).stopWorkshop();
            if (success) changed++;
        }

        std::cout << "Changed stations: " << changed << '\n';
        Logger::getInstance().write(
            "Batch edited " + std::to_string(changed) + " stations");
    } else {
        int deleted = 0;
        for (int id : selectedIds) {
            if (network.isStationUsed(id)) {
                std::cout << "Station " << id
                          << " is connected and cannot be deleted.\n";
            } else {
                stations.erase(id);
                deleted++;
            }
        }
        std::cout << "Deleted stations: " << deleted << '\n';
        Logger::getInstance().write(
            "Batch deleted " + std::to_string(deleted) + " stations");
    }
}

void Manager::workWithPipes() {
    if (pipes.empty()) {
        std::cout << "No pipes.\n";
        return;
    }

    std::vector<int> foundIds = findPipes();
    if (foundIds.empty()) {
        std::cout << "No pipes found.\n";
        return;
    }

    std::cout << "\nFound pipes: " << foundIds.size() << '\n';
    for (int id : foundIds) pipes.at(id).print();
    processFoundPipes(foundIds);
}

void Manager::workWithStations() {
    if (stations.empty()) {
        std::cout << "No stations.\n";
        return;
    }

    std::vector<int> foundIds = findStations();
    if (foundIds.empty()) {
        std::cout << "No stations found.\n";
        return;
    }

    std::cout << "\nFound stations: " << foundIds.size() << '\n';
    for (int id : foundIds) stations.at(id).print();
    processFoundStations(foundIds);
}

void Manager::connectStations() {
    if (stations.size() < 2) {
        std::cout << "Add at least two stations first.\n";
        return;
    }

    std::cout << "\nAvailable stations:\n";
    for (const auto& item : stations)
        std::cout << "ID " << item.first << ": "
                  << item.second.getName() << '\n';

    int inputId = readValue<int>("Input station ID: ", 1, 1000000000);
    int outputId = readValue<int>("Output station ID: ", 1, 1000000000);

    if (stations.find(inputId) == stations.end() ||
        stations.find(outputId) == stations.end()) {
        std::cout << "Station not found.\n";
        return;
    }
    if (inputId == outputId) {
        std::cout << "Input and output stations must be different.\n";
        return;
    }

    int diameter = readDiameter();
    int pipeId = 0;

    for (const auto& item : pipes) {
        const Pipe& pipe = item.second;
        if (pipe.getDiameter() == diameter &&
            !pipe.isRepairing() && !network.isPipeUsed(item.first)) {
            pipeId = item.first;
            break;
        }
    }

    if (pipeId == 0) {
        std::cout << "No free pipe with this diameter. Create a new pipe.\n";
        pipeId = createPipe(diameter);
    } else {
        std::cout << "Free pipe found. ID: " << pipeId << '\n';
    }

    network.connect(pipeId, inputId, outputId);
    std::cout << "Stations connected by pipe " << pipeId << ".\n";
    Logger::getInstance().write(
        "Connected station " + std::to_string(inputId) +
        " to station " + std::to_string(outputId) +
        " using pipe " + std::to_string(pipeId));
}

void Manager::disconnectPipe() {
    if (network.getConnections().empty()) {
        std::cout << "The network has no connections.\n";
        return;
    }

    showNetwork();
    int pipeId = readValue<int>("Pipe ID to disconnect: ", 1, 1000000000);

    if (network.disconnect(pipeId)) {
        std::cout << "Pipe disconnected.\n";
        Logger::getInstance().write(
            "Disconnected pipe ID " + std::to_string(pipeId));
    } else {
        std::cout << "Connected pipe not found.\n";
    }
}

void Manager::showNetwork() const {
    network.print(pipes, stations);
    Logger::getInstance().write("Viewed gas transportation network");
}

void Manager::showTopologicalSort() const {
    if (stations.empty()) {
        std::cout << "No stations.\n";
        return;
    }

    bool hasCycle;
    std::vector<int> order = network.topologicalSort(stations, hasCycle);

    if (hasCycle) {
        std::cout << "Topological sorting is impossible: the network has a cycle.\n";
        Logger::getInstance().write("Topological sort failed: cycle found");
        return;
    }

    std::cout << "Topological order:\n";
    for (int id : order)
        std::cout << "ID " << id << ": " << stations.at(id).getName() << '\n';
    Logger::getInstance().write("Performed topological sorting");
}

void Manager::save() const {
    std::string fileName = readLine("File name: ");
    std::ofstream file(fileName);

    if (!file) {
        std::cout << "Cannot open file.\n";
        Logger::getInstance().write("Could not save to " + fileName);
        return;
    }

    file << pipes.size() << '\n';
    for (const auto& item : pipes) file << item.second;

    file << stations.size() << '\n';
    for (const auto& item : stations) file << item.second;

    file << network;
    std::cout << "Data saved.\n";
    Logger::getInstance().write("Saved data to " + fileName);
}

void Manager::load() {
    std::string fileName = readLine("File name: ");
    std::ifstream file(fileName);

    if (!file) {
        std::cout << "File not found.\n";
        Logger::getInstance().write("Could not load " + fileName);
        return;
    }

    std::unordered_map<int, Pipe> loadedPipes;
    std::unordered_map<int, CompressorStation> loadedStations;
    GasNetwork loadedNetwork;
    int pipeCount, stationCount;

    if (!(file >> pipeCount) || pipeCount < 0) {
        std::cout << "Invalid file.\n";
        return;
    }
    file.ignore(10000, '\n');

    for (int i = 0; i < pipeCount; i++) {
        Pipe pipe;
        if (!(file >> pipe) || !isAllowedDiameter(pipe.getDiameter()) ||
            !loadedPipes.emplace(pipe.getId(), pipe).second) {
            std::cout << "Invalid pipe data.\n";
            return;
        }
    }

    if (!(file >> stationCount) || stationCount < 0) {
        std::cout << "Invalid station data.\n";
        return;
    }
    file.ignore(10000, '\n');

    for (int i = 0; i < stationCount; i++) {
        CompressorStation station;
        if (!(file >> station) ||
            !loadedStations.emplace(station.getId(), station).second) {
            std::cout << "Invalid station data.\n";
            return;
        }
    }

    if (!(file >> loadedNetwork)) {
        std::cout << "Invalid network data.\n";
        return;
    }

    for (const auto& item : loadedNetwork.getConnections()) {
        const Connection& connection = item.second;
        auto pipe = loadedPipes.find(connection.pipeId);
        if (pipe == loadedPipes.end() || pipe->second.isRepairing() ||
            loadedStations.find(connection.inputStationId) == loadedStations.end() ||
            loadedStations.find(connection.outputStationId) == loadedStations.end()) {
            std::cout << "Invalid network connection.\n";
            return;
        }
    }

    pipes = loadedPipes;
    stations = loadedStations;
    network = loadedNetwork;
    nextPipeId = 1;
    nextStationId = 1;

    for (const auto& item : pipes)
        if (item.first >= nextPipeId) nextPipeId = item.first + 1;
    for (const auto& item : stations)
        if (item.first >= nextStationId) nextStationId = item.first + 1;

    std::cout << "Data loaded.\n";
    Logger::getInstance().write("Loaded data from " + fileName);
}

void Manager::run() {
    Logger::getInstance().write("Program started");

    while (true) {
        printMenu();
        int command = readValue<int>("Choice: ", 0, 11);
        Logger::getInstance().write(
            "Selected menu item " + std::to_string(command));

        switch (command) {
        case 0:
            Logger::getInstance().write("Program finished");
            return;
        case 1: addPipe(); break;
        case 2: addStation(); break;
        case 3: showAll(); break;
        case 4: workWithPipes(); break;
        case 5: workWithStations(); break;
        case 6: connectStations(); break;
        case 7: disconnectPipe(); break;
        case 8: showNetwork(); break;
        case 9: showTopologicalSort(); break;
        case 10: save(); break;
        case 11: load(); break;
        }
    }
}
