#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <iomanip>
#include "BranchPredictor.hpp"

using namespace std;

// Function to read the branch trace file
vector<pair<uint32_t, bool>> readBranchTrace(string fileName) {
    vector<pair<uint32_t, bool>> trace;
    ifstream file(fileName);
    string line;
    while (getline(file, line)) {
        stringstream ss(line);
        uint32_t pc;
        bool taken;
        ss >> hex >> pc >> taken;
        trace.emplace_back(pc, taken);
    }
    return trace;
}

// Function to write the branch prediction results to a file
void writeBranchPredictionResult(string fileName, vector<bool> predictions) {
    ofstream file(fileName);
    for (auto prediction : predictions) {
        file << prediction << endl;
    }
}

// Function to compute the branch prediction accuracy
float computeBranchPredictionAccuracy(vector<bool> predictions, vector<pair<uint32_t, bool>> trace) {
    int correctPredictions = 0;
    for (int i = 0; i < predictions.size(); i++) {
        if (predictions[i] == trace[i].second) {
            correctPredictions++;
        }
    }
    return (float)correctPredictions / (float)predictions.size();
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        cout << "Usage: " << argv[0] << " <branch_trace_file> <output_file_prefix>" << endl;
        return 1;
    }
    // Read the branch trace file
    vector<pair<uint32_t, bool>> trace = readBranchTrace(argv[1]);

    // Saturating counters
    vector<int> saturatingValues = {0, 1, 2, 3};
    for (auto saturatingValue : saturatingValues) {
        SaturatingBranchPredictor predictor(saturatingValue);
        vector<bool> predictions;
        for (auto branch : trace) {
            predictions.push_back(predictor.predict(branch.first) ? 1 : 0);
            predictor.update(branch.first, branch.second);
        }
        float accuracy = computeBranchPredictionAccuracy(predictions, trace);
        cout << "Saturating Counters: start state = " << saturatingValue << ", accuracy = " << fixed << setprecision(5) << accuracy << endl;
        // uncomment below line to write predictions in a output file
        // writeBranchPredictionResult("output_sc_" + to_string(saturatingValue) + ".out", predictions);
    }

    // BHR
    vector<int> bhrValues = {0, 1, 2, 3};
    for (auto bhrValue : bhrValues) {
        BHRBranchPredictor predictor(bhrValue);
        vector<bool> predictions;
        for (auto branch : trace) {
            predictions.push_back(predictor.predict(branch.first) ? 1 : 0);
            predictor.update(branch.first, branch.second);
        }
        float accuracy = computeBranchPredictionAccuracy(predictions, trace);
        cout << "BHR: start state = " << bhrValue << ", accuracy = " << fixed << setprecision(5) << accuracy << endl;
        // uncomment below line to write predictions in a output file
        // writeBranchPredictionResult("output_bhr_" + to_string(bhrValue) + ".out", predictions);
    }

    // Saturating counters and BHR combination
    vector<pair<int, int>> combinationValues = {{0,1<< 16}, {1,1<< 16}, {2,1<< 16}, {3,1<< 16}};
    for (auto combinationValue : combinationValues){
        SaturatingBHRBranchPredictor predictor(combinationValue.first, combinationValue.second);
        vector<bool> predictions;
        for (auto branch : trace) {
            predictions.push_back(predictor.predict(branch.first) ? 1 : 0);
            predictor.update(branch.first, branch.second);
        }
        float accuracy = computeBranchPredictionAccuracy(predictions, trace);
        cout << "Combined: start state combination = value: " << combinationValue.first << " size: " << combinationValue.second << ", accuracy = " << fixed << setprecision(5) << accuracy << endl;
        // uncomment below line to write predictions in a output file
        // writeBranchPredictionResult("output_combined_" + to_string(combinationValue.first) + "_" + to_string(combinationValue.second) + ".out", predictions);
    }
    
}
