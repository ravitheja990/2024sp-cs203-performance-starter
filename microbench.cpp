#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include <unordered_set>
#include <algorithm>
#include <cstdint>
#include <vector>
#include <map>
#include <stdlib.h>
#ifdef CUDA
extern "C" {
#include <cuda.h>
uint64_t *baseline_double_cuda(uint64_t * _array, unsigned long int size);
uint64_t *baseline_int_cuda(uint64_t * _array, unsigned long int size);
}
#endif
#include "perfstats.h"
//#include "archlab_impl.hpp"

double start, stop;

#define CLINK extern "C"
#define OPT(a) __attribute__(a)

extern "C" uint64_t *__attribute__ ((optimize(0))) baseline_int(uint64_t * array, unsigned long int size) {
    //uint64_t * array = new uint64_t[size];
    for(uint i = 0; i < size; i++) {
        array[i] = 0;
    }

    for (uint j = 0; j < 3; j++) {
        for(uint i= 1 ; i < size; i++) {
            array[i] += i/(1+j)+array[i - 1];
        }
    }
//    std::cout << "Execution baseline_int complete :"<< size << "\n" ;
    return array;
}

extern "C" uint64_t *__attribute__ ((optimize(4))) baseline_int_O4 (uint64_t * array, unsigned long int size) {
    //uint64_t * array = new uint64_t[size];
    for(uint i = 0; i < size; i++) {
        array[i] = 0;
    }

    for (uint j = 0; j < 3; j++) {
        for(uint i= 1 ; i < size; i++) {
            array[i] += i/(1+j)+array[i - 1];
        }

    }
    return array;
}

extern "C" uint64_t *baseline_double(uint64_t * _array, unsigned long int size) {
    //double * array = new double[size];
    double * array = (double*)_array;
    for(uint i = 0; i < size; i++) {
        array[i] = 0;
    }

    for (uint j = 0; j < 3; j++) {
        for(uint i= 1 ; i < size; i++) {
            array[i] += i/(1+j)+array[i - 1];
        }
    }
    return (uint64_t*)array;
}

extern "C" uint64_t *baseline_float(uint64_t * _array, unsigned long int size) {
    //double * array = new double[size];
    float * array = (float*)_array;
    for(uint i = 0; i < size; i++) {
        array[i] = 0;
    }

    for (double j = 0; j < 3; j++) {
        for(uint i= 1 ; i < size; i++) {
            array[i] += i/(1+j)+array[i - 1];
        }
    }
    return (uint64_t*)array;
}

extern "C" uint64_t *baseline_char(uint64_t * _array, unsigned long int size) {
    //double * array = new double[size];
    char * array = (char*)_array;
    for(uint i = 0; i < size; i++) {
        array[i] = 0;
    }

    for (double j = 0; j < 3; j++) {
        for(uint i= 1 ; i < size; i++) {
            array[i] += i/(1+j)+array[i - 1];
        }
    }
    return (uint64_t*)array;
}

extern "C" uint64_t *__attribute__ ((optimize(4),noinline)) baseline_double_O4(uint64_t * _array, unsigned long int size) {
    //double * array = new double[size];
    double * array = (double*)_array;
    for(uint i = 0; i < size; i++) {
        array[i] = 0;
    }

    for (double j = 0; j < 3; j++) {
        for(uint i= 1 ; i < size; i++) {
            array[i] += i/(1+j)+array[i - 1];
        }
    }
    return (uint64_t*)array;
}


volatile int ROW_SIZE = 1024;
extern "C" uint64_t *__attribute__ ((optimize(4))) matrix_column_major(uint64_t * _array, unsigned long int size) {
#define ROW_SIZE 1024
    double * array = (double*)_array;

    for (int k = 0; k < ROW_SIZE; k++) {
        for(uint i= 0 ; i < size/ROW_SIZE; i++) {
            array[i*ROW_SIZE + k] *=array[i*ROW_SIZE + k]; // This Line
        }
    }
//    std::cout << "Execution matrix_column_major complete\n";
    
    return (uint64_t*)array;
}


//int ROW_SIZE = 1024;
extern "C" uint64_t *__attribute__ ((optimize(4))) matrix_row_major(uint64_t * _array, unsigned long int size) {

    double * array = (double*)_array;

    for(uint i= 0; i < size/ROW_SIZE; i++) {
        for (int k = 0; k < ROW_SIZE; k++) {
            array[i*ROW_SIZE + k] *= array[i*ROW_SIZE + k]; // This Line
        }
    }
    
    return (uint64_t*)array;
}

extern "C" uint64_t *__attribute__ ((optimize(0))) everything(uint64_t * array, unsigned long int size) {
    matrix_column_major(array, size);
    baseline_int(array,size);
    return array;
}


#ifdef CUDA
extern "C" uint64_t *__attribute__ ((optimize(0))) everything_opt(uint64_t * array, unsigned long int size) {
    matrix_column_major(array, size);
    baseline_int_cuda(array,size);
    return array;
}

extern "C" uint64_t *__attribute__ ((optimize(0))) option_1(uint64_t * array, unsigned long int size) {
    matrix_column_major(array, size);
    baseline_int_cuda(array,size);
    return array;
}

extern "C" uint64_t *__attribute__ ((optimize(0))) option_2(uint64_t * array, unsigned long int size) {

    matrix_row_major(array, size);
    baseline_int(array,size);
    return array;
}
#else
extern "C" uint64_t *__attribute__ ((optimize(0))) everything_opt(uint64_t * array, unsigned long int size) {
    matrix_column_major(array, size);
    baseline_int_O4(array,size);
    return array;
}
extern "C" uint64_t *__attribute__ ((optimize(0))) option_1(uint64_t * array, unsigned long int size) {
    matrix_column_major(array, size);
    baseline_int_O4(array,size);
    return array;
}

extern "C" uint64_t *__attribute__ ((optimize(0))) option_2(uint64_t * array, unsigned long int size) {

    matrix_row_major(array, size);
    baseline_int(array,size);
    return array;
}
#endif
uint64_t * (*power_virus)(uint64_t * array, unsigned long int size) = NULL;

// There's a lot of really unsafe multithreading going on here.  Don't ever write code like this...It will fail in the real world --Professor Swanson
//uint64_t * power_virus_array = NULL;
uint array_size;
unsigned long int power_virus_size = 0;
unsigned int power_virus_reps = 0;
int running_viruses = 0;
void *go_power_virus(void*arg) {
    uint64_t delay = (uint64_t)arg;

    uint64_t * power_virus_array = NULL;
    power_virus_array = new uint64_t[array_size]; // and 256 because we matrix_* needs a little extra space
    for(uint i = 0; i < array_size; i++) {
        power_virus_array[i] = 0;
    }
    sleep(delay);
    running_viruses++;
    for(uint i = 0; i < power_virus_reps; i++) {
        power_virus(power_virus_array, power_virus_size);
    }
    return NULL;
}

int main(int argc, char *argv[])
{
    int i;
    int reps=1,freq;
    unsigned long int size=1024;
    char *stat_file = NULL;
    char default_filename[] = "stat.csv";
    char preamble[1024];
    char epilogue[1024];
    std::vector<std::string> functions;
    std::vector<std::string> default_functions;
    std::vector<unsigned long int> sizes;
    std::vector<unsigned long int> default_sizes;
    std::vector<int> frequencies;
    std::vector<int> default_frequencies;
    for(i = 1; i < argc; i++)
    {
        // This is an option.
        if(argv[i][0]=='-')
        {
            switch(argv[i][1])
            {
                case 'o':
                    if(i+1 < argc && argv[i+1][0]!='-')
                                stat_file = argv[i+1];
                    break;
                case 'r':
                    if(i+1 < argc && argv[i+1][0]!='-')
                                reps = atoi(argv[i+1]);
                    break;
            case 's':
                for(;i+1<argc;i++)
                {
                    if(argv[i+1][0]!='-')
                    {
                        size = atoi(argv[i+1]);
                            sizes.push_back(size);
                }
                else
                    break;
                }
                break;
            case 'M':
                for(;i+1<argc;i++)
                {
                    if(argv[i+1][0]!='-')
                    {
                        freq = atoi(argv[i+1]);
                            frequencies.push_back(freq);
                }
                else
                    break;
                }
                break;
            case 'f':
                for(;i+1<argc;i++)
                {
                    if(argv[i+1][0]!='-')
                    {
                                functions.push_back(std::string(argv[i+1]));
                }
                else
                    break;
                }
                break;
            case 'h':
                break;
            }
        }
    }
    default_functions.push_back("baseline_int");
    default_frequencies.push_back(3300);
    default_sizes.push_back(1024*1024);
    if(sizes.size()==0)
        sizes = default_sizes;
    if(functions.size()==0)
        functions = default_functions;
    if(frequencies.size()==0)
        frequencies = default_frequencies;
    if(stat_file==NULL)
        stat_file = default_filename;
    std::map<const std::string, uint64_t*(*)(uint64_t *, unsigned long int)>
    function_map =
    {
#define FUNCTION(n) {#n, n}
        FUNCTION(baseline_int),
        FUNCTION(baseline_int_O4),
        FUNCTION(baseline_double),
        FUNCTION(baseline_float),
        FUNCTION(baseline_char),
        FUNCTION(baseline_double_O4),
#ifdef CUDA
        FUNCTION(baseline_double_cuda),
        FUNCTION(baseline_int_cuda),
#endif
        FUNCTION(matrix_row_major),
        FUNCTION(matrix_column_major),
        FUNCTION(everything),
        FUNCTION(option_1),
        FUNCTION(option_2),
        FUNCTION(everything_opt)
    };
    array_size  = *std::max_element(sizes.begin(), sizes.end())+256;
    uint64_t * array = new uint64_t[array_size]; // and 256 because we matrix_* needs a little extra space
    char header[]="size,rep,function,IC,Cycles,CPI,MHz,CT,ET,cmdlineMHz";

    perfstats_print_header(stat_file, header);
    for(uint i = 0; i < array_size; i++) {
        array[i] = 0;
    }
    for(auto & freq: frequencies ) {
        change_cpufrequnecy(freq);
        for(auto & size: sizes ) {
            for(uint r = 0; r < reps; r++) {
                for(auto & function : functions) {
                        flush_caches();
                        sprintf(preamble, "%lu,%d,%s,", size, r, function.c_str());
                        perfstats_init();
                        perfstats_enable();
//                        unsigned long int * t =
                        function_map[function](array, size);                    
                        perfstats_disable();
                        sprintf(epilogue, ",%d\n",freq);
                        perfstats_print(preamble, stat_file, epilogue);
                        perfstats_deinit();
                }                                
            }
        }
        restore_cpufrequnecy();
    }
    std::cout << "Execution complete\n" ;
    return 0;
}
