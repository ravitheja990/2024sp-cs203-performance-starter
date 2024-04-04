#include <iostream>
#include <cstdlib>
#include <sstream>
#include "archlab.h"
#include <unistd.h>
#include<algorithm>
#include<cstdint>
#include<unordered_set>
#include <cstring>
#include <vector>
#include <map>
#include "perfstats.h"
#include "fast_URBG.hpp"
#include <random>
#include "join_count.h"

int main(int argc, char *argv[])
{
    
    std::vector<int> mhz_s;
    std::vector<int> default_mhz;
//    fastest << cpu_frequencies_array[0];
    std::vector<std::string> functions;
    std::vector<std::string> default_functions;
    std::vector<uint64_t> left_table_sizes;
    std::vector<uint64_t> default_left_table_sizes;
    std::vector<uint64_t> right_table_size;
    std::vector<uint64_t> default_right_table_size;
    std::stringstream available_functions;
    int i;
    int reps=1,mhz;

    char *stat_file = NULL;
    char default_filename[] = "stat.csv";
    char preamble[1024];
    char epilogue[1024];

 // Parse user commands.    
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
                             unsigned long int size;
                             size = atoi(argv[i+1]);
                             left_table_sizes.push_back(size);
                     }
                     else
                         break;
                     }
                     break;
                 case 'q':
                     for(;i+1<argc;i++)
                     {
                         if(argv[i+1][0]!='-')
                         {
                             unsigned long int size;
                             size = atoi(argv[i+1]);
                             right_table_size.push_back(size);
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
                 case 'M':
                     for(;i+1<argc;i++)
                     {
                         if(argv[i+1][0]!='-')
                         {
                             mhz = atoi(argv[i+1]);
                             mhz_s.push_back(mhz);
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
    if(stat_file==NULL)
        stat_file = default_filename;


    default_functions.push_back("join_count");
    default_left_table_sizes.push_back(128*1024);
    default_right_table_size.push_back(64*1024);
    if(functions.size()==0)
        functions = default_functions;
    if(left_table_sizes.size()==0)
        left_table_sizes = default_left_table_sizes;
    if(right_table_size.size()==0)
        right_table_size = default_right_table_size;
    std::uniform_int_distribution<int> dist(1024, 1024*1024);
    std::random_device rd;
    uint64_t seed =dist(rd);
    uint64_t _seed = seed;
    uint64_t max_left_table_size = *std::max_element(left_table_sizes.begin(), left_table_sizes.end());
    uint64_t * left_table = new uint64_t[max_left_table_size];
    for(uint64_t i = 0; i < max_left_table_size; i++) {
        left_table[i] = fast_rand(&_seed)% (max_left_table_size*2);
    }
    std::shuffle(left_table, &left_table[max_left_table_size], fast_URBG(seed));
    std::map<const std::string, uint64_t(*)(uint64_t *, uint32_t, uint64_t*, uint32_t)>
        function_map =
        {
#define FUNCTION(n) {#n, n}
            FUNCTION(join_count_solution),
            FUNCTION(join_count)
        };
    // for(uint64_t a = 0; a < max_left_table_size; a++) {
    //     std::cerr << left_table[a] << "\n";
    // }
    uint64_t max_right_table_size = *std::max_element(right_table_size.begin(), right_table_size.end());
    uint64_t * right_table = new uint64_t[max_right_table_size];

    for(uint i = 0; i < max_right_table_size; i++) {
        right_table[i] = fast_rand(&_seed)% (max_left_table_size);
    }
    char perfstats_header[]="size,rep,function,right_table_size,IC,Cycles,CPI,MHz,CT,ET,cmdlineMHz,answer";
    perfstats_print_header(stat_file, perfstats_header);

    for(auto &mhz: mhz_s) 
    {
        change_cpufrequnecy(mhz);
//        set_cpu_clock_frequency(mhz);
        for(auto & left_table_size: left_table_sizes ) {            
            std::shuffle(left_table, &left_table[left_table_size], fast_URBG(seed));
            for(auto & right_table_size: right_table_size ) {
                for(uint r = 0; r < reps; r++) {
                    for(auto & function : functions) {
                        //pristine_machine();                    
                        uint64_t answer=0;
                        {                                
                            sprintf(preamble, "%lu,%d,%s,%lu,", left_table_size, r, function.c_str(),right_table_size);
                            perfstats_init();
                            perfstats_enable();
                            //enable_prefetcher();
                            //set_cpu_clock_frequency(cpu_frequencies[0]);
                            answer += function_map[function](left_table, left_table_size, right_table, right_table_size);
                            perfstats_disable();
                            sprintf(epilogue, ",%d,%lu\n",mhz,answer);
                            perfstats_print(preamble, stat_file, epilogue);
                            perfstats_deinit();
                        }

                        std::cerr << answer << "\n";
                    }
                }
            }
        }
        restore_cpufrequnecy();

    }
    delete [] left_table;
    delete [] right_table;
    std::cout << "Execution complete\n" ;
//    archlab_write_stats();
    return 0;
}
