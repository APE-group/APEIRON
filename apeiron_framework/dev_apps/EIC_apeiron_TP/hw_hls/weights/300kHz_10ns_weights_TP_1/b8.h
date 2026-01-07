//Numpy array shape [4]
//Min -0.046875000000
//Max 0.171875000000
//Number of zeros 0

#ifndef B8_H_
#define B8_H_

#ifndef __SYNTHESIS__
bias8_t b8[4];
#else
bias8_t b8[4] = {0.0156250, -0.0468750, 0.1718750, 0.0078125};

#endif

#endif
