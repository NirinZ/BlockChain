// BlockChain.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include "C:\Users\zniri\Desktop\Coding\Languages\C++\C++ Libs\bigint\bigint.cpp"
#include "C:\Users\zniri\Desktop\Coding\Languages\C++\C++ Libs\hashlib2plus\trunk\src\hl_sha256.cpp"
#include "C:\Users\zniri\Desktop\Coding\Languages\C++\C++ Libs\hashlib2plus\trunk\src\hl_sha256wrapper.cpp"
#include <chrono>
#include <cmath>
#include <codecvt>
#include <ctime>
#include <fstream>
#include <future>
#include <iostream>
#include <mutex>
#include <stdio.h> 
#include <stdlib.h> 
#include <string>
#include <thread>
#include <vector>
#include <windows.h>
#include "C:\Users\zniri\Desktop\Coding\Languages\C++\Projects\ThreadLoopPool\ThreadLoopPool.h"
#pragma execution_character_set( "utf-8" )

#ifdef _DEBUG
#define LOG(x) cout << x << endl
#define Loading
#define StartTM
#define Cls 
#elif NDEBUG
#define LOG(x)
#define Loading system("cls"); printf("LOADING ["); for (int j = 0; j < i; j++) printf("%s", load_character); for (int j = 0; j < load_bar_size - i - 1; j++) printf("   "); puts("]");
#define StartTM system("start Taskmgr.exe");
#define Cls system("cls");

#endif // DEBUG


using namespace std;
using namespace literals::chrono_literals;
//using namespace hashlibpp;




unsigned short max_threads = (short)thread::hardware_concurrency() * 20;


class Block
{
    public:
        int blockNo = 0;
        string data;
        Block* next;
        unsigned long long nonce = 0;
        string previous_hash = "0";
        int timestamp = time(NULL);

        Block()
        {
            this->data = "NULL";
        }

        Block(string data)
        {
            this->data = data;
        }

        static string phash(string a)
        {
            string b;
            std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> utf16conv;
            std::u16string utf16 = utf16conv.from_bytes(a);
            for (char16_t c : utf16)
            {
                b += to_string(c);
            }
            return b;
        }

        string hash(unsigned long long Nonce)
        {
            //cout << "PrevH -> " << this->previous_hash << endl;
            hashwrapper* myWrapper = new sha256wrapper();

            string hash1 = myWrapper->getHashFromString(
            phash(to_string(this -> blockNo)) +
            phash(this -> data) +
            phash(to_string(Nonce)) +
            phash(this -> previous_hash) +
            phash(to_string(this -> timestamp))
            );
            return hash1;
        }

        friend ostream& operator << (ostream& out, Block& b)
        {
            //cout << "=== printing ===\n";
            //cout << "P_rh -> " << b.previous_hash << endl;
            return out << "Block Hash: " + b.hash(b.nonce) + "\nBlockNo: " << b.blockNo << "\nBlock Data: " << 
                b.data << "\nHashes: " << b.nonce << "\n--------------";
        }
};

class Blockchain
{
    mutex mu;
    int diff = 0;
    bool found;
    unsigned long long maxNonce = pow(2 ,32);
    unsigned long scan_num = maxNonce / max_threads;
    RossiBigInt target = bi_pow(2,(256 - diff));
    Block block = Block("Genesis");
    Block head = block;
    
    public:
        void add(Block& block)
        {
            //cout << "~~~ adding ~~~\n";
            block.previous_hash = this->block.hash(block.nonce);
            //cout << "A_ph -> " << block.previous_hash << endl;
            block.blockNo = this->block.blockNo + 1;

            this->block.next = &block;
            this->block = (*this->block.next);

        }
        void change_diff(int new_diff)
        {
            this->diff = new_diff;
            target = bi_pow(2, (256 - diff));
        }

        static RossiBigInt bi_pow(int a, int b)
        {
            
            double p = pow(a, b);
            if (!(p > ULONG_MAX))
            {
                return RossiBigInt(p);
            }
            else
            {
                RossiBigInt ret(a);
                RossiBigInt temp(a);
                for (size_t i = 1; i < b; i++)
                {
                    ret = ret*temp;
                };
                return ret;
            }
        }
        
        static RossiBigInt hex_to_int(const string& hex , int portion)
        {
            RossiBigInt val(0);
            string r_hex = hex;
            reverse(r_hex.begin(), r_hex.end());
            string t_str;
            RossiBigInt temp;
            for (size_t i = 0; !(i > hex.length() - portion); i+=portion)
            {
                //cout << r_hex << endl << "I -> " << i << endl << "Portion -> " << portion<< "\n"<< r_hex.substr(i, i + portion) << endl;
                t_str = r_hex.substr(i, portion);
                reverse(t_str.begin(), t_str.end());
                RossiBigInt temp(stoul(t_str, nullptr, 16));
                val = val + temp * bi_pow(16,i);
            }


            return val;
        }

        template<typename RangeType>
        void mine(RangeType range, Block block);

        void start_mining(Block block)
        {
            this->found = false;
            //string h;
            vector<future<void>> fl;
            fl.reserve(max_threads);

            for (size_t i = 0; i < max_threads; i++)
            {
                fl.push_back(async(launch::async, [this](unsigned long range, Block block)
                    { this->mine(range, block);}, i * scan_num, block));
            }
            
        }
};



//static ThreadLoopPool<void(*)(RossiBigInt, Block), RossiBigInt, Block> tp;

int main()
{

ThreadLoopPool<> tp; // ** Make this a comment to make the project work! **
static Blockchain bc;
//int range = 100;
//auto l = ([](int range) { bc.mine(range, Block("bb"));});
    SetConsoleOutputCP(65001);

    StartTM

    int num;
    cout << "Type a number, try to get 100% CPU: ";
    cin >> num;
    
    max_threads = (short)thread::hardware_concurrency() * 1;

    printf("This is Thest4.\nPlease look at the Task Manager while the test is loadig, and report back to me what's the CPU percent usage.\nThanks in advanse!\n\nPress ENTER to start.");

    cin.ignore();

    //vector<future<void>> fl;
    //fl.reserve(max_threads);

    //for (size_t i = 0; i < max_threads; i++)
    //{
    //    fl.push_back(async(launch::async, blockchain::func, i * scan_num));
    //}

    system("cls");


    bc.change_diff(5); //11
    constexpr int load_bar_size = 12; //12
    char load_character[] = "███";

    short* cpuUsage;
    short user;
    short sys;

    for (size_t i = 0; i < load_bar_size; i++)
    {
        Loading

        bc.start_mining(Block("Block Number " + to_string(i + 1)));
        //cout << "\n\n\n==== UOS ====\n\n\n";
    }

    Cls

    cout << "\nNo err!\n";
    printf("Thanks for helping! Don't forget to tell me te resltes.");
    cin.ignore();


}

template<typename RangeType>
void Blockchain::mine(RangeType range, Block block)
{
    RossiBigInt th = hex_to_int(block.hash(range), 8);

    for (RangeType n = range; !(n > scan_num + range) && !this->found; n++)
    {
        //cin.get();
        //this_thread::sleep_for(1s);
        //cout << "=== Thread " << tid << " ===\n"<<"th     -> " << th << endl 
        //    << "target -> " << this->target << endl 
        //    << "found: " << noboolalpha << (this->found) << endl;
        if (!(th > this->target) && !this->found)
        {
            //cout << "HERE!!!\n";
            mu.try_lock();
            this->found = true;
            block.nonce = n;
            this->add(block);
            //cout << "=== Thread " << tid << " ===\n" "After found: " << noboolalpha << (this->found) << endl;
            LOG(block);
            mu.unlock();
            //cout << "HERE 2\n";
            break;
        }
        else
        {
            th = hex_to_int(block.hash(n), 8);
        }
    }
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
