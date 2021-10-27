#pragma once

#include <cassert>
#include <ostream>
#include <unordered_set>
#include <functional>

namespace secd {

    /** Very simple mark-sweep garbage collector.

        Super simple allocation if memory is available: return the top of the free list, advance free list to the next element.
        If free list is empty, perform garbage collection, which also determines which banks should be returned to the OS, if any.

        => global free list
        
     */
    class GC {
    private:
        enum class CellStatus : unsigned char {
            Used = 0,
            Marked = 1,
            Free = 0xff
        };
    public:

        static size_t constexpr BankSize = 1000;
        
        enum class CellKind {
            Integer,
            Symbol,
            Cons,
            Closure,
        }; // GC::CellKind

        static void PrintStats();

        /** Runs the GC.
         */
        static void Run();

        class Cell {
        private:
            friend class GC;
            
            CellStatus status;
        public:
            
            CellKind kind;
            
            union {
                int64_t valueInt;
                std::string const * name;
                struct {
                    GC::Cell * car; 
                    GC::Cell * cdr; 
                };
                struct {
                    GC::Cell * body;
                    GC::Cell * environment;
                };
            };

            Cell(CellKind kind, int64_t valueInt):
                kind(kind),
                valueInt(valueInt) {
            }

            Cell(std::string const * name):
                kind(CellKind::Symbol),
                name(name) {
            }

            Cell(CellKind kind, GC::Cell * car, GC::Cell * cdr):
                kind(kind),
                car(car),
                cdr(cdr) {
            }

            static void * operator new(size_t sz) {
                assert(sz == sizeof(Cell) && "Can only allocate single size at a time");
                return GC::AllocateCell();
            }
        };


        static void AddRoot(Cell * & cell) {
            roots_.insert(& cell);
            ++rootChanges_;
        }

        static void RemoveRoot(Cell *& cell) {
            assert(roots_.find(& cell) != roots_.end() && "Removing non-existing root");
            roots_.erase(& cell);
            ++rootChanges_;
        }
        
    private:

        friend class Cell;
        
        class Bank {
        public:

            /** Creates new bank and populates the free list with it.
             */
            Bank(Bank * next, Cell * & freeList);


            /** Pointer to the bank itself, which is just an array of the cells allocated when the bank is created.  
             */
            Cell * cells;

            /** Pointer to the next bank.
             */
            Bank * next;
        }; // GC::Bank

        static void * AllocateCell() {
            if (freeList_ == nullptr)
                GC::Run();
            freeList_->status = CellStatus::Used;
            void * result = freeList_;
            if (reinterpret_cast<char*>(freeList_->car) + 1 == nullptr)
                ++freeList_;
            else
                freeList_ = freeList_->car;
            ++allocations_;
            return result;
        }

        /** Mark phase of the collector where all cells reachable from the roots are marked as live.
         */
        static void Mark();

        /** Sweep phase of the collector where each bank is visited and any unmarked cells are returned to the free list.
         */
        static void Sweep();

        /** Number of allocations since last GC cycle.
         */
        static size_t allocations_;

        static size_t numBanks_;

        static size_t liveObjects_;

        /** Top bank.
         */
        static Bank * bank_;

        /** The beginning of the free list of the current bank.
         */
        static Cell * freeList_;

        static std::unordered_set<Cell **> roots_;

        static size_t rootChanges_;
        
    };
    
} // namespace secd
