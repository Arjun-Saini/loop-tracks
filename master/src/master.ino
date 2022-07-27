SYSTEM_MODE(MANUAL)

#include "HttpClient.h"
#include "JsonParserGeneratorRK.h"
#include "City.cpp"
#include "Adafruit_VL6180X.h"
#include "MQTT.h"
#include "Adafruit_GFX_RK.h"
#include "Adafruit_LEDBackpack_RK.h"

/*
all loop segment sizes must be the same
brown line receives all loop data for cta
no output segment can be 1, 12, or 24 pixels long, causes conflict with slave protocol
*/

// Chinatown to North/Clybourn
Railway redLineCTA = Railway(
    // checkpoints in order starting at slave position, should be a checkpoint at each bend/turn, no 3 adjacent checkpoints can form an angle smaller than 90 degrees
    {Checkpoint(41.853028, -87.63109), Checkpoint(41.9041, -87.628921), Checkpoint(41.903888, -87.639506), Checkpoint(41.913732, -87.652380), Checkpoint(41.9253, -87.65286)},
    {25, 3, 7, 5}, // pixels in between each checkpoint, should have 1 less element than checkpoint vector
    {0, 40, 0, 0}, // size of each output segment: before loop, after loop, in loop, in green
    "red",
    {"ff0000", "0a0000"}, // hex color values for head and body/tail of the train
    {0, 0, 0, 0}          // checkpoint bounds for lower loop, upper loop, lower green, upper green, these are only used for merging different rail colors onto one track
);

// Pulaski to Western (O'Hare branch)
Railway blueLineCTA = Railway(
    {Checkpoint(41.873647, -87.727931), Checkpoint(41.875666, -87.672961), Checkpoint(41.875293, -87.640976), Checkpoint(41.875660, -87.627620), Checkpoint(41.885738, -87.629540), Checkpoint(41.885698, -87.639828), Checkpoint(41.915497, -87.686258)},
    {12, 8, 5, 5, 5, 25},
    {0, 60, 0, 0},
    "blue",
    {"0000ff", "00000a"},
    {0, 0, 0, 0});

// Sedgewick to Loop
Railway brownLineCTA = Railway(
    {Checkpoint(41.9107586, -87.648068), Checkpoint(41.9103656, -87.6373962), Checkpoint(41.885840, -87.633990), Checkpoint(41.8770372, -87.6342823), Checkpoint(41.8767992, -87.6255196), Checkpoint(41.885921, -87.626137), Checkpoint(41.885840, -87.633990)},
    {5, 15, 10, 10, 10, 10},
    {20, 0, 40, 0},
    "brown",
    {"ffff00", "0a0a00"},
    {2, 6, 0, 0});

// Cermak-McCormick to Kedzie
Railway greenLineCTA = Railway(
    {Checkpoint(41.853115, -87.626402), Checkpoint(41.876946, -87.626046), Checkpoint(41.885921, -87.626137), Checkpoint(41.885724, -87.633945), Checkpoint(41.88422, -87.696234)},
    {15, 10, 10, 15},
    {15, 15, 20, 0},
    "green",
    {"00ff00", "000a00"},
    {1, 3, 0, 0});

// Halsted to Loop
Railway orangeLineCTA = Railway(
    {Checkpoint(41.84678, -87.648088), Checkpoint(41.85817, -87.627117), Checkpoint(41.875689, -87.626019), Checkpoint(41.876955, -87.626044), Checkpoint(41.885921, -87.626137), Checkpoint(41.885840, -87.633990), Checkpoint(41.876835, -87.633710), Checkpoint(41.8767992, -87.6255196)},
    {12, 7, 1, 10, 10, 10, 10},
    {12, 0, 40, 8},
    "orange",
    {"ff8000", "0a0500"},
    {3, 7, 1, 3});

// Sedgewick to Loop
Railway purpleLineCTA = Railway(
    {Checkpoint(41.9107586, -87.648068), Checkpoint(41.9103656, -87.6373962), Checkpoint(41.885840, -87.633990), Checkpoint(41.8770372, -87.6342823), Checkpoint(41.8767992, -87.6255196), Checkpoint(41.885921, -87.626137), Checkpoint(41.885840, -87.633990)},
    {5, 15, 10, 10, 10, 10},
    {20, 0, 40, 0},
    "purple",
    {"2000ff", "02000a"},
    {2, 6, 0, 0});

// Kedzie to Loop
Railway pinkLineCTA = Railway(
    {Checkpoint(41.853964, -87.705408), Checkpoint(41.854856, -87.6695341), Checkpoint(41.8849389, -87.6696133), Checkpoint(41.885840, -87.633990), Checkpoint(41.885921, -87.626137), Checkpoint(41.8767992, -87.6255196), Checkpoint(41.8770372, -87.6342823), Checkpoint(41.885840, -87.633990)},
    {7, 7, 6, 10, 10, 10, 10},
    {14, 0, 40, 6},
    "pink",
    {"ff8080", "0a0505"},
    {3, 7, 2, 3});

/// JFK/UMass to Harvard
Railway redLineMBTA = Railway{
    {Checkpoint(42.320414, -71.052139), Checkpoint(42.327649, -71.057760), Checkpoint(42.344627, -71.056986), Checkpoint(42.350559, -71.052613), Checkpoint(42.360982, -71.070116), Checkpoint(42.363490, -71.100487), Checkpoint(42.372535, -71.115947)},
    {5, 10, 5, 10, 10, 10},
    {0, 50, 0, 0},
    "red",
    {"ff0000", "0a0000"},
    {0, 0, 0, 0},
};

// Wood Island to Bowdoin
Railway blueLineMBTA = Railway{
    {Checkpoint(42.379551, -71.023236), Checkpoint(42.360008, -71.047623), Checkpoint(42.359257, -71.059615), Checkpoint(42.361149, -71.062128)},
    {20, 10, 10},
    {0, 40, 0, 0},
    "blue",
    {"0000ff", "00000a"},
    {0, 0, 0, 0},
};

// Roxbury Crossing to Wellington
Railway orangeLineMBTA = Railway{
    {Checkpoint(42.331520, -71.095285), Checkpoint(42.347474, -71.076055), Checkpoint(42.346967, -71.064553), Checkpoint(42.357606, -71.057324), Checkpoint(42.377410, -71.075941), Checkpoint(42.403125, -71.077024)},
    {10, 5, 10, 10, 10},
    {0, 45, 0, 0},
    "orange",
    {"ff8000", "0a0500"},
    {0, 0, 0, 0},
};

// Kenmore to Union Square
Railway greenLine1MBTA = Railway{
    {Checkpoint(42.348957, -71.095080), Checkpoint(42.349616, -71.079112), Checkpoint(42.351835, -71.070836), Checkpoint(42.352342, -71.064498), Checkpoint(42.362111, -71.057983), Checkpoint(42.366666, -71.061263), Checkpoint(42.365633, -71.064124), Checkpoint(42.377506, -71.095170)},
    {10, 10, 10, 10, 10, 10, 10},
    {0, 70, 0, 0},
    "green",
    {"00ff00", "000a00"},
    {0, 0, 0, 0},
};

// Longwood Medical Center to Back Bay(merges with green1)
Railway greenLine2MBTA = Railway{
    {Checkpoint(42.335878, -71.100229), Checkpoint(42.349616, -71.079112)},
    {15},
    {0, 15, 0, 0},
    "green",
    {"00ff00", "000a00"},
    {0, 0, 0, 0},
};

std::vector<City> cities;
std::vector<Railway> ctaRailways, mbtaRailways;

constexpr size_t I2C_BUFFER_SIZE = 512;

int brownLineCTAAdr = 0;
int greenLineCTAAdr[2] = {0, 0};

std::vector<int> addressArr;  // i2c addresses in numerical order
std::vector<int> sequenceArr; // organizes i2c addresses from addressArr
int slaveCount;

bool parseTrain(int trainIndex, Railway &currentRailway);
void onDataReceived(const uint8_t *data, size_t len, const BlePeerDevice &peer, void *context);
void lightshow(int length);
void callback(char *topic, byte *payload, unsigned int length);
void alphaDisplay(Adafruit_AlphaNum4 display, String str);

// const BleUuid serviceUuid("6E400001-B5A3-F393-E0A9-E50E24DCCA9E");
// const BleUuid rxUuid("6E400002-B5A3-F393-E0A9-E50E24DCCA9E");
// const BleUuid txUuid("6E400003-B5A3-F393-E0A9-E50E24DCCA9E");

const BleUuid serviceUuid("a73ba101-8192-4a51-b42d-ae9cd14b14a5");
const BleUuid rxUuid("6E400002-B5A3-F393-E0A9-E50E24DCCA9E");
const BleUuid txUuid("6E400003-B5A3-F393-E0A9-E50E24DCCA9E");

BleCharacteristic txCharacteristic("tx", BleCharacteristicProperty::NOTIFY, txUuid, serviceUuid);
BleCharacteristic rxCharacteristic("rx", BleCharacteristicProperty::WRITE_WO_RSP, rxUuid, serviceUuid, onDataReceived, NULL);

Adafruit_VL6180X vl = Adafruit_VL6180X();

MQTT client("lab.thewcl.com", 1883, callback);

http_header_t headers[] = {
    {"Accept", "/*/"},
    {NULL, NULL}};

http_request_t request;
http_response_t response;

HttpClient http;
JsonParserStatic<10000, 1000> parser;

String SSID = "";
String password = "";

Adafruit_AlphaNum4 display1 = Adafruit_AlphaNum4();

bool userInput = false;

int bleCount = 0;
int cityIndex = -1;
int railwayIndex = -1;
int cityIndexBuffer;

void setup()
{
    Serial.begin(9600);
    delay(500);

    // BLE setup
    BLE.on();
    BLE.addCharacteristic(txCharacteristic);
    BLE.addCharacteristic(rxCharacteristic);
    BleAdvertisingData data;
    data.appendServiceUUID(serviceUuid);
    BLE.advertise(&data);

    acquireWireBuffer();
    Wire.setClock(400000);
    Wire.begin();
    vl.begin();

    // request.hostname = "lapi.transitchicago.com";
    // request.port = 80;
    request.hostname = "trek.thewcl.com";
    request.port = 80;

    brownLineCTA.setLoopIndex(2, 6);
    orangeLineCTA.setLoopIndex(3, 7);
    purpleLineCTA.setLoopIndex(2, 6);
    pinkLineCTA.setLoopIndex(3, 7);
    ctaRailways = {brownLineCTA, purpleLineCTA, greenLineCTA, orangeLineCTA, pinkLineCTA, redLineCTA, blueLineCTA};

    // greenLine1 and greenLine2 must be in adjacent in the vector
    mbtaRailways = {redLineMBTA, greenLine1MBTA, greenLine2MBTA, blueLineMBTA, orangeLineMBTA};

    // 1 slave per line, except cta green which has 2 and cta purple which has 0 (7 for full cta)
    // there needs to be the same number of rail lines and slaves expected
    cities = {City(ctaRailways, "cta", 7), City(mbtaRailways, "mbta", 5)};

    display1.begin(0x71);
}

void loop()
{
    if (WiFi.hasCredentials() && userInput)
    {
        Serial.println("loop start");

        alphaDisplay(display1, "Test");

        for (int i : sequenceArr)
        {
            Serial.printf("%i, ", i);
        }
        Serial.println();

        cityIndexBuffer = cityIndex;
        for (int j = 0; j < cities[cityIndexBuffer].railways.size(); j++)
        {
            // rainbow led on when close proximity
            uint8_t range = vl.readRange();
            if (range <= 100)
            {
                Serial.println("proximity");
                lightshow(1000);
                return;
            }

            // MQTT
            if (client.isConnected())
            {
                client.subscribe("loop-tracks/twitter");
                client.loop();
                Serial.println("mqtt loop");
            }
            else
            {
                client.connect("sparkclient");
            }

            delay(1000);

            if (cityIndex == -1)
            {
                return;
            }
            // request.path = "/api/1.0/ttpositions.aspx?key=00ff09063caa46748434d5fa321d048f&rt=" + String(railways[j].name.c_str()) + "&outputType=JSON";
            request.path = "/loop-tracks/" + String(cities[cityIndexBuffer].name.c_str()) + "?lines=" + String(cities[cityIndexBuffer].railways[j].name.c_str());
            http.get(request, response, headers);

            Serial.println("parsing");
            parser.clear();
            parser.addString(response.body);
            if (!parser.parse())
            {
                Serial.println("parsing failed");
                return;
            }

            int trainIndex = 0;
            Railway currentRailway = cities[cityIndexBuffer].railways[j];

            // loop through each train, loop breaks when all trains have been parsed
            while (true)
            {
                if (parseTrain(trainIndex, currentRailway))
                {
                    break;
                }
                trainIndex++;
            }

            // outputs train data to slaves
            for (int i = 0; i < 4; i++)
            {
                // sets color of data being sent
                if (cityIndexBuffer == 0 && (i == 2 || currentRailway.name == purpleLineCTA.name))
                {
                    Wire.beginTransmission(brownLineCTAAdr);
                }
                else if (cityIndexBuffer == 0 && i == 3)
                {
                    if (currentRailway.name == orangeLineCTA.name)
                    {
                        Wire.beginTransmission(greenLineCTAAdr[0]);
                    }
                    else
                    {
                        Wire.beginTransmission(greenLineCTAAdr[1]);
                    }
                }
                else
                {
                    Wire.beginTransmission(sequenceArr[j * 2 + i]);
                }

                Wire.write(String(currentRailway.colors[0].c_str()));
                Wire.write(String(currentRailway.colors[1].c_str()));
                Wire.endTransmission();

                if (cityIndexBuffer == 0 && (i == 2 || currentRailway.name == purpleLineCTA.name))
                {
                    Wire.beginTransmission(brownLineCTAAdr);
                }
                else if (cityIndexBuffer == 0 && i == 3)
                {
                    if (currentRailway.name == orangeLineCTA.name)
                    {
                        Wire.beginTransmission(greenLineCTAAdr[0]);
                    }
                    else
                    {
                        Wire.beginTransmission(greenLineCTAAdr[1]);
                    }
                }
                else
                {
                    Wire.beginTransmission(sequenceArr[j * 2 + i]);
                }

                // padding for chicago
                if (cityIndexBuffer == 0)
                {
                    if (i == 2)
                    {
                        // pads green line sending to loop
                        if (currentRailway.name == greenLineCTA.name)
                        {
                            for (int j = 0; j < brownLineCTA.outputs[0].size() + (brownLineCTA.outputs[2].size() / 2); j++)
                            {
                                Wire.write('0');
                            }
                        }
                        // prevents brown/purple line from getting overriden
                        else if (currentRailway.name == brownLineCTA.name || currentRailway.name == purpleLineCTA.name)
                        {
                            for (int j = 0; j < brownLineCTA.outputs[0].size(); j++)
                            {
                                Wire.write((char)currentRailway.outputs[0][j] + '0');
                            }
                        }
                        // pads every other line sending to loop
                        else
                        {
                            for (int j = 0; j < brownLineCTA.outputs[0].size(); j++)
                            {
                                Wire.write('0');
                            }
                        }
                    }

                    // pads blank loop onto the brown/purple track
                    if (i == 1 && (currentRailway.name == brownLineCTA.name || currentRailway.name == purpleLineCTA.name))
                    {
                        for (int j = 0; j < currentRailway.outputs[2].size(); j++)
                        {
                            Wire.write('0');
                        }
                    }

                    // pad blank green segment onto orange and pink tracks
                    if (i == 3 && currentRailway.name == orangeLineCTA.name)
                    {
                        for (int j = 0; j < greenLineCTA.outputs[0].size() - orangeLineCTA.outputs[3].size(); j++)
                        {
                            Wire.write('0');
                        }
                    }
                    else if (i == 3 && currentRailway.name == pinkLineCTA.name)
                    {
                        for (int j = 0; j < greenLineCTA.outputs[0].size() - pinkLineCTA.outputs[3].size(); j++)
                        {
                            Wire.write('0');
                        }
                    }
                }

                // sends data to slave
                Serial.printf("%s rail part %i: ", currentRailway.name.c_str(), i);
                for (int j = 0; j < currentRailway.outputs[i].size(); j++)
                {
                    Wire.write((char)currentRailway.outputs[i][j] + '0');
                    Serial.print(currentRailway.outputs[i][j]);
                    // currentRailway.outputs[i][j] = 0;
                }
                Serial.println();
                Wire.endTransmission();
            }
            for (int i = 0; i < 4; i++)
            {
                for (int j = 0; j < currentRailway.outputs[i].size(); j++)
                {
                    currentRailway.outputs[i][j] = 0;
                }
            }
            Serial.println();
        }
        Serial.println();
    }
}
/**
 * Parses a train from the HTTP API and stores it in the railways array.
 * @param trainIndex The index of train in the railway.
 * @param currentRailway The current railway we are on.
 * @return true if there are no more trains to parse, false otherwise.
 */
bool parseTrain(int trainIndex, Railway &currentRailway)
{
    // parse json data returned from api
    JsonReference train = parser.getReference().key("lines").index(0).key("trains").index(trainIndex);
    String nextStation = train.key("next_stop").valueString();
    String destNm = train.key("destination").valueString();
    int trainDir = train.key("direction").valueInt();
    float lat = train.key("latitude").valueFloat();
    float lon = train.key("longitude").valueFloat();

    // break loop once all trains have been parsed
    if (nextStation.length() <= 1)
    {
        Serial.println("break");
        return true;
    }
    // Serial.print(String(currentRailway.name.c_str()) + " ");
    // Serial.printf("train %i: ", count);

    std::vector<Checkpoint> currentCheckpoints = currentRailway.checkpoints;
    int checkpointCount = currentCheckpoints.size();

    // finds index of closest checkpoint to train
    for (int i = 0; i < checkpointCount; i++)
    {
        currentRailway.distances[i] = currentCheckpoints[i].getDistance(lat, lon);
    }
    int closestIndex = std::min_element(currentRailway.distances.begin(), currentRailway.distances.end()) - currentRailway.distances.begin();
    if (closestIndex == currentRailway.tripleIndex)
    {
        closestIndex = currentRailway.loopIndex;
    }

    // calculates which checkpoint is on the other side of the train from the nearest checkpoint, works when turns are 90 degrees or less
    float x, x1, y, y1, slope, perpendicularSlope;
    int secondClosestIndex;

    x = lat;
    y = lon;
    x1 = currentCheckpoints[closestIndex].lat;
    y1 = currentCheckpoints[closestIndex].lon;

    if (currentCheckpoints[closestIndex].lat > lat)
    {
        slope = (y1 - y) / (x1 - x);
    }
    else
    {
        slope = (y - y1) / (x - x1);
    }

    // prevents division by zero or extremely large numbers
    if (slope == 0)
    {
        perpendicularSlope = __FLT_MAX__ / 10;
    }
    else if (slope >= __FLT_MAX__ / 10)
    {
        perpendicularSlope = 0;
    }
    else
    {
        perpendicularSlope = -1 / slope;
    }

    bool pointSide, nearestSide, loopPointSide;
    bool validTrain = true;

    // point slope formula to determine which checkpoint is on the other side of the train from the nearest checkpoint
    if (closestIndex == 0)
    {
        pointSide = (perpendicularSlope * (currentCheckpoints[closestIndex + 1].lat - x) + y) > currentCheckpoints[closestIndex + 1].lon;
        nearestSide = (perpendicularSlope * (currentCheckpoints[closestIndex].lat - x) + y) > currentCheckpoints[closestIndex].lon;
        if (currentRailway.loopIndex == closestIndex)
        {
            loopPointSide = (perpendicularSlope * (currentCheckpoints[currentRailway.tripleIndex - 1].lat - x) + y) > currentCheckpoints[currentRailway.tripleIndex - 1].lon;
            if (nearestSide != pointSide)
            {
                secondClosestIndex = 1;
            }
            else if (nearestSide != loopPointSide)
            {
                closestIndex = currentRailway.tripleIndex;
                secondClosestIndex = currentRailway.tripleIndex - 1;
            }
            else
            {
                closestIndex = currentRailway.tripleIndex;
                secondClosestIndex = currentRailway.tripleIndex + 1;
            }
        }
        else
        {
            if (pointSide == nearestSide)
            {
                validTrain = false;
            }
            else
            {
                secondClosestIndex = 1;
            }
        }
    }
    else
    {
        pointSide = (perpendicularSlope * (currentCheckpoints[closestIndex - 1].lat - x) + y) > currentCheckpoints[closestIndex - 1].lon;
        nearestSide = (perpendicularSlope * (currentCheckpoints[closestIndex].lat - x) + y) > currentCheckpoints[closestIndex].lon;
        if (closestIndex == checkpointCount - 1)
        {
            if (closestIndex == currentRailway.loopIndex)
            {
                loopPointSide = (perpendicularSlope * (currentCheckpoints[currentRailway.tripleIndex - 1].lat - x) + y) > currentCheckpoints[currentRailway.tripleIndex - 1].lon;
                if (nearestSide != pointSide)
                {
                    secondClosestIndex = checkpointCount - 2;
                }
                else if (nearestSide != loopPointSide)
                {
                    closestIndex = currentRailway.tripleIndex;
                    secondClosestIndex = currentRailway.tripleIndex - 1;
                }
                else
                {
                    closestIndex = currentRailway.tripleIndex;
                    secondClosestIndex = currentRailway.tripleIndex + 1;
                }
            }
            else
            {
                if (pointSide == nearestSide)
                {
                    validTrain = false;
                }
                else
                {
                    secondClosestIndex = checkpointCount - 2;
                }
            }
        }
        else
        {
            if (pointSide == nearestSide)
            {
                secondClosestIndex = closestIndex + 1;
            }
            else
            {
                secondClosestIndex = closestIndex - 1;
            }
        }
    }

    int pcbSegment;
    float segmentPos;
    bool inLoop = false;
    // calculates train position within the segment
    if (validTrain)
    {
        // Serial.printlnf("closestIndex: %i second: %i", closestIndex, secondClosestIndex);
        int lowerIndex, upperIndex;

        if (closestIndex < secondClosestIndex)
        {
            lowerIndex = closestIndex;
            upperIndex = secondClosestIndex;
        }
        else
        {
            lowerIndex = secondClosestIndex;
            upperIndex = closestIndex;
        }

        segmentPos = currentRailway.distances[lowerIndex] / (currentRailway.distances[lowerIndex] + currentRailway.distances[upperIndex]);
        segmentPos *= currentRailway.scalers[lowerIndex];

        int lowerScaleBound;
        // in green
        if (closestIndex <= currentRailway.upperGreenBound && closestIndex >= currentRailway.lowerGreenBound && secondClosestIndex <= currentRailway.upperGreenBound && secondClosestIndex >= currentRailway.lowerGreenBound)
        {
            pcbSegment = 3;
            lowerScaleBound = currentRailway.lowerGreenBound;
        }
        // before loop
        else if (closestIndex < currentRailway.lowerLoopBound || secondClosestIndex < currentRailway.lowerLoopBound)
        {
            lowerScaleBound = 0;
            pcbSegment = 0;
        }
        // after loop, also for lines that don't go through loop
        else if (closestIndex > currentRailway.upperLoopBound || secondClosestIndex > currentRailway.upperLoopBound)
        {
            lowerScaleBound = currentRailway.upperLoopBound;
            pcbSegment = 1;
        }
        // in loop
        else
        {
            inLoop = true;
            lowerScaleBound = currentRailway.lowerLoopBound;
            pcbSegment = 2;
        }

        for (int i = lowerScaleBound; i < lowerIndex; i++)
        {
            segmentPos += currentRailway.scalers[i];
        }

        // cta direction fixes
        if (cityIndexBuffer == 0)
        {
            if (inLoop)
            {
                // adjusts output array orientation to match brown line
                if (currentRailway.name == pinkLineCTA.name)
                {
                    segmentPos = (float)currentRailway.outputs[2].size() - segmentPos;
                }
                else if (currentRailway.name == orangeLineCTA.name)
                {
                    segmentPos = (int)(segmentPos + orangeLineCTA.outputs[2].size() / 2) % orangeLineCTA.outputs[2].size();
                }
                else if (currentRailway.name == purpleLineCTA.name)
                {
                    trainDir = 1;
                }
            }
            else
            {
                if (currentRailway.name == brownLineCTA.name || currentRailway.name == purpleLineCTA.name)
                {
                    trainDir = 6 - trainDir;
                }
            }
            if (pcbSegment == 1 && currentRailway.name == greenLineCTA.name)
            {
                segmentPos = (float)currentRailway.outputs[1].size() - segmentPos;
                trainDir = 6 - trainDir;
            }
            if (pcbSegment == 3 && currentRailway.name == pinkLineCTA.name)
            {
                trainDir = 6 - trainDir;
            }
        }
        // mbta direction fixes
        else if (cityIndexBuffer == 1)
        {
            if (currentRailway.name == orangeLineMBTA.name || currentRailway.name == greenLine1MBTA.name || currentRailway.name == redLineMBTA.name)
            {
                trainDir = 6 - trainDir;
            }
        }

        currentRailway.outputs[pcbSegment][(int)floor(segmentPos)] = trainDir;
    }

    return false;
}

String *deviceIDArr;
int iterationCount = 0;
int i2cRequestCount = 0;
// Clears up conflicts with multiple i2c slaves having the same address.
void randomizeAddress()
{
    deviceIDArr = new String[cities[cityIndex].slaveCountExpected];
    while (slaveCount < cities[cityIndex].slaveCountExpected)
    {
        Serial.printlnf("slaveCount: %i", slaveCount);
        slaveCount = 0;
        iterationCount++;
        for (int i = 8; i <= 111; i++)
        {
            i2cRequestCount++;
            if (i == 41)
            {
                continue;
            }
            Serial.println("\nrequest code 1, address: " + String(i));
            Wire.beginTransmission(i);
            Wire.write('1');
            Wire.endTransmission();

            // gets UUID from address and sends it back to slave, slave changes address if conflict
            Wire.requestFrom(i, 24);
            if (Wire.available() > 0)
            {
                Serial.println("transmission recieved from: " + String(i));

                String inputBuffer = "";
                char c;
                for (int j = 0; j < 24; j++)
                {
                    c = Wire.read();
                    inputBuffer += c;
                }
                Wire.beginTransmission(i);
                Wire.write(inputBuffer);
                Serial.println("device id: " + inputBuffer);
                deviceIDArr[slaveCount] = inputBuffer;
                Wire.endTransmission();
                Serial.println("transmission sent to: " + String(i));

                Wire.beginTransmission(i);
                Wire.write('2');
                Wire.endTransmission();

                Serial.println("request code 2, address: " + String(i));
                Wire.requestFrom(i, 4);
                inputBuffer = "";
                for (int j = 0; j < 4; j++)
                {
                    inputBuffer += (char)Wire.read();
                }

                Serial.println("conflict verification: " + inputBuffer);
                if (inputBuffer != "pass")
                {
                    break;
                }
                if (++slaveCount == cities[cityIndex].slaveCountExpected)
                {
                    break;
                }
            }
        }
    }

    Serial.println("\nConnected to: ");

    int count = 0;
    for (int i = 8; i <= 111; i++)
    {
        if (i == 41)
        {
            continue;
        }
        Wire.beginTransmission(i);
        Wire.write('1');
        Wire.endTransmission();

        Wire.requestFrom(i, 24);
        if (Wire.available() > 0)
        {
            Serial.print(i);
            Serial.print(", ");

            addressArr[count++] = i;
        }
    }
    Serial.println();
    for (int i = 0; i < slaveCount; i++)
    {
        Serial.println(deviceIDArr[i]);
    }
    Serial.println(iterationCount);
    Serial.println(i2cRequestCount);
}

// Communication with app, configures city and rail colors.
void onDataReceived(const uint8_t *data, size_t len, const BlePeerDevice &peer, void *context)
{
    txCharacteristic.setValue("ok");
    String inputBuffer = "";
    String nameBuffer;
    for (int i = 0; i < len; i++)
    {
        inputBuffer += (char)data[i];
    }
    Serial.println(inputBuffer);

    // set wifi credentials
    if (inputBuffer.indexOf("wifi:") == 0)
    {
        int commaIndex = inputBuffer.indexOf(",");
        SSID = inputBuffer.substring(5, commaIndex);
        password = inputBuffer.substring(commaIndex + 1);
        Serial.printlnf("SSID: %s, password: %s", SSID.c_str(), password.c_str());
        WiFi.setCredentials(SSID, password);
    }
    // set city
    else if (inputBuffer.indexOf("city:") == 0)
    {
        inputBuffer = inputBuffer.substring(5);
        for (int i = 0; i < cities.size(); i++)
        {
            if (inputBuffer == String(cities[i].name.c_str()))
            {
                cityIndex = i;
            }
        }
        if (cityIndex == -1)
        {
            txCharacteristic.setValue("incorrect city name");
            return;
        }
        sequenceArr = std::vector<int>(cities[cityIndex].railways.size() * 2, 0);
        addressArr = std::vector<int>(cities[cityIndex].slaveCountExpected, 0);
        randomizeAddress();
        txCharacteristic.setValue("slave addresses sorted");

        // turn on led on first device
        Wire.beginTransmission(addressArr[0]);
        Wire.write('3');
        Wire.endTransmission();
        Serial.printlnf("city: %s", cities[cityIndex].name.c_str());
    }
    // set lines
    else if (inputBuffer.indexOf("color:") == 0)
    {
        if (cityIndex == -1)
        {
            txCharacteristic.setValue("city not set");
            return;
        }
        inputBuffer = inputBuffer.substring(6);
        nameBuffer = inputBuffer;
        if (bleCount < cities[cityIndex].railways.size())
        {
            if ((cityIndex == 0 || cityIndex == 1) && (inputBuffer == "green1" || inputBuffer == "green2"))
            {
                nameBuffer = "green";
            }
            // finds which rail the address should be set to
            for (int i = 0; i < cities[cityIndex].railways.size(); i++)
            {
                if ((cityIndex != 0 || nameBuffer != String(purpleLineCTA.name.c_str())) && String(cities[cityIndex].railways[i].name.c_str()) == nameBuffer)
                {
                    railwayIndex = i;
                }
            }
            if (cityIndex == 1 && inputBuffer == "green1")
            {
                railwayIndex--;
            }
            Serial.printlnf("railway index: %i", railwayIndex);
            if (railwayIndex == -1)
            {
                txCharacteristic.setValue("incorrect railway color");
                return;
            }

            // organizes address into sequenceArr
            for (int i = 0; i < 2; i++)
            {
                Serial.printlnf("address: %i", addressArr[bleCount]);
                if (cities[cityIndex].railways[railwayIndex].outputs[i].size() == 0 || (cityIndex == 0 && cities[cityIndex].railways[railwayIndex].name == purpleLineCTA.name))
                {
                    sequenceArr[2 * railwayIndex + i] = 0;
                }
                else
                {
                    if (cityIndex == 0 && nameBuffer == "green")
                    {
                        if (inputBuffer == "green1")
                        {
                            sequenceArr[2 * railwayIndex] = addressArr[bleCount];
                        }
                        else if (inputBuffer == "green2")
                        {
                            sequenceArr[2 * railwayIndex + 1] = addressArr[bleCount];
                        }
                    }
                    else
                    {
                        sequenceArr[2 * railwayIndex + i] = addressArr[bleCount];
                    }
                    if (cityIndex == 0)
                    {
                        if (inputBuffer == String(brownLineCTA.name.c_str()))
                        {
                            brownLineCTAAdr = sequenceArr[2 * railwayIndex + i];
                        }
                        else if (inputBuffer == "green1")
                        {
                            greenLineCTAAdr[0] = sequenceArr[2 * railwayIndex];
                            // bleCount++;
                        }
                        else if (inputBuffer == "green2")
                        {
                            greenLineCTAAdr[1] = sequenceArr[2 * railwayIndex + 1];
                        }
                    }
                }
            }

            // turn off led, turn next device led on
            Wire.beginTransmission(addressArr[bleCount]);
            Wire.write('4');
            Wire.endTransmission();

            Wire.beginTransmission(addressArr[bleCount + 1]);
            Wire.write('3');
            Wire.endTransmission();
        }
        if (bleCount == cities[cityIndex].railways.size() - 1)
        {
            Serial.println("BLE finished");
            // for(int i = 0; i < addressArr.size(); i++){
            //   Serial.printlnf("turning off: %i", i);
            //   Wire.beginTransmission(addressArr[i]);
            //   Wire.write('4');
            //   Wire.endTransmission();
            // }
            userInput = true;
            WiFi.on();
            WiFi.connect();
            client.connect("sparkclient");
        }
        bleCount++;
    }
    else if (inputBuffer.indexOf("reset") == 0)
    {
        for (int i = 0; i < addressArr.size(); i++)
        {
            Serial.printlnf("turning off: %i", i);
            Wire.beginTransmission(addressArr[i]);
            Wire.write('4');
            Wire.endTransmission();
        }
        bleCount = 0;
        cityIndex = -1;
        railwayIndex = -1;
        userInput = false;
        Serial.println("reset done");
    }
}

// Increases I2C buffer size
hal_i2c_config_t acquireWireBuffer()
{
    hal_i2c_config_t config = {
        .size = sizeof(hal_i2c_config_t),
        .version = HAL_I2C_CONFIG_VERSION_1,
        .rx_buffer = new (std::nothrow) uint8_t[I2C_BUFFER_SIZE],
        .rx_buffer_size = I2C_BUFFER_SIZE,
        .tx_buffer = new (std::nothrow) uint8_t[I2C_BUFFER_SIZE],
        .tx_buffer_size = I2C_BUFFER_SIZE};
    return config;
}

/**
 * @brief Sends rainbow command to every slave.
 * @param length Duration in ms for rainbow to last.
 */
void lightshow(int length)
{
    for (int i = 0; i < addressArr.size(); i++)
    {
        Wire.beginTransmission(addressArr[i]);
        Wire.write('3');
        Wire.endTransmission();
    }
    delay(length);
    for (int i = 0; i < addressArr.size(); i++)
    {
        Wire.beginTransmission(addressArr[i]);
        Wire.write('4');
        Wire.endTransmission();
    }
}

// MQTT callback for twitter response
void callback(char *topic, byte *payload, unsigned int length)
{
    Serial.println("twitter");
    lightshow(3000);
}

// Display 4 characters on the display
void alphaDisplay(Adafruit_AlphaNum4 display, String str)
{
    for (int i = 0; i < 4; i++)
    {
        display.writeDigitAscii(i, str.charAt(i));
    }
    display.writeDisplay();
}