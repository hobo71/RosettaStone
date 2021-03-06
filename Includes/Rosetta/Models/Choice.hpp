// Copyright (c) 2019 Chris Ohk, Youngjoong Kim, SeungHyun Jeon

// We are making my contributions/submissions to this project solely in our
// personal capacity and are not conveying any rights to any intellectual
// property of any third parties.

#ifndef ROSETTASTONE_CHOICE_HPP
#define ROSETTASTONE_CHOICE_HPP

#include <vector>

namespace RosettaStone
{
//! The action type of choice.
enum class ChoiceAction
{
    INVALID = 0,    //!< Invalid action.
    HAND = 1,       //!< Hand.
};

//!
//! \brief Choice struct.
//!
//! This struct holds all configuration values to offer a choice of options.
//!
struct Choice
{
    ChoiceType choiceType = ChoiceType::INVALID;
    ChoiceAction choiceAction = ChoiceAction::INVALID;

    std::vector<std::size_t> choices;
};
}  // namespace RosettaStone

#endif  // ROSETTASTONE_CHOICE_HPP
