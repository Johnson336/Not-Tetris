#ifndef __INDIVIDUALHPP
#define __INDIVIDUALHPP

#include <random>
#include <map>
#include <sqlite3.h>
#include <sstream>


// Number of individuals in each generation
#define POPULATION_SIZE 1
#define GENOME_SIZE 1

// Valid Genes -- Possible Moves
const int NUM_GENES = 5;
const std::string GENES = "wsadz";
std::unordered_map<char, int> gene_index = {{'w',0},{'s',1},{'a',2},{'d',3},{'z',4}};
/*
    w = rotate
    s = soft drop
    a = move left
    d = move right
    z = drop piece
    e = hold piece // currently disabled for AI
*/

// Generate random numbers in a range
int random_num(int start, int end) {
  int range = (end-start)+1;
  int random_int = start+(rand()%range);
  return random_int;
}

// Create random gene for movement
char create_gene() {
    return GENES.at(random_num(0, NUM_GENES-1));
}


// Create random genes for mutation
char mutated_genes() {
    return GENES.at(random_num(0, GENES.size()-1));
}

std::string create_genome() {
    // this is where we generate a string that represents moving a piece
    std::string genome = "";
    for (int i=0;i<GENOME_SIZE;i++) {
        // assigning 10 random moves to our movement queue
        genome += mutated_genes();
    }
    return genome;
}

// Class representing an Individual of a population
class Individual {
    public:
        // Individual brain is a map of string keys to int[NUM_GENES] array
        // This holds the value of each move that we can take for a specific
        // Board state
        sqlite3 *db;
        // Take an existing database and create an Individual with it
        Individual(sqlite3 *database); 
        std::unordered_map<std::string, std::vector<float>> brain;
        // Take an existing brain as an argument and create an individual with it
        Individual(std::unordered_map<std::string, std::vector<float>> &brain);
        // This Individual performs mating with another Individual parameter
        Individual mate(Individual &parent2);
        // Find the map for a given board state and return the int array of values
        std::vector<float> &findMap(std::string &state);
        // Find the map for a given board state and return the value of the given action
        int getMoveValue(std::string &state, char action);
        // Store the value of an action for a given state
        void storeMoveValue(std::string &state, char action, float value);
        // find the best action to perform for a given board state
        char getBestAction(std::string &state, int chance, int theshold);
        // get total nodes in the database
        int getNodeCount();
};


std::vector<float> &Individual::findMap(std::string &state) {
  // get back an array of movement values for a given state
  std::unordered_map<std::string, std::vector<float>>::iterator it = this->brain.find(state);
  if (it == this->brain.end()) {
    this->brain[state] = std::vector<float>{0,0,0,0,0};
  }
  return this->brain[state];
}

Individual::Individual(std::unordered_map<std::string, std::vector<float>> &brain) {
    this->brain = brain;
};

Individual::Individual(sqlite3 *database) {
  this->db = database;
}

int Individual::getMoveValue(std::string &state, char action) {
  std::vector<float> neuron = findMap(state);
  return neuron[gene_index[action]];
}

void Individual::storeMoveValue(std::string &state, char action, float newValue) {
  sqlite3 *db = this->db;
  char* errMsg = nullptr;

  std::string createTableQuery = "CREATE TABLE IF NOT EXISTS brain (key TEXT PRIMARY KEY, value TEXT);";
  sqlite3_exec(db, createTableQuery.c_str(), nullptr, 0, &errMsg);
  // get our action index
  int index = gene_index[action];
  //printf("storing value %2.2f at char %c index %d\n", newValue, action, index);

  // check if the key exists in database and retrieve the vector of floats if it does
  std::vector<float> values;
  std::string selectQuery = "SELECT value FROM brain WHERE key='" + state + "';";
  sqlite3_stmt *stmt;
  int rc = sqlite3_prepare_v2(db, selectQuery.c_str(), -1, &stmt, nullptr);
  if (rc == SQLITE_OK) {
    if (sqlite3_step(stmt) == SQLITE_ROW) {
      std::string value(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)));

      // parse the string and convert it into a vector of floats
      std::istringstream iss(value);
      std::string numStr;
      while (getline(iss, numStr, ',')) {
        float f = stof(numStr);
        values.push_back(f);
      }
    }
  }
  sqlite3_finalize(stmt);
  // update the vector index with the new parameter value
  if (values.empty()) {
    values = {0,0,0,0,0};
  }
  if (newValue != values[index]) {
    //printf("Updating move %c value %2.2f with new value %2.2f\n", action, values[index], (values[index] + newValue) / 2);
    //values[index] = std::fmaxf(values[index], newValue);
    // update move value with new sequence value
    // average the existing value together with the new Value
    //if (newValue > 20) {
    //  printf("state[%s][%c]=%2.2f\n", state.c_str(), action, newValue);
    //}
    values[index] = newValue;
  }

  // convert vector of floats into comma-separated string for storage
  std::ostringstream oss;
  for (int i=0;i<values.size();i++) {
    oss << values[i];
    if (i != values.size()-1) {
       oss << ',';
    }
  }
  std::string strValues = oss.str();

  // insert or replace the data into the database
  std::string insertQuery = "INSERT OR REPLACE INTO brain (key, value) VALUES ('" + state + "', '" + strValues + "');";
  sqlite3_exec(db, insertQuery.c_str(), nullptr, 0, &errMsg);
}

char Individual::getBestAction(std::string &state, int chance, int threshold) {
  float maxValue = std::numeric_limits<float>::min();
  char *errMsg;
  int maxIndex = random_num(0, NUM_GENES-1);
  char bestCh = GENES.at(maxIndex);
  // if we rolled under the mutation threshold, perform the random action
  if (random_num(0, 1000) < chance) {
    return bestCh;
  }
  sqlite3 *db = this->db;

  // retrieve data from the sqlite database
  std::string selectQuery = "SELECT value FROM brain WHERE key='" + state + "';";
  sqlite3_stmt *stmt;
  int rc = sqlite3_prepare_v2(db, selectQuery.c_str(), -1, &stmt, nullptr);
  if (rc == SQLITE_OK) {
    std::vector<float> floats;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
      floats.clear();
      std::string value(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)));
      //printf("Retrieved db string: %s\n", value.c_str());

      // parse the float array back into a vector of floats
      std::istringstream iss(value);
      std::string numStr;
      while (getline(iss, numStr, ',')) {
        float f = stof(numStr);
        floats.push_back(f);
      }

      //printf("Floats: \n");
      // find max value in resulting float vector
      if (floats.empty()) { return bestCh; }
      for (int i=0;i<floats.size();i++) {
        //printf(" %2.2f", floats[i]);
        // only use a "best" value if it exceeds some given threshold
        if (floats[i] > maxValue && floats[i] > threshold) {
          maxIndex = i;
          maxValue = floats[i];
         // printf("Found move %c of value %2.2f\n", GENES.at(maxIndex), maxValue);
        }
      }
      //printf("\n");
    }
  }
  sqlite3_finalize(stmt);
  return GENES.at(maxIndex);
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

/*
void Individual::storeMoveValue(std::string &state, char action, float value) {
  bool found = false;
  std::vector<float> neuron = findMap(state);
  // neuron didn't exist, create it before trying to set it
  for (int i=0;i<NUM_GENES;i++) {
    if (GENES.at(i) == action) {
     // key exists, assign a new value
      found = true;
      // set it directly
      // keep the best value for this move
      //this->brain[state][i] = std::fmaxf(this->brain[state][i], value);
      // update value
      this->brain[state][i] = value;
      //neuron[i] = value;
      // get the neuron again to see if it actually updated
      //neuron = findMap(state);
      //for (int j=0;j<NUM_GENES;j++) {
      //  printf(" %d", neuron[j]);
      //}
      //printf("\n");
      //printf("brain[%s][%c]=%d\n", state.c_str(), action, value);
      break;
    }
  }
  if (!found) {
    printf("ERROR: Invalid action %c passed to storeMoveValue()\n", action);
  }
}
*/

/*
char Individual::getBestAction(std::string &state, int chance) {
  float best = 0;
  char bestCh = GENES.at(random_num(0, NUM_GENES-1)); // generate a random move
  // if we rolled under the mutation threshold, then we do a mutation and perform the random move
  if (random_num(0, 1000) < chance) {
    return bestCh;
  }
  // if we didn't make the mutation roll, then we perform the best saved action
  std::vector<float> neuron = findMap(state);
  for (int i=0;i<NUM_GENES;i++) {
    char ch = GENES.at(i);
    int b = neuron[i];
     if (b && b > best) {
      best = neuron[i];
      bestCh = ch;
    }
  }
  if (best > 0) {
    printf("found best action: %c of value: %2.2f\n", bestCh, best);
  }
  return bestCh;
}
*/


#endif
