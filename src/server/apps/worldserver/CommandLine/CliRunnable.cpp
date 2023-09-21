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

/// \addtogroup Acored
/// @{
/// \file

#include "CliRunnable.h"
#include "CliCommandMgr.h"
#include "Config.h"
#include "ObjectMgr.h"
#include <fmt/core.h>

#if WARHEAD_PLATFORM != WARHEAD_PLATFORM_WINDOWS
#include "Chat.h"
#include "ChatCommand.h"
#include <cstring>
#include <readline/history.h>
#include <readline/readline.h>
#endif

static constexpr char CLI_PREFIX[] = "AC> ";

static inline void PrintCliPrefix()
{
    fmt::print(CLI_PREFIX);
}

#if WARHEAD_PLATFORM != WARHEAD_PLATFORM_WINDOWS
namespace Warhead::Impl::Readline
{
    static std::vector<std::string> vec;
    char* cli_unpack_vector(char const*, int state)
    {
        static size_t i=0;
        if (!state)
            i = 0;
        if (i < vec.size())
            return strdup(vec[i++].c_str());
        else
            return nullptr;
    }

    char** cli_completion(char const* text, int /*start*/, int /*end*/)
    {
        ::rl_attempted_completion_over = 1;
        vec = Warhead::ChatCommands::GetAutoCompletionsFor(CliHandler(nullptr), text);
        return ::rl_completion_matches(text, &cli_unpack_vector);
    }

    int cli_hook_func()
    {
        if (World::IsStopped())
            ::rl_done = 1;
        return 0;
    }
}
#endif

void utf8print(std::string_view str)
{
#if WARHEAD_PLATFORM == WARHEAD_PLATFORM_WINDOWS
    WriteWinConsole(str);
#else
    fmt::print(str);
#endif
}

void commandFinished(bool /*success*/)
{
    PrintCliPrefix();
}

#if WARHEAD_PLATFORM == WARHEAD_PLATFORM_UNIX
// Non-blocking keypress detector, when return pressed, return 1, else always return 0
int kb_hit_return()
{
    struct timeval tv{};
    fd_set fds;
    tv.tv_sec = 0;
    tv.tv_usec = 0;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds);
    select(STDIN_FILENO+1, &fds, nullptr, nullptr, &tv);
    return FD_ISSET(STDIN_FILENO, &fds);
}
#endif

/// %Thread start
void CliThread()
{
#if WARHEAD_PLATFORM == WARHEAD_PLATFORM_WINDOWS
    // print this here the first time
    // later it will be printed after command queue updates
    PrintCliPrefix();
#else
    ::rl_attempted_completion_function = &Warhead::Impl::Readline::cli_completion;
    {
        static char BLANK = '\0';
        ::rl_completer_word_break_characters = &BLANK;
    }
    ::rl_event_hook = &Warhead::Impl::Readline::cli_hook_func;
#endif

    if (sConfigMgr->GetOption<bool>("BeepAtStart", true))
        printf("\a");   // \a = Alert

#if WARHEAD_PLATFORM == WARHEAD_PLATFORM_WINDOWS
    if (sConfigMgr->GetOption<bool>("FlashAtStart", true))
    {
        FLASHWINFO fInfo;
        fInfo.cbSize = sizeof(FLASHWINFO);
        fInfo.dwFlags = FLASHW_TRAY | FLASHW_TIMERNOFG;
        fInfo.hwnd = GetConsoleWindow();
        fInfo.uCount = 0;
        fInfo.dwTimeout = 0;
        FlashWindowEx(&fInfo);
    }
#endif

    ///- As long as the World is running (no World::m_stopEvent), get the command line and handle it
    while (!World::IsStopped())
    {
        fflush(stdout);

        std::string command;

#if WARHEAD_PLATFORM == WARHEAD_PLATFORM_WINDOWS
        if (!ReadWinConsole(command))
            continue;
#else
        char* command_str = readline(CLI_PREFIX);
        ::rl_bind_key('\t', ::rl_complete);
        if (command_str != nullptr)
        {
            command = command_str;
            free(command_str);
        }
#endif
        if (!command.empty())
        {
            Optional<std::size_t> nextLineIndex = RemoveCRLF(command);
            if (nextLineIndex && *nextLineIndex == 0)
            {
#if WARHEAD_PLATFORM == WARHEAD_PLATFORM_WINDOWS
                PrintCliPrefix();
#endif
                continue;
            }

            fflush(stdout);

            sCliCommandMgr->AddCommand(command, [](std::string_view command)
            {
                utf8print(command);
            },
            [](bool success)
            {
                commandFinished(success);
            });

#if WARHEAD_PLATFORM != WARHEAD_PLATFORM_WINDOWS
            add_history(command.c_str());
#endif
        }
        else if (feof(stdin))
        {
            World::StopNow(SHUTDOWN_EXIT_CODE);
        }
    }
}
