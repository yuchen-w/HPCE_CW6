#ifndef user_matrix_exponent_hpp
#define user_matrix_exponent_hpp

#include "puzzler/puzzles/matrix_exponent.hpp"

#include "tbb/parallel_for.h"
#include "tbb/blocked_range2d.h"

class MatrixExponentProvider
  : public puzzler::MatrixExponentPuzzle
{

private:
	//TODO: Taking ages! 
	static std::vector<uint32_t> MatrixMul_tbb(unsigned n, std::vector<uint32_t> a, std::vector<uint32_t> b)
	{
		std::vector<uint32_t> res(n*n, 0);

		unsigned K = 10;

		/*typedef tbb::blocked_range<unsigned> row_range_type;
		typedef tbb::blocked_range<unsigned> col_range_type;

		row_range_type range_r(0, n, K);
		col_range_type range_r(0, n, K);*/

		auto f = [&](const tbb::blocked_range2d<unsigned> &chunk) {
			for (unsigned r = chunk.rows().begin(); r !=chunk.rows().end(); r++){
				for (unsigned c = chunk.cols().begin(); c != chunk.cols().end(); c++){
					for (unsigned i = 0; i<n; i++){
						res[r*n + c] = Add(res[r*n + c], Mul(a[r*n + i], b[i*n + r]));
					}
				}
			}
		};
		tbb::parallel_for(tbb::blocked_range2d<unsigned>(0, n, K, 0, n, K), f , tbb::simple_partitioner() );

		return res;
	}

public:
  MatrixExponentProvider()
  {}

  virtual void Execute(
		       puzzler::ILog *log,
		       const puzzler::MatrixExponentInput *input,
		       puzzler::MatrixExponentOutput *output
		       ) const override {
	  std::vector<uint32_t> hash(input->steps);

	  log->LogVerbose("Setting up A and identity");
	  auto A = MatrixCreate(input->n, input->seed);
	  auto acc = MatrixIdentity(input->n);

	  log->LogVerbose("Beginning multiplication");
	  hash[0] = acc[0];
	  for (unsigned i = 1; i<input->steps; i++){
		  log->LogDebug("Iteration %d", i);
		  acc = MatrixMul_tbb(input->n, acc, A);
		  hash[i] = acc[0];
	  }
	  log->LogVerbose("Done");

	  output->hashes = hash;
  }

};

#endif
