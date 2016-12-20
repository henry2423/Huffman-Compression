//
//  main.cpp
//  Huffman Compression
//
//  Created by Henry on 18/12/2016.
//  Copyright © 2016 henry. All rights reserved.
//

#include <iostream>
#include <stdio.h>
#include <string>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <vector>
#include <sys/time.h>

using namespace std;

string readBitString(ifstream &stream)
{
    stringstream input;
    
    char c;
    
    while (stream.get(c))
    {
        for(int i = 7; i >= 0; i--)
        {
            input << ((c >> i) & 1);
        }
    }
    
    return input.str();
}

void writeBitString(ofstream &stream, string input)
{
    stringstream inputStream(input);
    
    char bits[9];
    char c;
    
    while(inputStream.get(bits, 9))
    {
        for(int i = 7; i >= 0; i--)
        {
            if(bits[7-i] == '1')
            {
                c |= 1 << i;
            }
            else
            {
                c &= ~(1 << i);
            }
        }
        
        stream << c;
    }
}


struct frequencyTable{
    unsigned char byte;
    long weight;
    /*
    frequencyTable(){}
    frequencyTable(unsigned char set_byte, long set_weight)
    {
        this->byte = set_byte;
        this->weight = set_weight;
    }
     */
};

typedef vector<frequencyTable> frequency_table;


bool is_empty_frequency(frequencyTable const &tb)
{
    return tb.weight==0;
}

bool frequency_comp(frequencyTable const &a, frequencyTable const &b)
{
    return a.weight < b.weight;
}

frequency_table make_frequency_table(istream &input){
    frequency_table table;
    frequency_table result_table;
    
    unsigned char byte;
    //每个字节作为一个单词，由于1字节内可以有0-255共256种可能的值，因此单词一共有256个
    //在词汇表中插入256个单词，令每个单词的权为0，作为初始值
    for(int i=0;i<256;++i){//i的类型不能是char，否则会在256的时候回绕到0
        frequencyTable temp_iter={static_cast<unsigned char>(i),0};
        table.push_back(temp_iter);
    }
    
    
    //遍历输入文件，统计每个单词出现的次数
    while(input){
        if(!input.read(reinterpret_cast<char*>(&byte),sizeof(byte))){
            break;
        }
        ++(table[byte].weight);//统计每个单词出现的次数
    }
    
    
    //将没出现过的单词过滤掉，只出现过的单词到我们的结果中
    remove_copy_if(table.begin(),table.end(),back_inserter(result_table),is_empty_frequency);
    //remove_copy_if是标准库的
    sort(result_table.begin(), result_table.end(), frequency_comp);
    
    return result_table;
}

class HuffmanTree
{
    
public:
    HuffmanTree(frequency_table frequency_table, long N);
    HuffmanTree(long N);
    vector<string> huffman_code(long len);
    
    long accumlate_nodes;
    long total_nodes;
    typedef struct HuffmanNode
    {
        bool used;
        long weight;
        long left_child, right_chlid, parent;
    } *h_node;
    h_node huffman;
    frequency_table choose_min_nodes(frequency_table frequency_table, long i,long *min1,long *min2);

};

frequency_table HuffmanTree::choose_min_nodes(frequency_table frequencyTables, long i, long *min1_pos,long *min2_pos){
    
    
    for(long index=0; index<total_nodes; index++)
    {
        if(huffman[index].used == 0 && frequencyTables[0].weight == huffman[index].weight)
        {
            *min1_pos = index;
            huffman[index].used = 1;
            break;
        }
    }
    
    for(long index=0; index<total_nodes; index++)
    {
        if(huffman[index].used == 0 && index != *min1_pos && frequencyTables[1].weight == huffman[index].weight)
        {
            *min2_pos = index;
            huffman[index].used = 1;
            break;
        }
    }
    
    
    frequencyTables.erase(frequencyTables.begin(),frequencyTables.begin()+2);
    frequencyTable temp{'0', huffman[*min1_pos].weight + huffman[*min2_pos].weight};
    
    frequencyTables.push_back(temp);
    sort(frequencyTables.begin(), frequencyTables.end(), frequency_comp);
    

    
    return frequencyTables;
    
    
}

HuffmanTree::HuffmanTree(frequency_table frequencyTables, long N){
    
    this->accumlate_nodes = N;  //權重節點
    this->total_nodes = 2*N - 1;
    this->huffman = new HuffmanNode[total_nodes];
    
    for(long i=0; i<total_nodes; i++)
    {
        if(i < accumlate_nodes)
            huffman[i].weight = frequencyTables[i].weight;
        else
            huffman[i].weight = -1;
        
        huffman[i].left_child = -1;
        huffman[i].right_chlid = -1;
        huffman[i].parent = -1;
        huffman[i].used = 0;
    }
    

    //形成樹
    for(long i=accumlate_nodes; i<total_nodes; i++)
    {
        long min1_pos;
        long min2_pos;
        frequencyTables = choose_min_nodes(frequencyTables, i, &min1_pos, &min2_pos);
        huffman[min1_pos].parent = i;
        huffman[min2_pos].parent = i;
        huffman[i].left_child = min1_pos;
        huffman[i].right_chlid = min2_pos;
        huffman[i].weight = huffman[min1_pos].weight + huffman[min2_pos].weight;
    }

}

HuffmanTree::HuffmanTree(long N)
{
    this->accumlate_nodes = N;  //權重節點
    this->total_nodes = 2*N - 1;
    this->huffman = new HuffmanNode[total_nodes];
}

vector<string> HuffmanTree::huffman_code(long len){
    
    long current = 0;
    long parent = 0;
    
    vector<string> huffman_code = vector<string>(len); //two-dim array
    
    for (long i = 0; i < len; i++){
        string temp = "";
        current = i;
        parent = huffman[current].parent;
        while (parent >= 0)
        {
            if (current == huffman[parent].left_child)
            {
                temp += "0";
            }
            else
            {
                temp += "1";
            }
            current = parent;
            parent = huffman[parent].parent;
        }
        reverse(temp.begin(), temp.end());
        huffman_code[i] = temp;
    }
    
    return huffman_code;
    
}


void encoded(frequency_table f_table, ifstream &input_file, ofstream &output_file)
{
    vector<string> huffman_code;
    HuffmanTree h_tree( f_table, f_table.size());
    huffman_code = h_tree.huffman_code(f_table.size());
    
    string encode_bitstream = "";
    char file_byte;
    
    input_file.clear();  // 重置 eof
    input_file.seekg(0);  // get 指標移至檔案首
    
    while(input_file.get(file_byte))
    {
        for (int i = 0; i<f_table.size(); i++) {
                if (f_table.at(i).byte == static_cast<unsigned char>(file_byte))
                    encode_bitstream += huffman_code[i];
        }
    }
    
    
    
    char actual_nodes = static_cast<unsigned char>(h_tree.accumlate_nodes - 1);
    //char total_nodes = char(h_tree.total_nodes);
    
    output_file << actual_nodes;
    //cout << actual_nodes << endl;
    
    for(int index=0; index<h_tree.total_nodes; index++)
    {
        if(index < h_tree.accumlate_nodes)
        {
            output_file << f_table[index].byte;
            //cout << f_table[index].byte << " ";
        }
        else
        {
            output_file << h_tree.huffman[index].left_child << " " << h_tree.huffman[index].right_chlid << " ";
        }
    }
    
    
    writeBitString(output_file, encode_bitstream);
    
}


void decoded(ifstream &input_file, ofstream &output_file)
{
    int actual_nodes;
    int total_nodes;
    
    char hader_byte;
    input_file.get(hader_byte);
    actual_nodes = static_cast<unsigned char>(hader_byte) + 1;
    total_nodes = actual_nodes*2 - 1;
    
    HuffmanTree h_decode_tree(actual_nodes);
    char decode_table[actual_nodes];
    
    for(int index=0; index<total_nodes; index++)
    {
        if(index < actual_nodes)
        {
            input_file.get(hader_byte);
            decode_table[index] = hader_byte;
            h_decode_tree.huffman[index].left_child = -1;
            h_decode_tree.huffman[index].right_chlid = -1;
            
        }
        else
        {
            int tree_pos;
            input_file >> tree_pos;
            h_decode_tree.huffman[index].left_child = tree_pos;;
            input_file >> tree_pos;
            h_decode_tree.huffman[index].right_chlid = tree_pos;;
        }
    }
    
    char temp_space;
    input_file.get(temp_space);
    
    string huffman_code;
    huffman_code = readBitString(input_file);
    long node_pos = total_nodes - 1;
    
    for(long index=0; index<huffman_code.length(); index++)
    {
        if(h_decode_tree.huffman[node_pos].left_child == -1 || h_decode_tree.huffman[node_pos].right_chlid == -1)
        {
            output_file << static_cast<unsigned char>(decode_table[node_pos]);
            node_pos = total_nodes - 1;
        }
        
        if(huffman_code[index] == '0')
        {
            
            node_pos = h_decode_tree.huffman[node_pos].left_child;
        }
        else
        {
            node_pos = h_decode_tree.huffman[node_pos].right_chlid;
        }
        
    }
    
    
}






int main(int argc, const char * argv[]) {
    
    int command;
    string filename;
    
    
    ifstream input_file;      //fstream 讀入input_file
    ofstream output_file;     //fstream 讀入output_file

    
    
    while(1)
    {
        cout << "Chose command: ";
        cin >> command;
        
        if(command == 3)
        {
            break;
        }
        
        //compress
        if(command == 1)
        {
            /*
            cout << "Compress" << endl;
            cout << "Input filename: ";
            cin >> filename;
            input_file.open(filename, ifstream::binary | ifstream::in);
            if(!input_file)
            {
                cout<<"Can't open the file"<<endl;
                continue;
            }

            cout << "Output filename: ";
            cin >> filename;
            output_file.open(filename, ofstream::binary | ofstream::out | ofstream::trunc);
            if(!output_file)
            {
                cout<<"Can't create the file"<<endl;
                continue;
            }
            */
            
            input_file.open(argv[1], ifstream::binary | ifstream::in);
            output_file.open(argv[2], ofstream::binary | ofstream::out | ofstream::trunc);
            
            
            cout << "Compressing..." << endl;
            
            /*---要計算的程式效率區域---*/
            struct timeval t_val;
            gettimeofday(&t_val, NULL);
            
            encoded( make_frequency_table(input_file), input_file, output_file);
            
            //計時程式
            struct timeval t_val_end;
            gettimeofday(&t_val_end, NULL);
            struct timeval t_result;
            timersub(&t_val_end, &t_val, &t_result);
            double consume = t_result.tv_sec*1000 + (1.0 * t_result.tv_usec)/1000;
            
            cout << "Done." << endl;
            
            printf("Cost %f ms \n", consume);

        }
        //decompress
        else if(command == 2)
        {
            /*
            cout << "Extract" << endl;
            cout << "Input filename: ";
            cin >> filename;
            input_file.open(filename, ifstream::binary | ifstream::in);
            if(!input_file)
            {
                cout<<"Can't open the file"<<endl;
                continue;
            }
            
            cout << "Output filename: ";
            cin >> filename;
            output_file.open(filename, ofstream::binary | ofstream::out | ofstream::trunc);
            if(!output_file)
            {
                cout<<"Can't create the file"<<endl;
                continue;
            }
            */
            
            
            input_file.open(argv[2], ifstream::binary | ifstream::in);
            output_file.open(argv[3], ofstream::binary | ofstream::out | ofstream::trunc);
            
            
            cout << "Decompressing..." << endl;
            
            /*---要計算的程式效率區域---*/
            struct timeval t_val;
            gettimeofday(&t_val, NULL);
            
            
            decoded(input_file, output_file);
            
            //計時程式
            struct timeval t_val_end;
            gettimeofday(&t_val_end, NULL);
            struct timeval t_result;
            timersub(&t_val_end, &t_val, &t_result);
            double consume = t_result.tv_sec*1000 + (1.0 * t_result.tv_usec)/1000;
            
            cout << "Done." << endl;
            
            printf("Cost %f ms \n", consume);
            
        }else
        {
            cout << "Wrong Command, Please key in again." << endl;
        }
        
        
        cout << endl;
        /*
         while(!input_file.eof())
         {
         input_file.get(a);
         output_file.put(a);
         
         }
         */

        input_file.close();     //關閉檔案
        output_file.close();
    }
    

    
    

    
    
    return 0;
}
