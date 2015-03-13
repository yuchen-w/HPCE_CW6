#ifndef user_life_hpp
#define user_life_hpp

#include "puzzler/puzzles/life.hpp"

#include "tbb/parallel_for.h"
#include "tbb/blocked_range2d.h"

class LifeProvider
  : public puzzler::LifePuzzle
{

private:
	bool update_unroll(int n, const std::vector<bool> &curr, int x, int y) const
	{
		int neighbours = 0;
		int ox;
		int oy;
		//dx = -1, dy = -1

		ox = (n + x  -1) % n; // handle wrap-around
		oy = (n + y  -1) % n;

		if (curr.at(oy*n + ox)) 
			neighbours++;

		//dx = -1, dy = 0
		ox = (n + x -1) % n; // handle wrap-around
		oy = (n + y) % n;

		if (curr.at(oy*n + ox)) 
			neighbours++;

		//dx = -1, dy = 1
		ox = (n + x -1) % n; // handle wrap-around
		oy = (n + y + 1) % n;

		if (curr.at(oy*n + ox)) 
			neighbours++;

		//dx = 0, dy = -1
		ox = (n + x) % n; // handle wrap-around
		oy = (n + y - 1) % n;

		if (curr.at(oy*n + ox))
			neighbours++;

		//dx = 0, dy = 0
		ox = (n + x) % n; // handle wrap-around
		oy = (n + y) % n;

		//no if loop because !(dx == 0 && dy == 0) = 0

		//dx = 0, dy = 1
		ox = (n + x) % n; // handle wrap-around
		oy = (n + y + 1) % n;

		if (curr.at(oy*n + ox))
			neighbours++;

		//dx = 1, dy = -1

		ox = (n + x + 1) % n; // handle wrap-around
		oy = (n + y - 1) % n;

		if (curr.at(oy*n + ox))
			neighbours++;

		//dx = 1, dy = 0
		ox = (n + x + 1) % n; // handle wrap-around
		oy = (n + y) % n;

		if (curr.at(oy*n + ox))
			neighbours++;

		//dx = 1, dy = 1
		ox = (n + x + 1) % n; // handle wrap-around
		oy = (n + y + 1) % n;

		if (curr.at(oy*n + ox))
			neighbours++;


		//for (int dx = -1; dx <= +1; dx++){
		//	for (int dy = -1; dy <= +1; dy++){
		//		int ox = (n + x + dx) % n; // handle wrap-around
		//		int oy = (n + y + dy) % n;

		//		if (curr.at(oy*n + ox) && !(dx == 0 && dy == 0))
		//			neighbours++;
		//	}
		//}

		if (curr[n*y + x]){
			// alive
			if (neighbours<2){
				return false;
			}
			else if (neighbours>3){
				return false;
			}
			else{
				return true;
			}
		}
		else{
			// dead
			if (neighbours == 3){
				return true;
			}
			else{
				return false;
			}
		}
	}
public:
  LifeProvider()
  {}

  virtual void Execute(
		       puzzler::ILog *log,
		       const puzzler::LifeInput *input,
		       puzzler::LifeOutput *output
		       ) const override {
    /*ReferenceExecute(log, input, output);*/

	  log->LogVerbose("About to start running iterations (total = %d)", input->steps);

	  unsigned n = input->n;
	  std::vector<bool> state = input->state;

	  log->Log(puzzler::Log_Debug, [&](std::ostream &dst){
		  dst << "\n";
		  for (unsigned y = 0; y<n; y++){
			  for (unsigned x = 0; x<n; x++){
				  dst << (state.at(y*n + x) ? 'x' : ' ');
			  }
			  dst << "\n";
		  }
	  });

	  for (unsigned i = 0; i<input->steps; i++){
		  log->LogVerbose("Starting iteration %d of %d\n", i, input->steps);

		  std::vector<bool> next(n*n);
		
		  //Parallelised next[]=
		  unsigned K = 10;

		  auto f = [&](const tbb::blocked_range2d<unsigned> &chunk) {
			  for (unsigned x = chunk.rows().begin(); x != chunk.rows().end(); x++){
				  for (unsigned y = chunk.cols().begin(); y != chunk.cols().end(); y++){
					  next[y*n + x] = update_unroll(n, state, x, y);
				  }
			  }
		  };
		  tbb::parallel_for(tbb::blocked_range2d<unsigned>(0, n, K, 0, n, K), f, tbb::simple_partitioner());

		  state = next;

		  // The weird form of log is so that there is little overhead
		  // if logging is disabled
		  log->Log(puzzler::Log_Debug, [&](std::ostream &dst){
			  dst << "\n";
			  for (unsigned y = 0; y<n; y++){
				  for (unsigned x = 0; x<n; x++){
					  dst << (state[y*n + x] ? 'x' : ' ');
				  }
				  dst << "\n";
			  }
		  });
	  }

	  log->LogVerbose("Finished steps");

	  output->state = state;


  }

};

#endif
