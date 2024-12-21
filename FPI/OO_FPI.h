#ifndef OO_FPI_H
#define OO_FPI_H

/*
 * On-Off sketch on finding persistent items
 */

#include "bitset.h"
#include "Abstract.h"

template<typename DATA_TYPE, typename COUNT_TYPE, uint32_t SLOT_NUM>
class OO_FPI : public Abstract<DATA_TYPE, COUNT_TYPE> {
public:

    struct Bucket {
        DATA_TYPE items[SLOT_NUM];
        COUNT_TYPE counters[SLOT_NUM];

        inline COUNT_TYPE Query(const DATA_TYPE item){
            for(uint32_t i = 0;i < SLOT_NUM;++i){
                if(items[i] == item)
                    return counters[i];
            }
            return 0;
        }
    };

    OO_FPI(uint64_t memory) :
            length((double)memory / (sizeof(Bucket) + sizeof(COUNT_TYPE) + (SLOT_NUM + 1) * BITSIZE)){
        buckets = new Bucket[length];
        sketch = new COUNT_TYPE [length];

        memset(buckets, 0, length * sizeof(Bucket));
        memset(sketch, 0, length * sizeof(COUNT_TYPE));

        bucketBitsets = new BitSet(SLOT_NUM * length);
        sketchBitsets = new BitSet(length);
    }

    ~OO_FPI(){
        std::cout << "OO_FPI Memory Usage: " << CalculateMemory() << " bytes" << std::endl;
        delete [] buckets;
        delete [] sketch;
        delete bucketBitsets;
        delete sketchBitsets;
    }

    void Insert(const DATA_TYPE item, const COUNT_TYPE window){
        uint32_t pos = this->hash(item) % length;
        uint32_t bucketBitPos = pos * SLOT_NUM;

        for(uint32_t i = 0;i < SLOT_NUM;++i){
            if(buckets[pos].items[i] == item){
                buckets[pos].counters[i] += (!bucketBitsets->SetNGet(bucketBitPos + i));
                return;
            }
        }

        if(!sketchBitsets->Get(pos)){
            for(uint32_t i = 0;i < SLOT_NUM;++i){
                if(buckets[pos].counters[i] == sketch[pos]){
                    buckets[pos].items[i] = item;
                    buckets[pos].counters[i] += 1;
                    bucketBitsets->Set(bucketBitPos + i);
                    return;
                }
            }

            sketch[pos] += 1;
            sketchBitsets->Set(pos);
        }
    }

    COUNT_TYPE Query(const DATA_TYPE item){
        return buckets[this->hash(item) % length].Query(item);
    }

    void NewWindow(const COUNT_TYPE window){
        bucketBitsets->Clear();
        sketchBitsets->Clear();
    }

private:
    const uint32_t length;

    BitSet* bucketBitsets;
    Bucket* buckets;

    BitSet* sketchBitsets;
    COUNT_TYPE* sketch;

    uint64_t CalculateMemory() const {
        uint64_t memoryUsage = 0;
        memoryUsage += length * sizeof(Bucket); // Memory used by buckets
        memoryUsage += length * sizeof(COUNT_TYPE); // Memory used by sketch
        memoryUsage += sizeof(bucketBitsets); // Memory used by bucketBitsets
        memoryUsage += sizeof(sketchBitsets); // Memory used by sketchBitsets
        return memoryUsage;
    }
};

#endif //OO_FPI_H
