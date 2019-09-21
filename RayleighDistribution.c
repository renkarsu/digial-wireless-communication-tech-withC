//gcc -std=c99 RayleighDistribution.c
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define PI acos(-1)

//return一様乱数[0.0,1.0]
double GenerateRateU(void)
{
    return ( (double)rand() / (double)RAND_MAX ) ;
}

//return正規乱数
double GenerateRateN(void)
{
    double s,r,t ;
    s = GenerateRateU() ;
    if(s == 0.0) s = 0.000000001 ;
    r = sqrt( -2.0 * log(s) ) ;
    t = 2.0 * PI * GenerateRateU() ;
    return ( r * sin(t) ) ;
}

double CalculateRayleigh(void)
{
    double n = GenerateRateN() ;
    double c = 1/sqrt(2.0) ;
    return sqrt( pow(n, 2.0) + pow(n, 2.0) ) * c ;
}

int main(void)
{
    unsigned i ;
    FILE *fp ;
    fp = fopen("check_rayl_result.txt", "w") ;

    for(i = 0; i < 100000; ++i) fprintf( fp, "%f\n", CalculateRayleigh() ) ;

    return 0 ;
}