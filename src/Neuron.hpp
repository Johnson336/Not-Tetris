#ifndef __NEURONHPP
#define __NEURONHPP

#include <random>
// a neuron consists of a set of moves that should be performed for any given 
// tetromino and a board state.
// maximum of 50 moves per sequence
#define SEQUENCE_LENGTH 50


// Valid Genes -- Possible Moves
const std::string GENES = "wsadez";

// Generate random numbers in a range
int random_num(int start, int end) {
    int range = (end-start)+1;
    int random_int = start+(rand()%range);
    return random_int;
}

// Create random gene for movement
char create_gene() {
    return GENES.at(random_num(0, GENES.size()-1));
}

// class representing a single neuron within an individual
class Neuron {
    public:
        std::string sequence;
        int fitness;
        int tetrominoInt;
        std::string boardState;
        Neuron(int tetrominoInt, std::string &boardState);
        void calcFitness(int, int);
        void setFitness(int);
        void setSequence(std::string sequence);

};

std::string generateMoves(int piece, std::string board) {
    std::string output = "";
    // give our neuron a random move
    for (int i=0;i<SEQUENCE_LENGTH;i++) {
        output += create_gene();
    }
    return output;
}

Neuron::Neuron(int tetrominoInt, std::string &boardState) {
    this->tetrominoInt = tetrominoInt;
    this->boardState = boardState;
    this->fitness = 0;
    sequence = generateMoves(tetrominoInt, boardState);
};

void Neuron::setSequence(std::string sequence) {
    this->sequence = sequence;
}
void Neuron::setFitness(int i) {
    this->fitness = i;
}
void Neuron::calcFitness(int score, int prevScore) {
    this->fitness = (score - prevScore);
    printf("Set own fitness to %d\n", this->fitness);
}

// Overload == operator for equality checks
bool operator==(const Neuron &ind1, const Neuron &ind2) {
    return (ind1.tetrominoInt == ind2.tetrominoInt) && (ind1.boardState == ind2.boardState);
}


#endif