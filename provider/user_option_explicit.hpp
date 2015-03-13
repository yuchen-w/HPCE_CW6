#ifndef user_option_explicit_hpp
#define user_option_explicit_hpp

#include <random>

#include "puzzler/core/puzzle.hpp"
#include "puzzler/puzzles/option_explicit.hpp"

class OptionExplicitProvider
  : public puzzler::OptionExplicitPuzzle
{
public:
  virtual void Execute(
		       puzzler::ILog *log,
		       const puzzler::OptionExplicitInput *input,
		       puzzler::OptionExplicitOutput *output
		       ) const override {
	  int n = input->n;
	  double u = input->u, d = input->d;

	  log->LogInfo("Params: u=%lg, d=%lg, wU=%lg, wM=%lg, wD=%lg", input->u, input->d, input->wU, input->wM, input->wD);

	  std::vector<double> state(n * 2 + 1);
	  double vU = input->S0, vD = input->S0;
	  state[input->n] = (std::max)(vU - input->K, 0.0);

	  unsigned K = 1000;
	  //parfor:
	  if (n > 4000){
		  typedef tbb::blocked_range<unsigned> my_range_t;
		  
		  my_range_t range(1, n, K);
		  auto f = [&](const my_range_t &chunk)
		  {
			  for (unsigned i = chunk.begin(); i != chunk.end(); i++)
			  {
				  vU = vU*u;
				  vD = vD*d;
				  state[n + i] = (std::max)(vU - input->K, 0.0);
				  state[n - i] = (std::max)(vD - input->K, 0.0);
			  }
		  };
		  tbb::parallel_for(range, f, tbb::simple_partitioner());
	  }
	  else
	  {
		  //TODO: parfor here
		  for (int i = 1; i <= n; i++){
			  vU = vU*u;
			  vD = vD*d;
			  state[n + i] = (std::max)(vU - input->K, 0.0);
			  state[n - i] = (std::max)(vD - input->K, 0.0);
		  }
	  }
	  double wU = input->wU, wD = input->wD, wM = input->wM;

	  //TODO:double parfor here
	  /*for (int t = n - 1; t >= 0; t--){		//can be replaced with  for (int t = 0; t < n; t++){
		  std::vector<double> tmp = state;

		  vU = input->S0, vD = input->S0;
		  for (int i = 0; i<n; i++){
			  double vCU = wU*state[n + i + 1] + wM*state[n + i] + wD*state[n + i - 1];
			  double vCD = wU*state[n - i + 1] + wM*state[n - i] + wD*state[n - i - 1];
			  vCU = (std::max)(vCU, vU - input->K);
			  vCD = (std::max)(vCD, vD - input->K);
			  tmp[n + i] = vCU;
			  tmp[n - i] = vCD;

			  vU = vU*u;
			  vD = vD*d;
		  }

		  state = tmp;
	  }*/

	  bool doubleparfor = false;
	  if (doubleparfor == true)		//double parfor doesn't produce the correct result. It would appear the outer loop can't be parallelised
	  {
		  auto f2 = [&](const tbb::blocked_range2d<unsigned> &chunk) {
			  for (unsigned t = chunk.rows().begin(); t != chunk.rows().end(); t++){
				  std::vector<double> tmp = state;

				  vU = input->S0, vD = input->S0;
				  for (unsigned i = chunk.cols().begin(); i != chunk.cols().end(); i++){
					  double vCU = wU*state[n + i + 1] + wM*state[n + i] + wD*state[n + i - 1];
					  double vCD = wU*state[n - i + 1] + wM*state[n - i] + wD*state[n - i - 1];
					  vCU = (std::max)(vCU, vU - input->K);
					  vCD = (std::max)(vCD, vD - input->K);
					  tmp[n + i] = vCU;
					  tmp[n - i] = vCD;

					  vU = input->S0*std::pow(u, (i + 1));	//Can raise this to the power
					  vD = input->S0*std::pow(d, (i + 1));
				  }
				  state = tmp;
			  }
		  };
		  tbb::parallel_for(tbb::blocked_range2d<unsigned>(0, n, K, 0, n, K), f2, tbb::simple_partitioner());
	  }
	  else
	  {
		  std::vector<double> tmp(state.size());	//std::vector<double> tmp = state;

		  //Original Code:
		  //for (int t = 0; t < n; t++){
		  // vU = input->S0, vD = input->S0;
		  // for (int i = 0; i < n; i++){
		  //  vU = input->S0*std::pow(u, (i));	//Can raise this to the power
		  //  vD = input->S0*std::pow(d, (i));
		  //  double vCU = wU*state[n + i + 1] + wM*state[n + i] + wD*state[n + i - 1];	//state depends on the previous iteration (of outer loop)'s result
		  //  double vCD = wU*state[n - i + 1] + wM*state[n - i] + wD*state[n - i - 1];
		  //  vCU = (std::max)(vCU, vU - input->K);	//vU depends on the previous iteration's result for vU
		  //  vCD = (std::max)(vCD, vD - input->K);
		  //  tmp[n + i] = vCU;
		  //  tmp[n - i] = vCD;

		  //  //vU = vU*u;	//Can raise this to the power
		  //  //vD = vD*d;
		  // }
		  // std::swap(state, tmp);	//state = tmp;
		  //}

		  //****parfor outer loop********
		  //my_range_t range2(0, n, K);
		  //auto f2 = [&](const my_range_t &chunk2)
		  //{
		  // for (unsigned t = chunk2.begin(); t != chunk2.end(); t++)
		  // {
		  //  std::vector<double> tmp = state;

		  //  vU = input->S0, vD = input->S0;
		  //  for (int i = 0; i < n; i++){
		  //	  double vCU = wU*state[n + i + 1] + wM*state[n + i] + wD*state[n + i - 1];
		  //	  double vCD = wU*state[n - i + 1] + wM*state[n - i] + wD*state[n - i - 1];
		  //	  vCU = (std::max)(vCU, vU - input->K);
		  //	  vCD = (std::max)(vCD, vD - input->K);
		  //	  tmp[n + i] = vCU;
		  //	  tmp[n - i] = vCD;

		  //	  vU = vU*u;
		  //	  vD = vD*d;
		  //  }
		  //  state = tmp;
		  // }
		  //};
		  //tbb::parallel_for(range2, f2, tbb::simple_partitioner());

		  //*********parfor inner loop**************
		  bool parfor_inner = true;
		  if (parfor_inner == true)
		  {
			  for (int t = 0; t < n; t++){
				  std::vector<double> tmp(state.size());	//std::vector<double> tmp = state;
				  typedef tbb::blocked_range<unsigned> my_range_t;
				  my_range_t range2(0, n, K);
				  auto f2 = [&](const my_range_t &chunk2){	//look at moving this out of the loop
					  for (unsigned i = chunk2.begin(); i != chunk2.end(); i++){
						  double vU = input->S0*std::pow(u, (i));	//This is expensive, do it outside of the i loop
						  double vD = input->S0*std::pow(d, (i));
						  double vCU = wU*state[n + i + 1] + wM*state[n + i] + wD*state[n + i - 1];	//state depends on the previous iteration (of outer loop)'s result
						  double vCD = wU*state[n - i + 1] + wM*state[n - i] + wD*state[n - i - 1];
						  vCU = (std::max)(vCU, vU - input->K);	//vU depends on the previous iteration's result for vU
						  vCD = (std::max)(vCD, vD - input->K);
						  tmp[n + i] = vCU;
						  tmp[n - i] = vCD;
					  }
				  };
				  tbb::parallel_for(range2, f2, tbb::simple_partitioner());
				  std::swap(state, tmp);	//state = tmp;
			  }
		  }
		  else
		  {
			  //Test code:
			  for (int t = 0; t < n; t++){
				  for (int i = 0; i < n/2; i++){
					  vU = input->S0*std::pow(u, (i));	//Can raise this to the power
					  vD = input->S0*std::pow(d, (i));
					  double vCU = wU*state[n + i + 1] + wM*state[n + i] + wD*state[n + i - 1];	//state depends on the previous iteration (of outer loop)'s result
					  double vCD = wU*state[n - i + 1] + wM*state[n - i] + wD*state[n - i - 1];
					  vCU = (std::max)(vCU, vU - input->K);	//vU depends on the previous iteration's result for vU
					  vCD = (std::max)(vCD, vD - input->K);
					  tmp[n + i] = vCU;
					  tmp[n - i] = vCD;
				  }
				  for (int i = n/2; i < n; i++){
					  vU = input->S0*std::pow(u, (i));	//Can raise this to the power
					  vD = input->S0*std::pow(d, (i));
					  double vCU = wU*state[n + i + 1] + wM*state[n + i] + wD*state[n + i - 1];	//state depends on the previous iteration (of outer loop)'s result
					  double vCD = wU*state[n - i + 1] + wM*state[n - i] + wD*state[n - i - 1];
					  vCU = (std::max)(vCU, vU - input->K);	//vU depends on the previous iteration's result for vU
					  vCD = (std::max)(vCD, vD - input->K);
					  tmp[n + i] = vCU;
					  tmp[n - i] = vCD;
				  }
				  std::swap(state, tmp);	//state = tmp;
			  }
		  }
	  }
	  output->steps = n;
	  output->value = state[n];

	  log->LogVerbose("Priced n=%d, S0=%lg, K=%lg, r=%lg, sigma=%lg, BU=%lg : value=%lg", n, input->S0, input->K, input->r, input->sigma, input->BU, output->value);
  }

};

#endif
