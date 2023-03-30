#include "ddsketch.h"
#include "level1.h"

// #define debug

typedef ddsketch::LogCollapsingLowestDenseDDSketch<50,16> DD16;
typedef ddsketch::LogCollapsingLowestDenseDDSketch<80,8> DD8;
typedef ddsketch::LogCollapsingLowestDenseDDSketch<97,4> DD4;

vector<pair<int, int>> to_vector(string a,double gama) {
        vector<pair<int, int>> res;
        int sz = a.size();
        int sta = 0;
        int idx=0,freq=0;
        for(int i=0;i<sz;i++){
            if(a[i]=='}') break;
            if(a[i]=='-'){
                while(a[i]!=','&&i<sz) {
                    i++;
                }
                continue;
            }
            if(!sta){
                if(a[i]>='0'&&a[i]<='9'){
                    idx = idx*10+a[i]-'0';
                }else if(a[i]==':'){
                    sta = 1;
                }
            }else if(sta){
                if(a[i]>='0'&&a[i]<='9'){
                    freq = freq*10+a[i]-'0';
                }else if(a[i]==','){
                    sta = 0;
                    int real_idx = (int)(2*pow(gama,idx)/(gama+1));
                    res.push_back(make_pair(real_idx,freq));
                    idx=0;freq=0;
                }
            }
        }
        return res;
    }

class elaSketch {
private:
    int len[4];
    int share_num[4];
    Tiny_Counter4 *a0;
    DD4 *a1; 
    DD8 *a2; 
    DD16 *a3; 
    BOBHash32 *hash[4][2];
    int hash_pos[4][2];

public:
    elaSketch(int memory_limit = 15000,string dataset="caida",int seed=0) {
        int memory_per_line = memory_limit*1000/10;

        len[0] = memory_per_line*1.50 / (5);
        len[1] = memory_per_line*3.00 / (4*1);
        len[2] = memory_per_line*5.00 / (8*2);
        len[3] = memory_per_line*0.50 / (16*4);

        share_num[0]=4;share_num[1]=1;share_num[2]=1;share_num[3]=1;
        for (int i = 0; i < 4; i++)
            cerr <<"ours bucket len[" << i << "] = " << len[i] << endl;
        a0 = new Tiny_Counter4[len[0]];
        a1 = new DD4[len[1]];
        a2 = new DD8[len[2]];
        a3 = new DD16[len[3]];
        
        for (int i = 0; i < 4; i++)
            for (int j=0;j<2;j++)
                hash[i][j] = new BOBHash32(seed*100+i*10+j);
    }

    ~elaSketch() {}

    void calc_hash(uint32_t flow_id) {
        for (int i = 0; i < 4; i++)
            for (int j=0;j<2;j++)
                hash_pos[i][j] = hash[i][j]->run(flow_id) % (len[i]*share_num[i]);
    }
    
    int query_level(int lev) {
        int cnt0=INF,cnt1=INF,cnt2=INF,cnt3=INF;
        for(int j=0;j<2;j++){
            if(lev==0) cnt0 = min(cnt0,a0[hash_pos[0][j]/share_num[0]].Toverflow(0,hash_pos[0][j]%share_num[0]) ? 
                                    INF : a0[hash_pos[0][j]/share_num[0]].true_twc(0,hash_pos[0][j]%share_num[0]));
            if(lev==1) cnt1 = min(cnt1,(int)a1[hash_pos[1][j]].num_values() >= (1<<8) ? INF : (int)a1[hash_pos[1][j]].num_values());
            if(lev==2) cnt2 = min(cnt2,(int)a2[hash_pos[2][j]].num_values() >= (1<<16) ? INF : (int)a2[hash_pos[2][j]].num_values());
            if(lev==3) cnt3 = min(cnt3,(int)a3[hash_pos[3][j]].num_values() >= (1<<32) ? INF : (int)a3[hash_pos[3][j]].num_values());
        }
        if(lev==0) return cnt0;
        if(lev==1) return cnt1;
        if(lev==2) return cnt2;
        if(lev==3) return cnt3;
    }
    int digest_level(bool debug1=0){
        int freq = query_level(0);

        if(freq<INF)
            return 0;
        freq = query_level(1);


        if(freq<INF)
            return 1;
        freq = query_level(2);
        if(freq<INF)
            return 2;
        return 3;
    }

    void insert(pair<uint32_t, int> packet, int num=1) {
        uint32_t flow_id = packet.first;
        int delay = packet.second;
        calc_hash(flow_id);
        int d_level = digest_level(0);
        
        if(d_level==0) for(int j=0;j<2;j++){
            a0[hash_pos[0][j]/share_num[0]].add_twc(num,0,hash_pos[0][j]%share_num[0]);
            a0[hash_pos[0][j]/share_num[0]].insert(delay,num=num,0);
        }
        if(d_level==1) for(int j=0;j<2;j++){
            a1[hash_pos[1][j]].add(delay,num);
        }
        if(d_level==2) for(int j=0;j<2;j++){
            a2[hash_pos[2][j]].add(delay,num);
        }
        if(d_level==3) for(int j=0;j<2;j++){
            a3[hash_pos[3][j]].add(delay,num);
        }
    }
    void PPrint(vector<pair<int, int>> tp){
        // int sz = tp.size();
        // for(int i=0;i<sz;i++){
        //     err<<tp[i].first<<"***"<<tp[i].second<<endl;
        // }
    }

    vector<pair<int, int>> merge_digest(vector<pair<int, int>> _b0, vector<pair<int, int>> _b1,int cnt0,int cnt1){
        vector<pair<int, int>> res;
        int sz0 = _b0.size(),sz1 = _b1.size(); 
        int i0=0,i1=0;
        while (i0<sz0&&i1<sz1)
        {
            if(_b0[i0].first==_b1[i1].first){
                res.push_back(make_pair(_b0[i0].first, min(_b0[i0].second,_b1[i1].second) ));
                i0++;i1++;
            }else if(_b0[i0].first<_b1[i1].first){
                // res.push_back(make_pair(_b0[i0].first, _b0[i0].second ));
                i0++;
            }else if(_b0[i0].first>_b1[i1].first){
                // res.push_back(make_pair(_b1[i1].first, _b1[i1].second ));
                i1++;
            }
        }
        while(i0<sz0){
            res.push_back(make_pair(_b0[i0].first, _b0[i0].second ));
            i0++;
        }
        while(i1<sz1){
            res.push_back(make_pair(_b1[i1].first, _b1[i1].second ));
            i1++;
        }
        return res;
    }
    vector<pair<int, int>> merge(uint32_t flow_id,int lev){
        calc_hash(flow_id);
        vector<pair<int, int>> res;
        if(lev==0){
            vector<pair<int, int>> _b0 = a0[hash_pos[0][0]/share_num[0]].to_vector();
            vector<pair<int, int>> _b1 = a0[hash_pos[0][1]/share_num[0]].to_vector();
            res.push_back( make_pair( min(_b0[0].first,_b1[0].first) ,-1) );
        }else if(lev==1){
            double gama = (1+0.97)/(1-0.97);
            int cnt0 = (int)a1[hash_pos[1][0]].num_values() >= (1<<8) ? INF : (int)a1[hash_pos[1][0]].num_values();
            int cnt1 = (int)a1[hash_pos[1][1]].num_values() >= (1<<8) ? INF : (int)a1[hash_pos[1][1]].num_values();
            
            if(((cnt0==INF)) && (!(cnt1==INF))){
                vector<pair<int, int>> _b = to_vector(a1[hash_pos[1][1]].store_.to_string(),gama);
#ifdef debug
                PPrint(_b);
#endif
                res = _b;
            }
            else if((!(cnt0==INF)) && ((cnt1==INF))){
                vector<pair<int, int>> _b = to_vector(a1[hash_pos[1][0]].store_.to_string(),gama);
#ifdef debug
                PPrint(_b);
#endif
                res = _b;
            }else{
                vector<pair<int, int>> _b0 = to_vector(a1[hash_pos[1][0]].store_.to_string(),gama);
                vector<pair<int, int>> _b1 = to_vector(a1[hash_pos[1][1]].store_.to_string(),gama);
#ifdef debug
                PPrint(_b0 );
                err<<"*------------------*"<<endl;
                PPrint(_b1);
#endif
                res = merge_digest(_b0,_b1,cnt0,cnt1);
            }
            
        }else if(lev==2){
            double gama = (1+0.8)/(1-0.8);
            int cnt0 = (int)a2[hash_pos[2][0]].num_values() >= (1<<16) ? INF : (int)a2[hash_pos[2][0]].num_values();
            int cnt1 = (int)a2[hash_pos[2][1]].num_values() >= (1<<16) ? INF : (int)a2[hash_pos[2][1]].num_values();

            vector<pair<int, int>> _b0 = to_vector(a2[hash_pos[2][0]].store_.to_string(),gama);
            vector<pair<int, int>> _b1 = to_vector(a2[hash_pos[2][1]].store_.to_string(),gama);
#ifdef debug
            PPrint(_b0 );
            err<<"*------------------*"<<endl;
            PPrint(_b1);
#endif
            res = merge_digest(_b0,_b1,cnt0,cnt1);
            // PPrint(res);
        }else if(lev==3){
            double gama = (1+0.5)/(1-0.5);
            int cnt0 = (int)a3[hash_pos[3][0]].num_values() >= (1<<32) ? INF : (int)a3[hash_pos[3][0]].num_values();
            int cnt1 = (int)a3[hash_pos[3][1]].num_values() >= (1<<32) ? INF : (int)a3[hash_pos[3][1]].num_values();

            vector<pair<int, int>> _b0 = to_vector(a3[hash_pos[3][0]].store_.to_string(),gama);
            vector<pair<int, int>> _b1 = to_vector(a3[hash_pos[3][1]].store_.to_string(),gama);
#ifdef debug
            PPrint(_b0 );
            err<<"*------------------*"<<endl;
            PPrint(_b1);
#endif
            res = merge_digest(_b0,_b1,cnt0,cnt1);
            // PPrint(res);
        }
        return res;
    }
    vector<pair<int, int>> query(uint32_t flow_id) {
        calc_hash(flow_id);
        int d_level = digest_level(0);
        // err<<"lev: "<<d_level<<endl;
        vector<pair<int, int>> _a;
        _a = merge(flow_id, d_level);
        
        return _a;
    }


};