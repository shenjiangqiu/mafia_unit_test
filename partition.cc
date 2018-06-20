#include"partition.hpp"
#include<cassert>
#include<fstream>

#ifdef SJQ_DEBUG
#include<iostream>
template<typename T> std::ostream& operator<<(std::ostream& out,std::list<T> m_list){
    typename std::list<T>::iterator it=m_list.begin();
    int i=0;
    while(it!=m_list.end()){
        out<<"["<<i++<<"]"<<*it<<" --> ";
        it++;
    }
    out<<"END"<<std::endl;
    return out;
}
#endif

partition_unit::partition_unit(const partition_config& config):m_config(config),best_partition(2,8){
    for(int i=0;i<m_config.app_num;i++){
        counter.push_back(std::vector<unsigned long long>());
        for(int j=0;j<m_config.n_assoc;j++){
            counter[i].push_back(0);
        }
    }
    if(m_config.samplingWidth==0){
        abort();
        return;
    }
    for(int i=0;i<m_config.n_set/m_config.samplingWidth;i++){
        local_counter.push_back(std::vector<std::vector<unsigned long long> >());
        for(int j=0;j<m_config.app_num;j++){
            local_counter[i].push_back(std::vector<unsigned long long>());
            for(int k=0;k<m_config.n_assoc;k++){
                local_counter[i][j].push_back(0);
            }
        }
    }
    
    for(int i=0;i<(m_config.n_set/m_config.samplingWidth);i++){//every sample
    
        l2_sim_stack_array.push_back(std::vector<std::list<new_addr_type> >());
        for(int j=0;j<2;j++){//every app
            l2_sim_stack_array[i].push_back(std::list<new_addr_type>());
            for(int k=0;k<m_config.n_assoc;k++){//every block
                l2_sim_stack_array[i][j].push_back(-1);
            }
        }

    }
    for(int i=0;i<(m_config.n_set/m_config.samplingWidth);i++){//every sample
        sampleing_access.push_back(std::vector<unsigned long long>());
        for(int j=0;j<m_config.app_num;j++){
            sampleing_access[i].push_back(0);
        }
    }
    
    num_stack=m_config.n_set/m_config.samplingWidth;

}
partition_config::partition_config(){}



void partition_unit::access(unsigned core_id,unsigned set_idx,new_addr_type tagId){
    unsigned setId = set_idx;
    unsigned appId=core_id/15;//TODO
    total_access++;
    if(setId%m_config.samplingWidth==0){
        unsigned stackId=setId/m_config.samplingWidth;
        assert(stackId<num_stack);

        sampleing_access[stackId][appId]++;
        std::list<new_addr_type>::iterator it=l2_sim_stack_array[stackId][appId].begin();
        int numToAdd=0;
        for(int i=0;i<m_config.n_assoc;i++){
            assert(it!=l2_sim_stack_array[stackId][appId].end());
            if(numToAdd==0&&*it==-1){//miss and have ivalid entry fixbug 2018.6.20 need to place the newest on at first way
                l2_sim_stack_array[stackId][appId].erase(it);
                l2_sim_stack_array[stackId][appId].push_front(tagId);
                break;
            }
            if(numToAdd==0&&*it==tagId){//hit //fix bug 2018 06 20
                numToAdd=1;
                l2_sim_stack_array[stackId][appId].push_front(tagId);
                it--;
                std::list<new_addr_type>::iterator newit=it;
                it++;
                l2_sim_stack_array[stackId][appId].erase(it);
                it=newit;
                
            }

            if(numToAdd==1){//hit
                local_counter[stackId][appId][i]++;
                counter[appId][i]++;
            }
            it++;

        }
        if(numToAdd==0&&it==l2_sim_stack_array[stackId][appId].end()){//miss and need to evict
            l2_sim_stack_array[stackId][appId].push_front(tagId);
            --it;
            l2_sim_stack_array[stackId][appId].erase(it);
            
            

        }
        #ifdef SJQ_DEBUG
         std::cout<<l2_sim_stack_array[stackId][appId]<<std::endl;
        #endif
    }
}


void partition_unit::setBestPartition() {
    std::vector<int> retVal(2,0);

    int max=0;
    for(int i=0;i<m_config.n_assoc-1;i++){//i=0~14
        if(counter[0][i]+counter[1][m_config.n_assoc-i-2]>max){
            max=counter[0][i]+counter[1][m_config.n_assoc-i-2];
            retVal[0]=i+1;
            retVal[1]=m_config.n_assoc-i-1;

        }
    }
    if(max==0){
        retVal[0]=m_config.n_assoc/2;
        retVal[1]=retVal[0];
    }
    best_partition=retVal;

}
const std::vector<int>& partition_unit::getBestPartition() const{
    return best_partition;
}
void partition_unit::reSet(){
    switch(m_config.reSetPolicy){
        case 0:
        for(int i=0;i<m_config.app_num;i++){
            for(int j=0;j<m_config.n_assoc;j++){
                counter[i][j]/=2;
            }
        }
        for(int i=0;i<m_config.n_set/m_config.samplingWidth;i++){
        
            for(int j=0;j<m_config.app_num;j++){
            
                for(int k=0;k<m_config.n_assoc;k++){
                    local_counter[i][j][k]/=2;
                }
            }
        }
        break;
        case 1:
        for(int i=0;i<m_config.app_num;i++){
            for(int j=0;j<m_config.n_assoc;j++){
                counter[i][j]=0;
            }
        }
        for(int i=0;i<m_config.n_set/m_config.samplingWidth;i++){    
            for(int j=0;j<m_config.app_num;j++){
                for(int k=0;k<m_config.n_assoc;k++){
                    local_counter[i][j][k]=0;
                }
            }
        }
        break;
        
    }
    partition_stat.push_back(get_best_local());
    
}
void partition_unit::printStat(){
    std::ofstream out("partiton_stat.txt");
    if(!out.is_open()){
        abort();
    }
    for(int i=0;i<partition_stat.size();i++){
        for(int j=0;j<partition_stat[i].size();j++){
            out<<partition_stat[i][j]<<" ";
        }
        out<<std::endl;
    }
    out.close();
}
std::vector<unsigned> partition_unit::get_best_local(){
    std::vector<unsigned> best_local;
    int max=0;
    for(int i=0;i<m_config.n_set/m_config.samplingWidth;i++){
        best_local.push_back(8);
        for(int j=0;j<m_config.n_assoc-1;j++){//i=0~14
            if(local_counter[i][0][j]+local_counter[i][1][m_config.n_assoc-j-2]>max){
                max=local_counter[i][0][j]+local_counter[i][1][m_config.n_assoc-j-2];
                best_local[i]=j+1;
                

            }
        }
        if(max==0){
            best_local[i]=m_config.n_assoc/2;
        }
    }
    return best_local;
}