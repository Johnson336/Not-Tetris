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
        int fitness; // fitness is now an average
        Individual(std::vector<Neuron> &brain);
        Individual mate(Individual &parent2);
        Neuron findNeuron(char tetrominoShape, std::string &boardState);
        void calc_fitness();
        void setFitness(int);
};


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
    Neuron bestNeuron = Neuron(tetrominoShape, boardState);
    for (Neuron neuron : this->brain) {
        if (neuron.tetrominoShape == tetrominoShape && neuron.boardState == boardState) {
            return neuron;
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
    float crossover_rate = 0.9;
    float mutation_rate = 0.1;
    int gene_length = this->brain.size();
    int gene_lenght2 = parent2.brain.size();
    const auto& p1 = this->brain.begin();
    const auto& p2 = parent2.brain.begin();
    std::vector<Neuron> child_brain;
    
    for (auto mother_neuron : this->brain) {
        for (auto father_neuron : parent2.brain) {
            if (mother_neuron == father_neuron) {
                // compare like neurons
                float crossover = (float)rand() / RAND_MAX;
                if (crossover < crossover_rate) {
                    // inherit better neuron
                    child_brain.push_back((mother_neuron > father_neuron) ? mother_neuron : father_neuron);
                } else {
                    // inherit worse neuron
                    child_brain.push_back((mother_neuron < father_neuron) ? mother_neuron : father_neuron);
                }
                break;
            }
        }
        // choose whether to copy over the unique mother neurons or mutate
        float mutation = (float)rand() / RAND_MAX;
        if (mutation < mutation_rate) {
            // mutate a new neuron
            Neuron newNeuron = Neuron(mother_neuron.tetrominoShape, mother_neuron.boardState);
            child_brain.push_back(newNeuron);
        } else if (mutation < crossover_rate) {
            // pass on mother neuron unchanged
            child_brain.push_back(mother_neuron);
        }
    }
    sort(child_brain.begin(), child_brain.end(), std::greater<Neuron>());
    // sort and cut off the lowest 30% of neurons to trim the fat
    int attritionIndex = child_brain.size() * 0.7;
    child_brain.erase(child_brain.begin()+attritionIndex, child_brain.end());

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


void Individual::setFitness(int i) {
    this->fitness = i;
}

// Calculate a fitness score, this is the meat and potatoes of 
// what the AI will consider to be a successful series of moves
void Individual::calc_fitness() {
    // Fitness of an individual is the average of the fitness 
    // of all of its neurons
    int totalFit = 0;
    int newFit = 0;
    for (int i=0;i<this->brain.size();i++) {
        totalFit += this->brain[i].fitness;
    }
    newFit = totalFit / this->brain.size();
    printf("Fitness: %d / %lu = %d\n", totalFit, this->brain.size(), newFit);
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