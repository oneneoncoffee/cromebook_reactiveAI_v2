/* This program shows the random number signiture output of our neural netowrk. 
 * As a result this allows fast number generation that uses low how CPU only resources. 
 * In theory its a signature, like a bilogical thumbprint. 
 * 
 * Notes: 
 * Make sure you have installed SQL lite 3 libary. Type this at the command line:
 * sudo apt-get install libsqlite3-dev
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <math.h>

// Define the structure for a neural network layer
typedef struct {
    int num_inputs;
    int num_outputs;
    double* weights;
    double* biases;
} Layer;

// Define the structure for a neural network
typedef struct {
    int num_layers;
    Layer* layers;
} NeuralNetwork;

// Function to create a new neural network layer
Layer* create_layer(int num_inputs, int num_outputs) {
    Layer* layer = (Layer*) malloc(sizeof(Layer));
    layer->num_inputs = num_inputs;
    layer->num_outputs = num_outputs;
    layer->weights = (double*) malloc(num_inputs * num_outputs * sizeof(double));
    layer->biases = (double*) malloc(num_outputs * sizeof(double));
    return layer;
}

// Function to create a new neural network
NeuralNetwork* create_neural_network(int num_layers, int* layer_sizes) {
    NeuralNetwork* network = (NeuralNetwork*) malloc(sizeof(NeuralNetwork));
    network->num_layers = num_layers;
    network->layers = (Layer*) malloc(num_layers * sizeof(Layer));
    for (int i = 0; i < num_layers; i++) {
        if (i == 0) {
            network->layers[i] = *create_layer(layer_sizes[0], layer_sizes[i + 1]);
        } else if (i == num_layers - 1) {
            network->layers[i] = *create_layer(layer_sizes[i], layer_sizes[i]);
        } else {
            network->layers[i] = *create_layer(layer_sizes[i], layer_sizes[i + 1]);
        }
    }
    return network;
}

// Function to train the neural network
void train_neural_network(NeuralNetwork* network, double* inputs, double* targets, int num_samples) {
    // Simple gradient descent algorithm
    for (int i = 0; i < num_samples; i++) {
        // Forward pass
        double* outputs = (double*) malloc(network->layers[0].num_outputs * sizeof(double));
        for (int j = 0; j < network->layers[0].num_outputs; j++) {
            outputs[j] = 0;
            for (int k = 0; k < network->layers[0].num_inputs; k++) {
                outputs[j] += inputs[i * network->layers[0].num_inputs + k] * network->layers[0].weights[k * network->layers[0].num_outputs + j];
            }
            outputs[j] += network->layers[0].biases[j];
            outputs[j] = 1 / (1 + exp(-outputs[j])); // Sigmoid activation function
        }

        // Backward pass
        double* errors = (double*) malloc(network->layers[0].num_outputs * sizeof(double));
        for (int j = 0; j < network->layers[0].num_outputs; j++) {
            errors[j] = targets[i * network->layers[0].num_outputs + j] - outputs[j];
        }

        // Weight update
        for (int j = 0; j < network->layers[0].num_inputs; j++) {
            for (int k = 0; k < network->layers[0].num_outputs; k++) {
                network->layers[0].weights[j * network->layers[0].num_outputs + k] += 0.1 * errors[k] * inputs[i * network->layers[0].num_inputs + j];
            }
        }

        free(outputs);
        free(errors);
    }
}
// Function to save the neural network to a SQLite database
void save_neural_network(NeuralNetwork* network, sqlite3* db) {
    // Create table to store neural network weights
    char* sql = "CREATE TABLE IF NOT EXISTS weights (layer INTEGER, input INTEGER, output INTEGER, weight REAL)";
    sqlite3_exec(db, sql, NULL, NULL, NULL);

    // Insert weights into table
    for (int i = 0; i < network->num_layers; i++) {
        for (int j = 0; j < network->layers[i].num_inputs; j++) {
            for (int k = 0; k < network->layers[i].num_outputs; k++) {
                sql = "INSERT INTO weights VALUES (?, ?, ?, ?)";
                sqlite3_stmt* stmt;
                sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
                sqlite3_bind_int(stmt, 1, i);
                sqlite3_bind_int(stmt, 2, j);
                sqlite3_bind_int(stmt, 3, k);
                sqlite3_bind_double(stmt, 4, network->layers[i].weights[j * network->layers[i].num_outputs + k]);
                sqlite3_step(stmt);
                sqlite3_finalize(stmt);
            }
        }
    }

    // Create table to store neural network biases
    sql = "CREATE TABLE IF NOT EXISTS biases (layer INTEGER, output INTEGER, bias REAL)";
    sqlite3_exec(db, sql, NULL, NULL, NULL);

    // Insert biases into table
    for (int i = 0; i < network->num_layers; i++) {
        for (int j = 0; j < network->layers[i].num_outputs; j++) {
            sql = "INSERT INTO biases VALUES (?, ?, ?)";
            sqlite3_stmt* stmt;
            sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
            sqlite3_bind_int(stmt, 1, i);
            sqlite3_bind_int(stmt, 2, j);
            sqlite3_bind_double(stmt, 3, network->layers[i].biases[j]);
            sqlite3_step(stmt);
            sqlite3_finalize(stmt);
        }
    }
}
// Function to load the neural network from a SQLite database
void load_neural_network(NeuralNetwork* network, sqlite3* db) {
    // Create table to store neural network weights
    char* sql = "SELECT * FROM weights";
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

    // Load weights from table
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int layer = sqlite3_column_int(stmt, 0);
        int input = sqlite3_column_int(stmt, 1);
        int output = sqlite3_column_int(stmt, 2);
        double weight = sqlite3_column_double(stmt, 3);
        network->layers[layer].weights[input * network->layers[layer].num_outputs + output] = weight;
    }
    sqlite3_finalize(stmt);

    // Create table to store neural network biases
    sql = "SELECT * FROM biases";
    sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

    // Load biases from table
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int layer = sqlite3_column_int(stmt, 0);
        int output = sqlite3_column_int(stmt, 1);
        double bias = sqlite3_column_double(stmt, 2);
        network->layers[layer].biases[output] = bias;
    }
    sqlite3_finalize(stmt);
}
// Function that output a truely random neural network alligment 
void generateRandomOutput(double output, double* random_output, int use_thresholding, int use_mapping, int use_modulation, int use_combination) {
    double threshold = 0.5;
    double mapped_output = output;
    double modulated_output = output;
    double combined_output = output;

    if (use_thresholding && output > threshold) {
        if (use_mapping) {
            mapped_output = sin(output * 3.14);
        }
        if (use_modulation) {
            double noise = (double) rand() / RAND_MAX - 0.5;
            modulated_output = mapped_output + noise * mapped_output;
        }
        if (use_combination) {
            double random_variable = (double) rand() / RAND_MAX;
            combined_output = modulated_output + random_variable * (1 - modulated_output);
        }
    } else {
        if (use_mapping) {
            mapped_output = sin(output * 3.14);
        }
        if (use_modulation) {
            double noise = (double) rand() / RAND_MAX - 0.5;
            modulated_output = output + noise * output;
        }
        if (use_combination) {
            double random_variable = (double) rand() / RAND_MAX;
            combined_output = modulated_output + random_variable * (1 - modulated_output);
        }
    }

    if (use_thresholding && output > threshold) {
        *random_output = combined_output;
    } else {
        *random_output = combined_output;
    }
}
// Function to test the neural network
void test_neural_network(NeuralNetwork* network, double* inputs, double* expected_outputs, int num_samples) {
    for (int i = 0; i < num_samples; i++) {
        double* outputs = (double*) malloc(network->layers[0].num_outputs * sizeof(double));
        for (int j = 0; j < network->layers[0].num_outputs; j++) {
            outputs[j] = 0;
            for (int k = 0; k < network->layers[0].num_inputs; k++) {
                outputs[j] += inputs[i * network->layers[0].num_inputs + k] * network->layers[0].weights[k * network->layers[0].num_outputs + j];
            }
            outputs[j] += network->layers[0].biases[j];
            outputs[j] = 1 / (1 + exp(-outputs[j])); // Sigmoid activation function
        }
        
        // Do true random neural network test 
        double random_output;
        generateRandomOutput(outputs[0], &random_output, 1, 1, 1, 1);

        printf("Input: ");
        for (int j = 0; j < network->layers[0].num_inputs; j++) {
            printf("%f ", inputs[i * network->layers[0].num_inputs + j]);
        }
        printf("\n");

        printf("Expected Output: ");
        for (int j = 0; j < network->layers[0].num_outputs; j++) {
            printf("%f ", expected_outputs[i * network->layers[0].num_outputs + j]);
        }
        printf("\n");

        printf("Actual Output: ");
        for (int j = 0; j < network->layers[0].num_outputs; j++) {
            printf("%f ", outputs[j]);
        }
        printf("\n");

        free(outputs);
    }
}

int main() {
    // Create a new neural network
    int num_layers = 2;
    int layer_sizes[] = {2, 1};
    NeuralNetwork* network = create_neural_network(num_layers, layer_sizes);

    // Initialize the neural network weights and biases
    for (int i = 0; i < network->num_layers; i++) {
        for (int j = 0; j < network->layers[i].num_inputs; j++) {
            for (int k = 0; k < network->layers[i].num_outputs; k++) {
                network->layers[i].weights[j * network->layers[i].num_outputs + k] = (double) rand() / RAND_MAX;
            }
        }
        for (int j = 0; j < network->layers[i].num_outputs; j++) {
            network->layers[i].biases[j] = (double) rand() / RAND_MAX;
        }
    }

    // Create a SQLite database
    sqlite3* db;
    sqlite3_open("neural_network.db", &db);

    // Save the neural network to the database
    save_neural_network(network, db);

    // Load the neural network from the database
    load_neural_network(network, db);

    // Test the neural network
    double inputs[] = {0, 0, 0, 1, 1, 0, 1, 1};
    double expected_outputs[] = {0, 1, 1, 0};
    test_neural_network(network, inputs, expected_outputs, 4);
    
    // Print the number results 
    printf("\nResults:\n");
    for (int i = 0; i < 4; i++) {
        double output;
        double* outputs = (double*) malloc(network->layers[0].num_outputs * sizeof(double));
        for (int j = 0; j < network->layers[0].num_outputs; j++) {
            outputs[j] = 0;
            for (int k = 0; k < network->layers[0].num_inputs; k++) {
                outputs[j] += inputs[i * network->layers[0].num_inputs + k] * network->layers[0].weights[k * network->layers[0].num_outputs + j];
            }
            outputs[j] += network->layers[0].biases[j];
            outputs[j] = 1 / (1 + exp(-outputs[j])); // Sigmoid activation function
        }
        double random_output;
        generateRandomOutput(outputs[0], &random_output, 1, 1, 1, 1);
        printf("Input: %f %f, Expected Output: %f, Actual Output: %f, [Number Random] Output: %f\n", inputs[i * 2], inputs[i * 2 + 1], expected_outputs[i], outputs[0], random_output);
        free(outputs);
    }
    // Close the database
    sqlite3_close(db);

    // Free the neural network memory
    for (int i = 0; i < network->num_layers; i++) {
        free(network->layers[i].weights);
        free(network->layers[i].biases);
    }
    free(network->layers);
    free(network);

    return 0;
}
