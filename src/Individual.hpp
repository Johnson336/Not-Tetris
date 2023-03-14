#ifndef __INDIVIDUALHPP
#define __INDIVIDUALHPP

#include <random>
#include "Neuron.hpp"
#include <map>

// Number of individuals in each generation
#define POPULATION_SIZE 50
#define GENOME_SIZE 20

// Valid Genes -- Possible Moves
extern const std::string GENES;
/*
    w = rotate
    s = soft drop
    a = move left
    d = move right
    e = hold piece // currently disabled for AI
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
        float fitness; // fitness is now an average
        Individual(std::vector<Neuron> &brain);
        Individual mate(Individual &parent2);
        void calc_fitness();
        void setFitness(float);
        Neuron findNeuron(char tetrominoShape, std::string &boardState);
};

extern std::vector<Individual> population;


Neuron Individual::findNeuron(char tetrominoShape, std::string &boardState) {
    /*
    // check our brain to see if a neuron for this piece and boardState exists
    for (int i=0;i<this->brain.size();i++) {
        if (this->brain[i].tetrominoShape == tetrominoShape && this->brain[i].boardState == boardState) {
            // printf("Loaded existing neuron: %s fit: %d\n", this->brain[i].sequence.c_str(), this->brain[i].fitness);
            return this->brain[i];
        }
    }
    // couldn't find an existing neuron, make a new one
    Neuron newNeuron = Neuron(tetrominoShape, boardState);
    // Don't store the neuron until it's been scored in game loop
    // this->brain.push_back(newNeuron);
    // printf("Grew new neuron: %s\n", newNeuron.sequence.c_str());
    return newNeuron;
    */
   // select best Neuron out of all our brains
    int maxFit = 0;
    Neuron bestNeuron = Neuron(tetrominoShape, boardState);
    for (auto individual : population) {
        for (auto neuron : individual.brain) {
            if (neuron.tetrominoShape == tetrominoShape && neuron.boardState == boardState) {
                if (neuron.fitness > maxFit) {
                    maxFit = neuron.fitness;
                    bestNeuron = neuron;
                }
            }
        }
    }
    return bestNeuron;
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
        return Neuron(n1.tetrominoShape, n1.boardState);
    }
}

Individual Individual::mate(Individual &parent2) {
    float CROSSOVER_RATE = 0.7;
    int gene_length = this->brain.size();
    int gene_lenght2 = parent2.brain.size();
    const auto& p1 = this->brain.begin();
    const auto& p2 = parent2.brain.begin();
    std::vector<Neuron> child_brain;
    
    for (auto i : this->brain) {
        for (auto j : parent2.brain) {
            if (i == j) {
                if ((float)rand() / RAND_MAX < CROSSOVER_RATE) {
                    // inherit best neuron from parents
                    if (i.fitness > j.fitness) {
                        child_brain.push_back(i);
                    } else {
                        child_brain.push_back(j);
                    }
                    break;
                } else {
                    // generate random neuron
                    Neuron newNeuron = Neuron(i.tetrominoShape, i.boardState);
                    newNeuron.setSequence(create_genome());
                    child_brain.push_back(newNeuron);
                }
            }
        }
    }
    return Individual(child_brain);

}

// Define a function to perform crossover between two parents
void crossover(Individual& parent1, Individual& parent2, Individual& child1, Individual& child2) {
    float CROSSOVER_RATE = 0.7;
    int GENE_LENGTH1 = parent1.brain.size();
    int GENE_LENGTH2 = parent2.brain.size();
    if ((float)rand() / RAND_MAX < CROSSOVER_RATE) {
        int crossoverPoint = rand() % GENE_LENGTH1;
        for (int i = 0; i < crossoverPoint; i++) {
            child1.brain.push_back(parent1.brain[i]);
            child2.brain.push_back(parent2.brain[i]);
        }
        for (int i = crossoverPoint; i < GENE_LENGTH2; i++) {
            child1.brain.push_back(parent2.brain[i]);
            child2.brain.push_back(parent1.brain[i]);
        }
    } else {
        child1.brain = parent1.brain;
        child2.brain = parent2.brain;
    }
}


void Individual::setFitness(float i) {
    this->fitness = i;
}

// Calculate a fitness score, this is the meat and potatoes of 
// what the AI will consider to be a successful series of moves
void Individual::calc_fitness() {
    // Fitness of an individual is the average of the fitness 
    // of all of its neurons
    int totalFit = 0;
    float newFit = 0;
    for (int i=0;i<this->brain.size();i++) {
        totalFit += this->brain[i].fitness;
    }
    newFit = float(totalFit / this->brain.size());
    printf("Fitness: %d / %lu = %f\n", totalFit, this->brain.size(), newFit);
    this->fitness = newFit;
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