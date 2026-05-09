#ifndef REGISTER_ALIAS_TABLE_HPP
#define REGISTER_ALIAS_TABLE_HPP

#include <vector>

class RegisterAliasTable {
private:
    std::vector<int> table;

public:
    RegisterAliasTable(int numRegisters = 32) {
        table.resize(numRegisters, -1);
    }
    int getProducer(int regIndex) const {
        return table[regIndex];
    }
    void setProducer(int regIndex, int rsTag) {
        table[regIndex] = rsTag;
    }
    void clearDependency(int regIndex, int rsTag) {
        if (table[regIndex] == rsTag) {
            table[regIndex] = -1;
        }
    }
};

#endif