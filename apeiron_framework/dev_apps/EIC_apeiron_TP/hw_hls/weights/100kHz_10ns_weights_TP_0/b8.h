//Numpy array shape [4]
//Min -0.320312500000
//Max 0.156250000000
//Number of zeros 1

#ifndef B8_H_
#define B8_H_

#ifndef __SYNTHESIS__
bias8_t b8[4];
#else
bias8_t b8[4] = {0.0000000, 0.0781250, 0.1562500, -0.3203125};

#endif

#endif
