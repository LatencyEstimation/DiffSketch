#include "cmp.h"
#include <bits/stdc++.h>
#include <chrono>
using namespace chrono;
// #define debug
#ifdef debug
    ofstream lm_cmp_cout("./res/lm_cmp.csv");
    ofstream lm_ela_cout("./res/lm_ela.csv");
#endif
// ofstream err1("./res/lo1.txt");
const int readnum = -1;
DistriSketch* ds;
unordered_map<uint32_t,perfect_DD> pDD;
elaSketch* es;

// ElasticSketch ds;
// DistributionSketch ds;
vector<pair<uint32_t, int>> input;
vector<pair<int, uint32_t>> gt;  // 包个数，流id
double lm_sum_cmp_MAE=0,lm_sum_ela_MAE=0,lm_sum_cmp_MRE=0,lm_sum_ela_MRE=0,tiny_sum_cmp=0,tiny_sum_ela=0;
long long lm_num_cmp=0,lm_num_ela=0,tiny_num_cmp=0,tiny_num_ela=0;
double lm_sum_cmp_rank_error=0, lm_sum_ela_rank_error=0;
double lm_sum_cmp_quantile_error=0, lm_sum_ela_quantile_error=0;
struct Info {
    int last_time;
    multiset<int> delays;
};
void Print(int rank,int num,vector<pair<int, int>> res){
    // string est_filename = "./result/top-" + to_string(rank) + "_" + to_string(num) + "_est_8000.csv";
    // ofstream fout(est_filename);
    // fout << fixed << setprecision(6);
    // fout << "interval,pkts,prob," << endl;
    // int sum = 0, tot = 0;
    // for (auto [avg, cnt]: res) {
    //     tot += cnt;
    // }
    // err << "est tot=" << tot << endl;
    // for (auto [avg, cnt]: res) {
        
    //     fout << avg << ',' << cnt << ',' << (double)cnt << ',' << endl;
    // }
    // fout.close();
}
void pprint(vector<pair<double,double>> a){
    // int sz = a.size();
    // for (int i=0;i<sz;i++){
    //     err1<<a[i].first<<"*"<<a[i].second<<" ";
    // }
    // err1<<endl;
}
void pprint(vector<pair<int,int>> a){
    // int sz = a.size();
    // for (int i=0;i<sz;i++){
    //     err1<<a[i].first<<"*"<<a[i].second<<" ";
    // }
    // err1<<endl;
}
unordered_map<uint32_t, Info> mp;
multiset<double> err_tf_cmp;
multiset<double> err_tf_ela;
double insert_cmp=0,insert_ela=0;
int insert_cmp_throu=0,insert_ela_throu=0;
double query_cmp=0,query_ela=0;
int query_cmp_throu=0,query_ela_throu=0;

double sample_num = 2000;
int mx=0;
bool flag[20]={};
void insert(uint32_t flow_id, int arrive_time,int test) {

    if (mp.find(flow_id) == mp.end()) {
        mp[flow_id].last_time = arrive_time;
        return;
    }
    int delay = arrive_time - mp[flow_id].last_time;
    mp[flow_id].last_time = arrive_time;
    mx = max(mx, delay);
    mp[flow_id].delays.insert(delay);
    pDD[flow_id].add(delay);

    if(test==2){
        auto start = std::chrono::high_resolution_clock::now();
        ds->insert(make_pair(flow_id, delay));
        auto end = std::chrono::high_resolution_clock::now();
        auto insert_duration = duration_cast<nanoseconds>(end - start);
        insert_cmp+=double(insert_duration.count())/1e9;

        start = std::chrono::high_resolution_clock::now();
        es->insert(make_pair(flow_id, delay));
        end = std::chrono::high_resolution_clock::now();
        insert_duration = duration_cast<nanoseconds>(end - start);
        insert_ela+=double(insert_duration.count())/1e9;

    }else{
        ds->insert(make_pair(flow_id, delay));
        es->insert(make_pair(flow_id, delay));
    }
}
vector<pair<double, double>> get_real(perfect_DD& dd){
    vector<pair<double, double>> ans;
    double p = 0.01;
    for(double i=0.01;i<1.00;i+=p)
        ans.push_back( make_pair(dd.get_quantile_value(i), i) );
    ans.push_back( make_pair(dd.get_quantile_value(0.9999), 0.9999) );
    return ans;
}

void _add(double* range_sum,double lb,double hb,double weight, vector<double>& po){
        int sz = po.size();
        for(int i=0;i<sz-1;i++){
            if(lb>=po[i]&&lb<po[i+1]&&hb>po[i+1]){
                range_sum[i]+=(( (double)(po[i+1]-lb)/(double)(hb-lb)*(double)weight ) );
            }else if(hb>po[i]&&hb<=po[i+1]&&lb<po[i]){
                range_sum[i]+=(( (double)(hb-po[i])/(double)(hb-lb)*(double)weight ) );
            }else if(lb>=po[i]&&hb<=po[i+1]){
                range_sum[i]+=(weight );
            }else if(lb<po[i]&&hb>po[i+1]){
                range_sum[i]+=(( (double)(po[i+1]-po[i])/(double)(hb-lb)*(double)weight ) );
            }
        }
    }
void _add(double* range_sum,int lb,int hb,double weight, vector<int>& po){
        int sz = po.size();
        for(int i=0;i<sz-1;i++){
            if(lb>=po[i]&&lb<po[i+1]&&hb>po[i+1]){
                range_sum[i]+=(( (double)(po[i+1]-lb)/(double)(hb-lb)*(double)weight ) );
            }else if(hb>po[i]&&hb<=po[i+1]&&lb<po[i]){
                range_sum[i]+=(( (double)(hb-po[i])/(double)(hb-lb)*(double)weight ) );
            }else if(lb>=po[i]&&hb<=po[i+1]){
                range_sum[i]+=(weight );
            }else if(lb<po[i]&&hb>po[i+1]){
                range_sum[i]+=(( (double)(po[i+1]-po[i])/(double)(hb-lb)*(double)weight ) );
            }
        }
    }

double RE(double real,double est){
    if(fabs(real)<1e-5){
        if(fabs(est)<1e-5) return 0;
        else return 1;
    }else{
        return fabs(real-est)/real;
    }
}

double query_rank(int rank,int test) {
    if (rank < 0)
        rank = gt.size() + rank;
    uint32_t flow_id;
    if(rank<gt.size()){
        flow_id = gt[rank].second;
        // err << "top-" << rank << ", " << "flow_id = " << flow_id << endl;
        // err1 << "top-" << rank << ", " << "flow_id = " << flow_id << endl;
        int tot = mp[flow_id].delays.size();
        // err << "real tot=" << tot << endl;
        // err1 << "real tot=" << tot << endl;
    }
    else{
        // err<<"gt_size:"<<gt.size()<<" rank:"<<rank<<endl;
        exit(0);
    }
    double gama = 1.1/0.9;
    //////////////////////////get res for three algos start////////////////////////////////////
    vector<pair<int, int>> real = to_vector(pDD[flow_id].store_.to_string(),gama);
    vector<pair<int, int>> res;
    vector<pair<int, int>> res_es;
    if(test==3){
        auto start = std::chrono::high_resolution_clock::now();
        res = ds->query(flow_id);
    
        auto end = std::chrono::high_resolution_clock::now();
        auto query_duration = duration_cast<nanoseconds>(end - start);
        query_cmp+=double(query_duration.count())/1e9;
        // err1<<"query_cmp: "<<double(query_duration.count())/1e9<<" "<<query_cmp<<endl;

        start = std::chrono::high_resolution_clock::now();
    
        res_es = es->query(flow_id);
    
        end = std::chrono::high_resolution_clock::now();
        query_duration = duration_cast<nanoseconds>(end - start);
        query_ela+=double(query_duration.count())/1e9;
        // err1<<"query_ela: "<<double(query_duration.count())/1e9<<" "<<query_ela<<endl;

    
    }else{
        res = ds->query(flow_id);
        res_es = es->query(flow_id);
    }

    if(real.empty()||res.empty()||res_es.empty()) {
        // err<<"empty result"<<endl;
        return 0;
    }
    //////////////////////////get res for three algos end////////////////////////////////////

    int sum = 0, tot = 0;

if(test==4||test==5){
    double real_total = (double)mp[flow_id].delays.size();

    for (auto [avg, cnt]: res) {
        tot += cnt;
    }

    double cmp_total = (double)tot;
    if(real_total<3){
        return 0;
    }
    if(test==4&&real_total<256)
        return 0;
    if(test==5&&real_total>=256)
        return 0;

    /////////////////////////////////process res_cmp start//////////////////////////////
    vector<pair<int, int>> points;
    points.push_back(make_pair(0, 0));
    double percentage = 0.75;

    double cmp90 = 0;
    bool flag_cmp=0;

    for (auto [avg, cnt]: res) {
        if(cnt==0) continue;
        sum+=cnt;
        double q = (double)sum/cmp_total;
        points.push_back(make_pair(avg,cnt));
        if(q>=percentage&&(!flag_cmp)) {
            cmp90=avg;
            flag_cmp=1;
        }
    }
    points.push_back(make_pair(1000000000, 1));
#ifdef debug
    // err1<<"cmp_mod:";
    // pprint(points);
#endif

    /////////////////////////////////process res_cmp end//////////////////////////////

    /////////////////////////////////process res_elas start//////////////////////////////

    sum = 0;
    tot = 0;
    for (auto [avg, cnt]: res_es) {
        tot += cnt;
    }
    // err << "est tot=" << tot << endl;
    double ela_total = (double)tot;
    vector<pair<double, double>> points_es;
    points_es.push_back(make_pair(0, 0));

    
    double ela90 = 0;
    bool flag_ela=0;
    for (auto [avg, cnt]: res_es) {
        if(cnt==0) continue;
        sum += cnt;
        double q = (double)sum / tot;
        points_es.push_back(make_pair(avg, cnt));

        if(q>=percentage&&(!flag_ela)) {
            ela90=avg;
            flag_ela=1;
        }

    }
    points_es.push_back(make_pair(1000000000, 1));
#ifdef debug
    // err1<<"elas:";
    // pprint(points_es);
#endif
    /////////////////////////////////process res_elas end//////////////////////////////


    /////////////////////////////////process res_real start//////////////////////////////
    double rank_error_cmp, rank_error_ela;
    double quantile_error_cmp, quantile_error_ela;
    vector<pair<int, int>> points_real;
    vector<int> po; po.push_back(0);
    points_real.push_back(make_pair(0, 0));
    sum = 0;flag_cmp=0;flag_ela=0; bool flag_quantile=0;
    for (auto [avg, cnt]: real) {
        if(cnt==0) continue;
        sum+=cnt;
        double q = (double)sum/real_total;
        points_real.push_back(make_pair(avg,cnt));
        po.push_back(avg);
        if(avg>=cmp90&&(!flag_cmp) ){
            rank_error_cmp=fabs(q-percentage);
            flag_cmp=1;
        }
        if(avg>=ela90&&(!flag_ela) ){
            rank_error_ela=fabs(q-percentage);
            flag_ela=1;
        }
        if(q>=percentage&&(!flag_quantile)){
            if(fabs(cmp90-0)<1e-5) quantile_error_cmp=1;
            else quantile_error_cmp=fabs(log2(avg/cmp90));

            if(fabs(ela90-0)<1e-5) quantile_error_ela=1;
            else quantile_error_ela=fabs(log2(avg/ela90));
            
            flag_quantile=1;
        }
    }
    points_real.push_back(make_pair(1000000000, 1));
    po.push_back(1000000000);
    int sz = po.size();

#ifdef debug
    // err1<<"real_mod:";
    // pprint(points_real);
    // err1<<"po:";
    
    // for(int i=0;i<sz;i++){
    //     err1<<po[i]<<" ";
    // }
    // err1<<endl;
#endif

    /////////////////////////////////process res_real end//////////////////////////////

    /////////////////////////////////mae and mre for cmp start//////////////////////////////

    double range_sum[sz]={};
    _add(range_sum, 0, points[0].first, (double)points[0].second,po);
    for(int i=1;i<points.size();i++){
        _add(range_sum, points[i-1].first, points[i].first, (double)points[i].second,po);
    }
#ifdef debug
    // err1<<"cmp_rangesum:";
    // for(int i=0;i<sz-1;i++){
    //     err1<<po[i]<<"*"<<range_sum[i]<<" ";
    // }
    // err1<<endl;
#endif
    double MAE=0,MRE=0;
    for(int i=0;i<sz-1;i++){
        MAE+=fabs(range_sum[i]-(points_real[i+1].second));
        MRE+=RE(points_real[i+1].second,range_sum[i]);
    }
    MRE/=(double)mp[flow_id].delays.size();

    /////////////////////////////////mae and mre for cmp end//////////////////////////////


    /////////////////////////////////mae and mre for res_elas start//////////////////////////////

    memset(range_sum,0,sizeof(range_sum));
    _add(range_sum, 0, points_es[0].first, (double)points_es[0].second,po);
    for(int i=1;i<points_es.size();i++){
        _add(range_sum, points_es[i-1].first, points_es[i].first, (double)points_es[i].second,po);
    }
#ifdef debug
    // err1<<"ela_rangesum:";
    // for(int i=0;i<sz-1;i++){
    //     err1<<po[i]<<"*"<<range_sum[i]<<" ";
    // }
    // err1<<endl;
#endif
    double MAE_es=0,MRE_es=0;
    for(int i=0;i<sz-1;i++){
        MAE_es+=fabs(range_sum[i]-(points_real[i+1].second));
        MRE_es+=RE(points_real[i+1].second,range_sum[i]);
    }
    MRE_es/=(double)mp[flow_id].delays.size();

    /////////////////////////////////mae and mre for res_elas end//////////////////////////////


    lm_num_cmp+=1;
    lm_sum_cmp_MAE+=MAE;
    lm_sum_cmp_MRE+=MRE;
    lm_sum_cmp_rank_error+=rank_error_cmp;
    lm_sum_cmp_quantile_error+=quantile_error_cmp;
    lm_num_ela+=1;
    lm_sum_ela_MAE+=MAE_es;
    lm_sum_ela_MRE+=MRE_es;
    lm_sum_ela_rank_error+=rank_error_ela;
    lm_sum_ela_quantile_error+=quantile_error_ela;

#ifdef debug
    lm_cmp_cout <<rank<<","<<MAE<<","<<mp[flow_id].delays.size()<<","<<endl;
    lm_ela_cout <<rank<<","<<MAE_es<<","<<mp[flow_id].delays.size()<<","<<endl;
#endif
    return 0;

}else if(test==1){
    if(mp[flow_id].delays.size()>2){
        return 0;
    }

    int mx_real=*mp[flow_id].delays.rbegin();
    int mx_est=res_es.back().first;
    int mx_cmp=res.back().first;

    tiny_num_ela++;
    tiny_sum_ela+=((double)abs(mx_real-mx_est)/(double)mx_real > 0.5);
    err_tf_ela.insert((double)abs(mx_real-mx_est)/(double)mx_real);

    tiny_num_cmp++;
    tiny_sum_cmp+=((double)abs(mx_real-mx_cmp)/(double)mx_real > 0.5);
    err_tf_cmp.insert((double)abs(mx_real-mx_cmp)/(double)mx_real);
    return 0;
}
    return 0;
}