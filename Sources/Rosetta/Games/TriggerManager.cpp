// Copyright (c) 2019 Chris Ohk, Youngjoong Kim, SeungHyun Jeon

// We are making my contributions/submissions to this project solely in our
// personal capacity and are not conveying any rights to any intellectual
// property of any third parties.

#include <Rosetta/Games/TriggerManager.hpp>

namespace RosettaStone
{
void TriggerManager::OnStartTurnTrigger(Player* player, Entity* sender) const
{
    if (startTurnTrigger != nullptr)
    {
        startTurnTrigger(player, sender);
    }
}

void TriggerManager::OnEndTurnTrigger(Player* player, Entity* sender) const
{
    if (endTurnTrigger != nullptr)
    {
        endTurnTrigger(player, sender);
    }
}

void TriggerManager::OnHealTrigger(Player* player, Entity* sender) const
{
    if (healTrigger != nullptr)
    {
        healTrigger(player, sender);
    }
}

void TriggerManager::OnAttackTrigger(Player* player, Entity* sender) const
{
    if (attackTrigger != nullptr)
    {
        attackTrigger(player, sender);
    }
}

void TriggerManager::OnSummonTrigger(Player* player, Entity* sender) const
{
    if (summonTrigger != nullptr)
    {
        summonTrigger(player, sender);
    }
}

void TriggerManager::OnTakeDamageTrigger(Player* player, Entity* sender) const
{
    if (takeDamageTrigger != nullptr)
    {
        takeDamageTrigger(player, sender);
    }
}
}  // namespace RosettaStone
