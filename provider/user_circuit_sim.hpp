#ifndef user_circuit_sim_hpp
#define user_circuit_sim_hpp

#include "puzzler/puzzles/circuit_sim.hpp"
#include "tbb/task_group.h"
#include "tbb/parallel_for.h"
#include "tbb/blocked_range2d.h"

class CircuitSimProvider
	: public puzzler::CircuitSimPuzzle
{
private:
	std::vector<int> vector_boolToInt(std::vector<bool> source) {
		std::vector<int> return_vector;
		for (bool value : source) {
			return_vector.push_back(value ? 1 : 0);
		}
		return return_vector;
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

	std::vector<bool> next_tbb(const std::vector<bool> &state, const puzzler::CircuitSimInput *input) const
	{
		std::vector<bool> res(state.size());
		/*for (unsigned i = 0; i<res.size(); i++){
		res[i] = calcSrc(input->flipFlopInputs[i], state, input);
		}*/

		if (res.size() > 4000){
			int K = 1000;
			typedef tbb::blocked_range<unsigned> my_range_t;
			my_range_t range(0, res.size(), K);
			auto f = [&](const my_range_t &chunk)
			{
				for (unsigned i = chunk.begin(); i != chunk.end(); i++)
				{
					res[i] = calcSrc(input->flipFlopInputs[i], state, input);
				}
			};
			tbb::parallel_for(range, f, tbb::simple_partitioner());
		}
		else
		{
			for (unsigned i = 0; i < res.size(); i++)
			{
				unsigned K = res.size();
				res[i] = calcSrc_tbb(input->flipFlopInputs[i], state, input, K/4);
				res[i] = calcSrc(input->flipFlopInputs[i], state, input);
			}
		}
		/*unsigned size = res.size();
		auto f = [&](unsigned i)
		{
			res[i] = calcSrc(input->flipFlopInputs[i], state, input);
		};
		tbb::parallel_for(0u, size, f);*/



		/*for (unsigned i = 0; i < res.size(); i++)
		{
			res[i] = calcSrc(input->flipFlopInputs[i], state, input);
		}*/

		return res;
	}
	

	int calcSrc_int(unsigned src, const std::vector<int> &state, const puzzler::CircuitSimInput *input) const
	{
		if (src < input->flipFlopCount){
			return state.at(src);
		}
		else{
			unsigned nandSrc = src - input->flipFlopCount;
			bool a = !!calcSrc_int(input->nandGateInputs.at(nandSrc).first, state, input);
			bool b = !!calcSrc_int(input->nandGateInputs.at(nandSrc).second, state, input);
			return !!!(a&&b);
		}
	}

	std::vector<int> next_int(const std::vector<int> &state, const puzzler::CircuitSimInput *input) const
	{
		std::vector<int> res(state.size());
		for (unsigned i = 0; i<res.size(); i++){
			res[i] = calcSrc_int(input->flipFlopInputs[i], state, input);
		}

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

		bool conversion = false;
		bool parallel = false;
		if (conversion == false)
		{
			if (parallel == true)	//Parallel outer loop
			{
				int K = 10;
				typedef tbb::blocked_range<unsigned> my_range_t;
				my_range_t range(0, input->clockCycles, K);
				auto f = [&](const my_range_t &chunk)
				{
					for (unsigned i = chunk.begin(); i != chunk.end(); i++)
					{
						log->LogVerbose("Starting iteration %d of %d\n", i, input->clockCycles);

						state = next(state, input);

						// The weird form of log is so that there is little overhead
						// if logging is disabled
						log->Log(puzzler::Log_Debug, [&](std::ostream &dst) {
							for (unsigned j = 0; i < state.size(); j++){
								dst << state[j];
							}
						});
					}
				};
				tbb::parallel_for(range, f, tbb::simple_partitioner());
				//tbb::parallel_for(0u, input->clockCycles, f);
			}
			else
			{	
				
				for (unsigned i = 0; i<input->clockCycles/2; i++){
					log->LogVerbose("Starting iteration %d of %d\n", i, input->clockCycles);

					state = next(state, input);

					// The weird form of log is so that there is little overhead
					// if logging is disabled
					log->Log(puzzler::Log_Debug, [&](std::ostream &dst) {
						for (unsigned i = 0; i<state.size(); i++){
							dst << state[i];
						}
					});
				}
				for (unsigned i = input->clockCycles/2; i<input->clockCycles; i++){
					log->LogVerbose("Starting iteration %d of %d\n", i, input->clockCycles);

					state = next(state, input);

					// The weird form of log is so that there is little overhead
					// if logging is disabled
					log->Log(puzzler::Log_Debug, [&](std::ostream &dst) {
						for (unsigned i = 0; i<state.size(); i++){
							dst << state[i];
						}
					});
				}
			}
		}
		else
		{	//Convert from bool to int
			std::vector<int> state_int;
			for (bool value : state) {
				state_int.push_back(value ? 1 : 0);
			}
			//conversion taken from reddit because of issues in VS with vector<bool>

			//int K = 10;
			//typedef tbb::blocked_range<unsigned> my_range_t;
			//my_range_t range(0, input->clockCycles, K);
			//auto f = [&](const my_range_t &chunk)
			//{
			//	for (unsigned i = chunk.begin(); i != chunk.end(); i++)
			//	{
			//		log->LogVerbose("Starting iteration %d of %d\n", i, input->clockCycles);

			//		state_int = next_int(state_int, input);

			//		// The weird form of log is so that there is little overhead
			//		// if logging is disabled
			//		log->Log(puzzler::Log_Debug, [&](std::ostream &dst) {
			//			for (unsigned j = 0; i < state.size(); j++){
			//				dst << state[j];
			//			}
			//		});
			//	}
			//};
			//tbb::parallel_for(range, f, tbb::simple_partitioner());

			for (unsigned i = 0; i<input->clockCycles; i++){
				log->LogVerbose("Starting iteration %d of %d\n", i, input->clockCycles);

				state_int = next_int(state_int, input);

				// The weird form of log is so that there is little overhead
				// if logging is disabled
				log->Log(puzzler::Log_Debug, [&](std::ostream &dst) {
					for (unsigned i = 0; i<state_int.size(); i++){
						dst << state_int[i];
					}
				});
			}

			for (int i : state_int) {
				state.at(i)=!!state_int.at(i);
			}
		}
		log->LogVerbose("Finished clock cycles");

		output->outputState = state;
	}

};

#endif
