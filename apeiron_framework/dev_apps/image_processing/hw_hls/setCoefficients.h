void set3x3SquareStructuringElement(short int &c00, short int &c01, short int &c02,
		   short int &c10, short int &c11, short int &c12,
		   short int &c20, short int &c21, short int &c22);;

void set3x3CrossStructuringElement(short int &c00, short int &c01, short int &c02,
		   short int &c10, short int &c11, short int &c12,
		   short int &c20, short int &c21, short int &c22);;

void set3x3RemoveDoubleVerticalEdgesStructuringElement(short int &c00, short int &c01, short int &c02,
													   short int &c10, short int &c11, short int &c12,
													   short int &c20, short int &c21, short int &c22);;

void set3x3RemoveDoubleHorizontalEdgesStructuringElement(short int &c00, short int &c01, short int &c02,
													   short int &c10, short int &c11, short int &c12,
													   short int &c20, short int &c21, short int &c22);;

void set3x3RemoveThickHorizontalEdgesStructuringElement(short int &c00, short int &c01, short int &c02,
													    short int &c10, short int &c11, short int &c12,
													    short int &c20, short int &c21, short int &c22);;

void set3x3RemoveThickVerticalEdgesStructuringElement(short int &c00, short int &c01, short int &c02,
													    short int &c10, short int &c11, short int &c12,
													    short int &c20, short int &c21, short int &c22);;

void setHEdgeDetectWeights(short int &c00, short int &c01, short int &c02,
						   short int &c10, short int &c11, short int &c12,
						   short int &c20, short int &c21, short int &c22);;


void setXSobelEdgeDetectWeights(short int &c00, short int &c01, short int &c02,
						   short int &c10, short int &c11, short int &c12,
						   short int &c20, short int &c21, short int &c22);;


void setYSobelEdgeDetectWeights(short int &c00, short int &c01, short int &c02,
						   short int &c10, short int &c11, short int &c12,
						   short int &c20, short int &c21, short int &c22);;


void setXYSobelEdgeDetectWeights(short int &c00, short int &c01, short int &c02,
						   short int &c10, short int &c11, short int &c12,
						   short int &c20, short int &c21, short int &c22);;

// highboost filtering with the following matrix
//	-1		-1		-1
//	-1		8+A		-1
//	-1		-1		-1
void setHighBoostFiltering(short int &c00, short int &c01, short int &c02,
						   short int &c10, short int &c11, short int &c12,
						   short int &c20, short int &c21, short int &c22,
						   short int A);;

void setVEdgeDetectWeights(short int &c00, short int &c01, short int &c02,
						   short int &c10, short int &c11, short int &c12,
						   short int &c20, short int &c21, short int &c22);;


void set3x3GaussianFilterWeights( short int &c00, short int &c01, short int &c02,
							   short int &c10, short int &c11, short int &c12,
							   short int &c20, short int &c21, short int &c22);

void set5x5GaussianFilterWeights_s_eq_1(short int &c00, short int &c01, short int &c02, short int &c03, short int &c04,
							   short int &c10, short int &c11, short int &c12, short int &c13, short int &c14,
							   short int &c20, short int &c21, short int &c22, short int &c23, short int &c24,
							   short int &c30, short int &c31, short int &c32, short int &c33, short int &c34,
							   short int &c40, short int &c41, short int &c42, short int &c43, short int &c44);

void set5x5GaussianFilterWeights_s_eq_2(short int &c00, short int &c01, short int &c02, short int &c03, short int &c04,
							   short int &c10, short int &c11, short int &c12, short int &c13, short int &c14,
							   short int &c20, short int &c21, short int &c22, short int &c23, short int &c24,
							   short int &c30, short int &c31, short int &c32, short int &c33, short int &c34,
							   short int &c40, short int &c41, short int &c42, short int &c43, short int &c44);

void set5x5GaussianFilterWeights_s_eq_3(short int &c00, short int &c01, short int &c02, short int &c03, short int &c04,
							   short int &c10, short int &c11, short int &c12, short int &c13, short int &c14,
							   short int &c20, short int &c21, short int &c22, short int &c23, short int &c24,
							   short int &c30, short int &c31, short int &c32, short int &c33, short int &c34,
							   short int &c40, short int &c41, short int &c42, short int &c43, short int &c44);

void set5x5AveragingFilterWeights(short int &c00, short int &c01, short int &c02, short int &c03, short int &c04,
							   short int &c10, short int &c11, short int &c12, short int &c13, short int &c14,
							   short int &c20, short int &c21, short int &c22, short int &c23, short int &c24,
							   short int &c30, short int &c31, short int &c32, short int &c33, short int &c34,
							   short int &c40, short int &c41, short int &c42, short int &c43, short int &c44);

void set7x7GaussianFilterWeights_s_eq_1(short int &c00, short int &c01, short int &c02, short int &c03, short int &c04, short int &c05, short int &c06,
								short int &c10, short int &c11, short int &c12, short int &c13, short int &c14, short int &c15, short int &c16,
								short int &c20, short int &c21, short int &c22, short int &c23, short int &c24, short int &c25, short int &c26,
								short int &c30, short int &c31, short int &c32, short int &c33, short int &c34, short int &c35, short int &c36,
								short int &c40, short int &c41, short int &c42, short int &c43, short int &c44, short int &c45, short int &c46,
								short int &c50, short int &c51, short int &c52, short int &c53, short int &c54, short int &c55, short int &c56,
								short int &c60, short int &c61, short int &c62, short int &c63, short int &c64, short int &c65, short int &c66);

void set7x7GaussianFilterWeights_s_eq_2(short int &c00, short int &c01, short int &c02, short int &c03, short int &c04, short int &c05, short int &c06,
								short int &c10, short int &c11, short int &c12, short int &c13, short int &c14, short int &c15, short int &c16,
								short int &c20, short int &c21, short int &c22, short int &c23, short int &c24, short int &c25, short int &c26,
								short int &c30, short int &c31, short int &c32, short int &c33, short int &c34, short int &c35, short int &c36,
								short int &c40, short int &c41, short int &c42, short int &c43, short int &c44, short int &c45, short int &c46,
								short int &c50, short int &c51, short int &c52, short int &c53, short int &c54, short int &c55, short int &c56,
								short int &c60, short int &c61, short int &c62, short int &c63, short int &c64, short int &c65, short int &c66);

void set7x7GaussianFilterWeights_s_eq_3(short int &c00, short int &c01, short int &c02, short int &c03, short int &c04, short int &c05, short int &c06,
								short int &c10, short int &c11, short int &c12, short int &c13, short int &c14, short int &c15, short int &c16,
								short int &c20, short int &c21, short int &c22, short int &c23, short int &c24, short int &c25, short int &c26,
								short int &c30, short int &c31, short int &c32, short int &c33, short int &c34, short int &c35, short int &c36,
								short int &c40, short int &c41, short int &c42, short int &c43, short int &c44, short int &c45, short int &c46,
								short int &c50, short int &c51, short int &c52, short int &c53, short int &c54, short int &c55, short int &c56,
								short int &c60, short int &c61, short int &c62, short int &c63, short int &c64, short int &c65, short int &c66);

void set7x7AveragingFilterWeights(short int &c00, short int &c01, short int &c02, short int &c03, short int &c04, short int &c05, short int &c06,
								short int &c10, short int &c11, short int &c12, short int &c13, short int &c14, short int &c15, short int &c16,
								short int &c20, short int &c21, short int &c22, short int &c23, short int &c24, short int &c25, short int &c26,
								short int &c30, short int &c31, short int &c32, short int &c33, short int &c34, short int &c35, short int &c36,
								short int &c40, short int &c41, short int &c42, short int &c43, short int &c44, short int &c45, short int &c46,
								short int &c50, short int &c51, short int &c52, short int &c53, short int &c54, short int &c55, short int &c56,
								short int &c60, short int &c61, short int &c62, short int &c63, short int &c64, short int &c65, short int &c66);

void setReconstructionFilterWeights( short int &c00, short int &c01, short int &c02,
							   short int &c10, short int &c11, short int &c12,
							   short int &c20, short int &c21, short int &c22);


