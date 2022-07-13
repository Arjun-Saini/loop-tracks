#include "Railway.cpp"

class City{
    public:
        std::vector<Railway> railways;
        std::string name;

        City(std::vector<Railway> railwaysInput, std::string nameInput){
            railways = railwaysInput;
            name = nameInput;
        }
};