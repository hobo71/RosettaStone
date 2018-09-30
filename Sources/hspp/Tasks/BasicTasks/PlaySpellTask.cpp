// Copyright (c) 2018 Chris Ohk, Youngjoong Kim, SeungHyun Jeon

// We are making my contributions/submissions to this project solely in our
// personal capacity and are not conveying any rights to any intellectual
// property of any third parties.

#include <hspp/Enums/TaskEnums.h>
#include <hspp/Tasks/BasicTasks/ModifyManaTask.h>
#include <hspp/Tasks/BasicTasks/PlaySpellTask.h>

#include <algorithm>

namespace Hearthstonepp::BasicTasks
{
PlaySpellTask::PlaySpellTask(TaskAgent& agent, Entity* entity)
    : m_entity(entity), m_requirement(TaskID::SELECT_TARGET, agent)
{
    // Do Nothing
}

TaskID PlaySpellTask::GetTaskID() const
{
    return TaskID::PLAY_SPELL;
}

MetaData PlaySpellTask::Impl(Player& player1, Player& player2)
{
    TaskMeta meta;
    // Get Position Response from GameInterface
    m_requirement.Interact(player1.id, meta);

    using ResponsePlaySpell = FlatData::ResponsePlaySpell;
    const auto& buffer = meta.GetBuffer();
    auto req = flatbuffers::GetRoot<ResponsePlaySpell>(buffer.get());

    if (req == nullptr)
    {
        return MetaData::PLAY_SPELL_FLATBUFFER_NULLPTR;
    }

    TargetType targetType = TargetType::_from_integral(req->targetType());
    BYTE position = req->position();

    if (targetType == +TargetType::MY_FIELD)
    {
        // Verify field position
        if (position > player1.field.size())
        {
            return MetaData::PLAY_SPELL_POSITION_OUT_OF_RANGE;
        }

        // Verify valid target
        if (player1.field[position] == nullptr)
        {
            return MetaData::PLAY_SPELL_INVALID_TARGET;
        }
    }

    BYTE cost = static_cast<BYTE>(m_entity->card->cost);
    MetaData modified = ModifyManaTask(NumMode::SUB, ManaMode::EXIST, cost)
                            .Run(player1, player2);

    // Process PowerTasks
    if (m_entity->card->power != nullptr)
    {
        for (auto& power : m_entity->card->power->powerTask)
        {
            power->target = player1.field[position];
            power->Run(player1, player2);
        }
    }

    if (modified == MetaData::MODIFY_MANA_SUCCESS)
    {
        return MetaData::PLAY_SPELL_SUCCESS;
    }

    return MetaData::PLAY_SPELL_MODIFY_MANA_FAIL;
}
}  // namespace Hearthstonepp::BasicTasks