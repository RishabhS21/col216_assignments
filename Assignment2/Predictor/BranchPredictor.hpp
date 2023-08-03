#ifndef __BRANCH_PREDICTOR_HPP__
#define __BRANCH_PREDICTOR_HPP__

#include <vector>
#include <bitset>
#include <cassert>

struct BranchPredictor {
    virtual bool predict(uint32_t pc) = 0;
    virtual void update(uint32_t pc, bool taken) = 0;
};


struct SaturatingBranchPredictor : public BranchPredictor
{
    std::vector<std::bitset<2>> table;
    SaturatingBranchPredictor(int value) : table(1 << 14, value) {}

    bool predict(uint32_t pc)
    {
        std::bitset<2> counter = table[pc & ((1 << 14) - 1)];
        return counter[1];
    }

    void update(uint32_t pc, bool taken)
    {
        std::bitset<2> &counter = table[pc & ((1 << 14) - 1)];
        if (taken)
        {
            if (counter == 0 || counter == 1 || counter == 2)
            {
                counter = counter.to_ulong() + 1;
            }
        }
        else
        {
            if (counter == 1 || counter == 2 || counter == 3)
            {
                counter = counter.to_ulong() - 1;
            }
        }
    }
};

struct BHRBranchPredictor : public BranchPredictor
{
    std::vector<std::bitset<2>> bhrTable;
    std::bitset<2> bhr;
    BHRBranchPredictor(int value) : bhrTable(1 << 2, value), bhr(value) {}

    bool predict(uint32_t pc)
    {
        std::bitset<2> &counter = bhrTable[bhr.to_ulong()];
        return counter[1];
    }

    void update(uint32_t pc, bool taken)
    {
        std::bitset<2> &counter = bhrTable[bhr.to_ulong()];
        if (taken)
        {
            if (counter == 0 || counter == 1 || counter == 2)
            {
                counter = counter.to_ulong() + 1;
            }
        }
        else
        {
            if (counter == 1 || counter == 2 || counter == 3)
            {
                counter = counter.to_ulong() - 1;
            }
        }
        bhr <<= 1;
        bhr[0] = taken;
    }
};


struct SaturatingBHRBranchPredictor : public BranchPredictor
{
    std::vector<std::bitset<2>> bhrTable;
    std::bitset<2> bhr;
    std::vector<std::bitset<2>> table;
    std::vector<std::bitset<2>> combination;
    SaturatingBHRBranchPredictor(int value, int size) : bhrTable(1 << 2, value), bhr(value), table(1 << 14, value), combination(size, value)
    {
        assert(size <= (1 << 16));
    }

    bool predict(uint32_t pc)
    {
        std::bitset<2> &counter1 = bhrTable[bhr.to_ulong()];
        std::bitset<2> &counter2 = table[pc & ((1 << 14) - 1)];
        std::bitset<2> &counter3 = combination[(pc & ((1 << 16) - 1)) ^ bhr.to_ulong()];
        return (counter1[1] + counter2[1] + counter3[1]) >= 2;
    }

    void update(uint32_t pc, bool taken)
    {
        std::bitset<2> &counter1 = bhrTable[bhr.to_ulong()];
        std::bitset<2> &counter2 = table[pc & ((1 << 14) - 1)];
        std::bitset<2> &counter3 = combination[(pc & ((1 << 16) - 1)) ^ bhr.to_ulong()];
        // bool prediction = counter1.to_ulong() >= 2 || counter2.to_ulong() >= 2 || counter3.to_ulong() >= 2;
        if (taken)
        {
            if (counter1.to_ulong() < 3)
                counter1 = counter1.to_ulong() + 1;
            if (counter2.to_ulong() < 3)
                counter2 = counter2.to_ulong() + 1;
            if (counter3.to_ulong() < 3)
                counter3 = counter3.to_ulong() + 1;
        }
        else
        {
            if (counter1.to_ulong() > 0)
                counter1 = counter1.to_ulong() - 1;
            if (counter2.to_ulong() > 0)
                counter2 = counter2.to_ulong() - 1;
            if (counter3.to_ulong() > 0)
                counter3 = counter3.to_ulong() - 1;
        }
        bhr <<= 1;
        bhr[0] = taken;
    }
};

#endif