#ifndef _TRACE_H_
#define _TRACE_H_

#include <bits/stdc++.h>
using namespace std;

vector<pair<uint32_t, int>> loadCAIDA18(int read_num = -1, string dataset="caida"
){
    const char* filename;
    if(dataset=="caida") filename="/share/datasets/CAIDA2018/dataset/130000.dat";
    else if (dataset=="imc") filename="/root/work/data/20.dat";
    else if (dataset=="MAWI") filename="/share/pcap_zhangyd/time07.dat";

    printf("Open %s \n", filename);
    FILE* pf = fopen(filename, "rb");
    if(!pf){
        printf("%s not found!\n", filename);
        exit(-1);
    }

    vector<pair<uint32_t, int>> vec;
    double ftime=-1;
    long long fime=-1;
    char trace[30];
    int i = 0;

    if(filename=="/share/datasets/CAIDA2018/dataset/130000.dat"){
        while(fread(trace, 1, 21, pf)/* && vec.size()<1e5*/){
            uint32_t tkey = *(uint32_t*) (trace);
            double ttime = *(double*) (trace+13);
            if(ftime<0)ftime=ttime;
            vec.push_back(pair<uint32_t, int>(tkey, int((ttime-ftime)*10000000)) );

            if(++i == read_num) break;
        }
    }else if(filename=="/share/datasets/webdocs_timestamp/webdocs_timestamp03.dat"){
        while(fread(trace, 1, 16, pf)/* && vec.size()<1e5*/){
            uint32_t tkey = *(uint32_t*) (trace);
            double ttime = *(double*) (trace+8);
            if(ftime<0)ftime=ttime;
            vec.push_back(pair<uint32_t, int>(tkey, int((ttime-ftime)*1000000000)) );

            if(++i == read_num) break;
        }
    }else if(filename=="/root/work/data/20.dat"){
        while(fread(trace, 1, 26, pf)/* && vec.size()<1e5*/){
            uint32_t tkey = *(uint32_t*) (trace);
            long long ttimee = *(long long*) (trace+18);
            if(fime<0)fime=ttimee;
            if(ttimee-fime<0) cout<<"lala";
            vec.push_back(pair<uint32_t, int>(tkey, int((ttimee-fime)/100)) );
            if(++i == read_num) break;
        }
    }else{
        while(fread(trace, 1, 21, pf)/* && vec.size()<1e5*/){
            uint32_t tkey = *(uint32_t*) (trace);
            long long ttime = *(long long*) (trace+13);
            if(fime<0)fime=ttime;
            vec.push_back(pair<uint32_t, int>(tkey, int(ttime-fime)*100000 ));
            if(++i == read_num) break;
        }
    }
    printf("load %d packets\n", i);
    fclose(pf);
    return vec;
}


vector<pair<int, uint32_t>> groundtruth(vector<pair<uint32_t, int>> &input, int read_num = -1){
    map<uint32_t, int> cnt;
    int i = 0;
    for(auto [tkey,ttime]: input){
        ++cnt[tkey];
        if(++i == read_num) break;
    }
    vector<pair<int, uint32_t>> ans;
    for(auto flow: cnt)
        ans.push_back({flow.second, flow.first});
    sort(ans.begin(), ans.end(), greater<pair<int, uint32_t>>());
    printf("there are %d flows\n", (int)ans.size());
    return ans;
}




#endif // _TRACE_H_

