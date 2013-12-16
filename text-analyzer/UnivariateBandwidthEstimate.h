
#ifndef UNIVARIATE_BANDWIDTH_ESTIMATE_H_BDBBD85C_650C_11E3_95CE_525400DA3F0D
#define UNIVARIATE_BANDWIDTH_ESTIMATE_H_BDBBD85C_650C_11E3_95CE_525400DA3F0D

class UnivariateBandwidthEstimate {
public:
	double Evaluate(unsigned int N, const double * X, int verbosity = 0);

protected:
	static double sum(unsigned int num, const double * data);
	static double min(unsigned int num, const double * data);
	static double max(unsigned int num, const double * data);
	static double std(unsigned int num, const double * data);

private:
	struct lm_data_struct {
		// Properties
		int N;
		double * X;
		double constant1;
		double constant2;
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

#endif // UNIVARIATE_BANDWIDTH_ESTIMATE_H_BDBBD85C_650C_11E3_95CE_525400DA3F0D
