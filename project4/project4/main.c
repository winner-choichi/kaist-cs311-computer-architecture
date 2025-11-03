
/* main.c

Page Table Entry (PTE) format:

Width: 32 bits

Bit 31 ~ 12     : 20-bit physical page number for the 2nd-level page table node or the actual physical page.
Bit 1           : Dirty bit
Bit 0           : Valid bit

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <tuple>
#include <iomanip>

#include "util.h" /* DO NOT DELETE */

/* addr_t type is the 32-bit address type defined in util.h */

struct TraceEntry
{
    TraceEntry(char op_, addr_t vaddr_) : op(op_), vaddr(vaddr_) {};
    char op;      // 'R' or 'W'
    addr_t vaddr; // 32-bit virtual address
};

std::vector<TraceEntry> read_trace_file(const std::string &filename)
{
    std::ifstream file(filename);
    std::vector<TraceEntry> traces;

    if (!file.is_open())
    {
        std::cerr << "Failed to open trace file: " << filename << std::endl;
        exit(1);
    }

    std::string line;
    while (std::getline(file, line))
    {
        std::istringstream iss(line);
        char op;
        std::string addr_str;
        if (iss >> op >> addr_str)
        {
            addr_t vaddr = std::stoul(addr_str, nullptr, 16);
            traces.push_back(TraceEntry(op, vaddr));
        }
    }

    return traces;
}

// Page Fault Handler
class PFH{
    public :
        static void handle_l1_fault(addr_t vpn){
            addr_t base_address = page_table_base_addr();
uint32_t l1_index = (vpn >> 10) & 0x3FF;
addr_t l1_address = base_address + (l1_index * 4);

addr_t new_l2_node = get_new_page_table_node();
uint32_t new_pte = ((new_l2_node >> 12) << 12);
new_pte |= 0x1;

// std::cout << "[L1 PFH] vpn: 0x" << std::hex << vpn
//           << ", l1_index: 0x" << l1_index
//           << ", l1_address: 0x" << l1_address
//           << ", new_l2_node: 0x" << new_l2_node
//           << ", new_pte: 0x" << new_pte << std::dec << std::endl;
mem_write_word32(l1_address, new_pte);
}

static void handle_l2_fault(addr_t vpn)
{
    addr_t base_address = page_table_base_addr();
    uint32_t l1_index = (vpn >> 10) & 0x3FF;
    uint32_t l2_index = vpn & 0x3FF;

    addr_t l1_address = base_address + (l1_index * 4);
    uint32_t l1_pte = mem_read_word32(l1_address);

    addr_t l2_base_address = (l1_pte >> 12) << 12;
    addr_t l2_address = l2_base_address + (l2_index * 4);

    addr_t new_physical_page = get_new_physical_page();
    uint32_t new_pte = ((new_physical_page >> 12) << 12);
    new_pte |= 0x1;

    // std::cout << "[L2 PFH] vpn: 0x" << std::hex << vpn
    //           << ", l1_index: 0x" << l1_index
    //           << ", l2_index: 0x" << l2_index
    //           << ", l2_address: 0x" << l2_address
    //           << ", new_physical_page: 0x" << new_physical_page
    //           << ", new_pte: 0x" << new_pte << std::dec << std::endl;
    mem_write_word32(l2_address, new_pte);
}
}
;

// Memory Management Unit
class MMU
{
public:
    int total_accesses = 0;
    int reads = 0;
    int writes = 0;
    int tlb_accesses = 0;
    int tlb_hits = 0;
    int tlb_misses = 0;
    int page_walks = 0;
    int page_faults = 0;

    enum class WalkResult
    {
        HIT,
        MISS_L1,
        MISS_L2
    };

    // Translation Lookaside Buffer
    class TLB
    {
    public:
        struct TLBEntry
        {
            addr_t tag = 0; // vpn
            addr_t ppn = 0;
            bool dirty = false;
            bool valid = false;
            uint64_t last_used = 0; // For LRU
        };

        TLB(MMU * parent_, int entries_, int assoc_) : parent(parent_), entries(entries_), assoc(assoc_)
        {
            num_sets = entries / assoc;
            sets.resize(num_sets, std::vector<TLBEntry>(assoc));
        };

        std::tuple<bool, addr_t, bool> access(char op, addr_t vpn, uint64_t current_time)
        {
            parent->tlb_accesses++;

            int set_index = vpn % num_sets;
            bool is_hit = false;
            bool dirty_update = false;

            for (int i = 0; i < assoc; ++i)
            {
                TLBEntry &entry = sets[set_index][i];
                if (entry.valid && entry.tag == (vpn / num_sets))
                {
                    is_hit = true;
                    entry.last_used = current_time;

                    if (op == 'W' && !entry.dirty)
                    {
                        dirty_update = true;
                        entry.dirty = true;
                    }
                    parent->tlb_hits++;
                    return {is_hit, entry.ppn, dirty_update};
                }
            }

            parent->tlb_misses++;
            return {is_hit, static_cast<addr_t>(-1), dirty_update};
        }

        void insert(char op, addr_t vpn, addr_t ppn, uint64_t current_time, bool is_dirty)
        {
            int set_index = vpn % num_sets;
            int32_t tag = vpn / num_sets;
            uint64_t lru_lateset_access_time = UINT64_MAX;
            int lru_index = 0;

            for (int i = 0; i < assoc; ++i)
            {
                TLBEntry &entry = sets[set_index][i];

                if (!entry.valid)
                {
                    entry.valid = true;
                    entry.tag = tag;
                    entry.ppn = ppn;
                    entry.last_used = current_time;

                    if (op == 'W' || is_dirty)
                        entry.dirty = true;

                    return;
                }

                if (entry.last_used < lru_lateset_access_time)
                {
                    lru_index = i;
                    lru_lateset_access_time = entry.last_used;
                }
            }

            TLBEntry &lru_entry = sets[set_index][lru_index];
            lru_entry.valid = true;
            lru_entry.tag = tag;
            lru_entry.ppn = ppn;
            lru_entry.last_used = current_time;
            lru_entry.dirty = (op == 'W' || is_dirty);

            return;
        }

        void dump()
        {
            std::cout << "TLB Content:\n"
                      << "-------------------------------------" << std::endl;

            std::cout << "    ";
            for (int way = 0; way < assoc; ++way)
            {
                std::cout << "      WAY[" << way << "]";
            }
            std::cout << '\n';

            // Print sets
            for (int set = 0; set < num_sets; ++set)
            {
                std::cout << "SET[" << set << "]:   ";
                for (int way = 0; way < assoc; ++way)
                {
                    const TLBEntry &entry = sets[set][way];
                    std::cout << " (v=" << entry.valid
                              << " tag=0x" << std::hex << std::setw(5) << std::setfill('0') << entry.tag
                              << " ppn=0x" << std::setw(5) << entry.ppn
                              << " d=" << std::dec << entry.dirty << ") |";
                }
                std::cout << std::endl;
            }
        }

    private:
        MMU *parent;
        int entries;
        int assoc;
        int num_sets;
        std::vector<std::vector<TLBEntry>> sets;
    };

    // Page Table Walker
    class PTW
    {
    public:
        PTW(MMU * parent_) : parent(parent_){};

        std::tuple<WalkResult, addr_t, bool> walk(char op, addr_t vpn)
        {
            parent->page_walks++;
            addr_t base_address = page_table_base_addr();

            uint32_t l1_index = (vpn >> 10) & 0x3FF;
            uint32_t l2_index = vpn & 0x3FF;

            addr_t l1_address = base_address + (l1_index * 4);
            uint32_t l1_pte = mem_read_word32(l1_address);

            if (!(l1_pte & 0x1)) // If l1 pte's valid == 0
            {
                return {WalkResult::MISS_L1, 0, false};
            }

            addr_t l2_base_address = (l1_pte >> 12) << 12;
            addr_t l2_address = l2_base_address + (l2_index * 4);
            uint32_t l2_pte = mem_read_word32(l2_address);

            if (!(l2_pte & 0x1)) // If l2 pte's valid == 0
            {
                return {WalkResult::MISS_L2, 0, false};
            }

            if (op == 'W')
            {
                l2_pte |= 0x2;
                mem_write_word32(l2_address, l2_pte);
            }

            addr_t ppn = (l2_pte >> 12);
            return {WalkResult::HIT, ppn, (l2_pte & 0x2)};
        }

    private:
        MMU *parent;
    };

    MMU(int entries, int assoc) : tlb_entries(entries), tlb_assoc(assoc), tlb(this, entries, assoc), ptw(this){};

    void access(char op, addr_t vaddr, uint64_t access_time)
    {
        ++total_accesses;
        switch (op)
        {
        case 'R':
            ++reads;
            break;
        case 'W':
            ++writes;
            break;
        }

        addr_t vpn = vaddr >> 12;

        // std::cout << "vaddr: 0x" << std::hex << vaddr
        //           << ", vpn: 0x" << vpn << std::dec
        //           << ", op: " << op
        //           << ", time: " << access_time << std::endl;
        bool resolved = false;
        while (!resolved)
        {
            std::tuple<bool, addr_t, bool> result = tlb.access(op, vpn, access_time);
            bool hit = std::get<0>(result);
            addr_t ppn = std::get<1>(result);
            bool dirty_update = std::get<2>(result);

            // TLB hit / miss
            if (hit)
            {
                // std::cout << "TLB HIT: VPN 0x" << std::hex << vpn << " -> PPN 0x" << ppn << std::dec << std::endl;
                if (dirty_update)
                {
                    ptw.walk(op, vpn);
                }
                resolved = true;
            }
            else
            {
                // std::cout << "TLB MISS: VPN 0x" << std::hex << vpn << std::dec << std::endl;

                std::tuple<WalkResult, addr_t, bool> res = ptw.walk(op, vpn);
                // if page fault
                if (std::get<0>(res) != WalkResult::HIT)
                {
                    ++page_faults;
                    if (std::get<0>(res) == WalkResult::MISS_L1)
                    {
                        PFH::handle_l1_fault(vpn);
                    }
                    PFH::handle_l2_fault(vpn);
                }
                else
                {
                    ppn = std::get<1>(res);
                    // std::cout << "[TLB INSERT] op: " << op
                    //           << " vpn: 0x" << std::hex << vpn
                    //           << " ppn: 0x" << ppn
                    //           << " time: " << std::dec << access_time << std::endl;
                    tlb.insert(op, vpn, ppn, access_time, std::get<2>(res));
                    resolved = true;
                }
            }
        }
    }

    void dump_tlb()
    {
        tlb.dump();
    }

private:
    int tlb_entries = 0;
    int tlb_assoc = 0;
    TLB tlb;
    PTW ptw;
};

// Page Table
class PT{};

int main(int argc, char *argv[])
{
    init(); /* DO NOT DELETE. */

    // params
    int tlb_entries = 0;
    int tlb_assoc = 0;
    bool do_dump = false;
    std::string trace_filename;

    // command line parsing
    if (argc < 4)
    {
        std::cerr << "Usage: " << argv[0] << " -c <entries:assoc> [-x] <trace_file>" << std::endl;
        return 1;
    }

    int i = 1;
    while (i < argc)
    {
        std::string arg = argv[i];

        if (arg == "-c")
        {
            if (i + 1 >= argc)
            {
                std::cerr << "Missing value after -c" << std::endl;
                return 1;
            }

            std::string config = argv[++i];
            size_t colon = config.find(':');
            if (colon == std::string::npos)
            {
                std::cerr << "Invalid format for -c option" << std::endl;
                return 1;
            }

            tlb_entries = std::stoi(config.substr(0, colon));
            tlb_assoc = std::stoi(config.substr(colon + 1));
        }
        else if (arg == "-x")
        {
            do_dump = true;
        }
        else
        {
            trace_filename = arg;
        }

        ++i;
    }

    if (tlb_entries == 0 || tlb_assoc == 0 || trace_filename.empty())
    {
        std::cerr << "Invalid arguments" << std::endl;
        return 1;
    }

    std::vector<TraceEntry> traces = read_trace_file(trace_filename);

    MMU mmu(tlb_entries, tlb_assoc);
    uint64_t time = 0;
    for (const auto &entry : traces)
    {
        mmu.access(entry.op, entry.vaddr, time++);
    }

    cdump(tlb_entries, tlb_assoc);
    sdump(mmu.total_accesses, mmu.reads, mmu.writes, mmu.tlb_accesses, mmu.tlb_hits, mmu.tlb_misses, mmu.page_walks, mmu.page_faults);
    mmu.dump_tlb();
    if (do_dump)
    {
        dump_page_table_area();
    }
    return 0;
}
