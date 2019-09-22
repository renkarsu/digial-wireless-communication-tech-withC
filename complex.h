#ifndef COMPLEX_H_
#define COMPLEX_H_

#include <math.h>

typedef struct
{
    double I ;
    double Q ;
}COMPLEX ;

COMPLEX SumCom(COMPLEX a, COMPLEX b)
{
    COMPLEX c ;
    c.I = a.I + b.I ;
    c.Q = a.Q + b.Q ;
    return c ;
}

COMPLEX SubCom(COMPLEX a, COMPLEX b)
{
    COMPLEX c ;
    c.I = a.I - b.I ;
    c.Q = a.Q - b.Q ;
    return c ;
}

COMPLEX ProCom(COMPLEX a, COMPLEX b)
{
    COMPLEX c ;
    c.I = a.I * b.I - a.Q * b.Q ;
    c.Q = a.I * b.Q + a.Q * b.I ;
    return c ;
}

COMPLEX MakeConjugate(COMPLEX a)
{
    a.Q = (-1) * a.Q ;
    return a ;
}

double MakeAbs(COMPLEX a)
{
    return ( sqrt(pow(a.I, 2) + pow(a.Q, 2)) ) ;
}

COMPLEX ConvertByEuler(double A, double phi)
{
    COMPLEX c ;
    c.I = A * cos(phi) ;
    c.Q = A * sin(phi) ;
    return c ;
}

#endif// COMPLEX_H_