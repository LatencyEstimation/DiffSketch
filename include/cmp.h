#include "ddsketch.h"
#include "ours.h"
using namespace std;

typedef ddsketch::LogCollapsingLowestDenseDDSketch<80,8> DD;
typedef ddsketch::DDSketch<10> perfect_DD;
class DistriSketch {
private:
    DD **a; 
    int num;
    BOBHash32 *hash[3];
    int hash_pos[3];

public:
    DistriSketch(int memory_limit = 15000,int seed=0) {
        num = memory_limit*1000/(8*4);
        cout<<"straw-man bucket len: "<<num<<endl;
        a = new DD*[num];
        for (int i = 0; i < num; i++) {
            a[i] = new DD;
        }
        for (int i = 0; i < 3; i++)
            hash[i] = new BOBHash32(seed*100+i*10);
    }

    ~DistriSketch() {}
    void calc_hash(uint32_t flow_id) {
        for (int i = 0; i < 3; i++)
            hash_pos[i] = hash[i]->run(flow_id) % (num);
    }

    void insert(pair<uint32_t, int> packet) {
        uint32_t flow_id = packet.first;
        int delay = packet.second;
        calc_hash(flow_id);
        a[hash_pos[0]]->add(delay);
    }

    vector<pair<int,int>> query(uint32_t flow_id){
        calc_hash(flow_id);
        double gama = 1.8/0.2;
        // double gama = 1.5/0.5;
        vector<pair<int, int>> ans = to_vector(a[hash_pos[0]]->store_.to_string(),gama);
        return ans;
    }

    double query_quantile(uint32_t flow_id, double percentage){
        calc_hash(flow_id);
        return a[hash_pos[0]]->get_quantile_value(percentage);
    }

    int tot(uint32_t flow_id){
        calc_hash(flow_id);
        return (int)a[hash_pos[0]]->num_values();
    }
};