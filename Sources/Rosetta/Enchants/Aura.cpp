// This code is based on Sabberstone project.
// Copyright (c) 2017-2019 SabberStone Team, darkfriend77 & rnilva
// RosettaStone is hearthstone simulator using C++ with reinforcement learning.
// Copyright (c) 2019 Chris Ohk, Youngjoong Kim, SeungHyun Jeon

#include <Rosetta/Cards/Cards.hpp>
#include <Rosetta/Enchants/Aura.hpp>
#include <Rosetta/Games/Game.hpp>
#include <Rosetta/Models/Battlefield.hpp>
#include <Rosetta/Models/Enchantment.hpp>
#include <Rosetta/Models/Player.hpp>

#include <algorithm>

namespace RosettaStone
{
Aura::Aura(std::string&& enchantmentID, AuraType type)
    : m_enchantmentID(enchantmentID), m_type(type)
{
    // Do nothing
}

Aura::Aura(Aura& prototype, Entity& owner)
    : condition(prototype.condition),
      restless(prototype.restless),
      m_enchantmentID(prototype.m_enchantmentID),
      m_type(prototype.m_type),
      m_owner(&owner),
      m_effects(prototype.m_effects),
      m_turnOn(prototype.m_turnOn)
{
    // Do nothing
}

void Aura::SetToBeUpdated(bool value)
{
    m_toBeUpdated = value;
}

void Aura::Activate(Entity& owner)
{
    Card card = Cards::FindCardByID(m_enchantmentID);

    if (m_effects.empty())
    {
        m_effects = card.power.GetEnchant().value().effects;
    }

    Aura* instance = new Aura(*this, owner);

    owner.owner->GetGame()->auras.emplace_back(instance);
    owner.onGoingEffect = instance;

    instance->AddToField();

    switch (m_type)
    {
        case AuraType::FIELD:
        {
            for (auto& minion : owner.owner->GetField().GetAllMinions())
            {
                if (condition == nullptr || condition->Evaluate(minion))
                {
                    if (card.power.GetTrigger().has_value())
                    {
                        const auto enchantment = Enchantment::GetInstance(
                            *owner.owner, card, minion);
                        card.power.GetTrigger().value().Activate(*enchantment);
                    }
                }

                instance->m_tempList.emplace_back(minion);
            }
            break;
        }
        case AuraType::FIELD_EXCEPT_SOURCE:
        {
            for (auto& minion : owner.owner->GetField().GetAllMinions())
            {
                if (minion == &owner)
                {
                    continue;
                }

                if (condition == nullptr || condition->Evaluate(minion))
                {
                    if (card.power.GetTrigger().has_value())
                    {
                        const auto enchantment = Enchantment::GetInstance(
                            *owner.owner, card, minion);
                        card.power.GetTrigger().value().Activate(*enchantment);
                    }
                }

                instance->m_tempList.emplace_back(minion);
            }
            break;
        }
        default:
            break;
    }
}

void Aura::Update()
{
    if (!m_toBeUpdated)
    {
        return;
    }

    UpdateInternal();
}

void Aura::Remove()
{
    m_turnOn = false;
    m_toBeUpdated = true;
    m_owner->onGoingEffect = nullptr;
}

void Aura::RemoveEntity(Entity& entity)
{
    if (&entity == m_owner)
    {
        Remove();
    }
    else
    {
        if (m_appliedEntities.empty())
        {
            return;
        }
        const auto iter = std::find(m_appliedEntities.cbegin(),
                                    m_appliedEntities.cend(), &entity);
        if (iter != m_appliedEntities.end())
        {
            m_appliedEntities.erase(iter);
        }
    }
}

void Aura::Apply(Entity& entity)
{
    const auto iter =
        std::find(m_appliedEntities.begin(), m_appliedEntities.end(), &entity);
    if (iter != m_appliedEntities.end())
    {
        if (!restless || (condition != nullptr && condition->Evaluate(&entity)))
        {
            return;
        }

        for (auto& effect : m_effects)
        {
            effect.Remove(*entity.auraEffects);
        }

        m_appliedEntities.erase(iter);
    }

    if (condition != nullptr && !condition->Evaluate(&entity))
    {
        return;
    }

    for (auto& effect : m_effects)
    {
        effect.Apply(*entity.auraEffects);
    }

    m_appliedEntities.emplace_back(&entity);
}

void Aura::AddToField()
{
    switch (m_type)
    {
        case AuraType::ADJACENT:
        case AuraType::FIELD:
        case AuraType::FIELD_EXCEPT_SOURCE:
            m_owner->owner->GetField().auras.emplace_back(this);
            break;
        default:
            throw std::invalid_argument(
                "Aura::AddToField() - Invalid aura type!");
    }
}

void Aura::UpdateInternal()
{
    if (m_turnOn)
    {
        if (!m_tempList.empty())
        {
            for (auto& temp : m_tempList)
            {
                Apply(*temp);
            }

            m_tempList.clear();
        }

        switch (m_type)
        {
            case AuraType::ADJACENT:
            {
                const auto minion = dynamic_cast<Minion*>(m_owner);
                auto& field = m_owner->owner->GetField();
                const int pos =
                    static_cast<int>(field.FindMinionPos(*minion).value());

                const int entitySize =
                    static_cast<int>(m_appliedEntities.size());
                for (int i = entitySize - 1; i >= 0; --i)
                {
                    Entity* entity = m_appliedEntities[i];
                    const int fieldPos = static_cast<int>(
                        field.FindMinionPos(*dynamic_cast<Minion*>(entity))
                            .value());

                    if (std::abs(pos - fieldPos) == 1)
                    {
                        continue;
                    }

                    for (auto& effect : m_effects)
                    {
                        effect.Remove(*entity->auraEffects);
                    }

                    auto iter = std::find(m_appliedEntities.begin(),
                                          m_appliedEntities.end(), entity);
                    if (iter != m_appliedEntities.end())
                    {
                        m_appliedEntities.erase(iter);
                    }
                }

                if (pos > 0)
                {
                    const auto leftMinion = field.GetMinion(pos - 1);
                    if (leftMinion != nullptr)
                    {
                        Apply(*leftMinion);
                    }
                }
                if (pos < static_cast<int>(FIELD_SIZE - 1))
                {
                    const auto rightMinion = field.GetMinion(pos + 1);
                    if (rightMinion != nullptr)
                    {
                        Apply(*rightMinion);
                    }
                }

                break;
            }
            case AuraType::FIELD:
                for (auto& minion : m_owner->owner->GetField().GetAllMinions())
                {
                    Apply(*minion);
                }
                break;
            case AuraType::FIELD_EXCEPT_SOURCE:
            {
                for (auto& minion : m_owner->owner->GetField().GetAllMinions())
                {
                    if (minion != m_owner)
                    {
                        Apply(*minion);
                    }
                }
                break;
            }
            default:
                throw std::invalid_argument(
                    "Aura::UpdateInternal() - Invalid aura type!");
        }

        if (!restless)
        {
            m_toBeUpdated = false;
        }
    }
    else
    {
        RemoveInternal();
    }
}

void Aura::RemoveInternal()
{
    switch (m_type)
    {
        case AuraType::ADJACENT:
        case AuraType::FIELD:
        case AuraType::FIELD_EXCEPT_SOURCE:
        {
            auto auras = m_owner->owner->GetField().auras;
            const auto iter = std::find(auras.begin(), auras.end(), this);
            auras.erase(iter);
            break;
        }
        default:
            throw std::invalid_argument(
                "Aura::RemoveInternal() - Invalid aura type!");
    }

    for (auto& entity : m_appliedEntities)
    {
        for (auto& effect : m_effects)
        {
            effect.Remove(*entity->auraEffects);
        }
    }

    auto auras = m_owner->owner->GetGame()->auras;
    const auto iter = std::find(auras.begin(), auras.end(), this);
    auras.erase(iter);
}

AuraType Aura::GetAuraType() const
{
    return m_type;
}

std::vector<Effect> Aura::GetEffects() const
{
    return m_effects;
}

std::vector<Entity*> Aura::GetAppliedEntities() const
{
    return m_appliedEntities;
}
}  // namespace RosettaStone
