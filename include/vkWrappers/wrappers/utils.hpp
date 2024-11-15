/*
 * Copyright (C) 2024 Adrien ARNAUD
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include <cstdint>
#include <cstdio>
#include <stdexcept>
#include <vector>
#include <vulkan/vk_enum_string_helper.h>

#define LOG_LEVEL_VERBOSE  0
#define LOG_LEVEL_INFO     1
#define LOG_LEVEL_WARNING  2
#define LOG_LEVEL_ERROR    3
#define LOG_LEVEL_CRITICAL 4

#ifndef LOG_LEVEL
#    define LOG_LEVEL LOG_LEVEL_INFO
#endif

#ifdef DEBUG
#    define LOG_DEBUG_VALUE 1
#else
#    define LOG_DEBUG_VALUE 0
#endif

// -------------------------------------------------------------------------------------------------

#define VKW_INIT_CHECK_VK(f)                                                                       \
    {                                                                                              \
        VkResult res = f;                                                                          \
        if(res != VK_SUCCESS)                                                                      \
        {                                                                                          \
            utils::Log::Error("wkw", #f ": %s", string_VkResult(res));                             \
            clear();                                                                               \
            return false;                                                                          \
        }                                                                                          \
    }
#define VKW_INIT_CHECK_BOOL(f)                                                                     \
    {                                                                                              \
        if(!f)                                                                                     \
        {                                                                                          \
            utils::Log::Error("wkw", #f ": failed");                                               \
            clear();                                                                               \
            return false;                                                                          \
        }                                                                                          \
    }
#define VKW_CHECK_VK_THROW(f, msg)                                                                 \
    {                                                                                              \
        VkResult err = f;                                                                          \
        if(err != VK_SUCCESS)                                                                      \
        {                                                                                          \
            utils::Log::Error("vkw", #f ": %s\n", string_VkResult(err));                           \
            throw std::runtime_error(msg);                                                         \
        }                                                                                          \
    }
#define CHECK_BOOL_RETURN_FALSE(f)                                                                 \
    {                                                                                              \
        bool res = f;                                                                              \
        if(!res)                                                                                   \
        {                                                                                          \
            utils::Log::Error("vkw", #f ": failed");                                               \
            return false;                                                                          \
        }                                                                                          \
    }

#define CHECK_VK(f, msg)                                                                           \
    {                                                                                              \
        VkResult err = f;                                                                          \
        if(err != VK_SUCCESS)                                                                      \
        {                                                                                          \
            fprintf(stderr, "%s : %s\n", msg, string_VkResult(err));                               \
            exit(1);                                                                               \
        }                                                                                          \
    }
#define CHECK_VK_RETURN_FALSE(f)                                                                   \
    {                                                                                              \
        VkResult err = f;                                                                          \
        if(err != VK_SUCCESS)                                                                      \
        {                                                                                          \
            fprintf(stderr, "%s\n", string_VkResult(err));                                         \
            return false;                                                                          \
        }                                                                                          \
    }
#define CHECK_VK_THROW(f, msg)                                                                     \
    {                                                                                              \
        VkResult err = f;                                                                          \
        if(err != VK_SUCCESS)                                                                      \
        {                                                                                          \
            char errMsg[512];                                                                      \
            sprintf(errMsg, "%s : %s\n", msg, string_VkResult(err));                               \
            throw std::runtime_error(msg);                                                         \
        }                                                                                          \
    }

#define CHECK_BOOL_THROW(f, msg)                                                                   \
    {                                                                                              \
        bool res = f;                                                                              \
        if(!res)                                                                                   \
        {                                                                                          \
            char errMsg[512];                                                                      \
            sprintf(errMsg, "%s : error\n", msg);                                                  \
            throw std::runtime_error(msg);                                                         \
        }                                                                                          \
    }

// -------------------------------------------------------------------------------------------------

namespace vkw
{
namespace utils
{
    inline uint32_t divUp(const uint32_t n, const uint32_t val) { return (n + val - 1) / val; }
    VkShaderModule createShaderModule(const VkDevice device, const std::vector<char>& src);
    std::vector<char> readShader(const std::string& filename);
} // namespace utils
} // namespace vkw

#ifdef __GNUC__
#    pragma GCC diagnostic push
#    pragma GCC diagnostic ignored "-Wformat-security"
#endif
namespace vkw
{
namespace utils
{
    class Log
    {
      public:
        template <typename... Args>
        static inline void Message(const char* format, Args... args)
        {
            if constexpr(LogLevel >= 0)
            {
                static thread_local char buf[lineSize];
                snprintf(buf, lineSize, format, args...);
                fprintf(stdout, "%s\n", buf);
            }
        }

        template <typename... Args>
        static inline void Time(const char* tag, const char* format, Args... args)
        {
            if constexpr(LogLevel <= LOG_LEVEL_VERBOSE)
            {
                static thread_local char buf[lineSize];
                snprintf(buf, lineSize, format, args...);
                fprintf(stdout, "\033[0;32m[T][%s]: %s\n\033[0m", tag, buf);
            }
        }

        template <typename... Args>
        static inline void Debug(const char* tag, const char* format, Args... args)
        {
            if constexpr(logDebug > 0)
            {
                static thread_local char buf[lineSize];
                snprintf(buf, lineSize, format, args...);
                fprintf(stdout, "\033[0;32m[D][%s]: %s\n\033[0m", tag, buf);
            }
        }
        template <typename... Args>
        static inline void Verbose(const char* tag, const char* format, Args... args)
        {
            if constexpr(LogLevel <= LOG_LEVEL_VERBOSE)
            {
                static thread_local char buf[lineSize];
                snprintf(buf, lineSize, format, args...);
                fprintf(stdout, "\033[0;34m[I][%s]: %s\n\033[0m", tag, buf);
            }
        }
        template <typename... Args>
        static inline void Info(const char* tag, const char* format, Args... args)
        {
            if constexpr(LogLevel <= LOG_LEVEL_INFO)
            {
                static thread_local char buf[lineSize];
                snprintf(buf, lineSize, format, args...);
                fprintf(stdout, "\033[0;34m[I][%s]: %s\n\033[0m", tag, buf);
            }
        }
        template <typename... Args>
        static inline void Warning(const char* tag, const char* format, Args... args)
        {
            if constexpr(LogLevel <= LOG_LEVEL_WARNING)
            {
                static thread_local char buf[lineSize];
                snprintf(buf, lineSize, format, args...);
                fprintf(stdout, "\033[0;33m[W][%s]: %s\n\033[0m", tag, buf);
                fflush(stdout);
            }
        }
        template <typename... Args>
        static inline void Error(const char* tag, const char* format, Args... args)
        {
            if constexpr(LogLevel <= LOG_LEVEL_ERROR)
            {
                static thread_local char buf[lineSize];
                snprintf(buf, lineSize, format, args...);
                fprintf(stdout, "\033[0;31m[E][%s]: %s\n\033[0m", tag, buf);
                fflush(stdout);
            }
        }

      private:
        static constexpr size_t lineSize = 1024;
        static constexpr int LogLevel = LOG_LEVEL;
        static constexpr int logDebug = LOG_DEBUG_VALUE;

        Log() = default;
    };
} // namespace utils
} // namespace vkw
#ifdef __GNUC__
#    pragma GCC diagnostic pop
#endif
