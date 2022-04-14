#include <bits/stdc++.h>

using namespace std;

using ll = long long;

const ll ONE_MINUTE = 100;
const int NUM_TEMP_SENSORS = 8;
const int MIN_TEMPERATURE = -100;
const int MAX_TEMPERATURE = 70;
const int INVALID = 123456789;

vector<thread> tempSensors;
vector<int> currentTemps(NUM_TEMP_SENSORS);
vector<mt19937> generators(NUM_TEMP_SENSORS);
vector<uniform_int_distribution<int>> distributions(NUM_TEMP_SENSORS);
vector<atomic<int>> readingDone(NUM_TEMP_SENSORS);
vector<int> differencesIn10Minutes(10, INVALID);
vector<int> minReadingsIn1Hour(60, INVALID);
vector<int> maxReadingsIn1Hour(60, INVALID);
multiset<int> maxReadings;
multiset<int> minReadings;

atomic<bool> sensorsOn = false;


 // random number in the range [-100, 70] (both inclusive)
int randInRange(int sensor) {
  // separate number generator for each thread
  return distributions[sensor](generators[sensor]);
}


// simulate the temperature readings
void readTemperature(int sensor) {
  // continuously loop until all we are ready to start reading sensor readings
  while(!sensorsOn);
  while(sensorsOn) {

    // sleep for "1 minute" (which in our case, is 100 milliseconds)
    this_thread::sleep_for(std::chrono::milliseconds(ONE_MINUTE));

    // get our random temperature
    int temp = randInRange(sensor);
    
    // set the current temperature of the sensor as the read temperature
    currentTemps[sensor] = temp;
    
    // mark that this sensor is done reading
    readingDone[sensor] = true;
  }
}

int main() {
  
  // seed rand()
  srand(time(0));

  // so that each random number generator doesn't generate the same sequence
  // give each generator a random seed with rand()
  for(int sensor = 0; sensor < NUM_TEMP_SENSORS; sensor++) {
    // a seed for each threads random number generator
    int seed = rand();
    generators[sensor] = mt19937(seed);
    distributions[sensor] = uniform_int_distribution<int>(MIN_TEMPERATURE, MAX_TEMPERATURE);
  }

  // create the threads
  for(int sensor = 0; sensor < NUM_TEMP_SENSORS; sensor++)
    tempSensors.push_back(thread(readTemperature, sensor));

  // we will simulate for 1 day total
  sensorsOn = true;
  for(int minute = 1; minute <= 60 * 24; minute++) {
    while(true) {
      // loop until all sensors have read a temperature
      int counter = 0;
      for(int sensor = 0; sensor < NUM_TEMP_SENSORS; sensor++)
        if(readingDone[sensor])
          counter++;
      if(counter == NUM_TEMP_SENSORS) break;
    }
    // in the real world scenario, we have a full minute to do everything past this point
    // what do we need to do:
    //  - find the maximum and minimum of the day: for loop of length 8
    //  - set the difference in the readings: 1 operation
    //  - remove old reading: 6 logarithmic operations on a size of 60 (log(60) is ~6)
    //  - if there's a 10 minute report, find max difference: for loop of length 10
    //  - if there's a 1 hour report, find the 5 max and 5 min reading in the last hour: 10 operations

    // So in the worse case, we are doing <100 operations, 100 milliseconds, which is
    // what I use to simulate a minute, is more than enough time to do all this

    // mark that all sensors need to read again
    for(int sensor = 0; sensor < NUM_TEMP_SENSORS; sensor++)
      readingDone[sensor] = false;

    // get the current minimum and maximum of this minute
    int currentMin = numeric_limits<int>::max();
    int currentMax = numeric_limits<int>::min();
    for(int sensor = 0; sensor < NUM_TEMP_SENSORS; sensor++) {
      currentMin = min(currentMin, currentTemps[sensor]);
      currentMax = max(currentMax, currentTemps[sensor]);
    }

    // record the difference between the min and max sensor for this past minute
    differencesIn10Minutes[minute % 10] = currentMax - currentMin;

    // get the old minimum and the old maximum
    int oldMin = minReadingsIn1Hour[minute % 60];
    int oldMax = maxReadingsIn1Hour[minute % 60];

    // set the current minimum and current maximum for the minute
    minReadingsIn1Hour[minute % 60] = currentMin;
    maxReadingsIn1Hour[minute % 60] = currentMax;

    // if there was an old value, remove it from the min readings multiset
    if(oldMin != INVALID) {
      auto iterator = minReadings.find(oldMin);
      minReadings.erase(iterator);
    }

    // if there was an old value, remove it from the max readings multiset
    if(oldMax != INVALID) {
      auto iterator = maxReadings.find(oldMax);
      maxReadings.erase(iterator);
    }

    // insert the minimum and maximum readings in the multisets
    minReadings.insert(currentMin);
    maxReadings.insert(currentMax);

    if(minute % 10 == 0) {
      cout << "-- Report --" << endl;

      // get the maximum difference
      int maxDifference = 0;
      for(int i = 0; i < 10; i++)
        maxDifference = max(maxDifference, differencesIn10Minutes[i]);
      cout << "Maximum difference over the last 10 minutes: " << maxDifference << endl << endl;
    }
    if(minute % 60 == 0) {
      // 1 hour report
      auto maxIterator = prev(end(maxReadings));
      auto minIterator = begin(minReadings);

      // use the iterators to find the maximum 5 readings and the minimum 5 readings
      cout << "5 Max Readings in the Last Hour:";
      for(int i = 0; i < 5; i++) {
        cout << " " << (*maxIterator);
        maxIterator = prev(maxIterator);
      }
      cout << endl;

      cout << "5 Min Readings in the Last Hour:";
      for(int i = 0; i < 5; i++) {
        cout << " " << (*minIterator);
        minIterator = next(minIterator);
      }
      cout << endl << endl;
    }
  }

  // mark the sensors as off and wait for all the threads to close
  // (they will technically run another reading, but it doesn't matter since we are gonna ignore it)
  sensorsOn = false;
  for(int sensor = 0; sensor < NUM_TEMP_SENSORS; sensor++)
    tempSensors[sensor].join();

  cout << "24 hours have elapsed." << endl;
  return 0;
}
