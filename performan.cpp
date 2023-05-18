/**
 * @brief	T树性能测试
 * @author	huangxx
*/

#include "ttree.h"
#include <string.h>
#include <stdio.h>
#include <thread>

#include <iostream>
#include <map>

int g_sleepMs = 2;

unsigned int g_keySize = 32;

struct Record
{
    int pk;

    int index1;

    int index2_A;
    char index2_B[256];

    char index3[128];

    int index4;
};

int fnPkComparator(const void* a, const void* b)
{
    return ((const Record*)a)->pk - ((const Record*)b)->pk;
}

int fnIndex1Comparator(const void* a, const void* b)
{
    return ((const Record*)a)->index1 - ((const Record*)b)->index1;
}

int fnIndex2Comparator(const void* a, const void* b)
{
    int rc = ((const Record*)a)->index2_A - ((const Record*)b)->index2_A;

    if(rc != 0)
        return rc;

    return strncmp(((const Record*)a)->index2_B, ((const Record*)b)->index2_B, sizeof(Record::index2_B));
}

int fnIndex3Comparator(const void* a, const void* b)
{
    return strncmp(((const Record*)a)->index3, ((const Record*)b)->index3, sizeof(Record::index3));
}

int fnIndex4Comparator(const void* a, const void* b)
{
    return ((const Record*)a)->index4 - ((const Record*)b)->index4;
}



class TableOfRecord
{
    public:
        TableOfRecord(unsigned int keySize) : m_pk(fnPkComparator, true, keySize), m_index{{fnIndex1Comparator, false, keySize}, {fnIndex2Comparator, false, keySize}, {fnIndex3Comparator, false, keySize}, {fnIndex4Comparator, false, keySize}}
        {

        }

        int Insert(Record* pRecord)
        {
            int rc = m_pk.Insert(pRecord);

            if(rc != 0)
                return rc;

            for(int i = 0; i < 4; i++)
            {
                m_index[i].Insert(pRecord);
            }

            return 0;
        }

    private:
        TTree   m_pk;
        TTree   m_index[4];
};


struct pkComparator
{
    bool operator()(const Record* a, const Record* b)
    {
        return fnPkComparator(a, b);
    }
};

struct index1Comparator
{
    bool operator()(const Record* a, const Record* b)
    {
        return fnIndex1Comparator(a, b);
    }
};

struct index2Comparator
{
    bool operator()(const Record* a, const Record* b)
    {
        return fnIndex2Comparator(a, b);
    }
};

struct index3Comparator
{
    bool operator()(const Record* a, const Record* b)
    {
        return fnIndex3Comparator(a, b);
    }
};

struct index4Comparator
{
    bool operator()(const Record* a, const Record* b)
    {
        return fnIndex4Comparator(a, b);
    }
};


class TableOfRecordWithMap
{
    public:
        int Insert(Record* pRecord)
        {
            m_pk.insert(std::make_pair(pRecord, pRecord));

            m_index1.insert(std::make_pair(pRecord, pRecord));
            m_index2.insert(std::make_pair(pRecord, pRecord));
            m_index3.insert(std::make_pair(pRecord, pRecord));
            m_index4.insert(std::make_pair(pRecord, pRecord));

            return 0;
        }

    private:

        std::map<Record*, Record*, pkComparator>  m_pk;
        std::multimap<Record*, Record*, index1Comparator> m_index1;
        std::multimap<Record*, Record*, index2Comparator> m_index2;
        std::multimap<Record*, Record*, index3Comparator> m_index3;
        std::multimap<Record*, Record*, index4Comparator> m_index4;

};

void BenchInsertN(size_t n, unsigned int keySize)
{
    TableOfRecord table(keySize);
    TableOfRecordWithMap maps;

    //std::shared_ptr<int64_t[]> tableElapse(new int64_t[n]);
    //std::shared_ptr<int64_t[]>  mapElapse(new int64_t[n]);

    std::shared_ptr<Record[]> recordPtr(new Record[n]);

    int ratio = (int)(0.9 * n);

    for(size_t i = 0; i < n; i++)
    {
        recordPtr[i].pk = i;

        //索引重复率 10% 
        recordPtr[i].index1 = i % ratio;

        recordPtr[i].index2_A = i % ratio;
        strncpy(recordPtr[i].index2_B, "test test test", sizeof(recordPtr[i].index2_B));

        sprintf(recordPtr[i].index3, "test test %d", i % ratio);

        recordPtr[i].index4 = i % ratio;

    }

    auto begin = std::chrono::steady_clock::now().time_since_epoch().count();
    for(size_t i = 0; i < n; i++)
    {
        table.Insert(&recordPtr[i]);
    }
    auto end = std::chrono::steady_clock::now().time_since_epoch().count();
    
    int64_t elapse = end - begin;

    begin = std::chrono::steady_clock::now().time_since_epoch().count();
    for(size_t i = 0; i < n; i++)
    {
        maps.Insert(&recordPtr[i]);
    }
    end = std::chrono::steady_clock::now().time_since_epoch().count();

    std::cout << "record size    : " << n << std::endl;
    std::cout << "ttree key size : " << keySize << std::endl;
    std::cout << "table elase(us): " << elapse/1000 << std::endl;
    std::cout << "maps elase(us) : " << (end - begin)/1000 << std::endl << std::endl;
    
}

void CompareInsert()
{
    int step = 10000;
    for(int i = 10000; i < 10 * step; i += step)
    {
        BenchInsertN(i, g_keySize);
    }

    step *= 10;
    for(int i = 100000; i < (1 << 8) * step; i *= 2)
    {
        BenchInsertN(i, g_keySize);
    }
}

/**
 * 测试存量记录的情况下插入单笔的绝对耗时
*/
void BenchTableInsert(unsigned int overCount)
{
    TableOfRecord table(g_keySize);
    TableOfRecordWithMap maps;


    std::shared_ptr<Record[]> recordPtr(new Record[overCount + 100]);

    int ratio = (int)(0.9 * (overCount + 100));

    for(size_t i = 0; i < overCount + 100; i++)
    {
        recordPtr[i].pk = i;

        //索引重复率 10% 
        recordPtr[i].index1 = i % ratio;

        recordPtr[i].index2_A = i % ratio;
        strncpy(recordPtr[i].index2_B, "test test test", sizeof(recordPtr[i].index2_B));

        sprintf(recordPtr[i].index3, "test test %d", i % ratio);

        recordPtr[i].index4 = i % ratio;

    }

    for(size_t i = 0; i < overCount; i++)
    {
        table.Insert(&recordPtr[i]);
    }

    int64_t tableElpase[100], mapElapse[100];

    int64_t begin;
    for(size_t i = overCount; i < overCount + 100; i++)
    {
        begin = std::chrono::steady_clock::now().time_since_epoch().count();
        table.Insert(&recordPtr[i]);
        tableElpase[i - overCount] = std::chrono::steady_clock::now().time_since_epoch().count() - begin;
    }
    
    for(size_t i = 0; i < overCount; i++)
    {
        maps.Insert(&recordPtr[i]);
    }

    for(size_t i = overCount; i < overCount + 100; i++)
    {
        begin = std::chrono::steady_clock::now().time_since_epoch().count();
        maps.Insert(&recordPtr[i]);
        mapElapse[i - overCount] = std::chrono::steady_clock::now().time_since_epoch().count() - begin;
    }

    std::cout << "tree,\tmap" << std::endl;
    int treeSum = 0, mapSum = 0;
    for(int i = 0; i < 100; i++)
    {
        treeSum += tableElpase[i];
        mapSum += mapElapse[i];
        std::cout << tableElpase[i] << ",\t" << mapElapse[i] << std::endl;
    }

    std::cout << "tree avg:" << treeSum/100 << std::endl;
    std::cout << "map avg :" << mapSum/100 << std::endl;
}

void GetOption(int argc, char** argv)
{
    for(int i = 1; i < argc; i++)
    {
        if(strcmp(argv[i], "--key-size") == 0)
        {
            g_keySize = atoi(argv[++i]);
        }
        else if(strcmp(argv[i], "--sleep") == 0)
        {
            g_sleepMs = atoi(argv[++i]);
        }
    }
}

int main(int argc, char** argv)
{
    GetOption(argc, argv);

    //CompareInsert();

    BenchTableInsert(50 * 10000);

    return 0;
}