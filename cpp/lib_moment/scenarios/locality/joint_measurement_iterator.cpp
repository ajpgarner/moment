/**
 * multi_mmt_iterator.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "joint_measurement_iterator.h"
#include "locality_context.h"

#include <stdexcept>

namespace Moment::Locality {
    namespace {
        std::vector<size_t> getMmtOpCounts(const JointMeasurementIterator &multiMmtIterator) {
            std::vector<size_t> output;
            output.reserve(multiMmtIterator.count_indices());
            for (auto mmIter: multiMmtIterator.iters()) {
                output.push_back(mmIter->num_operators());
            }
            return output;
        }

        std::vector<size_t> getMmtOutcomeCounts(const JointMeasurementIterator &multiMmtIterator) {
            std::vector<size_t> output;
            output.reserve(multiMmtIterator.count_indices());
            for (auto mmIter: multiMmtIterator.iters()) {
                output.push_back(mmIter->num_outcomes);
            }
            return output;
        }

    }

    JointMeasurementIterator::JointMeasurementIterator(const LocalityContext &contextRef, party_list_t &&list)
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

    void JointMeasurementIterator::next() noexcept {
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

    size_t JointMeasurementIterator::count_outcomes() const noexcept {
        size_t outcomes = 1;
        for (size_t index = 0; index < this->mmt_iters.size(); ++index) {
            if (this->mmt_iters[index] == this->party_list[index]->Measurements.end()) {
                return 0;
            }
            outcomes *= mmt_iters[index]->num_outcomes;
        }
        return outcomes;
    }

    size_t JointMeasurementIterator::count_operators() const noexcept {
        size_t outcomes = 1;
        for (size_t index = 0; index < this->mmt_iters.size(); ++index) {
            if (this->mmt_iters[index] == this->party_list[index]->Measurements.end()) {
                return 0;
            }
            outcomes *= (this->mmt_iters[index]->num_outcomes - 1);
        }

        return outcomes;
    }

    OutcomeIndexIterator JointMeasurementIterator::begin_outcomes() const noexcept {
        return OutcomeIndexIterator{getMmtOutcomeCounts(*this), false};
    }

    OutcomeIndexIterator JointMeasurementIterator::end_outcomes() const noexcept {
        return OutcomeIndexIterator{getMmtOutcomeCounts(*this), true};
    }

    JointMeasurementIterator::OpSeqIterator::OpSeqIterator(const JointMeasurementIterator &iterRef, bool end)
            : mmIter{&iterRef}, indexIter(getMmtOpCounts(iterRef), end) {

    }

    OperatorSequence JointMeasurementIterator::OpSeqIterator::operator*() const {
        assert(mmIter != nullptr);

        std::vector<oper_name_t > ops;
        ops.reserve(indexIter.limits().size());
        for (size_t i = 0; i < indexIter.limits().size(); ++i) {
            ops.emplace_back(this->mmIter->party_list[i]->measurement_outcome(
                    this->mmIter->mmt_iters[i]->Index().mmt,
                    indexIter[i]
            ));
        }

        return OperatorSequence(std::move(ops), *this->mmIter->contextPtr);
    }
}