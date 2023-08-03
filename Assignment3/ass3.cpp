#include <set>
#include <unordered_map>
#include <string>
#include <sstream>
#include <functional>
#include <vector>
#include <fstream>
#include <exception>
#include <iostream>
using namespace std;

int cachemanager(vector<string>act1,vector<long long int>address1 ,int size, int blocksize , int l1_size , int l1_assoc , int l2_size , int l2_assoc ){
    int l1_sets= (l1_size/l1_assoc)/blocksize;
    // block => tag, valid bit , dirty bit , LRU 
    long long int l1_cache[l1_sets][l1_assoc][4];
    for (int i=0;i<l1_sets;i++){
        for (int j =0 ;j<l1_assoc;j++){
            l1_cache[i][j][0] = -1;
            l1_cache[i][j][1]=0;
            l1_cache[i][j][2]=0;
            l1_cache[i][j][3]=j+1;
        }
    }
    int l2_sets= (l2_size/l2_assoc)/blocksize;
    // block => tag, valid bit , dirty bit , LRU
    int l2_cache[l2_sets][l2_assoc][4];
    for (int i=0;i<l2_sets;i++){
        for (int j =0 ;j<l2_assoc;j++){
            l2_cache[i][j][0]=-1;
            l2_cache[i][j][1]=0;
            l2_cache[i][j][2]=0;
            l2_cache[i][j][3]=j+1;
        }
    }
    int l1_read=0,l1_write=0,l1_readmiss=0,l1_writemiss=0,l1_hit=0,l1_miss=0,l1_writebacks=0;
    int l2_read=0,l2_write=0,l2_readmiss=0,l2_writemiss=0,l2_hit=0,l2_miss=0,l2_writebacks=0;
    int y=5;
    for(long long int i=0;i<size;i++){

        string act=act1[i];
        long long int address=(address1[i]);
    // l1-checking starts here
        if(act=="r"){
            l1_read=l1_read+1;
        }
        else{
            l1_write=l1_write+1;
        }
        int index = address%blocksize;
        int l1_set_no = (address/blocksize)%l1_sets;
        long long int l1_tag = ((address/blocksize)/l1_sets);
        int l1_found = 0;
        for(int j=0;j<l1_assoc;j++){
            if(l1_cache[l1_set_no][j][0]==l1_tag && l1_cache[l1_set_no][j][1]==1 ){
                l1_hit=l1_hit+1;
                l1_found=j+1;
                //cout<<"hit\n";
                if(act=="w"){
                    l1_cache[l1_set_no][j][2]=1;
                }

            }
        }

        if(l1_found==0){
            l1_miss=l1_miss+1;
            //cout<<"miss\n";
            if(act=="r"){
                l1_readmiss=l1_readmiss+1;
            }
            else{
                l1_writemiss=l1_writemiss+1;
            }
            int l1_Lruway;
            for(int j=0; j<l1_assoc;j++){
                //cout<<(l1_cache[l1_set_no][j][3])<<" ";
                if(l1_cache[l1_set_no][j][3]==1){
                    l1_Lruway=j;
                }
            }
            //cout<<"\n";
            // writeback into l2-cache
            if(l1_cache[l1_set_no][l1_Lruway][2]==1){
                l1_writebacks=l1_writebacks+1;
                l2_write=l2_write+1;
                int l2_found1=0;
                long long int l2_add= l1_cache[l1_set_no][l1_Lruway][0]*l1_sets+l1_set_no;
                int l2_set_no1 = l2_add%l2_sets;
                long long int l2_tag1 = (l2_add/l2_sets);
                for(int i=0;i<l2_assoc;i++){
                    if(l2_cache[l2_set_no1][i][0]==l2_tag1 && l2_cache[l2_set_no1][i][1]==1 ){
                        l2_hit=l2_hit+1;
                        l2_found1=i+1;
                        //cout<<"dirty bit change "<<l2_set_no1<<" "<<i<<"\n";
                        l2_cache[l2_set_no1][i][2]=1;
                    }
                }
                int g = l2_cache[l2_set_no1][l2_found1-1][3];
                l2_cache[l2_set_no1][l2_found1-1][3]=l2_assoc+1;
                for(int i=0; i<l2_assoc ; i++){
                    if(l2_cache[l2_set_no1][i][3]>g){
                        l2_cache[l2_set_no1][i][3]=l2_cache[l2_set_no1][i][3]-1;
                    }
                }
            }

            // storing the value in l1-cache
            l1_cache[l1_set_no][l1_Lruway][0]=l1_tag;
            l1_cache[l1_set_no][l1_Lruway][1]=1;
            if(act=="r"){
            l1_cache[l1_set_no][l1_Lruway][2]=0;}
            else{
               l1_cache[l1_set_no][l1_Lruway][2]=1; 
            }
            l1_cache[l1_set_no][l1_Lruway][3]=l1_assoc+1;
            for(int i=0; i<l1_assoc ; i++){
                l1_cache[l1_set_no][i][3]=l1_cache[l1_set_no][i][3]-1;
            }
            for(int i=0; i<l1_assoc ; i++){
                //cout<<l1_cache[l1_set_no][i][3]<<" ";
            }

            // l2-checking begins here..
            l2_read=l2_read+1;
            int l2_found=0;
            int l2_set_no = (address/blocksize)%l2_sets;
            long long int l2_tag = ((address/blocksize)/l2_sets);
            for(int i=0;i<l2_assoc;i++){
                if(l2_cache[l2_set_no][i][0]==l2_tag && l2_cache[l2_set_no][i][1]==1 ){
                    l2_hit=l2_hit+1;
                    //cout<<"l2_hit"<<"\n";
                    l2_found=i+1;
                }
            }
            if(l2_found==0){
                //cout<<"l2_miss"<<"\n";
                l2_miss=l2_miss+1;
                l2_readmiss=l2_readmiss+1;

                int l2_Lruway;
                for(int i=0; i<l2_assoc;i++){
                    //cout<<l2_cache[l2_set_no][i][3]<<" ";
                    if(l2_cache[l2_set_no][i][3]==1){
                        l2_Lruway=i;
                }
                }
                //cout<<"\n";
                //cout<<"dirty-bit "<<l2_cache[l2_set_no][l2_Lruway][2]<<"\n";
                if(l2_cache[l2_set_no][l2_Lruway][2]==1){
                    l2_writebacks=l2_writebacks+1;
                    //cout<<"writeback "<<l2_writebacks<<"\n";
                }

                //l2-cache store memory
                l2_cache[l2_set_no][l2_Lruway][0]=l2_tag;
                l2_cache[l2_set_no][l2_Lruway][1]=1;
                //cout<<"dirty bit change 2-- "<<l2_set_no<<" "<<l2_Lruway<<"\n";
                l2_cache[l2_set_no][l2_Lruway][2]=0;
                l2_cache[l2_set_no][l2_Lruway][3]=l2_assoc+1;
                for(int i=0; i<l2_assoc ; i++){
                    l2_cache[l2_set_no][i][3]=l2_cache[l2_set_no][i][3]-1;
                }
                for(int i=0; i<l2_assoc ; i++){
                    //cout<<l2_cache[l2_set_no][i][3]<<" ";
                }
                //cout<<"\n";
            }
            else{
                int g = l2_cache[l2_set_no][l2_found-1][3];
                l2_cache[l2_set_no][l2_found-1][3]=l2_assoc+1;
                for(int i=0; i<l2_assoc ; i++){
                    if(l2_cache[l2_set_no][i][3]>g){
                        l2_cache[l2_set_no][i][3]=l2_cache[l2_set_no][i][3]-1;
                    }
                }
            }

        }
        else{
            int m = l1_cache[l1_set_no][l1_found-1][3];
            l1_cache[l1_set_no][l1_found-1][3]=l1_assoc+1;
            for(int i=0; i<l1_assoc ; i++){
                if(l1_cache[l1_set_no][i][3]>m){
                    l1_cache[l1_set_no][i][3]=l1_cache[l1_set_no][i][3]-1;
                }
            }
        }
        }
    float l1_missrate = (l1_miss/(float)(l1_read+l1_write));
    float l2_missrate = (l2_miss/(float)(l2_read+l2_write));
    //printing the table
    cout<<"number of L1 reads:"<<l1_read<<"\n";
    cout<<"number of L1 read misses:"<<l1_readmiss<<"\n";
    cout<<"number of L1 writes:"<<l1_write<<"\n";
    cout<<"number of L1 write misses:"<<l1_writemiss<<"\n";
    cout<<"L1 miss rate:"<<l1_missrate<<"\n";
    cout<<"number of writebacks from L1 memory:"<<l1_writebacks<<"\n";
    cout<<"number of L2 reads:"<<l2_read<<"\n";
    cout<<"number of L2 read misses:"<<l2_readmiss<<"\n";
    cout<<"number of L2 writes:"<<l2_write<<"\n";
    cout<<"number of L2 write misses:"<<l2_writemiss<<"\n";
    cout<<"L2 miss rate:"<<l2_missrate<<"\n";
    cout<<"number of writebacks from L2 memory:"<<l2_writebacks<<"\n";
    return 0;
}

int main(int argc, char** argv){
    int blocksize=stoi(argv[1]);
    int l1_size = stoi(argv[2]);
    int l1_assoc = stoi(argv[3]);
    int l2_size = stoi(argv[4]);
    int l2_asooc = stoi(argv[5]);
    string filename = (argv[6]);
    long long int length =0;
    fstream tracefile;
    tracefile.open(filename,ios::in);
    string line;
    vector<string>act;
    vector<long long int>address;
    while(getline(tracefile,line)){
        if(line==""){
            continue;
        }
        length=length+1;
        string line1;
        for(auto &ch : line){
            if(ch=='r' || ch=='w'){
                string s(1,ch);
                act.push_back(s);
            }
            else if(!isalpha(ch) && !isdigit(ch)){
                continue;
            }
            else{
                line1=line1+ch;
            }
        }
        long long int iu;
        std::stringstream ss;
        ss << std::hex << line1;
        ss >> iu;
        address.push_back(iu);

    }
    tracefile.close();
    // for (int k=0;k<length;k++){
    //     cout<<act[k]<<"\n"<<address[k]<<"\n";
    cachemanager(act,address,length,blocksize,l1_size,l1_assoc,l2_size,l2_asooc);
    return 0;
}
