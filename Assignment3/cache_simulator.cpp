#include <iostream>
#include <fstream>
#include <iomanip>
#include <math.h>
#include <vector>
#include <algorithm>
#include <unordered_map>
#include <queue>
#include <utility>

using namespace std;

// Cache Parameters
int BLOCKSIZE;
int L1_SIZE;
int L1_ASSOC;
int L2_SIZE;
int L2_ASSOC;

// Latency Parameters (in ns)
const int L1_READ_TIME = 1;
const int L1_WRITE_TIME = 1;
const int L2_READ_TIME = 20;
const int L2_WRITE_TIME = 20;
const int DRAM_READ_TIME = 200;
const int DRAM_WRITE_TIME = 200;

// Structures
struct block
{
    bool valid = false;
    bool dirty = false;
    unsigned long long tag;
    unsigned long long last_used=0;
    block() {}
};

struct set1
{
    vector<block> blocks;
    set1()
    {
        for (int i = 0; i < L1_ASSOC; i++)
        {
            block new_block;
            new_block.last_used = 0;
            blocks.push_back(new_block);
        }
    }
};

struct set2
{
    vector<block> blocks;
    set2()
    {
        for (int i = 0; i < L2_ASSOC; i++)
        {
            block new_block;
            new_block.last_used = 0;
            blocks.push_back(new_block);
        }
    }
};

struct cache1
{
    vector<set1> sets;
    unsigned long long reads = 0;
    unsigned long long read_misses = 0;
    unsigned long long writes = 0;
    unsigned long long write_misses = 0;
    unsigned long long writebacks = 0;
    cache1()
    {
        int num_sets = L1_SIZE / (L1_ASSOC * BLOCKSIZE);
        for (int i = 0; i < num_sets; i++)
        {
            set1 new_set;
            sets.push_back(new_set);
        }
    }
};

struct cache2
{
    vector<set2> sets;
    unsigned long long reads = 0;
    unsigned long long read_misses = 0;
    unsigned long long writes = 0;
    unsigned long long write_misses = 0;
    unsigned long long writebacks = 0;
    cache2()
    {
        int num_sets = L2_SIZE / (L2_ASSOC * BLOCKSIZE);
        for (int i = 0; i < num_sets; i++)
        {
            set2 new_set;
            sets.push_back(new_set);
        }
    }
};

// Functions
unsigned long long get_tag(unsigned long long address)
{
    return address >> (int)(log2(BLOCKSIZE) + log2(L1_SIZE / (L1_ASSOC * BLOCKSIZE)));
}

unsigned long long get_index(unsigned long long address)
{
    return (address >> (int)(log2(BLOCKSIZE))) % (L1_SIZE / (L1_ASSOC * BLOCKSIZE));
}

unsigned long long get_tag_2(unsigned long long address)
{
    return address >> (int)(log2(BLOCKSIZE) + log2(L2_SIZE / (L2_ASSOC * BLOCKSIZE)));
}

unsigned long long get_index_2(unsigned long long address)
{
    return (address >> (int)(log2(BLOCKSIZE))) % (L2_SIZE / (L2_ASSOC * BLOCKSIZE));
}

void insert_block_2(set2 &curr_set, unsigned long long address, bool dirty, unsigned long long tag, cache2 &l2)
{
    int lru_block_ind = 0;
    for (int i = 0; i < L2_ASSOC; i++)
    {
        if (curr_set.blocks[i].last_used < curr_set.blocks[lru_block_ind].last_used)
        {
            lru_block_ind = i;
        }
    }
    bool x = curr_set.blocks[lru_block_ind].dirty;
    bool y = curr_set.blocks[lru_block_ind].valid;
    if ( x == true && y)
    {
        l2.writebacks++;
    }
    curr_set.blocks[lru_block_ind].last_used = l2.reads + l2.writes;
    curr_set.blocks[lru_block_ind].dirty = dirty;
    curr_set.blocks[lru_block_ind].tag = tag;
    curr_set.blocks[lru_block_ind].valid = true;
}

void insert_block_1(set1 &curr_set, unsigned long long address, bool dirty, unsigned long long tag, cache1 &l1, cache2 &l2)
{
    int lru_block_ind = 0;
    for (int i = 0; i < L1_ASSOC; i++)
    {
        if (curr_set.blocks[i].last_used < curr_set.blocks[lru_block_ind].last_used)
        {
            lru_block_ind = i;
        }
    }
    bool x = curr_set.blocks[lru_block_ind].dirty;
    curr_set.blocks[lru_block_ind].last_used = l1.reads + l1.writes;
    curr_set.blocks[lru_block_ind].dirty = dirty;
    curr_set.blocks[lru_block_ind].tag = tag;
    curr_set.blocks[lru_block_ind].valid = true;
    if ( x == true)
    {
        l1.writebacks++;
        l2.writes++;
    unsigned long long tag2 = get_tag_2(address);
    unsigned long long index2 = get_index_2(address);
    set2 &current_set2 = l2.sets[index2];
    bool hit = false;
    for (int i = 0; i < L2_ASSOC; i++)
    {
        if (current_set2.blocks[i].valid && current_set2.blocks[i].tag == tag2)
        {
            hit = true;
            current_set2.blocks[i].last_used = l2.reads + l2.writes;
            if(current_set2.blocks[i].dirty == true && x == 0){
                current_set2.blocks[i].dirty = false;
            }
            else{
                current_set2.blocks[i].dirty = true;
            }
             //.............................................................................
            break;
        }
    }
    if (!hit)
    {
        if(x){
            l2.write_misses++;
        }
        insert_block_2(current_set2, address, x, tag2, l2); //................................................................
    }
    }
}

void access_cache(unsigned long long address, char type, cache1 &l1, cache2 &l2)
{
    unsigned long long tag = get_tag(address);
    unsigned long long index = get_index(address);
    set1 &current_set = l1.sets[index];
    unsigned long long tag2 = get_tag_2(address);
    unsigned long long index2 = get_index_2(address);
    set2 &current_set2 = l2.sets[index2];
    bool hit = false;
    for (int i = 0; i < L1_ASSOC; i++)
    {
        if (current_set.blocks[i].valid && current_set.blocks[i].tag == tag)
        {
            hit = true;
            if (type == 'r')
                l1.reads++;
            else if (type == 'w')
            {
                l1.writes++;
            }
            current_set.blocks[i].last_used = l1.reads + l1.writes;
            if (type == 'w')
                current_set.blocks[i].dirty = true;
            break;
        }
    }
    if (!hit)
    {
        if (type == 'r')
        {
            l1.reads++;
            l1.read_misses++;
        }
        else if (type == 'w')
        {
            l1.writes++;
            l1.write_misses++;
        }
        l2.reads++;
        bool l2_hit = false;
        for (int i = 0; i < L2_ASSOC; i++)
        {
            block &current_block = current_set2.blocks[i];
            if (current_block.valid && current_block.tag == tag2)
            {
                l2_hit = true;

                current_block.last_used = l2.reads + l2.writes;
                // errorrrr
                // if (type == 'w')
                // {
                //     current_block.dirty = true;          //  .............................................................................
                // }
                break;
            }
        }
        if (l2_hit)
        {
            if (type == 'r')
            {

                insert_block_1(current_set, address, false, tag, l1, l2);
            }
            else if (type == 'w')
            {

                insert_block_1(current_set, address, true, tag, l1, l2);
            }
        }
        else
        {
            l2.read_misses++;
            // access_memory
            if (type == 'r')
            {
                insert_block_2(current_set2, address, false, tag2, l2);
                insert_block_1(current_set, address, false, tag, l1, l2);
            }
            else if (type == 'w')
            {
                // errorrrrr
                insert_block_2(current_set2, address, false, tag2, l2); //.............................................................................
                insert_block_1(current_set, address, true, tag, l1, l2);
            }
        }
    }
}

int main(int argc, char *argv[])
{
    if (argc != 7)
    {
        cout << "Usage: " << argv[0] << " <BLOCKSIZE> <L1_SIZE> <L1_ASSOC> <L2_SIZE> <L2_ASSOC> <File_Name>\n";
        return 1;
    }
    BLOCKSIZE = stoi(argv[1]);
    L1_SIZE = stoi(argv[2]);
    L1_ASSOC = stoi(argv[3]);
    L2_SIZE = stoi(argv[4]);
    L2_ASSOC = stoi(argv[5]);

    // Code for simulating cache access goes here
    cache1 l1;
    cache2 l2;
    ifstream trace(argv[6]);
    string line;
    while (getline(trace, line))
    {
        char type = line[0];
        unsigned long long address = stoull(line.substr(2), nullptr, 16);
        access_cache(address, type, l1, l2);
    }
    trace.close();
    unsigned long long total_accesses = l1.reads + l1.writes;
    unsigned long long l1_misses = l1.read_misses + l1.write_misses;
    unsigned long long total_l2_accesses = l2.reads + l2.writes;
    unsigned long long l2_misses = l2.read_misses + l2.write_misses;
    float l1_miss_rate = (float)l1_misses / total_accesses;
    float l2_miss_rate = (float)l2_misses / total_l2_accesses;
    // double avg_l1_access_time = L1_READ_TIME + (l1_miss_rate * (L2_READ_TIME + L2_WRITE_TIME + L1_WRITE_TIME));
    // double avg_mem_access_time = avg_l1_access_time + ((double)l1.writebacks / total_accesses) * (DRAM_READ_TIME + DRAM_WRITE_TIME);
    cout << fixed << setprecision(2);
    cout << "i. number of L1 reads: " << l1.reads << endl;
    cout << "ii. number of L1 read misses: " << l1.read_misses << endl;
    cout << "iii. number of L1 writes: " << l1.writes << endl;
    cout << "iv. number of L1 write misses: " << l1.write_misses << endl;
    cout << "v. L1 miss rate: " << fixed << setprecision(4) << l1_miss_rate << endl;
    cout << "vi. number of writebacks from L1 memory: " << l1.writebacks << endl;
    cout << "vii. number of L2 reads: " << l2.reads << endl;
    cout << "viii. number of L2 read misses: " << fixed << setprecision(4) << l2.read_misses << endl;
    cout << "ix. number of L2 writes: " << l2.writes << endl;
    cout << "x. number of L2 write misses: " << l2.write_misses << endl;
    cout << "xi. L2 miss rate: " << l2_miss_rate << endl;
    cout << "xii. number of writebacks from L2 memory: " << l2.writebacks << endl;

    return 0;
}
