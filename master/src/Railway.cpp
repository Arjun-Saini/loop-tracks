#include "LinkedList.cpp"
#include <string>

//container for all data for each rail line
class Railway{
  public:
    LinkedList checkpointList;
    std::vector<Checkpoint> checkpointVector;
    std::vector<float> distances;
    std::vector<int> scalers;
    std::vector<int> outputs;
    std::string name;
    std::vector<std::string> colors;
    int loopIndex = -1;

    Railway(LinkedList checkpointInput, std::vector<int> scalerInput, int outputSize, std::string nameInput, std::vector<std::string> colorInput){
      checkpointList = checkpointInput;
      checkpointVector = checkpointList.vectorForm;
      distances = std::vector<float>(checkpointVector.size(), 0);
      scalers = scalerInput;
      outputs = std::vector<int>(outputSize, 0);
      name = nameInput;
      colors = colorInput;
    }

    void addLoop(Checkpoint *input, int index, int loopIndex){
        checkpointList.createLoop(input, index, loopIndex);
        distances.push_back(0);
        loopIndex = index;
    }
};