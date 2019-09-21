//gcc -std=c99 BPSKsim-array.c

//BPSKのBERを計算機シミュレーションで求め、理論値と比較する。
//配列によるversion, 信号とノイズを配列に格納する
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#define PI acos(-1) //3.1415926535
#define L 10000     //送信データビット数
#define SNR_STEP 10 //誤り率を計算するSNRの値の数. SNR(signal-noise ratio): SN比

typedef struct
{
    double I ;
    double Q ;
}COMPLEX ;

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

//ランダムデータ発生
void MakeRandData(unsigned *d, unsigned length)
{
    unsigned i ;
    double x ;
    for(i = 0; i < length; ++i)
    {
        x = GenerateRateU() ;
        if(x >= 0.5) *(d+i) = 1.0 ;
        else         *(d+i) = 0.0 ;
    }
}

//AWGN発生
void MakeAwgn(COMPLEX *n, unsigned length, double Pn)
{
    unsigned i ;
    for(int i = 0; i < length; ++i)
    {
        (n + i) -> I = GenerateRateN() * sqrt(Pn/2) ;
        (n + i) -> Q = GenerateRateN() * sqrt(Pn/2) ;
    }
}

//雑音電力計算
double CalculateSnrDb2NoisePower(double c_dB)
{
    return pow( 10, (-1) * c_dB / 10 ) ;
}

//BPSK変調器(Modulator)
void ModulateBpsk(COMPLEX *s, unsigned *data, unsigned length)
{
    unsigned i ;
    for(i = 0; i < length; ++i) (s+i) -> I = ( *(data + i) - 0.5) * 2.0 ;
}

//ベクトル加算（系列同士の加算）
void SumVector(COMPLEX *a, COMPLEX *b, COMPLEX *c, unsigned length)
{
    unsigned i ;
    for(i = 0; i < length; ++i)
    {
        (a + i) -> I = (b + i) -> I + (c + i) -> I ;
        (a + i) -> Q = (b + i) -> Q + (c + i) -> Q ;
    }
}

//BPSK復調器(Demodulator)
void DemodulateBPSK(unsigned *rData, COMPLEX *rs, unsigned length)
{
    unsigned i ;
    for(i = 0; i < length; ++i)
    {
        if( (rs + i) -> I > 0 ) *(rData + i) = 1 ;
        else                    *(rData + i) = 0 ;
    }
}

//BER計算
double CalculateBER(unsigned *data, unsigned *rData, unsigned length)
{
    unsigned i ;
    unsigned sum = 0 ;
    double BER ;
    for(i = 0; i < length; ++i)
    {
        sum += abs( *(data + i) - *(rData + i) ) ;
    }
    BER = (double)sum / length ;
    return (BER) ;
}

//BERをファイルへ書き出す
void PrintBER(unsigned snr_step, double *SNRdB, double *BER)
{
    unsigned i ;
    FILE *fp ;
    if( ( fp = fopen("BER.txt", "w") ) == NULL )
    {
        printf("Error: cannot open file \n") ;
        exit(1) ;
    }
    else
    {
        for(i = 0; i < snr_step; ++i)
        {
            printf("%f [dB] BER = %f\n", *(SNRdB + i), *(BER + i) ) ;
            fprintf(fp, "%f %f\n", *(SNRdB + i), *(BER + i) );
        }
        fclose(fp);
    }
}

int main(void)
{
    unsigned txData[L] ;//送信データ
    unsigned rxData[L] ;//受信データ
    unsigned i ;
    COMPLEX txSymbol[L] ;//送信BPSKシンボル
    COMPLEX noise[L] ;
    COMPLEX rxSymbol[L] ;//受信BPSKシンボル
    double BER[SNR_STEP] ;
    double SNRdB[SNR_STEP] ;
    for(i = 0; i < SNR_STEP; ++i) SNRdB[i] = (double)i ;
    for(i = 0; i < SNR_STEP; ++i)
    {
        
        srand( time(NULL) ) ;

        MakeRandData(txData, L) ;

        ModulateBpsk(txSymbol, txData, L) ;

        MakeAwgn(noise, L, CalculateSnrDb2NoisePower(SNRdB[i]) ) ;

        SumVector(rxSymbol, txSymbol, noise, L) ;

        DemodulateBPSK(rxData, rxSymbol, L) ;

        BER[i] = CalculateBER(txData, rxData, L) ;
    }

    PrintBER(SNR_STEP, SNRdB, BER) ;
    
    return 0 ;
}