### Instructions to run the branch predictor
1. Run the command `make predictor` to create an executable file named predictor.
2. To run the predictor, run the following command: `./predictor <filename>`, where <filename> is the name of the input file containing branch instructions (e.g., branchtrace.txt).
3. The predictor will create 12 output files, each containing the corresponding predictions for the given input file and strategy. The files will be named `output_X_Y.out`, where X is the strategy and Y is the starting state.
4. The predictor will also print the accuracy for each strategy to the terminal. The strategies are Saturating Counters only, BHR only, and Saturating Counters and BHR combined, and each strategy is tested with starting states **00, 01, 10, and 11**. The accuracy is calculated by dividing the number of correctly predicted branches by the total number of branches in the input file.
5. To clean up the generated output files, run the command make clean.