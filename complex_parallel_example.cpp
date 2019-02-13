#include <complex>
#include <vector>
#include <algorithm>
#include <functional>
#include <iostream>
#include <omp.h>

typedef std::vector< std::complex<float> > TCmplxVec;

#pragma omp declare reduction( + : TCmplxVec : \
        std::transform(omp_in.begin( ),  omp_in.end( ), \
                       omp_out.begin( ), omp_out.begin( ), \
                       std::plus< std::complex<float> >( )) ) \
                       initializer (omp_priv(omp_orig))

int main(int argc, char *argv[]) {

    int size;

    if (argc < 2)
        size = 10;
    else
        size = atoi(argv[1]);

    TCmplxVec result(size,0);

    for(int i =0;i<size;i++){
        std::cout <<  i << ": " << result[i] << "\n";
    }

    #pragma omp parallel reduction( + : result )
    {
        int tid=omp_get_thread_num();

        for (int i=0; i<std::min(tid+1,size); i++) 
            result[i] += tid;
    }

    for (int i=0; i<size; i++) 
        std::cout << i << "\t" << result[i] << std::endl;

    return 0;
}