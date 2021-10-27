#include <cstring>
#include <iostream>
#include <vector>

#include "gc.h"
#include "common/colors.h"

namespace secd {

    size_t GC::allocations_ = 0;
    
    size_t GC::numBanks_ = 0;
    
    size_t GC::liveObjects_ = 0;

    GC::Bank * GC::bank_ = nullptr;

    GC::Cell * GC::freeList_ = nullptr;

    std::unordered_set<GC::Cell **> GC::roots_;

    size_t GC::rootChanges_ = 0;

    void GC::PrintStats() {
        std::cout << tiny::color::gray;
        std::cout << "Allocations:  " << allocations_ << std::endl;
        std::cout << "Live objects: " << liveObjects_ << std::endl;
        std::cout << "Active banks: " << numBanks_ << std::endl;
        std::cout << "Root changes: " << rootChanges_ << std::endl;
        std::cout << tiny::color::reset;
    }

    void GC::Run() {
        Mark();
        Sweep();
        if (freeList_ == nullptr) {
            bank_ = new Bank(bank_, freeList_);
            ++numBanks_;
            std::cout << "New bank created..." << std::endl;
        }
        allocations_ = 0;
    }

    void GC::Mark() {
        liveObjects_ = 0;
        std::vector<Cell *> q;
        for (auto i : roots_)
            q.push_back(*i);
        while (!q.empty()) {
            Cell * x = q.back();
            q.pop_back();
            if (x->status == CellStatus::Marked)
                continue;
            x->status = CellStatus::Marked;
            ++liveObjects_;
            switch (x->kind) {
            case CellKind::Cons:
                q.push_back(x->car);
                q.push_back(x->cdr);
                break;
            case CellKind::Closure:
                q.push_back(x->car);
                q.push_back(x->cdr);
            default:
                break;
            }
        }
    }

    void GC::Sweep() {
        Bank * b = bank_;
        size_t recovered = 0;
        while (b != nullptr) {
            for (Cell * c = b->cells, * e = b->cells + BankSize; c != e; ++c) {
                switch (c->status) {
                case CellStatus::Marked:
                    c->status = CellStatus::Used;
                    break;
                case CellStatus::Used:
                    c->car = freeList_;
                    freeList_ = c;
                    c->status = CellStatus::Free;
                    ++recovered;
                    break;
                default:
                    break;
                }
            }
            b = b->next;
        }
        std::cout << "GC Run: allocations " << allocations_ << ", live objects: " << liveObjects_ << ", recovered " << recovered << std::endl;
    }
    
    GC::Bank::Bank(GC::Bank * next, GC::Cell * & freeList):
        next(next) {
        // create the memory 
        char * rawMem = (new char[sizeof (GC::Cell) * GC::BankSize]);
        memset(rawMem, 0xff, sizeof(GC::Cell) * GC::BankSize);
        cells = reinterpret_cast<Cell *>(rawMem);
        // the last cell in the bank should point to the existing free list
        cells[GC::BankSize - 1].car = freeList;
        // and the free list now points to the first cell in the benk
        freeList = cells;
    }

} // namespace secd
