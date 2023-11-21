/**
 * osg_pair.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once

#include "operator_sequence_generator.h"

#include <cassert>
#include <memory>
#include <utility>

namespace Moment {
    /**
     * An operator sequence generator and its conjugate
     */
    struct OSGPair {
    private:
        std::unique_ptr<OperatorSequenceGenerator> forward_osg;
        std::unique_ptr<OperatorSequenceGenerator> conjugate_osg;

    public:
        explicit OSGPair(std::unique_ptr<OperatorSequenceGenerator> fwd,
                         std::unique_ptr<OperatorSequenceGenerator> conjugated = nullptr) noexcept
                : forward_osg{std::move(fwd)}, conjugate_osg{std::move(conjugated)} {
            assert(forward_osg != nullptr);
        }

        OSGPair(OSGPair&& rhs) = default;

        /**
         * True if there is no conjugate OSG.
         */
        [[nodiscard]] inline bool self_adjoint() const noexcept {
            return (conjugate_osg == nullptr);
        }

        /**
         * Return the operator sequence generator.
         */
        [[nodiscard]] const OperatorSequenceGenerator& operator()() const noexcept {
            return *forward_osg;
        }

        /**
         * Return the conjugate of the operator sequence generator (or the generator itself if self-adjoint).
         */
        [[nodiscard]] const OperatorSequenceGenerator& conjugate() const noexcept {
            return (conjugate_osg ? *conjugate_osg : *forward_osg);
        }

        /**
         * Returns a pair consisting of the operator sequence generator and its conjugate.
         * @return
         */
        [[nodiscard]] std::pair<const OperatorSequenceGenerator&, OperatorSequenceGenerator&> pair() const noexcept {
            return {*forward_osg, conjugate_osg ? *conjugate_osg : *forward_osg};
        };

    };
}
