#ifndef __NEURONHPP
#define __NEURONHPP

#include <random>
// a neuron consists of a set of moves that should be performed for any given 
// tetromino and a board state.
// maximum of 20 moves per sequence
#define SEQUENCE_LENGTH 20


// Valid Genes -- Possible Moves
const std::string GENES = "wsadz";

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

std::string bisect_gene(std::string gene1, std::string gene2) {
    // dual bisect the given genes
    // find the dual bisection at first and third quarter
    // -----|----------|-----
    int len = gene1.size();
    int split = len / 4;
    std::string output = "";
    output += gene1.substr(0, split);
    output += gene2.substr(split, (split*2));
    output += gene1.substr(split*2, len);
    return output;
}

// class representing a single neuron within an individual
class Neuron {
    public:
        std::string sequence;
        int fitness;
        char tetrominoShape;
        std::string boardState;
        Neuron(char tetrominoShape, std::string &boardState);
        void calcFitness(int, int);
        void setFitness(int);
        void setSequence(std::string sequence);

};

std::string generateMoves(char piece, std::string &board) {
    std::string output = "";
    // give our neuron a random move
    for (int i=0;i<SEQUENCE_LENGTH;i++) {
        output += create_gene();
    }
    return output;
}


Neuron::Neuron(char tetrominoShape, std::string &boardState) {
    this->tetrominoShape = tetrominoShape;
    this->boardState = boardState;
    this->fitness = 0;
    sequence = generateMoves(tetrominoShape, boardState);
};

void Neuron::setSequence(std::string sequence) {
    this->sequence = sequence;
}
void Neuron::setFitness(int i) {
    this->fitness = i;
}
void Neuron::calcFitness(int score, int prevScore) {
    this->fitness = (score - prevScore);
}

// Overload == operator for equality checks
bool operator==(const Neuron &ind1, const Neuron &ind2) {
    return (ind1.tetrominoShape == ind2.tetrominoShape) && (ind1.boardState == ind2.boardState);
}


#endif