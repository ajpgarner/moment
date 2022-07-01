/**
 * multi_mmt_iterator.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "multi_mmt_iterator.h"


namespace NPATK {
    namespace {
        std::vector<size_t> getMmtOpCounts(const MultiMmtIterator &multiMmtIterator) {
            std::vector<size_t> output;
            output.reserve(multiMmtIterator.dimension());
            for (auto mmIter: multiMmtIterator.iters()) {
                output.push_back(mmIter->num_operators());
            }
            return output;
        }

        std::vector<size_t> getMmtOutcomeCounts(const MultiMmtIterator &multiMmtIterator) {
            std::vector<size_t> output;
            output.reserve(multiMmtIterator.dimension());
            for (auto mmIter: multiMmtIterator.iters()) {
                output.push_back(mmIter->num_outcomes);
            }
            return output;
        }
    }

    MultiMmtIterator::MultiMmtIterator(const Context &contextRef, party_list_t &&list)
            : contextPtr{&contextRef}, party_list(std::move(list)) {
        this->mmt_iters.reserve(party_list.size());
        this->mmt_indices.reserve(party_list.size());
        this->global_mmt_indices.reserve(party_list.size());

        for (const auto &party: party_list) {
            if (party->Measurements.empty()) {
                throw std::logic_error{"Cannot iterate if one included Party has no measurements."};
            }

            this->mmt_iters.push_back(party->Measurements.begin());
            const auto &last_iter = this->mmt_iters.back();
            this->mmt_indices.push_back(0);
            if (last_iter != party->Measurements.end()) {
                this->global_mmt_indices.push_back(last_iter->Index().global_mmt);
            } else {
                this->global_mmt_indices.push_back(0);
            }
        }
    }

    void MultiMmtIterator::next() noexcept {
        size_t recurse_depth = this->party_list.size() - 1;
        bool recursing = true;
        while (recursing) {
            ++this->mmt_indices[recurse_depth];
            ++this->mmt_iters[recurse_depth];
            if (this->mmt_iters[recurse_depth] == this->party_list[recurse_depth]->Measurements.end()) {
                this->mmt_indices[recurse_depth] = 0;
                this->mmt_iters[recurse_depth] = this->party_list[recurse_depth]->Measurements.begin();
                this->global_mmt_indices[recurse_depth] = this->mmt_iters[recurse_depth]->Index().global_mmt;

                if (recurse_depth > 0) {
                    --recurse_depth;
                } else {
                    this->is_done = true;
                    recursing = false;
                }
            } else {
                // Party is not at end...
                this->global_mmt_indices[recurse_depth] = this->mmt_iters[recurse_depth]->Index().global_mmt;
                recursing = false;
            }
        }
    }

    size_t MultiMmtIterator::count_outcomes() const noexcept {
        size_t outcomes = 1;
        for (size_t index = 0; index < this->mmt_iters.size(); ++index) {
            if (this->mmt_iters[index] == this->party_list[index]->Measurements.end()) {
                return 0;
            }
            outcomes *= mmt_iters[index]->num_outcomes;
        }
        return outcomes;
    }

    size_t MultiMmtIterator::count_operators() const noexcept {
        size_t outcomes = 1;
        for (size_t index = 0; index < this->mmt_iters.size(); ++index) {
            if (this->mmt_iters[index] == this->party_list[index]->Measurements.end()) {
                return 0;
            }
            outcomes *= (this->mmt_iters[index]->num_outcomes - 1);
        }

        return outcomes;
    }

    MultiMmtIterator::OpSeqIterator::OpSeqIterator(const MultiMmtIterator &iterRef, bool end)
            : mmIter{&iterRef}, indexIter(getMmtOpCounts(iterRef), end) {

    }

    MultiMmtIterator::OutcomeIndexIterator::OutcomeIndexIterator(const MultiMmtIterator &theIter, bool end)
            : mmIter(&theIter), indexIter(getMmtOutcomeCounts(theIter), end), is_implicit(theIter.dimension(), false) {
        check_implicit();
    }

    void MultiMmtIterator::OutcomeIndexIterator::check_implicit() {
        if (this->indexIter.done()) {
            return;
        }

        this->num_implicit = 0;
        for (size_t mIndex = 0; mIndex < mmIter->dimension(); ++mIndex) {
            const bool elemImpl = (this->indexIter[mIndex] >= mmIter->mmt_iters[mIndex]->num_operators());
            this->is_implicit[mIndex] = elemImpl;
            if (elemImpl) {
                ++this->num_implicit;
            }
        }
    }

    std::vector<std::pair<MultiMmtIterator::mmt_iter_t, size_t>>
    MultiMmtIterator::OutcomeIndexIterator::implicit_indices(const bool getImplicit) const {
        auto output = std::vector<std::pair<MultiMmtIterator::mmt_iter_t, size_t>>{};
        if (getImplicit) {
            output.reserve(this->num_implicit);
        } else {
            output.reserve(mmIter->dimension() - this->num_implicit);
        }

        for (size_t mIndex = 0; mIndex < mmIter->dimension(); ++mIndex) {
            if (getImplicit == this->is_implicit[mIndex]) {
                output.emplace_back(this->mmIter->mmt_iters[mIndex], this->indexIter[mIndex]);
            }
        }
        return output;
    }

    MultiMmtIterator::OutcomeIndexIterator &MultiMmtIterator::OutcomeIndexIterator::operator++() noexcept {
        ++indexIter;
        check_implicit();
        if (0 == this->num_implicit) {
            ++operNumber;
        }
        return *this;
    }

    OperatorSequence MultiMmtIterator::OpSeqIterator::operator*() const {
        assert(mmIter != nullptr);

        std::vector<Operator> ops;
        ops.reserve(indexIter.limits().size());
        for (size_t i = 0; i < indexIter.limits().size(); ++i) {
            ops.emplace_back(this->mmIter->party_list[i]->measurement_outcome(
                    this->mmIter->mmt_iters[i]->Index().mmt,
                    indexIter[i]
            ));
        }

        return OperatorSequence(std::move(ops), this->mmIter->contextPtr);
    }
}