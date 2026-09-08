#ifndef _MATH_H
#define _MATH_H

#define HUGE_VAL (__builtin_huge_val())
#define HUGE_VALF (__builtin_huge_valf())
#define HUGE_VALL (__builtin_huge_vall())
#define INFINITY (__builtin_inf())
#define NAN (__builtin_nan(""))

#define M_E 2.7182818284590452354
#define M_PI 3.14159265358979323846

double fabs(double x);
double floor(double x);
double ceil(double x);
double sqrt(double x);
double pow(double x, double y);
double sin(double x);
double cos(double x);
double log(double x);
double exp(double x);

#endif
