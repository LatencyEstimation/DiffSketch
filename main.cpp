#include <bits/stdc++.h>
#include "./include/utils.h"
#include "./include/trace.h"
using namespace std;


void tiny_err_dist(string dataset){
    string tiny_filename = "./res/tiny_cmp_dist_"+dataset+".csv";
    ofstream fout(tiny_filename);
    fout << "err,prob," << endl;
    int sum = 0;
    int tot = err_tf_cmp.size();
    // err << "tiny tot=" << tot << endl;
    auto &errtf = err_tf_cmp;
    for (auto delay: errtf) {
        sum++;
        fout << delay << ',' << (double)sum / tot << ',' << endl;
    }
    fout.close();

    tiny_filename = "./res/tiny_ela_dist_"+dataset+".csv";
    fout.open(tiny_filename);
    fout << "err,prob," << endl;
    sum = 0;
    tot = err_tf_ela.size();
    // err << "tiny tot=" << tot << endl;
    auto &errtf1 = err_tf_ela;
    for (auto delay: errtf1) {
        sum++;
        fout << delay << ',' << (double)sum / tot << ',' << endl;
    }
    fout.close();
}

void test3(int memory,int test,string dataset,int repeat) {

    vector<int> qrk;
    int nflows;
    if(dataset=="caida") nflows=165000;
    if(dataset=="imc") nflows=4811;
    if(dataset=="MAWI") nflows=76623;

    for(int i=nflows;i>=0;i--){
        qrk.push_back(i);
    }

    bool flag_cmp=0,flag_ela=0;
    int cnt=0;
    for (auto [flow_id, arrive_time]: input) {
        cnt++;
        insert(flow_id, arrive_time,test);

        if(test==2){
            if(insert_cmp>1&&flag_cmp==0){
                insert_cmp_throu+=cnt;
                flag_cmp=1;
            }
            if(insert_ela>1&&flag_ela==0){
                insert_ela_throu+=cnt;
                flag_ela=1;
            }
            if(insert_cmp>1&&insert_ela>1){
                break;
            }
        }
    }

    cout<<"max_interval: "<<mx<<endl;

    cnt=0;
    
    if(test!=2)
        if(test==3){
            bool fla=1;
            while(fla){
                for (int rk: qrk){
                    cnt++;
                    double a111 = query_rank(rk,test);
                    if(query_cmp>1&&flag_cmp==0){
                        query_cmp_throu+=cnt;
                        flag_cmp=1;
                    }
                    if(query_ela>1&&flag_ela==0){
                        query_ela_throu+=cnt;
                        flag_ela=1;
                    }
                    if(query_cmp>1&&query_ela>1){
                        fla=0;
                        break;
                    }
                    
                }
            }
        }else{
            for (int rk: qrk)
                double a111 = query_rank(rk,test);
        }
    if(test==1)
        tiny_err_dist(dataset);
    
}

int main(int argc, char* argv[]) {
#ifdef debug
    lm_cmp_cout <<"rank,MAE,size,"<<endl;
    lm_ela_cout <<"rank,MAE,size,"<<endl;
#endif
    cout<<argc<<endl;
    int memory = atoi(argv[1]);
    cout<<"memory:"<<memory<<endl;
    int test = atoi(argv[2]);
    cout<<"test:"<<test<<endl;
    int repeat = atoi(argv[3]);
    cout<<"repeat:"<<repeat<<endl;
    string dataset = (argv[4]);
    cout<<"dataset:"<<dataset<<endl;
    int tn;

    repeat/=10;
    input = loadCAIDA18(readnum,dataset);
    gt = groundtruth(input, readnum);
    for(int i=0;i<repeat;i++){
        cout<<"start"<<endl;
        mp.clear();
        err_tf_cmp.clear();
        err_tf_ela.clear();

        srand(i);
        ds = new DistriSketch(memory,i);
        pDD.clear();
        es = new elaSketch(memory,dataset,i);

        insert_cmp=0;insert_ela=0;query_cmp=0;query_ela=0;
        if(test==2||test==3||test==4||test==5)
            test3(memory,test,dataset,repeat);
        else{
            test3(memory,test,dataset,repeat);
            break;
        }
        delete ds;delete es;
        cout<<"end"<<endl;

    }

    if(test==4){
        double res_lm_est_MAE=lm_sum_ela_MAE/(double)lm_num_ela;
        double res_lm_est_MRE=lm_sum_ela_MRE/(double)lm_num_ela;
        double res_rank_est=lm_sum_ela_rank_error/(double)lm_num_ela;
        double res_quantile_est=lm_sum_ela_quantile_error/(double)lm_num_ela;
        cout<<"large_est_MAE:"<<res_lm_est_MAE<<" large_est_MRE:"<<res_lm_est_MRE<<" large_est_rankErr:"<<res_rank_est<<" large_est_quantilErr:"<<res_quantile_est<<endl;
        string res_filename = "./res/res_ela_large_"+dataset+".csv";
        ofstream resout(res_filename,ios::app);
        resout<<memory<<","<<res_lm_est_MAE<<","<<res_lm_est_MRE<<","<<res_rank_est<<","<<res_quantile_est<<","<<endl;
        resout.close();

        double res_lm_cmp_MAE=lm_sum_cmp_MAE/(double)lm_num_cmp;
        double res_lm_cmp_MRE=lm_sum_cmp_MRE/(double)lm_num_cmp;
        double res_rank_cmp=lm_sum_cmp_rank_error/(double)lm_num_cmp;
        double res_quantile_cmp=lm_sum_cmp_quantile_error/(double)lm_num_cmp;
        cout<<"large_cmp_MAE:"<<res_lm_cmp_MAE<<" large_cmp_MRE:"<<res_lm_cmp_MRE<<" large_cmp_rankErr:"<<res_rank_cmp<<" large_cmp_quantilErr:"<<res_quantile_cmp<<endl;
        res_filename = "./res/res_cmp_large_"+dataset+".csv";
        resout.open(res_filename,ios::app);
        resout<<memory<<","<<res_lm_cmp_MAE<<","<<res_lm_cmp_MRE<<","<<res_rank_cmp<<","<<res_quantile_cmp<<","<<endl;
        resout.close();
    }
    if(test==5){
        double res_lm_est_MAE=lm_sum_ela_MAE/(double)lm_num_ela;
        double res_lm_est_MRE=lm_sum_ela_MRE/(double)lm_num_ela;
        double res_rank_est=lm_sum_ela_rank_error/(double)lm_num_ela;
        double res_quantile_est=lm_sum_ela_quantile_error/(double)lm_num_ela;
        cout<<"medium_est_MAE:"<<res_lm_est_MAE<<" medium_est_MRE:"<<res_lm_est_MRE<<" medium_est_rankErr:"<<res_rank_est<<" medium_est_quantilErr:"<<res_quantile_est<<endl;
        string res_filename = "./res/res_ela_medium_"+dataset+".csv";
        ofstream resout(res_filename,ios::app);
        resout<<memory<<","<<res_lm_est_MAE<<","<<res_lm_est_MRE<<","<<res_rank_est<<","<<res_quantile_est<<","<<endl;
        resout.close();

        double res_lm_cmp_MAE=lm_sum_cmp_MAE/(double)lm_num_cmp;
        double res_lm_cmp_MRE=lm_sum_cmp_MRE/(double)lm_num_cmp;
        double res_rank_cmp=lm_sum_cmp_rank_error/(double)lm_num_cmp;
        double res_quantile_cmp=lm_sum_cmp_quantile_error/(double)lm_num_cmp;
        cout<<"medium_cmp_MAE:"<<res_lm_cmp_MAE<<" medium_cmp_MRE:"<<res_lm_cmp_MRE<<" medium_cmp_rankErr:"<<res_rank_cmp<<" medium_cmp_quantilErr:"<<res_quantile_cmp<<endl;
        res_filename = "./res/res_cmp_medium_"+dataset+".csv";
        resout.open(res_filename,ios::app);
        resout<<memory<<","<<res_lm_cmp_MAE<<","<<res_lm_cmp_MRE<<","<<res_rank_cmp<<","<<res_quantile_cmp<<","<<endl;
        resout.close();
    }
    if(test==2){
        string filename = "./res/insert_speed_cmp_"+dataset+".csv";
        ofstream fout_insert(filename,ios::app);
        fout_insert<<memory<<","<< (double)insert_cmp_throu/1000000/repeat <<","<<endl;
        fout_insert.close();

        filename = "./res/insert_speed_ela_"+dataset+".csv";
        fout_insert.open(filename,ios::app);
        fout_insert<<memory<<","<< (double)insert_ela_throu/1000000/repeat <<","<<endl;
        fout_insert.close();
    }
    if(test==3){
        string filename = "./res/query_speed_cmp_"+dataset+".csv";
        ofstream fout_query(filename,ios::app);
        fout_query<<memory<<","<< (double)query_cmp_throu/1000000/repeat <<","<<endl;
        fout_query.close();

        filename = "./res/query_speed_ela_"+dataset+".csv";
        fout_query.open(filename,ios::app);
        fout_query<<memory<<","<< (double)query_ela_throu/1000000/repeat <<","<<endl;
        fout_query.close();
    }
    return 0;
}

