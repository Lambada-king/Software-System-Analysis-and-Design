#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <memory>
#include <cassert>
using namespace std;

// -------------------------
// Animal Hierarchy
// -------------------------
// Base abstract class representing a generic animal.
// It stores the animal's name and days lived.
class Animal {
protected:
    const string name;    // Animal's name (immutable after creation)
    int daysLived;        // Number of days the animal has lived
public:
    // Constructor initializes the name and days lived.
    Animal(const string& name, int daysLived) : name(name), daysLived(daysLived) {}

    // Copy constructor.
    Animal(const Animal& other) : name(other.name), daysLived(other.daysLived) {}

    // Virtual destructor for proper polymorphic cleanup.
    virtual ~Animal() = default;

    // Getter for daysLived.
    int getDaysLived() const { return daysLived; }
    // Getter for name.
    string getName() const { return name; }
    // Setter to update daysLived.
    void setDaysLived(int newDays) { daysLived = newDays; }

    // Prints the animal's name and days lived.
    void sayName() const {
        cout << "My name is " << name << ", days lived: " << daysLived << "\n";
    }

    // Pure virtual method: Each derived class must implement its attack behavior.
    virtual void attack(Animal& other) = 0;

    // Operator < is defined for sorting:
    // First by daysLived, then lexicographically by name.
    bool operator < (const Animal& other) const {
        if(daysLived != other.daysLived)
            return daysLived < other.daysLived;
        return name < other.name;
    }
};

// Derived class for Fish.
class Fish : public Animal {
public:
    Fish(const string& name, int daysLived) : Animal(name, daysLived) {}
    void attack(Animal& other) override {
        cout << "Fish is attacking\n";
    }
};

// Derived class for Bird.
class Bird : public Animal {
public:
    Bird(const string& name, int daysLived) : Animal(name, daysLived) {}
    void attack(Animal& other) override {
        cout << "Bird is attacking\n";
    }
};

// Derived class for Mouse.
class Mouse : public Animal {
public:
    Mouse(const string& name, int daysLived) : Animal(name, daysLived) {}
    void attack(Animal& other) override {
        cout << "Mouse is attacking\n";
    }
};

// "Better" versions of animals are created by applying the substance.
// Their days lived are halved (rounded up).

class BetterFish : public Fish {
public:
    BetterFish(const string& name, int daysLived) : Fish(name, daysLived) {}
    BetterFish(Fish& fish) : Fish(fish.getName(), (fish.getDaysLived() + 1) / 2) {}
    void attack(Animal& other) override {
        cout << "BetterFish is attacking\n";
    }
};

class BetterBird : public Bird {
public:
    BetterBird(const string& name, int daysLived) : Bird(name, daysLived) {}
    BetterBird(Bird& bird) : Bird(bird.getName(), (bird.getDaysLived() + 1) / 2) {}
    void attack(Animal& other) override {
        cout << "BetterBird is attacking\n";
    }
};

class BetterMouse : public Mouse {
public:
    BetterMouse(const string& name, int daysLived) : Mouse(name, daysLived) {}
    BetterMouse(Mouse& mouse) : Mouse(mouse.getName(), (mouse.getDaysLived() + 1) / 2) {}
    void attack(Animal& other) override {
        cout << "BetterMouse is attacking\n";
    }
};

// Monster type is created when substance is applied on an already "better" animal.
// A Monster always lives 1 day.
class Monster : public Animal {
public:
    Monster(const string& name) : Animal(name, 1) {}
    Monster(Animal& animal) : Animal(animal.getName(), 1) {}
    void attack(Animal& other) override {
        // Monster's attack does not output text as per specification.
    }
};

// -------------------------
// Container Templates
// -------------------------
// A generic container template to store pointers to animals of a specific type.
// It provides common operations: add, remove, sort, and period update.
template<typename T>
class Container {
public:
    vector<T*> animals; // Vector holding pointers to animals

    // Adds an animal pointer to the container and sorts it.
    void add(T* animal) {
        animals.push_back(animal);
        sortContainer();
    }

    // Removes an animal from the container at a given index (without deleting it).
    T* removeAt(int pos) {
        if(pos < 0 || pos >= animals.size())
            return nullptr;
        T* ptr = animals[pos];
        animals.erase(animals.begin() + pos);
        return ptr;
    }

    // Deletes the animal at the given index (for when the animal dies).
    void deleteAt(int pos) {
        if(pos < 0 || pos >= animals.size())
            return;
        delete animals[pos];
        animals.erase(animals.begin() + pos);
    }

    // Sorts the animals by days lived and name.
    void sortContainer() {
        sort(animals.begin(), animals.end(), [](T* a, T* b){
            if(a->getDaysLived() != b->getDaysLived())
                return a->getDaysLived() < b->getDaysLived();
            return a->getName() < b->getName();
        });
    }

    // PERIOD update: adds +1 day to each animal; if daysLived exceeds 10, the animal dies.
    // Returns a vector of names of animals that died due to old age.
    vector<string> periodUpdate() {
        vector<string> died;
        for (int i = 0; i < animals.size(); ) {
            T* a = animals[i];
            a->setDaysLived(a->getDaysLived() + 1);
            if(a->getDaysLived() > 10) {
                died.push_back(a->getName());
                delete a;
                animals.erase(animals.begin() + i);
            } else {
                ++i;
            }
        }
        sortContainer();
        return died;
    }

    // Destructor to free all dynamically allocated animals.
    ~Container() {
        for(auto a : animals) 
            delete a;
    }
};

// Template specializations for Fish and Bird to simulate deleted constructors
// where certain container types are disallowed.

template<>
class Container<Fish> {
public:
    vector<Fish*> animals;
    Container() = default;
    // The constructor is not explicitly deleted here, but in our usage,
    // we will never create a Cage<Fish> (as required by the task).
    void add(Fish* animal) {
        animals.push_back(animal);
        sortContainer();
    }
    Fish* removeAt(int pos) {
        if(pos < 0 || pos >= animals.size())
            return nullptr;
        Fish* ptr = animals[pos];
        animals.erase(animals.begin() + pos);
        return ptr;
    }
    void deleteAt(int pos) {
        if(pos < 0 || pos >= animals.size())
            return;
        delete animals[pos];
        animals.erase(animals.begin() + pos);
    }
    void sortContainer() {
        sort(animals.begin(), animals.end(), [](Fish* a, Fish* b){
            if(a->getDaysLived() != b->getDaysLived())
                return a->getDaysLived() < b->getDaysLived();
            return a->getName() < b->getName();
        });
    }
    vector<string> periodUpdate() {
        vector<string> died;
        for (int i = 0; i < animals.size(); ) {
            Fish* a = animals[i];
            a->setDaysLived(a->getDaysLived() + 1);
            if(a->getDaysLived() > 10) {
                died.push_back(a->getName());
                delete a;
                animals.erase(animals.begin() + i);
            } else {
                ++i;
            }
        }
        sortContainer();
        return died;
    }
    ~Container() {
        for(auto a : animals) 
            delete a;
    }
};

template<>
class Container<Bird> {
public:
    vector<Bird*> animals;
    Container() = default;
    // For Aquarium<Bird>, the constructor is conceptually deleted (we won't create such a container).
    Container(const Container&) = delete;
    Container& operator=(const Container&) = delete;
    void add(Bird* animal) {
        animals.push_back(animal);
        sortContainer();
    }
    Bird* removeAt(int pos) {
        if(pos < 0 || pos >= animals.size())
            return nullptr;
        Bird* ptr = animals[pos];
        animals.erase(animals.begin() + pos);
        return ptr;
    }
    void deleteAt(int pos) {
        if(pos < 0 || pos >= animals.size())
            return;
        delete animals[pos];
        animals.erase(animals.begin() + pos);
    }
    void sortContainer() {
        sort(animals.begin(), animals.end(), [](Bird* a, Bird* b){
            if(a->getDaysLived() != b->getDaysLived())
                return a->getDaysLived() < b->getDaysLived();
            return a->getName() < b->getName();
        });
    }
    vector<string> periodUpdate() {
        vector<string> died;
        for (int i = 0; i < animals.size(); ) {
            Bird* a = animals[i];
            a->setDaysLived(a->getDaysLived() + 1);
            if(a->getDaysLived() > 10) {
                died.push_back(a->getName());
                delete a;
                animals.erase(animals.begin() + i);
            } else {
                ++i;
            }
        }
        sortContainer();
        return died;
    }
    ~Container() {
        for(auto a : animals) 
            delete a;
    }
};

// For clarity, we alias our containers to the names required by the task.
template<typename T>
using Cage = Container<T>;
template<typename T>
using Aquarium = Container<T>;
template<typename T>
using Freedom = Container<T>;

// -------------------------
// Global Pre-Created Containers
// -------------------------
// These containers are pre-created for each allowed animal type.
// Note that "Freedom" is generic (can hold any Animal), and other containers
// are specialized for each type.
Cage<Bird> cageB;
Cage<BetterBird> cageBB;
Cage<Mouse> cageM;
Cage<BetterMouse> cageBM;

Aquarium<Fish> aquF;
Aquarium<BetterFish> aquBF;
Aquarium<Mouse> aquM;
Aquarium<BetterMouse> aquBM;

Freedom<Animal> freeZone;

// -------------------------
// Helper Functions & Type Parsing
// -------------------------
// Maps a type code (e.g., "M", "BM", etc.) to an enumerated type.
enum class AnimalType { M, BM, F, BF, B, BB, UNKNOWN };

AnimalType parseAnimalType(const string& code) {
    if(code == "M") return AnimalType::M;
    if(code == "BM") return AnimalType::BM;
    if(code == "F") return AnimalType::F;
    if(code == "BF") return AnimalType::BF;
    if(code == "B") return AnimalType::B;
    if(code == "BB") return AnimalType::BB;
    return AnimalType::UNKNOWN;
}

// -------------------------
// CREATE Command Handler
// -------------------------
// This function creates an animal of a given type with specified name and days lived,
// and then adds it to the specified container.
void createAnimal(AnimalType type, const string& name, const string& containerName, int days) {
    if(containerName == "Freedom") {
        Animal* animal = nullptr;
        // Create the appropriate animal type.
        switch(type) {
            case AnimalType::M: animal = new Mouse(name, days); break;
            case AnimalType::BM: animal = new BetterMouse(name, days); break;
            case AnimalType::F: animal = new Fish(name, days); break;
            case AnimalType::BF: animal = new BetterFish(name, days); break;
            case AnimalType::B: animal = new Bird(name, days); break;
            case AnimalType::BB: animal = new BetterBird(name, days); break;
            default: break;
        }
        if(animal) {
            freeZone.add(animal);
            animal->sayName();
        }
        return;
    }

    // For Cage and Aquarium, add the animal to the appropriate container.
    if(containerName == "Cage") {
        if(type == AnimalType::M) {
            Mouse* animal = new Mouse(name, days);
            cageM.add(animal);
            animal->sayName();
        } else if(type == AnimalType::BM) {
            BetterMouse* animal = new BetterMouse(name, days);
            cageBM.add(animal);
            animal->sayName();
        } else if(type == AnimalType::B) {
            Bird* animal = new Bird(name, days);
            cageB.add(animal);
            animal->sayName();
        } else if(type == AnimalType::BB) {
            BetterBird* animal = new BetterBird(name, days);
            cageBB.add(animal);
            animal->sayName();
        }
    } else if(containerName == "Aquarium") {
        if(type == AnimalType::F) {
            Fish* animal = new Fish(name, days);
            aquF.add(animal);
            animal->sayName();
        } else if(type == AnimalType::BF) {
            BetterFish* animal = new BetterFish(name, days);
            aquBF.add(animal);
            animal->sayName();
        } else if(type == AnimalType::M) {
            Mouse* animal = new Mouse(name, days);
            aquM.add(animal);
            animal->sayName();
        } else if(type == AnimalType::BM) {
            BetterMouse* animal = new BetterMouse(name, days);
            aquBM.add(animal);
            animal->sayName();
        }
    }
}

// -------------------------
// Utility: Get Animal Pointer from Container
// -------------------------
// This function returns a pointer to the animal at the given position in the specified container.
// For the Freedom container, the type code is not needed.
Animal* getAnimal(const string& containerName, AnimalType type, int pos) {
    if(containerName == "Freedom") {
        if(pos < 0 || pos >= freeZone.animals.size()) return nullptr;
        return freeZone.animals[pos];
    } else if(containerName == "Cage") {
        if(type == AnimalType::M) {
            if(pos < 0 || pos >= cageM.animals.size()) return nullptr;
            return cageM.animals[pos];
        } else if(type == AnimalType::BM) {
            if(pos < 0 || pos >= cageBM.animals.size()) return nullptr;
            return cageBM.animals[pos];
        } else if(type == AnimalType::B) {
            if(pos < 0 || pos >= cageB.animals.size()) return nullptr;
            return cageB.animals[pos];
        } else if(type == AnimalType::BB) {
            if(pos < 0 || pos >= cageBB.animals.size()) return nullptr;
            return cageBB.animals[pos];
        }
    } else if(containerName == "Aquarium") {
        if(type == AnimalType::F) {
            if(pos < 0 || pos >= aquF.animals.size()) return nullptr;
            return aquF.animals[pos];
        } else if(type == AnimalType::BF) {
            if(pos < 0 || pos >= aquBF.animals.size()) return nullptr;
            return aquBF.animals[pos];
        } else if(type == AnimalType::M) {
            if(pos < 0 || pos >= aquM.animals.size()) return nullptr;
            return aquM.animals[pos];
        } else if(type == AnimalType::BM) {
            if(pos < 0 || pos >= aquBM.animals.size()) return nullptr;
            return aquBM.animals[pos];
        }
    }
    return nullptr;
}

// -------------------------
// Utility: Remove Animal from Container
// -------------------------
// This function removes an animal (without deleting it) from a container and returns its pointer.
Animal* removeAnimalFromContainer(const string& containerName, AnimalType type, int pos) {
    if(containerName == "Freedom") {
        if(pos < 0 || pos >= freeZone.animals.size()) return nullptr;
        Animal* ptr = freeZone.animals[pos];
        freeZone.animals.erase(freeZone.animals.begin() + pos);
        return ptr;
    } else if(containerName == "Cage") {
        if(type == AnimalType::M) {
            if(pos < 0 || pos >= cageM.animals.size()) return nullptr;
            Animal* ptr = cageM.animals[pos];
            cageM.animals.erase(cageM.animals.begin() + pos);
            return ptr;
        } else if(type == AnimalType::BM) {
            if(pos < 0 || pos >= cageBM.animals.size()) return nullptr;
            Animal* ptr = cageBM.animals[pos];
            cageBM.animals.erase(cageBM.animals.begin() + pos);
            return ptr;
        } else if(type == AnimalType::B) {
            if(pos < 0 || pos >= cageB.animals.size()) return nullptr;
            Animal* ptr = cageB.animals[pos];
            cageB.animals.erase(cageB.animals.begin() + pos);
            return ptr;
        } else if(type == AnimalType::BB) {
            if(pos < 0 || pos >= cageBB.animals.size()) return nullptr;
            Animal* ptr = cageBB.animals[pos];
            cageBB.animals.erase(cageBB.animals.begin() + pos);
            return ptr;
        }
    } else if(containerName == "Aquarium") {
        if(type == AnimalType::F) {
            if(pos < 0 || pos >= aquF.animals.size()) return nullptr;
            Animal* ptr = aquF.animals[pos];
            aquF.animals.erase(aquF.animals.begin() + pos);
            return ptr;
        } else if(type == AnimalType::BF) {
            if(pos < 0 || pos >= aquBF.animals.size()) return nullptr;
            Animal* ptr = aquBF.animals[pos];
            aquBF.animals.erase(aquBF.animals.begin() + pos);
            return ptr;
        } else if(type == AnimalType::M) {
            if(pos < 0 || pos >= aquM.animals.size()) return nullptr;
            Animal* ptr = aquM.animals[pos];
            aquM.animals.erase(aquM.animals.begin() + pos);
            return ptr;
        } else if(type == AnimalType::BM) {
            if(pos < 0 || pos >= aquBM.animals.size()) return nullptr;
            Animal* ptr = aquBM.animals[pos];
            aquBM.animals.erase(aquBM.animals.begin() + pos);
            return ptr;
        }
    }
    return nullptr;
}

// -------------------------
// Utility: Sort Container after Modifications
// -------------------------
// This function triggers the sort on the specified container after modifications.
void sortContainerByName(const string& containerName, AnimalType type) {
    if(containerName == "Freedom") {
        freeZone.sortContainer();
    } else if(containerName == "Cage") {
        if(type == AnimalType::M)
            cageM.sortContainer();
        else if(type == AnimalType::BM)
            cageBM.sortContainer();
        else if(type == AnimalType::B)
            cageB.sortContainer();
        else if(type == AnimalType::BB)
            cageBB.sortContainer();
    } else if(containerName == "Aquarium") {
        if(type == AnimalType::F)
            aquF.sortContainer();
        else if(type == AnimalType::BF)
            aquBF.sortContainer();
        else if(type == AnimalType::M)
            aquM.sortContainer();
        else if(type == AnimalType::BM)
            aquBM.sortContainer();
    }
}

// -------------------------
// Command Handlers
// -------------------------

// APPLY_SUBSTANCE: Applies the substance to transform an animal.
// If the animal is already of a better type, it becomes a Monster and the entire container is cleared.
void applySubstance(const string& containerName, AnimalType type, int pos) {
    // Substance cannot be applied in Freedom.
    if(containerName == "Freedom") {
        cout << "Substance cannot be applied in freedom" << "\n";
        return;
    }
    Animal* a = getAnimal(containerName, type, pos);
    if(!a) {
        cout << "Animal not found" << "\n";
        return;
    }
    // Check if animal is normal (e.g., Mouse, Fish, Bird) and not already a better type.
    if(dynamic_cast<Mouse*>(a) && !dynamic_cast<BetterMouse*>(a)) {
        Animal* ptr = removeAnimalFromContainer(containerName, type, pos);
        int newDays = (ptr->getDaysLived() + 1) / 2;
        BetterMouse* bm = new BetterMouse(ptr->getName(), newDays);
        delete ptr;
        if(containerName == "Cage")
            cageBM.add(bm);
        else if(containerName == "Aquarium")
            aquBM.add(bm);
    } else if(dynamic_cast<Fish*>(a) && !dynamic_cast<BetterFish*>(a)) {
        Animal* ptr = removeAnimalFromContainer(containerName, type, pos);
        int newDays = (ptr->getDaysLived() + 1) / 2;
        BetterFish* bf = new BetterFish(ptr->getName(), newDays);
        delete ptr;
        if(containerName == "Aquarium")
            aquBF.add(bf);
    } else if(dynamic_cast<Bird*>(a) && !dynamic_cast<BetterBird*>(a)) {
        Animal* ptr = removeAnimalFromContainer(containerName, type, pos);
        int newDays = (ptr->getDaysLived() + 1) / 2;
        BetterBird* bb = new BetterBird(ptr->getName(), newDays);
        delete ptr;
        if(containerName == "Cage")
            cageBB.add(bb);
    }
    else {
        // If the animal is already a better type, applying substance converts it to a Monster.
        Animal* ptr = removeAnimalFromContainer(containerName, type, pos);
        Monster* monster = new Monster(*ptr);
        delete ptr;
        // Clear the entire container of the corresponding better type.
        if(containerName == "Cage") {
            if(type == AnimalType::BM) {
                for(auto a : cageBM.animals) { if(a) delete a; }
                cageBM.animals.clear();
            } else if(type == AnimalType::BB) {
                for(auto a : cageBB.animals) { if(a) delete a; }
                cageBB.animals.clear();
            }
        } else if(containerName == "Aquarium") {
            if(type == AnimalType::BF) {
                for(auto a : aquBF.animals) { if(a) delete a; }
                aquBF.animals.clear();
            } else if(type == AnimalType::BM) {
                for(auto a : aquBM.animals) { if(a) delete a; }
                aquBM.animals.clear();
            }
        }
        // Place the Monster in the Freedom container.
        freeZone.add(monster);
    }
}

// REMOVE_SUBSTANCE: Reverts a better type animal back to its normal type by doubling its days lived.
// Only valid for better types.
void removeSubstance(const string& containerName, AnimalType type, int pos) {
    if(containerName == "Freedom") {
        cout << "Substance cannot be removed in freedom" << "\n";
        return;
    }
    Animal* a = getAnimal(containerName, type, pos);
    if(!a) {
        cout << "Animal not found" << "\n";
        return;
    }
    if(dynamic_cast<BetterMouse*>(a)) {
        Animal* ptr = removeAnimalFromContainer(containerName, type, pos);
        int newDays = ptr->getDaysLived() * 2;
        Mouse* normal = new Mouse(ptr->getName(), newDays);
        delete ptr;
        if(containerName == "Cage")
            cageM.add(normal);
        else if(containerName == "Aquarium")
            aquM.add(normal);
    } else if(dynamic_cast<BetterFish*>(a)) {
        Animal* ptr = removeAnimalFromContainer(containerName, type, pos);
        int newDays = ptr->getDaysLived() * 2;
        Fish* normal = new Fish(ptr->getName(), newDays);
        delete ptr;
        if(containerName == "Aquarium")
            aquF.add(normal);
    } else if(dynamic_cast<BetterBird*>(a)) {
        Animal* ptr = removeAnimalFromContainer(containerName, type, pos);
        int newDays = ptr->getDaysLived() * 2;
        Bird* normal = new Bird(ptr->getName(), newDays);
        delete ptr;
        if(containerName == "Cage")
            cageB.add(normal);
    } else {
        cout << "Invalid substance removal" << "\n";
    }
}

// ATTACK: The animal at the first given position attacks the animal at the second given position.
// After the attack, the attacked animal is removed from the container.
void attackCommand(const string& containerName, AnimalType type, int pos1, int pos2) {
    if(containerName == "Freedom") {
        cout << "Animals cannot attack in Freedom" << "\n";
        return;
    }
    Animal* attacker = getAnimal(containerName, type, pos1);
    Animal* target = getAnimal(containerName, type, pos2);
    if(!attacker || !target) {
        cout << "Animal not found" << "\n";
        return;
    }
    attacker->attack(*target);
    removeAnimalFromContainer(containerName, type, pos2);
    sortContainerByName(containerName, type);
}

// TALK for non-Freedom containers: expects a type code and position.
void talkCommand(const string& containerName, AnimalType type, int pos) {
    Animal* a = getAnimal(containerName, type, pos);
    if(!a) {
        cout << "Animal not found" << "\n";
        return;
    }
    a->sayName();
}

// TALK for Freedom container: only the position is needed.
void talkFreedom(int pos) {
    if(pos < 0 || pos >= freeZone.animals.size()){
        cout << "Animal not found" << "\n";
        return;
    }
    freeZone.animals[pos]->sayName();
}

// -------------------------
// PERIOD Command
// -------------------------
// The PERIOD command increments the age of each animal.
// If an animal exceeds 10 days lived, it dies and a message is printed.
// Special handling is done for Freedom container: Monsters die immediately.
void periodCommand() {
    vector<string> died;
    vector<string> d;
    
    // Update Freedom container with special rules:
    // - If an animal is a Monster, it dies immediately.
    // - Otherwise, increment age and check if it exceeds 10.
    for (int i = 0; i < freeZone.animals.size(); ) {
        Animal* a = freeZone.animals[i];
        if(dynamic_cast<Monster*>(a) != nullptr) {
            died.push_back(a->getName());
            delete a;
            freeZone.animals.erase(freeZone.animals.begin() + i);
        } else {
            a->setDaysLived(a->getDaysLived() + 1);
            if(a->getDaysLived() > 10) {
                died.push_back(a->getName());
                delete a;
                freeZone.animals.erase(freeZone.animals.begin() + i);
            } else {
                i++;
            }
        }
    }
    
    // Update other containers in the order specified by the task.
    d = cageB.periodUpdate();       died.insert(died.end(), d.begin(), d.end());
    d = cageBB.periodUpdate();      died.insert(died.end(), d.begin(), d.end());
    d = cageM.periodUpdate();       died.insert(died.end(), d.begin(), d.end());
    d = cageBM.periodUpdate();      died.insert(died.end(), d.begin(), d.end());
    d = aquF.periodUpdate();        died.insert(died.end(), d.begin(), d.end());
    d = aquBF.periodUpdate();       died.insert(died.end(), d.begin(), d.end());
    d = aquM.periodUpdate();        died.insert(died.end(), d.begin(), d.end());
    d = aquBM.periodUpdate();       died.insert(died.end(), d.begin(), d.end());
    
    // Print death messages.
    for(auto& name : died)
        cout << name << " has died of old days" << "\n";
}

// -------------------------
// Main Function
// -------------------------
// This function reads commands from the input, parses them, and calls the appropriate handlers.
int main(){
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int C; // Number of commands
    cin >> C;
    string dummy;
    getline(cin, dummy); // Consume the rest of the line

    for (int i = 0; i < C; i++) {
        string line;
        getline(cin, line);
        istringstream iss(line);
        string command;
        iss >> command;
        if(command == "CREATE") {
            string typeCode, name, inToken, containerName;
            int days;
            iss >> typeCode >> name >> inToken >> containerName >> days;
            AnimalType type = parseAnimalType(typeCode);
            createAnimal(type, name, containerName, days);
        }
        else if(command == "APPLY_SUBSTANCE") {
            string containerName, typeCode;
            int pos;
            iss >> containerName >> typeCode >> pos;
            AnimalType type = parseAnimalType(typeCode);
            applySubstance(containerName, type, pos);
        }
        else if(command == "REMOVE_SUBSTANCE") {
            string containerName, typeCode;
            int pos;
            iss >> containerName >> typeCode >> pos;
            AnimalType type = parseAnimalType(typeCode);
            removeSubstance(containerName, type, pos);
        }
        else if(command == "ATTACK") {
            string containerName, typeCode;
            int pos1, pos2;
            iss >> containerName >> typeCode >> pos1 >> pos2;
            AnimalType type = parseAnimalType(typeCode);
            attackCommand(containerName, type, pos1, pos2);
        }
        else if(command == "TALK") {
            // For Freedom container, the TALK command only provides container name and position.
            string containerName;
            iss >> containerName;
            if(containerName == "Freedom") {
                int pos;
                iss >> pos;
                talkFreedom(pos);
            } else {
                // For other containers, TALK command provides type code and position.
                string typeCode;
                int pos;
                iss >> typeCode >> pos;
                AnimalType type = parseAnimalType(typeCode);
                talkCommand(containerName, type, pos);
            }
        }
        else if(command == "PERIOD") {
            periodCommand();
        }
    }
    return 0;
}
