/*
 * Copyright (c) 2025 Micro Architecture Santa Cruz
 * and Texer.ai. All rights reserved.
 */

#pragma once

#include <cstdint>
#include <functional>
#include <vector>
#include <deque>

#include "m3/data.hpp"
#include "m3/memory.hpp"
#include "third_party/robin_hood.hpp"
#include "utils/explicit_type.hpp"

namespace m3
{
    using Inst_id = Explicit_type<int, struct Inst_id_struct, 0>;

    enum class Mem_op { Invalid, Load, Store, Ack, Rel, AckRel };

    class MemoryMarionette {
    public:
        /** inorder
        * Gets the in-order ID for instructions through the pipeline
        */
        Inst_id inorder();

        /** set_type
        * Sets the type of operation. Must be called before set_safe, some checks
        * are dependent on the memory oeration
        */
        void set_type(Inst_id iid, Mem_op op);

        /** set_safe
        * When the memory operation can not be undone (no possible to nuke
        * afterwards) Some tests for validity are perform. If fail, it means a bug.
        */
        void set_safe(Inst_id iid);

        /** nuke
        * Any instructions between the nuke_id and the lastest instid is
        * flushed/discarded from the pipeline. Those instructions should never
        * become safe. Spurious updates to ld_globally_perform, set_type will be
        * tolerated without side effects
        */
        void nuke(Inst_id nuke_id);

        /** st_data_ref
        * Pointer to the Data for the stored data in this operation. May be
        * used to populate address and/or data.
        */
        Data &st_data_ref(Inst_id iid);

        /** ld_data_ref
        * Pointer to the Data for the stored data in this operation. May be
        * used to populate address. The data will be computed when
        * ld_globally_perform is called
        */
        Data &ld_data_ref(Inst_id iid);

        /** ld_globally_perform
        * When a load (or load part of atomic ops) is performed. The time that the
        * value is bound to the register
        */
        const Data &ld_perform(Inst_id iid);

        /** st_locally_perform
        * Time when the address/data in the st_data_ref is populated. It is used to
        * detect forwarding values to loads. If a previous executed load was
        * performed with different data and set_safe without ld_perform again, it
        * will trigger a bug detection message.
        */
        void st_locally_perform(Inst_id iid);

        /** st_locally_merged
        * Once a store is locally_perform and set_safe, it can be merged with other
        * stores
        */
        void st_locally_merged(Inst_id iid1, Inst_id iid2);

        /** st_globally_perform
        * Once a store is locally_perform and set_safe, it can be globally
        * performed. This could be a iid that was previously merged.
        */
        void st_globally_perform(Inst_id iid);

        bool has_error(Inst_id iid) const;

        /** dump
        * print/dump the internal state
        */
        void dump() const;

        /** Constructor
        *
        */
        explicit MemoryMarionette(Memory &m, int id);

    protected:
        struct Rob_entry {
            Rob_entry(Inst_id i) : rid(i), op(Mem_op::Invalid), performed(false) {}
            void dump(const std::string &extra) const {
                std::cout << extra << " rid:" << rid << (performed ? " X" : " W") << " op:" << static_cast<int>(op);
                if (error.empty())
                    std::cout << "\n";
                else
                    std::cout << " error:" << error << "\n";

                if (ld_data.has_data()) {
                    std::cout << extra << " rid:" << rid << " ld_data ";
                    ld_data.dump();
                    std::cout << "\n";
                }
                if (st_data.has_data()) {
                    std::cout << extra << " rid:" << rid << " st_data ";
                    st_data.dump();
                    std::cout << "\n";
                }
            }

            void st_merge(const Rob_entry &d2) {
                assert(rid < d2.rid);
                st_data.add_newer(d2.st_data);
                error += d2.error;
                performed |= d2.performed;
            }

            Inst_id     rid;
            Data        ld_data;
            Data        st_data;
            Mem_op      op;
            bool        performed = false;
            std::string error;
        };

        Rob_entry &find_entry(Inst_id iid);

        Memory &mem;

        static Inst_id global_instid;

        using Rob_queue = std::deque<Rob_entry>;

        Rob_queue rob;

        Inst_id pnr;

        int cid;
    };
}
