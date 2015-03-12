#ifndef user_string_search_hpp
#define user_string_search_hpp

#include <random>

#include "puzzler/core/puzzle.hpp"
#include "puzzler/puzzles/string_search.hpp"

class StringSearchProvider
  : public puzzler::StringSearchPuzzle
{
public:
  virtual void Execute(
		       puzzler::ILog *log,
		       const puzzler::StringSearchInput *input,
		       puzzler::StringSearchOutput *output
		       ) const override
  {
	  std::vector<uint32_t> histogram(input->patterns.size(), 0);
	  std::string data = MakeString(input->stringLength, input->seed);

	  for (unsigned i = 0; i < input->stringLength; i++){
		  for (unsigned p = 0; p<input->patterns.size(); p++){
			  unsigned len = Matches(data, i, input->patterns[p]);
			  if (len>0){
				  log->Log(puzzler::Log_Debug, [&](std::ostream &dst){
					  dst << "  Found " << input->patterns.at(p) << " at offset " << i << ", match=" << data.substr(i, len);
				  });
				  histogram[p]++;	//Q: What does this do?
				  i += len - 1;
				  break;
			  }
		  }
	  }

	 /* typedef tbb::blocked_range<unsigned> my_range_t;
	  unsigned K = 100;
	  my_range_t range(0, input->stringLength, K);
	  auto f = [&](const my_range_t &chunk)
	  {
		  for (unsigned i = chunk.begin(); i != chunk.end(); i++)
		  {
			  for (unsigned p = 0; p<input->patterns.size(); p++){
				  unsigned len = Matches(data, i, input->patterns[p]);
				  if (len>0){
					  log->Log(puzzler::Log_Debug, [&](std::ostream &dst){
						  dst << "  Found " << input->patterns.at(p) << " at offset " << i << ", match=" << data.substr(i, len);
					  });
					  histogram[p]++;
					  i += len - 1;
					  break;
				  }
			  }
		  }
	  };
	  tbb::parallel_for(range, f, tbb::simple_partitioner());*/

	  for (unsigned i = 0; i<histogram.size(); i++){
		  log->Log(puzzler::Log_Debug, [&](std::ostream &dst){
			  dst << input->patterns[i].c_str() << " : " << histogram[i];
		  });
	  }


	  output->occurences = histogram;
  }

};

#endif
