#include "Railway.cpp"

class City{
    public:
        std::vector<Railway> railways;
        std::string name;
        int slaveCountExpected;

        City(std::vector<Railway> railwaysInput, std::string nameInput, int slaveCountExpectedInput){
            railways = railwaysInput;
            name = nameInput;
            slaveCountExpected = slaveCountExpectedInput;
        }
};