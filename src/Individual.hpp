#ifndef __INDIVIDUALHPP
#define __INDIVIDUALHPP

#include <random>

// Number of individuals in each generation
#define POPULATION_SIZE 100
#define GENOME_SIZE 50

extern unsigned long long score;
extern std::string boardState;

// Valid Genes -- Possible Moves
const std::string GENES = "wsadez";
/*
    w = rotate
    s = soft drop
    a = move left
    d = move right
    e = hold piece
    z = drop piece
*/

const unsigned long long TARGET = 100000; // score we want to reach

// Generate random numbers in a range
int random_num(int start, int end) {
    int range = (end-start)+1;
    int random_int = start+(rand()%range);
    return random_int;
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
        std::string chromosome;
        unsigned long long fitness;
        Individual(std::string &chromosome);
        Individual mate(Individual &parent2);
        unsigned long long calc_fitness();
        void setFitness(unsigned long long);
};

Individual::Individual(std::string &chromosome) {
    this->chromosome = chromosome;
    this->fitness = 0;
    // We don't know fitness until we've played a game
    // fitness = calc_fitness();
};

// perform mating and produce a new offspring
Individual Individual::mate(Individual &parent2) {
    // chromosome for offspring
    std::string child_chromosome = "";
    int len = chromosome.size();
    for (int i=0;i<len;i++) {
        // random probability
        float p = random_num(0, 100)/100;

        // if prob is less than 0.45, insert gene from parent 1
        if (p < 0.45)
            child_chromosome += chromosome[i];

        // if prob is between 0.45 and 0.90, insert gene from parent 2
        else if (p < 0.90)
            child_chromosome += parent2.chromosome[i];
        
        // otherwise insert a random mutated gene to maintain 
        // diversity
        else
            child_chromosome += mutated_genes();
    }

    // create a new Individual (offspring) using
    // the new combined chromosomes for offspring
    return Individual(child_chromosome);
};


void Individual::setFitness(unsigned long long i) {
    this->fitness = i;
}

// Calculate a fitness score, this is the meat and potatoes of 
// what the AI will consider to be a successful series of moves
unsigned long long Individual::calc_fitness() {
    // Fitness will just be total score earned by doing a series
    // of moves. This should reward good gameplay and punish bad
    return score;
}

// Overload < operator for comparisons
bool operator<(const Individual &ind1, const Individual &ind2) {
    return ind1.fitness < ind2.fitness;
}
// Overload < operator for comparisons
bool operator>(const Individual &ind1, const Individual &ind2) {
    return ind1.fitness > ind2.fitness;
}







#endif