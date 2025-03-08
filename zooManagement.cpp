//
//  zooManagement.cpp
//  Zoo
//
//  Created by Luke Contreras on 3/7/25.
//

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <cstdlib>
#include <ctime>
#include <algorithm>

using namespace std;

// Data structure to hold information about each animal.
struct Animal {
    int age;            // Animal's age
    string species;     // Animal's species (e.g., "Hyena", "Lion")
    string birthSeason; // Description of the animal's birth season
    string color;       // Color description of the animal
    double weight;      // Animal's weight
    string origin;      // Origin information (combined from two parts)
    string name;        // Assigned name from the animal names file
};

// Helper function that removes any leading and trailing whitespace from a string.
string trim(const string& s) {
    size_t start = s.find_first_not_of(" \t");
    if (start == string::npos)
        return "";
    size_t end = s.find_last_not_of(" \t");
    return s.substr(start, end - start + 1);
}

// Function to load animal names from a file.
// The file should have headers like "Hyena Names:" followed by a line of comma-separated names.
map<string, vector<string>> loadAnimalNames(const string& filename) {
    map<string, vector<string>> namesMap;
    ifstream file(filename);
    if (!file) {
        cerr << "Error opening file: " << filename << endl;
        return namesMap;
    }
    string line;
    string currentSpecies;  // Holds the species name from the header line.
    while(getline(file, line)) {
        line = trim(line);
        if (line.empty())
            continue;
        // If the line ends with "Names:" (case sensitive check), it marks a new species section.
        if (line.size() >= 6 && (line.substr(line.size() - 6) == "Names:" || line.substr(line.size() - 6) == "names:")) {
            // Extract the species name from the header (e.g., "Hyena Names:" -> "Hyena").
            currentSpecies = trim(line.substr(0, line.size() - 6));
            namesMap[currentSpecies] = vector<string>(); // Initialize an empty vector for this species.
        } else {
            // Otherwise, the line contains comma-separated names.
            stringstream ss(line);
            string name;
            while(getline(ss, name, ',')) {
                name = trim(name);
                if (!name.empty() && !currentSpecies.empty())
                    namesMap[currentSpecies].push_back(name);
            }
        }
    }
    file.close();
    return namesMap;
}

// Function to load arriving animal records from a file.
// Each record should be in one line with exactly six comma-separated fields:
// Field 0: Age and species (e.g., "4 Hyena")
// Field 1: Birth season (e.g., "born in spring")
// Field 2: Color description
// Field 3: Weight (numeric)
// Field 4: Origin part 1
// Field 5: Origin part 2
vector<Animal> loadArrivingAnimals(const string& filename) {
    vector<Animal> animals;
    ifstream file(filename);
    if (!file) {
        cerr << "Error opening file: " << filename << endl;
        return animals;
    }
    string line;
    while(getline(file, line)) {
        line = trim(line);
        if(line.empty())
            continue;
        // Split the line by commas into individual parts.
        vector<string> parts;
        stringstream ss(line);
        string part;
        while(getline(ss, part, ',')) {
            parts.push_back(trim(part));
        }
        // Check if we have exactly 6 fields; if not, report an invalid record.
        if (parts.size() < 6) {
            cerr << "Invalid record: " << line << endl;
            continue;
        }
        Animal animal;
        // Process Field 0: it contains both age and species.
        {
            stringstream partStream(parts[0]);
            string ageStr;
            partStream >> ageStr; // Extract the age (first word).
            animal.age = stoi(ageStr); // Convert age string to integer.
            // The rest of Field 0 is treated as the species.
            string species;
            getline(partStream, species);
            animal.species = trim(species);
        }
        // Assign remaining fields from the record.
        animal.birthSeason = parts[1];
        animal.color = parts[2];
        animal.weight = stod(parts[3]); // Convert weight string to double.
        animal.origin = parts[4] + " " + parts[5]; // Combine the two parts of the origin.
        // The name will be assigned later based on the species.
        animals.push_back(animal);
    }
    file.close();
    return animals;
}

// Function to assign a random name to an animal based on its species.
// Searches the names map for a key matching the species; if not found, attempts a case-insensitive match.
// Returns "Unnamed" if no matching name is found.
string assignName(const string& species, map<string, vector<string>>& namesMap) {
    auto it = namesMap.find(species);
    if (it == namesMap.end()) {
        // Perform a case-insensitive search if the direct lookup fails.
        for (auto& pair : namesMap) {
            string keyLower = pair.first;
            string speciesLower = species;
            transform(keyLower.begin(), keyLower.end(), keyLower.begin(), ::tolower);
            transform(speciesLower.begin(), speciesLower.end(), speciesLower.begin(), ::tolower);
            if (keyLower == speciesLower) {
                it = namesMap.find(pair.first);
                break;
            }
        }
    }
    if (it != namesMap.end() && !it->second.empty()) {
        int index = rand() % it->second.size(); // Randomly choose one of the available names.
        return it->second[index];
    }
    return "Unnamed"; // Return a default name if no match is found.
}

// Function to update the animal report file by appending new animal records.
// The file is written in CSV format with the following fields:
// name, species, age, birth season, color, weight, origin.
// Note: The output report file is now "newAnimals.txt" instead of "zooPopulation.txt".
void updateZooPopulation(const string& filename, const vector<Animal>& animals) {
    // Open the file in append mode.
    ofstream file(filename, ios::app);
    if (!file) {
        cerr << "Error opening file for writing: " << filename << endl;
        return;
    }
    // Write each animal's data in CSV format.
    for (const auto& animal : animals) {
        file << animal.name << ", "
             << animal.species << ", "
             << animal.age << ", "
             << animal.birthSeason << ", "
             << animal.color << ", "
             << animal.weight << ", "
             << animal.origin << "\n";
    }
    file.close();
}

int main() {
    // Seed the random number generator with the current time.
    srand(static_cast<unsigned int>(time(NULL)));
    
    // Load animal names from the file "animalNames.txt".
    map<string, vector<string>> animalNamesMap = loadAnimalNames("animalNames.txt");
    
    // Load arriving animal records from the file "arrivingAnimals.txt".
    vector<Animal> arrivingAnimals = loadArrivingAnimals("arrivingAnimals.txt");
    
    // For each arriving animal, assign a random name based on its species.
    for (auto &animal : arrivingAnimals) {
        animal.name = assignName(animal.species, animalNamesMap);
    }
    
    // Append the new animal records to the report file "newAnimals.txt".
    updateZooPopulation("newAnimals.txt", arrivingAnimals);
    
    cout << "Zoo population updated successfully." << endl;
    
    // Display the updated contents of the "newAnimals.txt" file.
    ifstream populationFile("newAnimals.txt");
    if (!populationFile) {
        cerr << "Error opening newAnimals file." << endl;
        return 1;
    }
    string line;
    cout << "\nUpdated Zoo Population:\n";
    while(getline(populationFile, line)) {
        cout << line << endl;
    }
    populationFile.close();
    
    return 0;
}
