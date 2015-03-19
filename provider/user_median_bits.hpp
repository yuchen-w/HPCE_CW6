#ifndef user_median_bits_hpp
#define user_median_bits_hpp

#include "puzzler/puzzles/median_bits.hpp"


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
   // ReferenceExecute(log, input, output);

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
		//double parfor this!
		
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
	log->LogInfo("Finding median, delta=%lg", puzzler::now() - tic);
	tic = puzzler::now();

	std::sort(temp.begin(), temp.end());

	output->median = temp[temp.size() / 2];

	log->LogInfo("Done, median=%u (%lg), delta=%lg", output->median, output->median / pow(2.0, 32), puzzler::now() - tic);
  }

};

#endif
