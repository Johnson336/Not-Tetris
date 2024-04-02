#ifndef __INDIVIDUALHPP
#define __INDIVIDUALHPP

#include <iostream>
#include <random>
#include <map>
#include <sqlite3.h>
#include <sstream>
#include "math.h"


// Valid Moves -- Possible Moves
const int NUM_MOVES= 5;
const std::string MOVES = "wsadz";
std::unordered_map<char, int> moves_index = {{'w',0},{'s',1},{'a',2},{'d',3},{'z',4}};
/*
    w = rotate
    s = soft drop
    a = move left
    d = move right
    z = drop piece
    e = hold piece // currently disabled for AI
*/

// Generate random numbers in a range
int random_num(int min, int max) {
  const int value = (rand()% (max + 1 - min)) + min;
  return value;
}

// convert from string to vector with delimiter
std::vector<float> strToVect(std::string str, char delim) {
  std::istringstream iss(str);
  std::vector<float> out;
  std::string element;
  while (getline(iss, element, delim)) {
    float f = stof(element);
    out.push_back(f);
  }
  return out;
}

// convert from vector to string with delimiter
std::string vectToStr(std::vector<float> values, char delim) {
  std::ostringstream oss;
  for (int i=0;i<values.size();i++) {
    oss << values[i];
    if (i != values.size()-1) {
       oss << delim;
    }
  }
  return oss.str();
}

static std::random_device rd;
static std::mt19937 gen(rd());
static std::uniform_real_distribution<float> dis(0.0, 1.0);

// Class representing an Individual of a population
class Individual {
private:
  // This holds the value of each move that we can take for a specific
  // Individual brain is a map of string keys to int[NUM_MOVES] array
  // Board state
  sqlite3 *db;
  float learningRate;
  float gamma;
  float explorationRate;
  float minExplorationRate;
  float explorationDecayRate;


public:
  // Take an existing database and create an Individual with it
  Individual(sqlite3 *database, float learningRate, float gamma, float explorationRate, float minExplorationRate, float explorationDecayRate); 
  // Find the map for a given board state and return the int array of values
  std::vector<float> findMap(const std::string &state);
  // Find the map for a given board state and return the value of the given action
  int getMoveValue(std::string &state, char action);
  // Store the value of an action for a given state
  void storeMoveValue(const std::string &state, char action, float value, std::string &nextState);
  // find the best action to perform for a given board state
  char getBestAction(std::string &state);
  // get total nodes in the database
  int getNodeCount();
  // decay the rate at which we explore down to the minimum
  void decayExplorationRate();
  // get max q-value for a given state
  float getMaxQValue(std::string &state);
  // get a random int between 2 limits, upper and lower inclusive
  int getRandomInt(int lower, int upper);
  //get a random double between 2 limits, upper and lower limit inclusive
  double getRandomDouble(double lower, double upper);
  void storeHighScore(int gameNum, int score);
  int getNumScores();
  std::vector<std::pair<int, int>> getHighScores();
};

int Individual::getRandomInt(int lower, int upper) {
  int range = upper-lower;
  return (dis(gen)*range);
}

double Individual::getRandomDouble(double lower, double upper) {
  double range = (upper-lower); 
  return (dis(gen)*range);
};

extern void printBoardSilhouette(const std::string &board);
extern void printBoardState(const std::string &board);

// get a vector of floats for a given game state
std::vector<float> Individual::findMap(const std::string &state) {
  // get back an array of movement values for a given state
  char* errMsg = nullptr;

  std::string createTableQuery = "CREATE TABLE IF NOT EXISTS brain (key TEXT PRIMARY KEY, value TEXT);";
  sqlite3_exec(db, createTableQuery.c_str(), nullptr, 0, &errMsg);

  // check if the key exists in database and retrieve the vector of floats if it does
  std::vector<float> values;
  std::string selectQuery = "SELECT value FROM brain WHERE key='" + state + "';";
  sqlite3_stmt *stmt;
  int rc = sqlite3_prepare_v2(db, selectQuery.c_str(), -1, &stmt, nullptr);
  if (rc == SQLITE_OK) {
    if (sqlite3_step(stmt) == SQLITE_ROW) {
      std::string value(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)));

      // parse the string and convert it into a vector of floats
      values = strToVect(value, ',');
    }
  }
  sqlite3_finalize(stmt);
  if (values.empty()) {
    values = {0,0,0,0,0};
  }
  //printBoardState(state);
  //printf("Pull Map:");
  //for (int i=0;i<values.size();i++) {
  //  printf(" %2.2f", values[i]);
  //}
  //printf("\n");
  return values;
}

void Individual::storeHighScore(int gameNum, int score) {
  char* errMsg = nullptr;

  std::string createTableQuery = "CREATE TABLE IF NOT EXISTS scores (game INTEGER PRIMARY KEY, score INTEGER);";
  sqlite3_exec(db, createTableQuery.c_str(), nullptr, 0, &errMsg);

  std::string insertQuery = "INSERT OR REPLACE INTO scores (game, score) VALUES ('" + std::to_string(gameNum) + "', '" + std::to_string(score) + "');";
  sqlite3_exec(db, insertQuery.c_str(), nullptr, 0, &errMsg);

}

int Individual::getNumScores() {
  int count = 0;
  char *errMsg;

  std::string countQuery = "SELECT COUNT(*) FROM scores;";
  sqlite3_stmt *stmt;
  int rc = sqlite3_prepare_v2(db, countQuery.c_str(), -1, &stmt, nullptr);
  if (rc == SQLITE_OK) {
    if (sqlite3_step(stmt) == SQLITE_ROW) {
      count = sqlite3_column_int(stmt, 0);
    }
  }
  sqlite3_finalize(stmt);

  return count;
}

std::vector<std::pair<int, int>> Individual::getHighScores(void) {
  std::vector<std::pair<int, int>> scores;
  sqlite3_stmt *stmt;
  char *errMsg;

  // assert table creation
  std::string createTableQuery = "CREATE TABLE IF NOT EXISTS scores (game INTEGER PRIMARY KEY, score INTEGER);";
  sqlite3_exec(db, createTableQuery.c_str(), nullptr, 0, &errMsg);

  // retrieve scores
  std::string query = "SELECT game, score FROM scores;";
  if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
    while (sqlite3_step(stmt) == SQLITE_ROW) {
      int game = sqlite3_column_int(stmt, 0);
      int score = sqlite3_column_int(stmt, 1);
      scores.push_back({game, score});
    }
    sqlite3_finalize(stmt);
  } else {
    std::cerr << "SQL Error: " << sqlite3_errmsg(db) << std::endl;
  }
  return scores;
}


Individual::Individual(sqlite3 *database, float learningRate, float gamma, float explorationRate, float minExplorationRate, float explorationDecayRate) {
  db = database;
  this->learningRate = learningRate;
  this->gamma = gamma;
  this->explorationRate = explorationRate;
  this->minExplorationRate = minExplorationRate;
  this->explorationDecayRate = explorationDecayRate;  
  std::srand(static_cast<unsigned int>(time(0))); // seed number generator on creation
}


void Individual::storeMoveValue(const std::string &state, char action, float reward, std::string &nextState) {
  std::vector<float> values = findMap(state);
  int index = moves_index[action];
  char *errMsg;
  // qLearning algorithm update
  float oldValue = values[index];
  float bestFutureValue = getMaxQValue(nextState);
  //float newValue = oldValue + learningRate * (reward + gamma * bestFutureValue - oldValue);
  float newValue = (1-learningRate) * oldValue + learningRate * (reward + gamma * bestFutureValue);
  values[index] = newValue;
  // OUTDATED BELOW
  //if (newValue != values[index]) {
    //printf("Updating move %c value %2.2f with new value %2.2f\n", action, values[index], (values[index] + newValue) / 2);
    //values[index] = std::fmaxf(values[index], newValue);
    // update move value with new sequence value
    // average the existing value together with the new Value
    //if (newValue > 20) {
    //  printf("state[%s][%c]=%2.2f\n", state.c_str(), action, newValue);
    //}
    //values[index] = newValue;
  //}

  // convert vector of floats into comma-separated string for storage
  std::string strValues = vectToStr(values, ',');

  // insert or replace the data into the database
  std::string insertQuery = "INSERT OR REPLACE INTO brain (key, value) VALUES ('" + state + "', '" + strValues + "');";
  sqlite3_exec(db, insertQuery.c_str(), nullptr, 0, &errMsg);
  //printBoardState(state);
  //printf("Push Map: %s %c\n", strValues.c_str(), action);
}


char Individual::getBestAction(std::string &state) {
  float maxValue = 0;
  //printf("Best move state: %s\n", state.c_str());
  char bestAction = MOVES.at(random_num(0, NUM_MOVES-1));
  if (dis(gen) < explorationRate) {
    // choose a random action
    return bestAction;
  } else {
    std::vector<float> floats = findMap(state);

    // find max value in resulting float vector
    for (int i=0;i<floats.size();i++) {
      // only use a "best" value if it exceeds some given threshold
      if (floats[i] > maxValue) {
        maxValue = floats[i];
        bestAction = MOVES.at(i);
      }
    }
    //printf("Action %c value %2.2f\n", bestAction, maxValue);
    return bestAction;
    //printf("\n");
  }
}

int Individual::getNodeCount() {
  int count = 0;
  char *errMsg;

  // execute query to count rows
  std::string countQuery = "SELECT COUNT(*) FROM brain;";
  sqlite3_stmt *stmt;
  int rc = sqlite3_prepare_v2(db, countQuery.c_str(), -1, &stmt, nullptr);
  if (rc == SQLITE_OK) {
    if (sqlite3_step(stmt) == SQLITE_ROW) {
      count = sqlite3_column_int(stmt, 0);
    }
  }
  sqlite3_finalize(stmt);

  return count;
}

void Individual::decayExplorationRate() {
  explorationRate = std::fmaxf(minExplorationRate, std::expf(-explorationDecayRate*M_E));
  //explorationRate = std::fmaxf(minExplorationRate, explorationRate * explorationDecayRate);
}


float Individual::getMaxQValue(std::string &state) {
  float maxValue = 0;
  std::vector<float> floats = findMap(state);

  //printf("Floats: \n");
  // find max value in resulting float vector
  // if (floats.empty()) { maxValue = 0.0f; }
  for (int i=0;i<floats.size();i++) {
    //printf(" %2.2f", floats[i]);
    // only use a "best" value if it exceeds some given threshold
    maxValue = (floats[i] > maxValue ? floats[i] : maxValue);
  }
  //printBoardState(state);
  //printf("maxQ: %2.2f\n", maxValue);
  //printf("\n");
  return maxValue;
}

#endif
