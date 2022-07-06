#include "math.h"
#include <vector>

//checkpoints mark each turn on a line
class Checkpoint{
  public:
    float lat;
    float lon;

    Checkpoint(float la, float lo){
      lat = la;
      lon = lo;
    }

    float getDistance(float trainLat, float trainLon){
      return sqrt(pow((trainLat - lat), 2) + pow((trainLon - lon), 2));
    }
};