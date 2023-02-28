#ifndef __INDIVIDUALHPP
#define __INDIVIDUALHPP

#include <random>
#include "Neuron.hpp"

// Number of individuals in each generation
#define POPULATION_SIZE 100
#define GENOME_SIZE 50

// Valid Genes -- Possible Moves
extern const std::string GENES;
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
extern int random_num(int start, int end);

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
        std::vector<Neuron> brain;
        std::string chromosome;
        int fitness;
        Individual(std::vector<Neuron> &brain);
        Individual mate(Individual &parent2);
        void calc_fitness();
        void setFitness(int);
        Neuron findNeuron(int, std::string);
};

Neuron Individual::findNeuron(int tetrominoInt, std::string boardState) {
    // check our brain to see if a neuron for this piece and boardState exists
    for (int i=0;i<this->brain.size();i++) {
        if (this->brain[i].tetrominoInt == tetrominoInt && this->brain[i].boardState == boardState) {
            return this->brain[i];
        }
    }
    // couldn't find an existing neuron, make a new one
    Neuron newNeuron = Neuron(tetrominoInt, boardState);
    this->brain.push_back(newNeuron);
    return newNeuron;
}

Individual::Individual(std::vector<Neuron> &brain) {
    this->brain = brain;
    this->fitness = 0;
    // We don't know fitness until we've played a game
    // fitness = calc_fitness();
};

Neuron probability_pick_neuron(Neuron &n1, Neuron &n2) {
    float p = random_num(0, 100) / 100;
    if (p < 0.45) {
        // 45% chance to pick parent 1
        return n1;
    } else if (p < 0.90) {
        // 45% chance to pick parent 2
        return n2;
    } else {
        // 10% chance to generate a new neuron for diversity
        return Neuron(n1.tetrominoInt, n1.boardState);
    }
}

// perform mating and produce a new offspring
Individual Individual::mate(Individual &parent2) {
    // chromosome for offspring
    std::vector<Neuron> child_brain = std::vector<Neuron>();
    auto& genes1 = this->brain;
    auto& genes2 = parent2.brain;
    auto g1 = genes1.begin();
    auto g2 = genes2.begin();
    while (g1 != genes1.end() or g2 != genes2.end()) {
        if (g1 == genes1.end()) {
            while (g2 != genes2.end())
                child_brain.push_back(*g2++);
            break;
        }

        if (g2 == genes2.end()) {
            while (g1 != genes1.end())
                child_brain.push_back(*g1++);
            break;
        }
        const int &pg1 = (*g1).fitness;
        const int &pg2 = (*g2).fitness;

        if ((*g1) == (*g2)) {
            if (pg1 < pg2) {
                child_brain.push_back(*g2++);
                g1++;
            } else {
                child_brain.push_back(*g1++);
                g2++;
            }
        } else {
            child_brain.push_back(*g1++);
        }
    }

    // create a new Individual (offspring) using
    // the new combined chromosomes for offspring
    return Individual(child_brain);
};


void Individual::setFitness(int i) {
    this->fitness = i;
}

// Calculate a fitness score, this is the meat and potatoes of 
// what the AI will consider to be a successful series of moves
void Individual::calc_fitness() {
    // Fitness of an individual is the sum of the fitness 
    // of all of its neurons
    int fitness = 0;
    for (int i=0;i<this->brain.size();i++) {
        int n_f = this->brain[i].fitness;
        fitness += n_f;
        printf("Fitness: %d Neuron fitness: %d\n", fitness, n_f);
    }
    this->fitness = fitness;
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