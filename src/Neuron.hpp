#ifndef __NEURONHPP
#define __NEURONHPP

#include <random>
// a neuron consists of a set of moves that should be performed for any given 
// tetromino location and a board state.
// maximum of 1 moves per sequence
#define SEQUENCE_LENGTH 1


// Valid Genes -- Possible Moves
const int NUM_GENES = 5;
const std::string GENES = "wsadz";

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

// class representing a single neuron within an individual
class Neuron {
    public:
        std::string boardState;
        Neuron(std::string &boardState);
        void calcFitness(int, int, int, int);
        void setFitness(int);

};

std::string generateMoves(std::string &board) {
    std::string output = "";
    // give our neuron a random move
    for (int i=0;i<SEQUENCE_LENGTH;i++) {
        output += create_gene();
    }
    return output;
}


Neuron::Neuron(std::string &boardState) {
    this->boardState = boardState;
};


// Overload == operator for equality checks
bool operator==(const Neuron &ind1, const Neuron &ind2) {
    return (ind1.boardState == ind2.boardState);
}


#endif
