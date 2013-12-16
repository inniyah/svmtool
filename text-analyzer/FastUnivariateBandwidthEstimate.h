
#ifndef FAST_UNIVARIATE_BANDWIDTH_ESTIMATE_H_BDBBD9EC_650C_11E3_95CF_525400DA3F0D
#define FAST_UNIVARIATE_BANDWIDTH_ESTIMATE_H_BDBBD9EC_650C_11E3_95CF_525400DA3F0D

#include "UnivariateBandwidthEstimate.h"

class FastUnivariateBandwidthEstimate : public UnivariateBandwidthEstimate {
public:
	double Evaluate(unsigned int N, const double * X, double epsil, int verbosity = 0);

private:
	struct lm_data_struct {
		// Properties
		int N;
		double * X;
		double constant1;
		double constant2;
		double epsil;
		// Methods
		double h_function(double h);
	};

	static void lm_evaluate(
		const double * par,
		int m_dat,
		const void * p_data,
		double * fvec,
		int * userbreak
	);
};

#endif // FAST_UNIVARIATE_BANDWIDTH_ESTIMATE_H_BDBBD9EC_650C_11E3_95CF_525400DA3F0D
