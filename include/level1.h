// #include "trace.h"
#include "BOBHash32.h"
#include <bits/stdc++.h>
// #include "CU_Sketch.h"
#include <typeinfo>
#define COUNTER_PER_BUCKET 8
using namespace std;
const int INF = 0x3f3f3f3f;
const int SIZE_THRETHOLD = 4000;

// ofstream err("./res/log.txt");






template<class T, int N, uint32_t CNT_MAX>
class Tiny_Counter {
public:
    T total_cnt,tower_cnt;
    int max_delay;

    Tiny_Counter() {
        clear();
    }

    void clear() {
        total_cnt = 0;
        tower_cnt = 0;
        max_delay = 0;
    }
    bool overflow() {
        return total_cnt == CNT_MAX;
    }
    T true_twc(int lev=-1,int idx=-1){
        T twc=tower_cnt;
        // cout<<"lala"<<hex<<(int)tower_cnt<<endl;
        if(lev==-1||lev==2||lev==3){
            twc=tower_cnt;
        }else if(lev==1){
            twc=(tower_cnt<<(4*idx));
            twc=((twc)>>(4*(1)));
        }else if(lev==0){
            twc=(tower_cnt<<(2*idx));
            twc=((twc)>>(2*(3)));
        }
        return twc;
    }
    bool Toverflow(int lev=-1,int idx=-1){
        T twc =true_twc(lev,idx);
        // cout<<"*"<<(int)twc<<"*";
        if(lev==-1||lev==2||lev==3){
            if(CNT_MAX==twc)
                return 1;
        }else if(lev==1){
            if((CNT_MAX>>4)==twc)
                return 1;
        }else if(lev==0){
            if((CNT_MAX>>6)==twc)
                return 1;
        }
        return 0;
    }
    void add_twc(int num,int lev=-1,int idx=-1){
        T twc = true_twc(lev,idx);
        T num0;
            
        if(lev==0){
            if((CNT_MAX>>6)-twc<num)
                num0 = (CNT_MAX>>6)-twc;
            else
                num0 = num;

            tower_cnt+=(num0<<(2*(3-idx)));
        }
    }
    
    void insert(int delay,int num=1,int lev=-1) {
        if (overflow())
            return;  
        if(CNT_MAX-total_cnt<num)
            num = CNT_MAX-total_cnt;
    
        total_cnt+=num;
        max_delay = max(max_delay,delay);
        return;
    }
    vector<pair<int, int>> to_vector() {
        vector<pair<int, int>> res;
        
        res.push_back(make_pair( max_delay,-1 ));
        return res;
    }
};

typedef Tiny_Counter<uint8_t, 1, 0xff> Tiny_Counter4;  // 4bit


