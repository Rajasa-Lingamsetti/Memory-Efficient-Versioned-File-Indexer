#include <iostream>
#include <fstream>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <cctype>
#include <stdexcept>
#include <chrono>

using namespace std;

const int BUFFER_SIZE = 512;

// Template example
template <typename T>
void printVector(vector<T> v)
{
    for(auto &x : v)
        cout << x << " ";
    cout << endl;
}

class Tokenizer
{
public:
    string toLower(string word)
    {
        for(char &c : word)
            c = tolower(c);
        return word;
    }
};

class BufferedFileReader
{
public:
    unordered_map<string,int> readFile(string filename)
    {
        unordered_map<string,int> freq;

        ifstream file(filename);

        if(!file)
            throw runtime_error("File cannot be opened");

        Tokenizer tokenizer;
        string word;

        while(file >> word)
        {
            word = tokenizer.toLower(word);
            freq[word]++;
        }

        file.close();

        return freq;
    }
};

class Indexer
{
public:
    unordered_map<string,int> buildIndex(string filename)
    {
        BufferedFileReader reader;
        return reader.readFile(filename);
    }
};

class Query
{
public:
    virtual void execute() = 0;
};

class WordQuery : public Query
{
    string filename;
    string word;
    string version;

public:
    WordQuery(string f,string w,string v)
    {
        filename = f;
        word = w;
        version = v;
    }

    void execute()
    {
        Indexer indexer;
        Tokenizer tokenizer;

        auto freq = indexer.buildIndex(filename);

        word = tokenizer.toLower(word);

        cout << "Version: " << version << endl;

        cout << "\nWord Count Query\n";

        if(freq.find(word)!=freq.end())
            cout << "Frequency of " << word << " = " << freq[word] << endl;
        else
            cout << "Word not found\n";
    }
};

class TopKQuery : public Query
{
    string filename;
    int k;
    string version;

public:
    TopKQuery(string f,int val,string v)
    {
        filename = f;
        k = val;
        version = v;
    }

    void execute()
    {
        Indexer indexer;

        auto freq = indexer.buildIndex(filename);

        vector<pair<string,int>> vec(freq.begin(),freq.end());

        sort(vec.begin(),vec.end(),
        [](pair<string,int> &a,pair<string,int> &b)
        {
            return a.second>b.second;
        });

        cout << "Version: " << version << endl;

        cout << "\nTop " << k << " Words\n";

        for(int i=0;i<k && i<vec.size();i++)
            cout << vec[i].first << " : " << vec[i].second << endl;
    }
};

class DiffQuery : public Query
{
    string file1,file2,word;
    string version1,version2;

public:
    DiffQuery(string f1,string f2,string w,string v1,string v2)
    {
        file1 = f1;
        file2 = f2;
        word = w;
        version1 = v1;
        version2 = v2;
    }

    void execute()
    {
        Indexer indexer;
        Tokenizer tokenizer;

        auto freq1 = indexer.buildIndex(file1);
        auto freq2 = indexer.buildIndex(file2);

        word = tokenizer.toLower(word);

        int c1 = freq1[word];
        int c2 = freq2[word];

        cout << "Version1: " << version1 << endl;
        cout << "Version2: " << version2 << endl;

        cout << "\nDifference Query\n";

        cout << "File1 count = " << c1 << endl;
        cout << "File2 count = " << c2 << endl;

        cout << "Difference = " << c2 - c1 << endl;
    }
};

// Function overloading example
void printResult(string word,int count)
{
    cout << word << " : " << count << endl;
}

void printResult(string word,int c1,int c2)
{
    cout << word << " difference = " << c2 - c1 << endl;
}

int main(int argc, char* argv[])
{
    try
    {
        auto start = chrono::high_resolution_clock::now();

        string file,file1,file2,word;
        string version,version1,version2;
        string query;

        int buffer = BUFFER_SIZE;
        int topk = 5;

        for(int i=1;i<argc;i++)
        {
            string arg = argv[i];

            if(arg=="--file") file = argv[++i];
            else if(arg=="--file1") file1 = argv[++i];
            else if(arg=="--file2") file2 = argv[++i];
            else if(arg=="--word") word = argv[++i];
            else if(arg=="--version") version = argv[++i];
            else if(arg=="--version1") version1 = argv[++i];
            else if(arg=="--version2") version2 = argv[++i];
            else if(arg=="--query") query = argv[++i];
            else if(arg=="--buffer") buffer = stoi(argv[++i]);
            else if(arg=="--top") topk = stoi(argv[++i]);
        }

        Query* q = nullptr;

        if(query=="word")
            q = new WordQuery(file,word,version);

        else if(query=="top")
            q = new TopKQuery(file,topk,version);

        else if(query=="diff")
            q = new DiffQuery(file1,file2,word,version1,version2);

        else
            throw runtime_error("Invalid query");

        cout << "Buffer Size: " << buffer << " KB\n\n";

        q->execute();

        auto end = chrono::high_resolution_clock::now();

        chrono::duration<double> duration = end - start;

        cout << "\nExecution Time: " << duration.count() << " seconds\n";

        delete q;
    }
    catch(exception &e)
    {
        cout << "Error: " << e.what() << endl;
    }

    return 0;
}