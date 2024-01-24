void set3x3SquareStructuringElement(short int &c00, short int &c01, short int &c02,
		   short int &c10, short int &c11, short int &c12,
		   short int &c20, short int &c21, short int &c22)
{
	c00 = 255;
	c01 = 255;
	c02 = 255;

	c10 = 255;
	c11 = 255;
	c12 = 255;

	c20 = 255;
	c21 = 255;
	c22 = 255;
}

void set3x3CrossStructuringElement(short int &c00, short int &c01, short int &c02,
		   short int &c10, short int &c11, short int &c12,
		   short int &c20, short int &c21, short int &c22)
{
	c00 = 0;
	c01 = 255;
	c02 = 0;

	c10 = 255;
	c11 = 255;
	c12 = 255;

	c20 = 0;
	c21 = 255;
	c22 = 0;
}

void set3x3RemoveDoubleVerticalEdgesStructuringElement(short int &c00, short int &c01, short int &c02,
													   short int &c10, short int &c11, short int &c12,
													   short int &c20, short int &c21, short int &c22)
{
c00 = 255;
c01 = 0;
c02 = 255;

c10 = 255;
c11 = 0;
c12 = 255;

c20 = 255;
c21 = 0;
c22 = 255;
}
void set3x3RemoveDoubleHorizontalEdgesStructuringElement(short int &c00, short int &c01, short int &c02,
													   short int &c10, short int &c11, short int &c12,
													   short int &c20, short int &c21, short int &c22)
{
c00 = 255;
c01 = 255;
c02 = 255;

c10 = 0;
c11 = 0;
c12 = 0;

c20 = 255;
c21 = 255;
c22 = 255;
}

void set3x3RemoveThickHorizontalEdgesStructuringElement(short int &c00, short int &c01, short int &c02,
													    short int &c10, short int &c11, short int &c12,
													    short int &c20, short int &c21, short int &c22)
{
c00 = 0;
c01 = 0;
c02 = 0;

c10 = 255;
c11 = 255;
c12 = 255;

c20 = 255;
c21 = 255;
c22 = 255;
}

void set3x3RemoveThickVerticalEdgesStructuringElement(short int &c00, short int &c01, short int &c02,
													    short int &c10, short int &c11, short int &c12,
													    short int &c20, short int &c21, short int &c22)
{
c00 = 0;
c01 = 255;
c02 = 255;

c10 = 0;
c11 = 255;
c12 = 255;

c20 = 0;
c21 = 255;
c22 = 255;
}
void setHEdgeDetectWeights(short int &c00, short int &c01, short int &c02,
						   short int &c10, short int &c11, short int &c12,
						   short int &c20, short int &c21, short int &c22)
{
    c00 = 116;  //filter coefficients for derivative of gaussian; H edge detect + gaussian filtering; weights are multiplied by 2^10 and converted to integer values
    c10 = 0;
    c20 = -116;

    c01 = 792;
    c11 = 0;
    c21 = -792;

    c02 = 116;
    c12 = 0;
    c22 = -116;
}

void setXSobelEdgeDetectWeights(short int &c00, short int &c01, short int &c02,
						   short int &c10, short int &c11, short int &c12,
						   short int &c20, short int &c21, short int &c22)
{
    c00 = 1024;  //filter coefficients for derivative of gaussian; H edge detect + gaussian filtering; weights are multiplied by 2^10 and converted to integer values
    c10 = 2048;
    c20 = 1024;

    c01 = 0;
    c11 = 0;
    c21 = 0;

    c02 = -1024;
    c12 = -2048;
    c22 = -1024;
}

void setYSobelEdgeDetectWeights(short int &c00, short int &c01, short int &c02,
						   short int &c10, short int &c11, short int &c12,
						   short int &c20, short int &c21, short int &c22)
{
    c00 = 1024;  //filter coefficients for derivative of gaussian; H edge detect + gaussian filtering; weights are multiplied by 2^10 and converted to integer values
    c10 = 0;
    c20 = -1024;

    c01 = 2048;
    c11 = 0;
    c21 = -2048;

    c02 = 1024;
    c12 = 0;
    c22 = -1024;
}


void setXYSobelEdgeDetectWeights(short int &c00, short int &c01, short int &c02,
						   short int &c10, short int &c11, short int &c12,
						   short int &c20, short int &c21, short int &c22)
{
    c00 = 2048;  //filter coefficients for derivative of gaussian; H edge detect + gaussian filtering; weights are multiplied by 2^10 and converted to integer values
    c10 = 2048;
    c20 = 0;

    c01 = 2048;
    c11 = 0;
    c21 = -2048;

    c02 = 0;
    c12 = -2048;
    c22 = -2048;
}

// highboost filtering with the following matrix
//	-1		-1		-1
//	-1		8+A		-1
//	-1		-1		-1
void setHighBoostFiltering(short int &c00, short int &c01, short int &c02,
						   short int &c10, short int &c11, short int &c12,
						   short int &c20, short int &c21, short int &c22,
						   short int A)
{
    c00 = -1024;  //filter coefficients for derivative of gaussian; H edge detect + gaussian filtering; weights are multiplied by 2^10 and converted to integer values
    c10 = -1024;
    c20 = -1024;

    c01 = -1024;
    c11 = 8192 + A*512;
    c21 = -1024;

    c02 = -1024;
    c12 = -1024;
    c22 = -1024;
}


void setVEdgeDetectWeights(short int &c00, short int &c01, short int &c02,
						   short int &c10, short int &c11, short int &c12,
						   short int &c20, short int &c21, short int &c22)
{
	c00 = 116;  //filter coefficients for derivative of gaussian; V edge detect + gaussian filtering; weights are multiplied by 2^10 and converted to integer values
	c10 = 792;
	c20 = 116;

	c01 = 0;
	c11 = 0;
	c21 = 0;

	c02 = -116;
	c12 = -792;
	c22 = -116;
}

void set3x3GaussianFilterWeights( short int &c00, short int &c01, short int &c02,
							   short int &c10, short int &c11, short int &c12,
							   short int &c20, short int &c21, short int &c22)
{
    c00 = 12;  //filter coefficients are multiplied by 2^10 and converted to integer values
    c10 = 86;
    c20 = 12;

    c01 = 86;
    c11 = 632;
    c21 = 86;

    c02 = 12;
    c12 = 86;
    c22 = 12;
}

void set5x5GaussianFilterWeights_s_eq_1(short int &c00, short int &c01, short int &c02, short int &c03, short int &c04,
							   short int &c10, short int &c11, short int &c12, short int &c13, short int &c14,
							   short int &c20, short int &c21, short int &c22, short int &c23, short int &c24,
							   short int &c30, short int &c31, short int &c32, short int &c33, short int &c34,
							   short int &c40, short int &c41, short int &c42, short int &c43, short int &c44)
{
    c00 = 3;  //filter coefficients are multiplied by 2^10 and converted to integer values
    c10 = 14;
    c20 = 22;
    c30 = c10;
	c40 = c00;

	c01 = 14;
    c11 = 61;
    c21 = 101;
    c31 = c11;
	c41 = c40;

	c02 = 22;
    c12 = 101;
    c22 = 166;
    c32 = c12;
	c42 = c02;

	c03 = c01;
    c13 = c11;
    c23 = c21;
    c33 = c31;
	c43 = c41;

	c04 = c00;
    c14 = c10;
    c24 = c20;
    c34 = c30;
	c44 = c40;
}

void set5x5GaussianFilterWeights_s_eq_2(short int &c00, short int &c01, short int &c02, short int &c03, short int &c04,
							   short int &c10, short int &c11, short int &c12, short int &c13, short int &c14,
							   short int &c20, short int &c21, short int &c22, short int &c23, short int &c24,
							   short int &c30, short int &c31, short int &c32, short int &c33, short int &c34,
							   short int &c40, short int &c41, short int &c42, short int &c43, short int &c44)
{
    c00 = 24;  //filter coefficients are multiplied by 2^10 and converted to integer values
    c10 = 35;
    c20 = 39;
    c30 = c10;
	c40 = c00;

	c01 = 35;
    c11 = 50;
    c21 = 57;
    c31 = c11;
	c41 = c40;

	c02 = 39;
    c12 = 57;
    c22 = 65;
    c32 = c12;
	c42 = c02;

	c03 = c01;
    c13 = c11;
    c23 = c21;
    c33 = c31;
	c43 = c41;

	c04 = c00;
    c14 = c10;
    c24 = c20;
    c34 = c30;
	c44 = c40;
}

void set5x5GaussianFilterWeights_s_eq_3(short int &c00, short int &c01, short int &c02, short int &c03, short int &c04,
							   short int &c10, short int &c11, short int &c12, short int &c13, short int &c14,
							   short int &c20, short int &c21, short int &c22, short int &c23, short int &c24,
							   short int &c30, short int &c31, short int &c32, short int &c33, short int &c34,
							   short int &c40, short int &c41, short int &c42, short int &c43, short int &c44)
{
    c00 = 33;  //filter coefficients are multiplied by 2^10 and converted to integer values
    c10 = 38;
    c20 = 41;
    c30 = c10;
	c40 = c00;

	c01 = 38;
    c11 = 45;
    c21 = 48;
    c31 = c11;
	c41 = c40;

	c02 = 41;
    c12 = 48;
    c22 = 51;
    c32 = c12;
	c42 = c02;

	c03 = c01;
    c13 = c11;
    c23 = c21;
    c33 = c31;
	c43 = c41;

	c04 = c00;
    c14 = c10;
    c24 = c20;
    c34 = c30;
	c44 = c40;
}

void set5x5AveragingFilterWeights(short int &c00, short int &c01, short int &c02, short int &c03, short int &c04,
							   short int &c10, short int &c11, short int &c12, short int &c13, short int &c14,
							   short int &c20, short int &c21, short int &c22, short int &c23, short int &c24,
							   short int &c30, short int &c31, short int &c32, short int &c33, short int &c34,
							   short int &c40, short int &c41, short int &c42, short int &c43, short int &c44)
{
    c00 = 41;  //filter coefficients are multiplied by 2^10 and converted to integer values
    c10 = 41;
    c20 = 41;
    c30 = 41;
	c40 = 41;

	c01 = 41;
    c11 = 41;
    c21 = 41;
    c31 = 41;
	c41 = 41;

	c02 = 41;
    c12 = 41;
    c22 = 41;
    c32 = 41;
	c42 = 41;

	c03 = 41;
    c13 = 41;
    c23 = 41;
    c33 = 41;
	c43 = 41;

	c04 = 41;
    c14 = 41;
    c24 = 41;
    c34 = 41;
	c44 = 41;
}

void set7x7GaussianFilterWeights_s_eq_1(short int &c00, short int &c01, short int &c02, short int &c03, short int &c04, short int &c05, short int &c06,
								short int &c10, short int &c11, short int &c12, short int &c13, short int &c14, short int &c15, short int &c16,
								short int &c20, short int &c21, short int &c22, short int &c23, short int &c24, short int &c25, short int &c26,
								short int &c30, short int &c31, short int &c32, short int &c33, short int &c34, short int &c35, short int &c36,
								short int &c40, short int &c41, short int &c42, short int &c43, short int &c44, short int &c45, short int &c46,
								short int &c50, short int &c51, short int &c52, short int &c53, short int &c54, short int &c55, short int &c56,
								short int &c60, short int &c61, short int &c62, short int &c63, short int &c64, short int &c65, short int &c66)
{
    c00 = 0;  //filter coefficients are multiplied by 2^10 and converted to integer values
    c10 = 0;
    c20 = 1;
    c30 = 2;
	c40 = 1;
	c50 = 0;
	c60 = 0;

	c01 = 0;
    c11 = 3;
    c21 = 13;
    c31 = 22;
	c41 = 13;
	c51 = 3;
    c61 = 0;

	c02 = 1;
    c12 = 13;
    c22 = 60;
    c32 = 99;
	c42 = 60;
	c52 = 13;
	c62 = 1;

	c03 = 2;
    c13 = 22;
    c23 = 99;
    c33 = 163;
	c43 = 99;
	c53 = 22;
	c63 = 2;

	c04 = 1;
    c14 = 13;
    c24 = 60;
    c34 = 99;
	c44 = 60;
	c54 = 13;
	c64 = 1;

	c05 = 0;
    c15 = 3;
    c25 = 13;
    c35 = 22;
	c45 = 13;
	c55 = 3;
    c65 = 0;

    c06 = 0;
    c16 = 0;
    c26 = 1;
    c36 = 2;
	c46 = 1;
	c56 = 0;
	c66 = 0;

}

void set7x7GaussianFilterWeights_s_eq_2(short int &c00, short int &c01, short int &c02, short int &c03, short int &c04, short int &c05, short int &c06,
								short int &c10, short int &c11, short int &c12, short int &c13, short int &c14, short int &c15, short int &c16,
								short int &c20, short int &c21, short int &c22, short int &c23, short int &c24, short int &c25, short int &c26,
								short int &c30, short int &c31, short int &c32, short int &c33, short int &c34, short int &c35, short int &c36,
								short int &c40, short int &c41, short int &c42, short int &c43, short int &c44, short int &c45, short int &c46,
								short int &c50, short int &c51, short int &c52, short int &c53, short int &c54, short int &c55, short int &c56,
								short int &c60, short int &c61, short int &c62, short int &c63, short int &c64, short int &c65, short int &c66)
{
    c00 = 5;  //filter coefficients are multiplied by 2^10 and converted to integer values
    c10 = 9;
    c20 = 14;
    c30 = 16;
	c40 = 14;
	c50 = 9;
	c60 = 5;

	c01 = 9;
    c11 = 18;
    c21 = 26;
    c31 = 29;
	c41 = 26;
	c51 = 18;
    c61 = 9;

	c02 = 14;
    c12 = 26;
    c22 = 37;
    c32 = 42;
	c42 = 37;
	c52 = 26;
	c62 = 14;

	c03 = 16;
    c13 = 29;
    c23 = 42;
    c33 = 48;
	c43 = 42;
	c53 = 29;
	c63 = 16;

	c04 = 14;
    c14 = 26;
    c24 = 37;
    c34 = 42;
	c44 = 37;
	c54 = 26;
	c64 = 14;


	c05 = 9;
    c15 = 18;
    c25 = 26;
    c35 = 29;
	c45 = 26;
	c55 = 18;
    c65 = 9;

    c06 = 5;
    c16 = 9;
    c26 = 14;
    c36 = 16;
	c46 = 14;
	c56 = 9;
	c66 = 5;

}

void set7x7GaussianFilterWeights_s_eq_3(short int &c00, short int &c01, short int &c02, short int &c03, short int &c04, short int &c05, short int &c06,
								short int &c10, short int &c11, short int &c12, short int &c13, short int &c14, short int &c15, short int &c16,
								short int &c20, short int &c21, short int &c22, short int &c23, short int &c24, short int &c25, short int &c26,
								short int &c30, short int &c31, short int &c32, short int &c33, short int &c34, short int &c35, short int &c36,
								short int &c40, short int &c41, short int &c42, short int &c43, short int &c44, short int &c45, short int &c46,
								short int &c50, short int &c51, short int &c52, short int &c53, short int &c54, short int &c55, short int &c56,
								short int &c60, short int &c61, short int &c62, short int &c63, short int &c64, short int &c65, short int &c66)
{
    c00 = 12;  //filter coefficients are multiplied by 2^10 and converted to integer values
    c10 = 15;
    c20 = 18;
    c30 = 19;
	c40 = 18;
	c50 = 15;
	c60 = 12;

	c01 = 15;
    c11 = 20;
    c21 = 24;
    c31 = 25;
	c41 = 24;
	c51 = 20;
    c61 = 15;

	c02 = 18;
    c12 = 24;
    c22 = 28;
    c32 = 30;
	c42 = 28;
	c52 = 24;
	c62 = 18;

	c03 = 19;
    c13 = 25;
    c23 = 30;
    c33 = 31;
	c43 = 20;
	c53 = 25;
	c63 = 19;

	c04 = 18;
    c14 = 24;
    c24 = 28;
    c34 = 30;
	c44 = 28;
	c54 = 24;
	c64 = 18;


	c05 = 15;
    c15 = 20;
    c25 = 24;
    c35 = 25;
	c45 = 24;
	c55 = 20;
    c65 = 15;

    c06 = 12;
    c16 = 15;
    c26 = 18;
    c36 = 19;
	c46 = 18;
	c56 = 15;
	c66 = 12;

}

void set7x7AveragingFilterWeights(short int &c00, short int &c01, short int &c02, short int &c03, short int &c04, short int &c05, short int &c06,
								short int &c10, short int &c11, short int &c12, short int &c13, short int &c14, short int &c15, short int &c16,
								short int &c20, short int &c21, short int &c22, short int &c23, short int &c24, short int &c25, short int &c26,
								short int &c30, short int &c31, short int &c32, short int &c33, short int &c34, short int &c35, short int &c36,
								short int &c40, short int &c41, short int &c42, short int &c43, short int &c44, short int &c45, short int &c46,
								short int &c50, short int &c51, short int &c52, short int &c53, short int &c54, short int &c55, short int &c56,
								short int &c60, short int &c61, short int &c62, short int &c63, short int &c64, short int &c65, short int &c66)
{
    c00 = 21;  //filter coefficients are multiplied by 2^10 and converted to integer values
    c10 = 21;
    c20 = 21;
    c30 = 21;
	c40 = 21;
	c50 = 21;
	c60 = 21;

	c01 = 21;
    c11 = 21;
    c21 = 21;
    c31 = 21;
	c41 = 21;
	c51 = 21;
    c61 = 21;

	c02 = 21;
    c12 = 21;
    c22 = 21;
    c32 = 21;
	c42 = 21;
	c52 = 21;
	c62 = 11;

	c03 = 21;
    c13 = 21;
    c23 = 21;
    c33 = 21;
	c43 = 21;
	c53 = 21;
	c63 = 21;

	c04 = 21;
    c14 = 21;
    c24 = 21;
    c34 = 21;
	c44 = 21;
	c54 = 21;
	c64 = 21;

	c05 = 21;
    c15 = 21;
    c25 = 21;
    c35 = 21;
	c45 = 21;
	c55 = 21;
    c65 = 21;

    c06 = 21;
    c16 = 21;
    c26 = 21;
    c36 = 21;
	c46 = 21;
	c56 = 21;
	c66 = 21;

}
// reconstruction filter . Derived from
//        1024 1024 1024
//  (4/9) 1024 1024 1024
//        1024 1024 1024
// it is applied to an upsampled image (by 2)
void setReconstructionFilterWeights( short int &c00, short int &c01, short int &c02,
							   short int &c10, short int &c11, short int &c12,
							   short int &c20, short int &c21, short int &c22)
{
    c00 = 455;  //filter coefficients are multiplied by 2^10 and converted to integer values
    c10 = 455;
    c20 = 455;

    c01 = 455;
    c11 = 455;
    c21 = 455;

    c02 = 455;
    c12 = 455;
    c22 = 455;
}
