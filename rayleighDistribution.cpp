#include <iostream>
#include <random>
#include <cmath>
#include <fstream>

double PI =  acos(-1);
//return正規乱数
double GenerateRateN(void)
{

  std::random_device seed_gen;
  std::default_random_engine engine(seed_gen());
  // 0.0以上1.0未満の値を等確率で発生させる
  std::uniform_real_distribution<> dist(0.0, 1.0);

  double s = dist(engine);
  if(s == 0.0) s = 0.000000001 ;
  double r = std::sqrt( -2.0 * std::log(s) ) ;
  double t = 2.0 * PI * dist(engine) ;
  return ( r * std::sin(t) ) ;
}

double CalculateRayleigh(void)
{
    double n = GenerateRateN() ;
    double m = GenerateRateN() ;
    double c = 1/std::sqrt(2.0) ;
    return std::sqrt( std::pow(n, 2.0) + std::pow(m, 2.0) ) * c ;
}

int main()
{
    std::ofstream resf("check_rayl_result.txt") ;

    for(std::size_t i = 0; i < 100000; ++i)
      {
	double res = CalculateRayleigh();
	resf << res << "\n";
      }

}
