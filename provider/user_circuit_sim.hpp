//Notes: consider coverting from vector<bool> to vector<char>

#ifndef user_circuit_sim_hpp
#define user_circuit_sim_hpp

#include "puzzler/puzzles/circuit_sim.hpp"
#include "tbb/task_group.h"
#include "tbb/parallel_for.h"
#include "tbb/blocked_range2d.h"
//#include "tbb/concurrent_vector.h"

class CircuitSimProvider
	: public puzzler::CircuitSimPuzzle
{
private:
	bool calcSrc(unsigned src, const std::vector<bool> &state, const puzzler::CircuitSimInput *input) const
	{
		if (src < input->flipFlopCount){
			return state.at(src);
		}
		else{
			unsigned nandSrc = src - input->flipFlopCount;
			bool a = calcSrc(input->nandGateInputs.at(nandSrc).first, state, input);
			bool b = calcSrc(input->nandGateInputs.at(nandSrc).second, state, input);
			return !(a&&b);
		}
	}

	char calcSrc(unsigned src, const std::vector<char> &state, const puzzler::CircuitSimInput *input) const
	{
		if (src < input->flipFlopCount){
			return state.at(src);
		}
		else{
			unsigned nandSrc = src - input->flipFlopCount;
			bool a = calcSrc(input->nandGateInputs.at(nandSrc).first, state, input);
			bool b = calcSrc(input->nandGateInputs.at(nandSrc).second, state, input);
			return (char) !(a&&b);
		}
	}

	bool calcSrc_tbb(unsigned src, const std::vector<bool> &state, const puzzler::CircuitSimInput *input, unsigned K) const
	{
		if (src < input->flipFlopCount){
			return state.at(src);
		}
		else{
			//TODO: Parallel across number of processors to reduce overhead
			unsigned nandSrc = src - input->flipFlopCount;
			
			if (K < 100)
				return calcSrc(input->nandGateInputs.at(nandSrc).first, state, input);
			else
			{
				tbb::task_group calcSrc_group;
				bool a, b;
				auto calcSrc_first = [&]() {a = calcSrc_tbb(input->nandGateInputs.at(nandSrc).first, state, input, K-1); };
				auto calcSrc_second = [&]() {b = calcSrc_tbb(input->nandGateInputs.at(nandSrc).second, state, input, K-1); };
				calcSrc_group.run(calcSrc_first);
				calcSrc_group.run(calcSrc_second);
				calcSrc_group.wait();
				return !(a&&b);
			}
		}
	}

	char calcSrc_tbb(unsigned src, const std::vector<char> &state, const puzzler::CircuitSimInput *input, unsigned K) const
	{
		if (src < input->flipFlopCount){
			return state.at(src);
		}
		else{
			//TODO: Parallel across number of processors to reduce overhead
			unsigned nandSrc = src - input->flipFlopCount;

			if (K < 100)
				return calcSrc(input->nandGateInputs.at(nandSrc).first, state, input);
			else
			{
				tbb::task_group calcSrc_group;
				bool a, b;
				auto calcSrc_first = [&]() {a = calcSrc_tbb(input->nandGateInputs.at(nandSrc).first, state, input, K - 1); };
				auto calcSrc_second = [&]() {b = calcSrc_tbb(input->nandGateInputs.at(nandSrc).second, state, input, K - 1); };
				calcSrc_group.run(calcSrc_first);
				calcSrc_group.run(calcSrc_second);
				calcSrc_group.wait();
				return !(a&&b);
			}
		}
	}

	std::vector<char> next(const std::vector<char> &state, const puzzler::CircuitSimInput *input) const
	{
		std::vector<char> res(state.size());
		for (unsigned i = 0; i < res.size()/2; i++){
			res[i] = calcSrc(input->flipFlopInputs[i], state, input);
		}

		for (unsigned i = res.size()/2; i < res.size(); i++){
			res[i] = calcSrc(input->flipFlopInputs[i], state, input);
		}

		return res;
	}

	std::vector<char> next_tbb(const std::vector<char> &state, const puzzler::CircuitSimInput *input) const
	{
		std::vector<char> res(state.size());
		int K = 100;
		typedef tbb::blocked_range<unsigned> my_range_t;
		my_range_t range(0, res.size(), K);
		auto f = [&](const my_range_t &chunk)
		{
			for (unsigned i = chunk.begin(); i != chunk.end(); i++){
				res[i] = calcSrc(input->flipFlopInputs[i], state, input);
			}
		};
		tbb::parallel_for(range, f, tbb::simple_partitioner());
		return res;
	}


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
		
		//convert state to state_char
		std::vector<char> state_char(input->inputState.size());

		for (unsigned i = 0; i < state_char.size(); i++)
		{
			state_char[i] = (int)state[i];
		}

		for (unsigned i = 0; i < input->clockCycles; i++){
			log->LogVerbose("Starting iteration %d of %d\n", i, input->clockCycles);

			state_char = next_tbb(state_char, input);

			// The weird form of log is so that there is little overhead
			// if logging is disabled
			log->Log(puzzler::Log_Debug, [&](std::ostream &dst) {
				for (unsigned i = 0; i < state.size(); i++){
					dst << state[i];
				}
			});
		}
		for (unsigned i = 0; i < state_char.size(); i++)
		{
			state[i] = (bool)state_char[i];
		}

		log->LogVerbose("Finished clock cycles");

		output->outputState = state;
	}

};

#endif
