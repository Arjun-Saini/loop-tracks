#include "Checkpoint.cpp"
#include <string>

//container for all data for each rail line
class Railway{
  public:
    //LinkedList checkpointList;
    std::vector<Checkpoint> checkpoints;
    std::vector<float> distances;
    std::vector<int> scalers;
    std::vector<std::vector<int>> outputs; //before, after, loop, green
    std::string name;
    std::vector<std::string> colors;
    int tripleIndex = -1;
    int loopIndex = -1;
    int lowerLoopBound, upperLoopBound, lowerGreenBound, upperGreenBound;

    Railway(std::vector<Checkpoint> checkpointInput, std::vector<int> scalerInput, std::vector<int> outputSize, std::string nameInput, std::vector<std::string> colorInput, std::vector<int> boundInput = {0, 0, 0, 0}){
      checkpoints = checkpointInput;
      distances = std::vector<float>(checkpoints.size(), 0);
      scalers = scalerInput;
      outputs = {std::vector<int>(outputSize[0], 0), std::vector<int>(outputSize[1], 0), std::vector<int>(outputSize[2], 0), std::vector<int>(outputSize[3], 0)};
      name = nameInput;
      colors = colorInput;
      lowerLoopBound = boundInput[0];
      upperLoopBound = boundInput[1];
      lowerGreenBound = boundInput[3];
      upperGreenBound = boundInput[4];
    }

    //loopIndex must point to either the first or last checkpoint, tripleIndex cannot be 0
    void setLoopIndex(int triple, int loop){
        tripleIndex = triple;
        loopIndex = loop;
    }
};