#include<gtest/gtest.h>

#include<iostream>

#include"partition.hpp"

class PartitionTest:public ::testing::Test{
    public:
    PartitionTest(){}
    protected:
    
    virtual void SetUp(){
        m_config=new partition_config;
        m_config->n_assoc=16;
        m_config->n_set=64;
        m_config->enable_partition_unit=true;
        m_config->app_num=2;
        m_config->activeCycles=50000;
        m_config->samplingWidth=8;
        m_config->reSetPolicy=0;

        m_unit=new partition_unit(*m_config);
    }
    partition_config *m_config;
    partition_unit *m_unit;

};
TEST_F(PartitionTest,AllTest){
    ASSERT_EQ(8,m_unit->l2_sim_stack_array.size());
    ASSERT_EQ(2,m_unit->l2_sim_stack_array[0].size());
    ASSERT_EQ(16,m_unit->l2_sim_stack_array[0][0].size());

    for(int i=0;i<100;i++){
        m_unit->access(0,0,i%20);
        //m_unit->access(15,0,i%16);//only the last can hit
        m_unit->access(15,0,i%15);//the last two can hit
        ASSERT_EQ(8,m_unit->l2_sim_stack_array.size());
        ASSERT_EQ(2,m_unit->l2_sim_stack_array[0].size());
        ASSERT_EQ(16,m_unit->l2_sim_stack_array[0][0].size());
        EXPECT_EQ(i%20,m_unit->l2_sim_stack_array[0][0].front());
        EXPECT_EQ(i%15,m_unit->l2_sim_stack_array[0][1].front());
    }
    auto it=m_unit->l2_sim_stack_array[0][0].begin();
    auto it2=m_unit->l2_sim_stack_array[0][1].begin();
    for(int i=0;i<16;i++){
        
        ASSERT_NE(it,m_unit->l2_sim_stack_array[0][0].end());
        EXPECT_EQ(19-i,*it);
        if(i!=15)
        EXPECT_EQ(((99%15)-i+15)%15,*it2);
        it++;
        it2++;

    }
    for(int i=0;i<16;i++){
    
        EXPECT_EQ(0,m_unit->counter[0][i]);
        if(i<14)
            EXPECT_EQ(0,m_unit->counter[1][i]);
        else{
            EXPECT_EQ(100-15,m_unit->counter[1][i]);
        }
    }
    m_unit->setBestPartition();
    auto bestPart=m_unit->getBestPartition();
    EXPECT_EQ(std::vector<int>({1,15}),bestPart);
    auto bestLocal=m_unit->get_best_local();
    EXPECT_EQ(std::vector<unsigned int>({1,8,8,8,8,8,8,8}),bestLocal);

}

int main(int argc, char  *argv[])
{
    ::testing::InitGoogleTest(&argc,argv);
    return RUN_ALL_TESTS();
   
}
