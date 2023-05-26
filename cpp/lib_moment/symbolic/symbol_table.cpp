/**
 * symbol_table.cpp
 * 
 * @copyright Copyright (c) 2022-2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "symbol_table.h"

#include "matrix/operator_sequence_generator.h"
#include "scenarios/context.h"

#include "utilities/dynamic_bitset.h"

#include <iostream>
#include <sstream>

namespace Moment {

    namespace errors {
        namespace {
            std::string make_zs_err_msg(symbol_name_t id) {
                std::stringstream errMsg;
                errMsg << "Symbol " << id << " is identically zero; but zero should be uniquely represented as \"0\"";
                return errMsg.str();
            }

            std::string make_us_err_msg(symbol_name_t id) {
                std::stringstream errMsg;
                errMsg << "Symbol " << id << " is not defined in symbol table.";
                return errMsg.str();
            }

            std::string make_ube_err_msg(bool real, ptrdiff_t id) {
                std::stringstream errMsg;
                if (real) {
                    errMsg << "Real";
                } else {
                    errMsg << "Imaginary";
                }
                errMsg << " basis element " << id << " is not defined in symbol table.";
                return errMsg.str();
            }
        }

        zero_symbol::zero_symbol(symbol_name_t sid) : std::runtime_error{make_zs_err_msg(sid)}, id{sid} { }

        unknown_symbol::unknown_symbol(Moment::symbol_name_t sid) : std::range_error{make_us_err_msg(sid)}, id{sid} { }

        unknown_basis_elem::unknown_basis_elem(bool real_or, ptrdiff_t basis_id)
        : std::range_error{make_ube_err_msg(real_or, basis_id)}, real{real_or}, id{basis_id} { }
    }

    UniqueSequence::UniqueSequence(OperatorSequence sequence,
                                   OperatorSequence conjSequence):
            opSeq{std::move(sequence)},
            conjSeq{std::move(conjSequence)},
            hermitian{false}, antihermitian{false}, real_index{-1}, img_index{-1},
            fwd_sequence_str{opSeq->formatted_string()} {

        int compare = OperatorSequence::compare_same_negation(*opSeq, *conjSeq);
        if (1 == compare) {
            this->hermitian = true;
        } else if (-1 == compare) {
            this->antihermitian = true;
        }
    }

    SymbolTable::SymbolTable(const Context& context)
        : Basis{*this}, context{context}, OSGIndex{context, *this} {

        // Zero and identity are always in symbol table, in indices 0 and 1 respectively.
        this->unique_sequences.emplace_back(UniqueSequence::Zero(this->context));
        this->unique_sequences.emplace_back(UniqueSequence::Identity(this->context));

        this->hash_table.insert({this->unique_sequences[0].hash(), 0});
        this->hash_table.insert({this->unique_sequences[1].hash(), 1});

        // '1' is always in real basis.
        this->Basis.push_back(1, true, false);
    }


    std::set<symbol_name_t> SymbolTable::merge_in(std::vector<UniqueSequence> &&build_unique, size_t * newly_added) {
        std::set<symbol_name_t> included_symbols;

        size_t added = 0;
        size_t symbol_max = this->unique_sequences.size();

        for (auto& elem : build_unique) {
            const auto symbol_name = this->merge_in(std::move(elem));
            included_symbols.emplace(symbol_name);
            if (symbol_name >= symbol_max) {
                ++added;
                symbol_max = this->unique_sequences.size();
            }
        }
        if (newly_added != nullptr) {
            *newly_added = added;
        }

        return included_symbols;
    }

    symbol_name_t SymbolTable::merge_in(OperatorSequence&& sequence) {
        auto conj_seq = sequence.conjugate();
        return this->merge_in(UniqueSequence{std::move(sequence), std::move(conj_seq)});
    }

    symbol_name_t SymbolTable::merge_in(UniqueSequence&& elem) {
        // Not unique, do not add...
        auto existing_iter = this->hash_table.find(elem.hash());
        if (existing_iter != this->hash_table.end()) {
            const ptrdiff_t stIndex = existing_iter->second >= 0 ? existing_iter->second : -existing_iter->second;
            return this->unique_sequences[stIndex].id;
        }

        // Otherwise, query
        const auto next_index = static_cast<symbol_name_t>(this->unique_sequences.size());

        // Does context know about nullity?
        auto [re_zero, im_zero] = this->context.is_sequence_null(elem.sequence());

        // Is element hermitian
        const bool is_hermitian = elem.is_hermitian();
        if (is_hermitian) {
            im_zero = true;
        }

        // Is element anti-hermitian
        const bool is_anti_hermitian = elem.is_antihermitian();
        if (is_anti_hermitian) {
            re_zero = true;
        }

        // Make element
        elem.id = next_index;

        // Add to basis
        const auto [re_index, im_index] = this->Basis.push_back(next_index, !re_zero, !im_zero);
        elem.real_index = re_index;
        elem.img_index = im_index;

        // Add hash(es)
        this->hash_table.emplace(std::make_pair(elem.hash(), next_index));
        if (!is_hermitian) {
            this->hash_table.emplace(std::make_pair(elem.hash_conj(), -next_index));
        }

        // Register element
        this->unique_sequences.emplace_back(std::move(elem));

        return next_index;
    }



    symbol_name_t SymbolTable::create(const bool has_real, const bool has_imaginary, std::string fwd_name) {
        auto next_id = static_cast<symbol_name_t>(this->unique_sequences.size());
        UniqueSequence blank;
        blank.id = next_id;
        blank.fwd_sequence_str = std::move(fwd_name);

        auto [re_index, im_index] = this->Basis.push_back(next_id, has_real, has_imaginary);
        blank.real_index = re_index;
        blank.img_index = im_index;

        if (!has_real) {
            blank.antihermitian = true;
        }
        if (!has_imaginary) {
            blank.hermitian = true;
        }

        this->unique_sequences.emplace_back(std::move(blank));

        return next_id;
    }

    symbol_name_t SymbolTable::create(const size_t count, const bool has_real, const bool has_imaginary) {
        const auto first_id = static_cast<symbol_name_t>(this->unique_sequences.size());
        const auto range_end = first_id + count;

        this->unique_sequences.reserve(range_end);
        if (has_real) {
            this->Basis.real_symbols.reserve(this->Basis.real_symbols.size() + count);
        }
        if (has_imaginary) {
            this->Basis.imaginary_symbols.reserve(this->Basis.imaginary_symbols.size() + count);
        }

        for (size_t next_id = first_id; next_id < range_end; ++next_id) {
            UniqueSequence blank;
            blank.id = static_cast<symbol_name_t>(next_id);

            auto [re_index, im_index] = this->Basis.push_back(blank.id, has_real, has_imaginary);
            blank.real_index = re_index;
            blank.img_index = im_index;

            if (!has_real) {
                blank.antihermitian = true;
            }
            if (!has_imaginary) {
                blank.hermitian = true;
            }

            this->unique_sequences.emplace_back(std::move(blank));
       }
        return first_id;
    }

    bool SymbolTable::merge_in(const DynamicBitset<uint64_t>& can_be_real,
                               const DynamicBitset<uint64_t>& can_be_imaginary) {
        assert(can_be_real.bit_size == can_be_imaginary.bit_size);
        bool changes = false;

        // Ensure enough elements exist
        const size_t initial_elems = this->unique_sequences.size();
        const size_t elems = can_be_real.bit_size;
        if (elems > initial_elems) {
            this->create(elems - initial_elems);
        }

        // Go through symbols, flagging where they must be real / imaginary
        for (auto& symbol : this->unique_sequences) {
            if (0 == symbol.id) {
                symbol.hermitian = true;
                symbol.antihermitian = true;
                continue;
            }

            const bool sym_has_real = can_be_real.test(symbol.id);
            const bool sym_has_imaginary = can_be_imaginary.test(symbol.id);
            if (sym_has_real) {
                if (!sym_has_imaginary) {
                    if (!symbol.hermitian) {
                        changes = true;
                    }
                    symbol.hermitian = true;
                    if (symbol.antihermitian) {
                        throw errors::zero_symbol{symbol.id};
                    }
                }
            } else if (sym_has_imaginary) {
                if (!symbol.antihermitian) {
                    changes = true;
                }
                symbol.antihermitian = true;
                if (symbol.hermitian) {
                    throw errors::zero_symbol{symbol.id};
                }
            } else {
                throw errors::zero_symbol{symbol.id};
            }
        }

        // With new real/imaginary information, re-count the bases
        this->Basis.renumerate_bases();

        return changes;
    }


    const UniqueSequence *
    SymbolTable::where(const OperatorSequence &seq) const noexcept {
        auto [obj, is_conj] = where_and_is_conjugated(seq);
        return obj;
    }

    std::pair<const UniqueSequence *, bool>
    SymbolTable::where_and_is_conjugated(const OperatorSequence &seq) const noexcept {
        size_t hash = this->context.hash(seq);

        auto [id, conj] = this->hash_to_index(hash);
        if (id == std::numeric_limits<ptrdiff_t>::max()) {
            return {nullptr, false};
        }

        assert(id < this->unique_sequences.size());
        return {&this->unique_sequences[id], conj};
    }

    SymbolExpression SymbolTable::to_symbol(const OperatorSequence &seq) const noexcept {
        size_t hash = this->context.hash(seq);
        auto [id, conj] = this->hash_to_index(hash);
        if (id == std::numeric_limits<ptrdiff_t>::max()) {
            return SymbolExpression{0};
        }

        return SymbolExpression{this->unique_sequences[id].id, conj};
    }


    std::pair<ptrdiff_t, bool> SymbolTable::hash_to_index(size_t hash) const noexcept {
        auto hash_iter = this->hash_table.find(hash);
        if (hash_iter == this->hash_table.end()) {
            return {std::numeric_limits<ptrdiff_t>::max(), false};
        }

        if (hash_iter->second >= 0) {
            return {hash_iter->second, false};
        } else {
            return {-hash_iter->second, true};
        }
    }


    std::string UniqueSequence::formatted_sequence() const {
        if (this->opSeq.has_value()) {
            return this->opSeq->formatted_string();
        }

        std::stringstream ss;
        ss << "#" << this->id;
        return ss.str();
    }

    std::string UniqueSequence::formatted_sequence_conj() const {
        if (this->conjSeq.has_value()) {
            return this->conjSeq->formatted_string();
        } else if (this->hermitian && this->opSeq.has_value()) {
            return this->opSeq->formatted_string();
        }

        std::stringstream ss;
        ss << "#" << this->id;
        if (!this->hermitian) {
            ss << "*";
        }
        return ss.str();
    }

    std::pair<size_t, size_t> SymbolTable::fill_to_word_length(size_t word_length) {
        const auto& osg = this->context.operator_sequence_generator(word_length);
        std::vector<UniqueSequence> build_unique;
        build_unique.reserve(osg.size());
        for (const auto& op_seq : osg) {
            auto conj_seq = op_seq.conjugate();
            if (op_seq == conj_seq) {
                build_unique.emplace_back(op_seq);
            } else {
                build_unique.emplace_back(op_seq, std::move(conj_seq));
            }
        }
        size_t new_symbols{};
        this->merge_in(std::move(build_unique), &new_symbols);

        this->OSGIndex.update();

        return std::make_pair(osg.size(), new_symbols);
    }


    std::pair<size_t, size_t> SymbolTable::BasisView::renumerate_bases() {
        ptrdiff_t real_index = 0;
        ptrdiff_t imaginary_index = 0;
        this->real_symbols.clear();
        this->imaginary_symbols.clear();
        this->im_of_real.clear();
        this->re_of_imaginary.clear();

        for (auto& symbol : symbol_table.unique_sequences) {
            const bool has_real = !symbol.antihermitian;
            const bool has_im = !symbol.hermitian;

            if (has_real) {
                this->real_symbols.emplace_back(symbol.id);
                symbol.real_index = real_index;
                ++real_index;
            } else {
                symbol.real_index = -1;
            }

            if (has_im) {
                this->imaginary_symbols.emplace_back(symbol.id);
                symbol.img_index = imaginary_index;
                this->re_of_imaginary.emplace_back(symbol.real_index);
                ++imaginary_index;
            } else {
                symbol.img_index = -1;
            }

            if (has_real) {
                this->im_of_real.emplace_back(symbol.img_index);
            }
        }
        return {real_index, imaginary_index};
    }


    std::pair<ptrdiff_t, ptrdiff_t> SymbolTable::BasisView::push_back(const Moment::symbol_name_t symbol_id,
                                                                      const bool has_real, const bool has_im) {
        std::pair<ptrdiff_t, ptrdiff_t> output{-1, -1};
        if (has_real) {
            output.first = static_cast<ptrdiff_t>(this->real_symbols.size());
            this->real_symbols.emplace_back(symbol_id);
        }
        if (has_im) {
            output.second = static_cast<ptrdiff_t>(this->imaginary_symbols.size());
            this->imaginary_symbols.emplace_back(symbol_id);
            this->re_of_imaginary.emplace_back(output.first);
        }
        if (has_real) {
            this->im_of_real.emplace_back(output.second);
        }
        return output;
    }

    std::ostream& operator<<(std::ostream& os, const UniqueSequence& seq) {
        const bool has_fwd = seq.opSeq.has_value();
        const bool has_rev = seq.opSeq.has_value();

        os << "#" << seq.id << ":\t";
        if (!seq.fwd_sequence_str.empty()) {
            if (!has_fwd) {
                os << "[";
            }
            os << seq.fwd_sequence_str;
            if (!has_fwd) {
                os << "]";
            }
        } else {
            os << "[No sequence]";
        }
        if (seq.fwd_sequence_str.length() >= 80) {
            os << "\n\t";
        } else {
            os << ":\t";
        }

        if (seq.real_index>=0) {
            if (seq.img_index>=0) {
                os << "Complex";
            } else {
                os << "Real";
            }
        } else if (seq.img_index>=0) {
            os << "Imaginary";
        } else {
            os << "Zero";
        }

        if (seq.hermitian) {
            os << ", Hermitian";
        }
        if (seq.real_index>=0) {
            os << ", Re#=" << seq.real_index;
        }
        if (seq.img_index>=0) {
            os << ", Im#=" << seq.img_index;
        }

        if (has_fwd) {
            os << ", hash=" << seq.hash();
            if (seq.hash_conj() != seq.hash()) {
                os << "/" << seq.hash_conj();
            }
        } else {
            os << ", unhashable";
        }
        return os;
    }

    std::ostream& operator<<(std::ostream& os, const SymbolTable& table) {
        const auto& real_symbols = table.Basis.RealSymbols();
        const auto& im_symbols = table.Basis.ImaginarySymbols();

        os << "Symbol table with ";
        os << table.unique_sequences.size()
           << " unique sequence" << ((table.unique_sequences.size() != 1)  ? "s" : "") << ", ";
        os << real_symbols.size() << " with real parts, "
           << im_symbols.size() << " with imaginary parts:\n";

        // List real symbol IDs
        if (real_symbols.empty()) {
            os << "No symbols with real parts.\n";
        } else {
            os << "Symbols with real parts: ";
            bool one_real = false;
            for (auto i: real_symbols) {
                if (one_real) {
                    os << ", ";
                }
                os << i;
                one_real = true;
            }
            os << "\n";
        }

        // List imaginary symbol IDs
        if (im_symbols.empty()) {
            os << "No symbols with imaginary parts.\n";
        } else {
            os << "Symbols with imaginary parts: ";
            bool one_im = false;
            for (auto i: im_symbols) {
                if (one_im) {
                    os << ", ";
                }
                os << i;
                one_im = true;
            }
            os << "\n";
        }

        // List symbols
        for (const auto& us : table.unique_sequences) {
            os << us << "\n";
        }

        return os;
    }

}