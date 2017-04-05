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

// *********************************************** //
// This cpp code need to use -std=c++11 to compile //
// *********************************************** //

using namespace std;


//讀取file以bit流的形式”1010110001“
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


//把string("10101011110") 輸入進去stream裡面 因為沒辦法使用一個一個bit寫入 所以將8個bit放入一個byte在放入檔案
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


//把從檔案讀入的字 計算其數量（因為其實最多只會有256種組合在一個bit 所以其實只會有256個）
struct frequencyTable{
    unsigned char byte;     //字母
    long weight;            //出現次數
    /*
    frequencyTable(){}
    frequencyTable(unsigned char set_byte, long set_weight)
    {
        this->byte = set_byte;
        this->weight = set_weight;
    }
     */
};

//把前面的 frequencyTable 做成vector
typedef vector<frequencyTable> frequency_table;


//檢查frequencyTable其中一個字母的出現數量
bool is_empty_frequency(frequencyTable const &tb)
{
    return tb.weight==0;
}

//後面sort可以使用此compare function進行出現數量的比較
bool frequency_comp(frequencyTable const &a, frequencyTable const &b)
{
    return a.weight < b.weight;
}

//製作frequencyTable的主function
frequency_table make_frequency_table(istream &input, ofstream &output_file){
    frequency_table table;
    frequency_table result_table;
    
    unsigned char byte = '\0';
    
    //在frequencyTable中插入256個單字，讓每一個單字的出現次數預設為0
    for(int i=0;i<256;++i){
        frequencyTable temp_iter={static_cast<unsigned char>(i),0};
        table.push_back(temp_iter);
    }
    
    
    //開始讀文件 並且計算每一個字的出現次數
    while(input){
        if(!input.read(reinterpret_cast<char*>(&byte),sizeof(byte))){
            break;
        }
        ++(table[byte].weight);
    }
    
    //先將最後一個字寫入output的標頭檔 因為最後在存儲bitstream時 有可能因為最後一組bitstream再存入檔案時未滿8組
    //假設最後一個byte a = "10101"
    //而最後一組bitstream可能為 101010000 後面四個為空白為了湊滿一個byte寫入file
    //但這樣會導致解壓縮時 會因為多了0000輸出一個新的字母在結尾 所以把最後一個字記錄下來以確保結尾點
    output_file << byte;
    
    //將出現次數為0的字母刪掉 只留下有出現過的字做frequencyTable
    remove_copy_if(table.begin(),table.end(),back_inserter(result_table),is_empty_frequency);
    
    //將剩下的字依照出現次數進行sort
    sort(result_table.begin(), result_table.end(), frequency_comp);
    
    return result_table;
}

//製作huffmantree的class
//Huffman Tree 的大致樣子
//       0  1  2  3  4  5  6  7  8  9  10  11 ...
// 字母   B  C  D  A  E
// 權重   1  2  3  8  10 3  6
// used  1  1  1  0  0  1  0  ...
// Left  -1 -1 -1 -1 -1 0  2  ...
// Right -1 -1 -1 -1 -1 1  5  ...
// Parent 5 5  6  -1 -1 6 -1 ...
class HuffmanTree
{
public:
    HuffmanTree(frequency_table frequency_table, long N);
    HuffmanTree(long N);
    vector<string> huffman_code(long len);      //將全部的字依照huffman tree輸出成huffman code並且以vector的形式存起來
    
    long accumlate_nodes;       //此為儲存實際有權重的結點數
    long total_nodes;           //此為建造tree所需的全部的結點數
    typedef struct HuffmanNode  //huffman node所需要的基本資訊
    {
        bool used;              //記錄此節點是否已經被放入tree中
        long weight;            //記錄此節點的權重 或是記錄此節點左右支權重的總和
        long left_child, right_chlid, parent;
    } *h_node;
    h_node huffman;
    
    //製作tree需要不斷找最小的兩個node出來建樹
    frequency_table choose_min_nodes(frequency_table frequency_table,long *min1,long *min2);

};

//將根據frequency_table的最小的兩個（理論上為0,1 因為有sort）然後尋找是在huffmanTree的哪個位置
frequency_table HuffmanTree::choose_min_nodes(frequency_table frequencyTables, long *min1_pos,long *min2_pos){
    
    
    for(long index=0; index<total_nodes; index++)
    {
        //找frequency_table最小的那個權重（第0個）是在huffman tree index中的哪一個
        if(huffman[index].used == 0 && frequencyTables[0].weight == huffman[index].weight)
        {
            *min1_pos = index;
            huffman[index].used = 1;       //在huffman tree index標註其已經被放入tree了
            break;
        }
    }
    
    for(long index=0; index<total_nodes; index++)
    {
        //找frequency_table第2小的那個權重（第1個）是在huffman tree index中的哪一個
        if(huffman[index].used == 0 && index != *min1_pos && frequencyTables[1].weight == huffman[index].weight)
        {
            *min2_pos = index;
            huffman[index].used = 1;
            break;
        }
    }
    
    //把frequencyTables中最小的兩個（第0,1個）從table中刪除
    frequencyTables.erase(frequencyTables.begin(),frequencyTables.begin()+2);
    
    //將新產生的節點加入table中 好讓之後可以進行排序
    frequencyTable temp{'0', huffman[*min1_pos].weight + huffman[*min2_pos].weight};
    frequencyTables.push_back(temp);
    
    //再重新進行一次排序
    sort(frequencyTables.begin(), frequencyTables.end(), frequency_comp);
    
    return frequencyTables;
    
    
}

//進行建樹
HuffmanTree::HuffmanTree(frequency_table frequencyTables, long N){
    
    this->accumlate_nodes = N;      //權重節點
    this->total_nodes = 2*N - 1;    //所有節點
    this->huffman = new HuffmanNode[total_nodes];
    
    for(long i=0; i<total_nodes; i++)
    {
        if(i < accumlate_nodes)
            huffman[i].weight = frequencyTables[i].weight;  //把權重節點的權重寫入
        else
            huffman[i].weight = -1;     //剩下後面的節點先預設01
        
        //其他值先預設-1
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
        
        //從frequencyTables找出最小跟第二小的節點位置
        frequencyTables = choose_min_nodes(frequencyTables, &min1_pos, &min2_pos);
        
        //設定最小的第二小的節點parent為i
        huffman[min1_pos].parent = i;
        huffman[min2_pos].parent = i;
        
        //設定i的左子.右子為最小跟第二小的節點 並計算其權重總和
        huffman[i].left_child = min1_pos;
        huffman[i].right_chlid = min2_pos;
        huffman[i].weight = huffman[min1_pos].weight + huffman[min2_pos].weight;
    }

}

//此constructor 是為了decode時存儲tree資訊使用
HuffmanTree::HuffmanTree(long N)
{
    this->accumlate_nodes = N;  //權重節點
    this->total_nodes = 2*N - 1;
    this->huffman = new HuffmanNode[total_nodes];
}

//將所有的huffman code生產出來根據huffman tree
vector<string> HuffmanTree::huffman_code(long len){
    
    long current = 0;
    long parent = 0;
    
    //記錄所從有的huffman code
    vector<string> huffman_code = vector<string>(len); //two-dim array
    
    //從最底層的權重節點trace上去 再將得到的string反轉即為huffman code
    for (long i = 0; i < len; i++){
        string temp = "";
        current = i;    //第幾個權重點
        parent = huffman[current].parent;   //trace上面的parent
        while (parent >= 0)  //當parent小於0（即為-1）代表已經到最上層 所以trace結束
        {
            if (current == huffman[parent].left_child)  //左子為0 右子為1
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
        reverse(temp.begin(), temp.end());      //反轉string
        huffman_code[i] = temp;                 //存入huffman_code table
    }
    
    return huffman_code;
    
}


//進行compressing的main function
void encoded(frequency_table f_table, ifstream &input_file, ofstream &output_file)
{
    vector<string> huffman_code;
    HuffmanTree h_tree( f_table, f_table.size());
    huffman_code = h_tree.huffman_code(f_table.size());
    
    string encode_bitstream = "";
    char file_byte;
    
    //因為前面建frequency_table已經讀過一次檔案 所以必須重新設定檔案指標
    input_file.clear();  // 重置 eof
    input_file.seekg(0);  // get 指標移至檔案首
    
    //開始讀檔並且根據huffman_code把字母轉成bitstream
    //因為f_table 跟 huffman_code index 的順序皆為由小到大 所以可以直接透過f_table比對byte再將其index給huffman_code
    while(input_file.get(file_byte))
    {
        for (int i = 0; i<f_table.size(); i++) {
                if (f_table.at(i).byte == static_cast<unsigned char>(file_byte))
                    encode_bitstream += huffman_code[i];
        }
    }
    
    //開始寫入壓縮過後的資訊
    //內容如下
    //header
    // 1.最後一個byte的資訊（前面建frequency_table輸入）
    // 2.權重節點的數量
    // 3.權重節點的byte為何
    // 4.權重節點之後的節點 其left_child, right_child 資訊（即huffman tree資訊）
    //content
    // 轉換完成的bitstream 以bit的方式寫數output
    

    char actual_nodes = static_cast<unsigned char>(h_tree.accumlate_nodes - 1);
    output_file << actual_nodes;
    
    
    //寫入3,4的資訊
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
    
    //寫入content資訊 透過writeBitString function
    writeBitString(output_file, encode_bitstream);
    
}

//此為decompressing的main function
void decoded(ifstream &input_file, ofstream &output_file)
{
    int actual_nodes;
    int total_nodes;
    unsigned char last_word;
    
    //開始根據前面壓縮的規格將資訊解析出來
    //為何需要最後一個字? 因為最後在存儲bitstream時 有可能因為最後一組bitstream再存入檔案時未滿8組
    //假設最後一個byte 為 a = "10101"
    //而最後一組bitstream可能為 101010000 後面四個為空白為了湊滿一個byte寫入file
    //但這樣會導致解壓縮時 會因為多了0000輸出一個新的字母在結尾 所以把最後一個字記錄下來以確保結尾點
    char hader_byte;
    input_file.get(hader_byte);
    last_word = static_cast<unsigned char>(hader_byte);
    
    input_file.get(hader_byte);
    actual_nodes = static_cast<unsigned char>(hader_byte) + 1;
    total_nodes = actual_nodes*2 - 1;
    
    HuffmanTree h_decode_tree(actual_nodes);
    char decode_table[actual_nodes];
    
    
    //將壓縮檔內的權重節點byte 與之後節點的tree資訊讀出重新造樹
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
    
    //因為存入int後面多一個空白
    char temp_space;
    input_file.get(temp_space);
    
    //開始解析huffman_code將其轉換成string
    string huffman_code;
    string decode_words = "";
    huffman_code = readBitString(input_file);
    long node_pos = total_nodes - 1;
    
    //從tree的最後一個節點 根據tree走下去 當走到最後沒有left right child的時候 就是我們要的byte
    for(long index=0; index <= huffman_code.length(); index++)
    {
        //最後沒有child的時候就將其放入decode_words
        if(h_decode_tree.huffman[node_pos].left_child == -1 || h_decode_tree.huffman[node_pos].right_chlid == -1)
        {
            decode_words += static_cast<unsigned char>(decode_table[node_pos]);
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
    

    //開始進行最後一個byte的判斷 如果最後一個byte不是所預期的那個（代表bitstream多解析一個）就將其刪除 最後正確的decode_words再存入output_file
    for(long index=decode_words.length()-1; index>=0; index--)
    {
        if(static_cast<unsigned char>(decode_words[index]) == last_word)
        {
            output_file << decode_words;
            return ;
        }
        else
        {
            decode_words[index] = '\0';
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
        cout << "1.Compress" << endl;
        cout << "2.Decompress" << endl;
        cout << "3.Exit" << endl;
        cout << "Chose command: ";
        cin >> command;
        
        if(command == 3)
        {
            input_file.close();     //關閉檔案
            output_file.close();
            break;
        }
        
        //compress
        if(command == 1)
        {
            
            cout << "Compress" << endl;
            cout << "Input filename: ";
            cin >> filename;
            input_file.open(filename, ifstream::binary | ifstream::in); //將file以binary的模式讀取
            if(!input_file)
            {
                cout<<"Can't open the file" << endl << endl;
                continue;
            }

            cout << "Output filename: ";
            cin >> filename;
            output_file.open(filename, ofstream::binary | ofstream::out | ofstream::trunc);//將file以binary的模式讀出
            if(!output_file)
            {
                cout<<"Can't create the file" << endl << endl;
                continue;
            }
            
            
            /*
            input_file.open(argv[1], ifstream::binary | ifstream::in);
            output_file.open(argv[2], ofstream::binary | ofstream::out | ofstream::trunc);
            */
            
            cout << "Compressing..." << endl;
            
            /*---要計算的程式效率區域---*/
            struct timeval t_val;
            gettimeofday(&t_val, NULL);
            
            //進行壓縮
            encoded( make_frequency_table(input_file, output_file), input_file, output_file);
            
            
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
            cout << "Extract" << endl;
            cout << "Input filename: ";
            cin >> filename;
            input_file.open(filename, ifstream::binary | ifstream::in);
            if(!input_file)
            {
                cout<<"Can't open the file" << endl << endl;
                continue;
            }
            
            cout << "Output filename: ";
            cin >> filename;
            output_file.open(filename, ofstream::binary | ofstream::out | ofstream::trunc);
            if(!output_file)
            {
                cout<<"Can't create the file" <<endl << endl;
                continue;
            }
            
            
            /*
            input_file.open(argv[2], ifstream::binary | ifstream::in);
            output_file.open(argv[3], ofstream::binary | ofstream::out | ofstream::trunc);
            */
            
            cout << "Decompressing..." << endl;
            
            /*---要計算的程式效率區域---*/
            struct timeval t_val;
            gettimeofday(&t_val, NULL);
            
            //進行解壓縮
            decoded(input_file, output_file);
            
            
            //計時程式
            struct timeval t_val_end;
            gettimeofday(&t_val_end, NULL);
            struct timeval t_result;
            timersub(&t_val_end, &t_val, &t_result);
            double consume = t_result.tv_sec*1000 + (1.0 * t_result.tv_usec)/1000;
            
            cout << "Done." << endl;
            
            printf("Cost %f ms \n", consume);
            
        }
        
        //比對解壓縮之後的檔案是否為原檔
        else if(command == 4)
        {
            
            ifstream input_file2;      //fstream 讀入input_file
            cout << "Comparing" << endl;
            cout << "Input filename: ";
            cin >> filename;
            input_file.open(filename, ifstream::binary | ifstream::in);
            if(!input_file)
            {
                cout<<"Can't open the file"<<endl;
                continue;
            }
            
            cout << "Compare filename: ";
            cin >> filename;
            input_file2.open(filename, ifstream::binary | ifstream::in);
            if(!input_file2)
            {
                cout<<"Can't create the file"<<endl;
                continue;
            }
            
            
            
            /*
            ifstream input_file2;      //fstream 讀入input_file
            input_file.open(argv[1], ifstream::binary | ifstream::in);
            input_file2.open(argv[3], ifstream::binary | ifstream::in);
            */
            
            
            unsigned char temp1;
            unsigned char temp2;
            
            long count = 0;
            bool dif = 0;
           
            while( input_file >> temp1 )
            {
                
                count++;
                input_file2 >> temp2;
                
                if( temp1 != temp2 )
                {
                    cout << "At:: "<< count << endl;
                    cout << "diff:: " << temp1 << " " << temp2 << endl;
                    dif = 1;
                }
                
            }
            
            if(dif == 0)
            {
                cout << "It's a same file." << endl;
            }
            
            
            
            
            
        }
        else
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
