#ifndef user_median_bits_hpp
#define user_median_bits_hpp

#include "puzzler/puzzles/median_bits.hpp"
#include "tbb/parallel_sort.h"

class MedianBitsProvider
  : public puzzler::MedianBitsPuzzle
{
public:
  MedianBitsProvider()
  {}

  virtual void Execute(
		       puzzler::ILog *log,
		       const puzzler::MedianBitsInput *input,
		       puzzler::MedianBitsOutput *output
		       ) const override {

	log->LogInfo("Generating bits.");
	double tic = puzzler::now();

	std::vector<uint32_t> temp(input->n);
	bool parallel = true;
	if (parallel == false)
	{
		for (unsigned i = 0; i < input->n; i++){
			uint32_t x = i*(7 + input->seed);
			uint32_t y = 0;
			uint32_t z = 0;
			uint32_t w = 0;

			for (unsigned j = 0; j < (unsigned)(std::log(16 + input->n) / std::log(1.1)); j++){
				uint32_t t = x ^ (x << 11);
				x = y; y = z; z = w;
				w = w ^ (w >> 19) ^ t ^ (t >> 8);
			}

			temp[i] = w;
		}
	}
	else
	{
		//double parfor!
		unsigned tbb_K = 1000;
		auto f = [&](const tbb::blocked_range2d<unsigned> &chunk) {
			for (unsigned i = chunk.rows().begin(); i != chunk.rows().end(); i++){
				uint32_t x = i*(7 + input->seed);
				uint32_t y = 0;
				uint32_t z = 0;
				uint32_t w = 0;
				for (unsigned j = chunk.cols().begin(); j != chunk.cols().end(); j++){
					uint32_t t = x ^ (x << 11);
					x = y; y = z; z = w;
					w = w ^ (w >> 19) ^ t ^ (t >> 8);
				}
				std::swap(temp[i], w);
			}
		};
		tbb::parallel_for(tbb::blocked_range2d<unsigned>(0, input->n, tbb_K, 0, (unsigned)(std::log(16 + input->n) / std::log(1.1)), tbb_K), f, tbb::simple_partitioner());
	}
	log->LogInfo("Finding median, delta=%lg", puzzler::now() - tic);
	tic = puzzler::now();

	tbb::parallel_sort(temp.begin(), temp.end());

	output->median = temp[temp.size() / 2];

	log->LogInfo("Done, median=%u (%lg), delta=%lg", output->median, output->median / pow(2.0, 32), puzzler::now() - tic);
  }

};

#endif
