// BlockChain.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#define NOMINMAX
#include <windows.h>

#include "..\..\C++ Libs\bigint\bigint.cpp"
#include "..\..\C++ Libs\hashlib2plus\trunk\src\hl_sha256.cpp"
#include "..\..\C++ Libs\hashlib2plus\trunk\src\hl_sha256wrapper.cpp"
#include <chrono>
#include <cmath>
#include <codecvt>
#include <ctime>
#include <fstream>
#include <future>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <sstream>
#include <stdio.h> 
#include <stdlib.h> 
#include <string>
#include <thread>
#include <vector>
//#include "C:\Users\zniri\Desktop\Coding\Languages\C++\Projects\ThreadLoopPool\ThreadLoopPool.h"
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

#else
#define LOG(x) cout << x << endl
#define Loading
#define StartTM
#define Cls

#endif // DEBUG


using namespace std;
using namespace literals::chrono_literals;
//using namespace hashlibpp;




// Copilot-Explain: Global worker count used by mining; initialized from CPU core count.
unsigned short max_threads = (short)thread::hardware_concurrency();


class Block
{
    public:
        // Copilot-Explain: Sequential index of the block in this in-memory chain.
        int blockNo = 0;
        // Copilot-Explain: User payload for the block.
        string data;
        // Copilot-Explain: Pointer to the next block (singly linked list style).
        Block* next;
        // Copilot-Explain: Proof-of-work value found by mining.
        unsigned long long nonce = 0;
        // Copilot-Explain: Hash of the previous block to connect the chain.
        string previous_hash = "0";
        // Copilot-Explain: Creation time captured when the block object is constructed.
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
            // Copilot-Explain: Converts text to a deterministic numeric string by joining UTF-16 code units.
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
            // Copilot-Explain: Creates a SHA-256 wrapper and hashes block fields + nonce.
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
            // Copilot-Explain: Pretty-print helper for debug output of a mined block.

            // Copilot-Explain: Convert the block's raw Unix timestamp to `std::time_t`
            // so it can be transformed into a human-readable local date/time.
            std::time_t ts = static_cast<std::time_t>(b.timestamp);

            // Copilot-Explain: Zero-initialize the destination `tm` structure that will hold
            // the broken-down local time components (year, month, day, hour, etc.).
            std::tm tm_buf{};

            // Copilot-Explain: Use the secure CRT function to convert `ts` into local time
            // and write the result into `tm_buf`.
            localtime_s(&tm_buf, &ts);

            // Copilot-Explain: Build a formatted timestamp string for display output.
            std::ostringstream timeText;

            // Copilot-Explain: Format as `YYYY-MM-DD HH:MM:SS` using stream manipulators.
            timeText << std::put_time(&tm_buf, "%Y-%m-%d %H:%M:%S");

            return out << "Block Hash: " + b.hash(b.nonce)
                       << "\nBlockNo: " << b.blockNo
                       << "\nBlock Data: " << b.data
                       << "\nHashes: " << b.nonce
                       << "\nTimeStamp: " << timeText.str()
                       << "\n--------------";
        }
};

class Blockchain
{
    // Copilot-Explain: Mutex used to protect shared state when multiple mining threads race.
    mutex mu;
    // Copilot-Explain: Mining difficulty; larger value means stricter target.
    int diff = 0;
    // Copilot-Explain: Shared flag set when any thread finds a valid nonce.
    bool found;
    // Copilot-Explain: Total nonce search space (2^32).
    unsigned long long maxNonce = pow(2 ,32);
    // Copilot-Explain: Per-thread nonce chunk size.
    unsigned long scan_num = maxNonce / max_threads;
    // Copilot-Explain: Mining target threshold represented as big integer.
    RossiBigInt target = bi_pow(2,(256 - diff));
    // Copilot-Explain: Current tail block of the chain; starts with Genesis.
    Block block = Block("Genesis");
    // Copilot-Explain: Snapshot copy of the initial block; not actively used later.
    Block head = block;
    
    public:
        void add(Block& block)
        {
            //cout << "~~~ adding ~~~\n";
            // Copilot-Explain: Link new block to current tail by storing the previous block hash.
            block.previous_hash = this->block.hash(block.nonce);
            //cout << "A_ph -> " << block.previous_hash << endl;
            // Copilot-Explain: Increment block height from previous tail.
            block.blockNo = this->block.blockNo + 1;

            // Copilot-Explain: Attach and move chain tail forward.
            this->block.next = &block;
            this->block = (*this->block.next);

        }
        void change_diff(int new_diff)
        {
            // Copilot-Explain: Recompute mining target whenever difficulty changes.
            this->diff = new_diff;
            target = bi_pow(2, (256 - diff));
        }

        static RossiBigInt bi_pow(int a, int b)
        {
            
            // Copilot-Explain: Fast path for small results that fit into native range.
            double p = pow(a, b);
            if (p <= ULONG_MAX)
            {
                return RossiBigInt(p);
            }
            else
            {
                // Copilot-Explain: Big-int fallback for large powers used in hash target math.
                RossiBigInt ret(a);
                RossiBigInt temp(a);
                for (size_t i = 1; i < b; i++)
                {
                    ret = ret*temp;
                }
                return ret;
            }
        }
        
        static RossiBigInt hex_to_int(const string& hex , int portion)
        { // portion needs to be devided by the base (16 for hex)
            // Copilot-Explain: Converts a hexadecimal hash string into a big integer by chunks.
            RossiBigInt val(0);
            string r_hex = hex;
            reverse(r_hex.begin(), r_hex.end());
            string t_str;
            for (size_t i = 0; i <= hex.length() - portion; i+=portion)
            {
                //cout << r_hex << endl << "I -> " << i << endl << "Portion -> " << portion<< "\n"<< r_hex.substr(i, portion) << endl;
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
            // Copilot-Explain: Keep thread count valid before recomputing per-thread range size.
            if (max_threads == 0)
            {
                max_threads = 1;
            }

            // Copilot-Explain: Recalculate worker scan window from the latest max_threads value.
            this->scan_num = this->maxNonce / max_threads;

            // Copilot-Explain: Reset shared success flag before launching a new mining round.
            this->found = false;
            //string h;
            // Copilot-Explain: Keep futures alive so launched async jobs are not destroyed immediately.
            vector<future<void>> fl;
            fl.reserve(max_threads);

            // Copilot-Explain: Each worker scans a different nonce range segment.
            for (size_t i = 0; i < max_threads; i++)
            {
                fl.emplace_back(async(launch::async, [this](unsigned long range, Block block)
                    { this->mine(range, block);}, i * scan_num, block));
            }
            
        }
};



//static ThreadLoopPool<void(*)(RossiBigInt, Block), RossiBigInt, Block> tp;

int main()
{

    //Block t("t");

    //cout << t << endl;

    //cout << Blockchain::hex_to_int("9876543210", 5) << endl;

    //return 0;


    //ThreadLoopPool<> tlp;

    static Blockchain bc;

    //int range = 100;
    //auto l = ([](int range) { bc.mine(range, Block("bb"));});

    // Copilot-Explain: Forces Windows console output to UTF-8 (needed for block characters).
    SetConsoleOutputCP(65001);

    StartTM

    double num;
    cout << "Type a number, try to get 100% CPU: ";
    cin >> num;
    
    // Copilot-Explain: Reconfigure workers to one task per hardware thread for this run.
    max_threads = (short)thread::hardware_concurrency() * num;

    printf("This is Thest4.\nPlease look at the Task Manager while the test is loadig, and report back to me what's the CPU percent usage.\nThanks in advanse!\n\nPress ENTER to start.");

    cin.ignore();

    //vector<future<void>> fl;
    //fl.reserve(max_threads);

    //for (size_t i = 0; i < max_threads; i++)
    //{
    //    fl.push_back(async(launch::async, blockchain::func, i * scan_num));
    //}

    system("cls");


    // Copilot-Explain: Difficulty 8 means a relatively easy target for demonstration.
    bc.change_diff(11); //11
    constexpr int load_bar_size = 12; //12
    char load_character[] = "███";

    short* cpuUsage;
    short user;
    short sys;

	int beginning = GetTickCount();
    // Copilot-Explain: Mine 12 blocks sequentially; each round itself is multi-threaded.
    for (size_t i = 0; i < load_bar_size; i++)
    {
        Loading

        bc.start_mining(Block("Block Number " + to_string(i + 1)));
        //cout << "\n\n\n==== UOS ====\n\n\n";
    }
    
    Cls

    int end = GetTickCount();
    unsigned long long elapsed = static_cast<unsigned long long>(end - beginning);

    cout << "\nElapsed time: " << elapsed << " ms";

    if (elapsed >= 1000ULL)
    {
        double seconds = elapsed / 1000.0;
        cout << " (" << fixed << setprecision(3) << seconds << " s";

        if (seconds >= 60.0)
        {
            double minutes = seconds / 60.0;
            cout << ", " << minutes << " min";

            if (minutes >= 60.0)
            {
                double hours = minutes / 60.0;
                cout << ", " << hours << " h";

                if (hours >= 24.0)
                {
                    double days = hours / 24.0;
                    cout << ", " << days << " d";
                }
            }
        }

        cout << ")" << defaultfloat;
    }

    cout << "\n";
    cout << "\nNo err!\n";
    printf("Thanks for helping! Don't forget to tell me te resltes.");
    cin.ignore();


}

template<typename RangeType>
void Blockchain::mine(RangeType range, Block block)
{
    // Copilot-Explain: Initialize with the first hash candidate in this thread's range.
    RossiBigInt th = hex_to_int(block.hash(range), 8);

    // Copilot-Explain: Scan this worker's nonce window until a solution is found globally.
    for (RangeType n = range; n <= scan_num + range && !this->found; n++)
    {
        //cin.get();
        //this_thread::sleep_for(1s);
        //cout << "=== Thread " << tid << " ===\n"<<"th     -> " << th << endl 
        //    << "target -> " << this->target << endl 
        //    << "found: " << noboolalpha << (this->found) << endl;
        if (th <= this->target && !this->found)
        {
            if (mu.try_lock())
            {
                // Copilot-Explain: Re-check after locking so only one winner writes shared chain state.
                if (!this->found)
                {
                    //cout << "HERE!!!\n";
                    // Copilot-Explain: Winner thread marks completion, records nonce, and appends block.
                    this->found = true;
                    block.nonce = n;
                    this->add(block);
                    //cout << "=== Thread " << tid << " ===\n" "After found: " << noboolalpha << (this->found) << endl;
                    LOG(block);
                    mu.unlock();
                    //cout << "HERE 2\n";
                    break;
                }

                mu.unlock();
            }
        }
        else
        {
            // Copilot-Explain: Try next nonce candidate and compare again in next loop iteration.
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
