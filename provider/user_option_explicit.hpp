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
	  for (int i = 1; i <= n; i++){
		  vU = vU*u;
		  vD = vD*d;
		  state[n + i] = (std::max)(vU - input->K, 0.0);
		  state[n - i] = (std::max)(vD - input->K, 0.0);
	  }

	  double wU = input->wU, wD = input->wD, wM = input->wM;
	  for (int t = n - 1; t >= 0; t--){
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
	  }

	  output->steps = n;
	  output->value = state[n];

	  log->LogVerbose("Priced n=%d, S0=%lg, K=%lg, r=%lg, sigma=%lg, BU=%lg : value=%lg", n, input->S0, input->K, input->r, input->sigma, input->BU, output->value);
  }

};

#endif
