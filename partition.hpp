#ifndef PARTITION_HPP
#define PARTITION_HPP
#include<gtest/gtest.h>
//code here

/*author sjq
**email ivy22233qiu@live.com
**create time:2018-06-10
**introduction:algorithm for l2 dynamic partition
*/
//#include "../option_parser.h"
#include <vector>
#include <list>
//#include "gpu-sim.h"
//#include "gpu-cache.h"
typedef unsigned long long new_addr_type;
typedef unsigned long long cycle_size;
struct partition_config
{
    partition_config();
    partition_config(int assoc,int set){
        n_assoc=assoc;
        n_set=set;
    }
    void init(int assoc,int set){
        n_assoc=assoc;
        n_set=set;
        
    }
    //void reg_options( OptionParser * opp);

    //members
    int n_assoc;
    int n_set;
    bool enable_partition_unit;
    int app_num;
    cycle_size activeCycles;
    int samplingWidth;

    int reSetPolicy;//0=devide by 2,1=set to 0

};
class partition_unit{
    friend class PartitionTest;
    friend class testing::Test;
    friend class Test;
    public: 
    partition_unit(const partition_config&);
    void access(unsigned core_id,unsigned set_idx,new_addr_type tagId);
    const std::vector<int>& getBestPartition() const;
    void setBestPartition();
    void reSet();
    void printStat();
    std::vector<int> currPartition;
    const partition_config& get_config() const{
        return m_config;
    }
    std::vector<unsigned> get_best_local();
    public:
    unsigned long long total_access;
    std::vector<std::vector<unsigned long long > > sampleing_access;
    std::vector<int> best_partition;
    unsigned num_stack;
    std::vector<std::vector<std::list<new_addr_type> > > l2_sim_stack_array;//8*2*16
    std::vector<std::vector<unsigned long long> > counter;//overall counter2*16
    std::vector<std::vector<std::vector<unsigned long long> > > local_counter;//local counter8*2*16
    const partition_config& m_config;
    
    std::vector<std::vector<unsigned > >partition_stat;
};


#endif