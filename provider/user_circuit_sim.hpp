#ifndef user_circuit_sim_hpp
#define user_circuit_sim_hpp

#include "puzzler/puzzles/circuit_sim.hpp"
//#include "puzzler\core\log.hpp"

class CircuitSimProvider
  : public puzzler::CircuitSimPuzzle
{
public:
  CircuitSimProvider()
  {}

  virtual void Execute(
		       puzzler::ILog *log,
		       const puzzler::CircuitSimInput *input,
		       puzzler::CircuitSimOutput *output
		       ) const override {
	log->LogVerbose("About to start running clock cycles (total = %d", input->clockCycles);
	std::vector<bool> state = input->inputState;
	std::vector<bool> state_temp(state.size());

	//TODO: OPTMISE
	for (unsigned i = 0; i<input->clockCycles; i++){
		log->LogVerbose("Starting iteration %d of %d\n", i, input->clockCycles);

		//state = next(state, input);
		for (unsigned j = 0; j<state.size(); j++){
			state_temp[j] = calcSrc(input->flipFlopInputs[j], state, input);
		}
		state = state_temp;	//ToDo:swap pointers here

		// The weird form of log is so that there is little overhead
		// if logging is disabled
		log->Log(puzzler::Log_Debug, [&](std::ostream &dst) {
			for (unsigned i = 0; i<state.size(); i++){
				dst << state[i];
			}
		});
	}

	log->LogVerbose("Finished clock cycles");

	output->outputState = state;
  }

};

#endif
