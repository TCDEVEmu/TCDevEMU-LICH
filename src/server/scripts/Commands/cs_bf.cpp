/*
 * This file is part of the WarheadCore Project. See AUTHORS file for Copyright information
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Affero General Public License as published by the
 * Free Software Foundation; either version 3 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/* ScriptData
Name: bf_commandscript
%Complete: 100
Comment: All bf related commands
Category: commandscripts
EndScriptData */

#include "BattlefieldMgr.h"
#include "Chat.h"
#include "ScriptObject.h"

#if WARHEAD_COMPILER == WARHEAD_COMPILER_GNU
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

using namespace Warhead::ChatCommands;

class bf_commandscript : public CommandScript
{
public:
    bf_commandscript() : CommandScript("bf_commandscript") { }

    ChatCommandTable GetCommands() const override
    {
        static ChatCommandTable battlefieldcommandTable =
        {
            { "start",  HandleBattlefieldStart,  SEC_ADMINISTRATOR, Console::No },
            { "stop",   HandleBattlefieldEnd,    SEC_ADMINISTRATOR, Console::No },
            { "switch", HandleBattlefieldSwitch, SEC_ADMINISTRATOR, Console::No },
            { "timer",  HandleBattlefieldTimer,  SEC_ADMINISTRATOR, Console::No },
            { "enable", HandleBattlefieldEnable, SEC_ADMINISTRATOR, Console::No }
        };
        static ChatCommandTable commandTable =
        {
            { "bf", battlefieldcommandTable }
        };
        return commandTable;
    }

    static bool HandleBattlefieldStart(ChatHandler* handler, uint32 battleId)
    {
        Battlefield* bf = sBattlefieldMgr->GetBattlefieldByBattleId(battleId);

        if (!bf)
            return false;

        bf->StartBattle();

        if (battleId == 1)
            handler->SendGlobalGMSysMessage("Wintergrasp (Command start used)");

        return true;
    }

    static bool HandleBattlefieldEnd(ChatHandler* handler, uint32 battleId)
    {
        Battlefield* bf = sBattlefieldMgr->GetBattlefieldByBattleId(battleId);

        if (!bf)
            return false;

        bf->EndBattle(true);

        if (battleId == 1)
            handler->SendGlobalGMSysMessage("Wintergrasp (Command stop used)");

        return true;
    }

    static bool HandleBattlefieldEnable(ChatHandler* handler, uint32 battleId)
    {
        Battlefield* bf = sBattlefieldMgr->GetBattlefieldByBattleId(battleId);

        if (!bf)
            return false;

        if (bf->IsEnabled())
        {
            bf->ToggleBattlefield(false);
            if (battleId == 1)
                handler->SendGlobalGMSysMessage("Wintergrasp is disabled");
        }
        else
        {
            bf->ToggleBattlefield(true);
            if (battleId == 1)
                handler->SendGlobalGMSysMessage("Wintergrasp is enabled");
        }

        return true;
    }

    static bool HandleBattlefieldSwitch(ChatHandler* handler, uint32 battleId)
    {
        Battlefield* bf = sBattlefieldMgr->GetBattlefieldByBattleId(battleId);

        if (!bf)
            return false;

        bf->EndBattle(false);
        if (battleId == 1)
            handler->SendGlobalGMSysMessage("Wintergrasp (Command switch used)");

        return true;
    }

    static bool HandleBattlefieldTimer(ChatHandler* handler, uint32 battleId, std::string_view timeStr)
    {
        if (timeStr.empty())
        {
            return false;
        }

        if (Warhead::StringTo<int32>(timeStr).value_or(0) < 0)
        {
            handler->SendSysMessage(LANG_BAD_VALUE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        Seconds time = Warhead::Time::TimeStringTo(timeStr);
        if (time == 0s)
        {
            handler->SendSysMessage(LANG_BAD_VALUE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        Battlefield* bf = sBattlefieldMgr->GetBattlefieldByBattleId(battleId);

        if (!bf)
            return false;

        bf->SetTimer(time.count() * IN_MILLISECONDS);
        bf->SendInitWorldStatesToAll();

        if (battleId == 1)
            handler->SendGlobalGMSysMessage("Wintergrasp (Command timer used)");

        return true;
    }
};

void AddSC_bf_commandscript()
{
    new bf_commandscript();
}
