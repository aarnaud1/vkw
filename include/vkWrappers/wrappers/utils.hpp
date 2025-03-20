/*
 * Copyright (C) 2025 Adrien ARNAUD
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
#include <stdexcept>
#include <vector>
#include <volk.h>

#ifdef __ANDROID__
#    include <android/log.h>
#else
#    include <cstdio>
#    define LOG_LEVEL_VERBOSE  0
#    define LOG_LEVEL_INFO     1
#    define LOG_LEVEL_WARNING  2
#    define LOG_LEVEL_ERROR    3
#    define LOG_LEVEL_CRITICAL 4

#    ifndef LOG_LEVEL
#        define LOG_LEVEL LOG_LEVEL_INFO
#    endif

#    ifdef DEBUG
#        define LOG_DEBUG_VALUE 1
#    else
#        define LOG_DEBUG_VALUE 0
#    endif
#endif

#ifndef LOG_TAG
#    define LOG_TAG "vkw"
#endif

static inline const char* getStringResult(const VkResult res)
{
    switch(res)
    {
        case VK_SUCCESS:
            return "VK_SUCCESS";
            break;
        case VK_NOT_READY:
            return "VK_NOT_READY";
            break;
        case VK_TIMEOUT:
            return "VK_TIMEOUT";
            break;
        case VK_EVENT_SET:
            return "VK_EVENT_SET";
            break;
        case VK_EVENT_RESET:
            return "VK_EVENT_RESET";
            break;
        case VK_INCOMPLETE:
            return "VK_INCOMPLETE";
            break;
        case VK_ERROR_OUT_OF_HOST_MEMORY:
            return "VK_ERROR_OUT_OF_HOST_MEMORY";
            break;
        case VK_ERROR_OUT_OF_DEVICE_MEMORY:
            return "VK_ERROR_OUT_OF_DEVICE_MEMORY";
            break;
        case VK_ERROR_INITIALIZATION_FAILED:
            return "VK_ERROR_INITIALIZATION_FAILED";
            break;
        case VK_ERROR_DEVICE_LOST:
            return "VK_ERROR_DEVICE_LOST";
            break;
        case VK_ERROR_MEMORY_MAP_FAILED:
            return "VK_ERROR_MEMORY_MAP_FAILED";
            break;
        case VK_ERROR_LAYER_NOT_PRESENT:
            return "VK_ERROR_LAYER_NOT_PRESENT";
            break;
        case VK_ERROR_EXTENSION_NOT_PRESENT:
            return "VK_ERROR_EXTENSION_NOT_PRESENT";
            break;
        case VK_ERROR_FEATURE_NOT_PRESENT:
            return "VK_ERROR_FEATURE_NOT_PRESENT";
            break;
        case VK_ERROR_INCOMPATIBLE_DRIVER:
            return "VK_ERROR_INCOMPATIBLE_DRIVER";
            break;
        case VK_ERROR_TOO_MANY_OBJECTS:
            return "VK_ERROR_TOO_MANY_OBJECTS";
            break;
        case VK_ERROR_FORMAT_NOT_SUPPORTED:
            return "VK_ERROR_FORMAT_NOT_SUPPORTED";
            break;
        case VK_ERROR_FRAGMENTED_POOL:
            return "VK_ERROR_FRAGMENTED_POOL";
            break;
        case VK_ERROR_UNKNOWN:
            return "VK_ERROR_UNKNOWN";
            break;
        case VK_ERROR_OUT_OF_POOL_MEMORY:
            return "VK_ERROR_OUT_OF_POOL_MEMORY";
            break;
        case VK_ERROR_INVALID_EXTERNAL_HANDLE:
            return "VK_ERROR_INVALID_EXTERNAL_HANDLE";
            break;
        case VK_ERROR_FRAGMENTATION:
            return "VK_ERROR_FRAGMENTATION";
            break;
        case VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS:
            return "VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS";
            break;
        case VK_PIPELINE_COMPILE_REQUIRED:
            return "VK_PIPELINE_COMPILE_REQUIRED";
            break;
#ifdef VK_VERSION_1_4
        case VK_ERROR_NOT_PERMITTED:
            return "VK_ERROR_NOT_PERMITTED";
            break;
#endif
        case VK_ERROR_SURFACE_LOST_KHR:
            return "VK_ERROR_SURFACE_LOST_KHR";
            break;
        case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
            return "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR";
            break;
        case VK_SUBOPTIMAL_KHR:
            return "VK_SUBOPTIMAL_KHR";
            break;
        case VK_ERROR_OUT_OF_DATE_KHR:
            return "VK_ERROR_OUT_OF_DATE_KHR";
            break;
        case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:
            return "VK_ERROR_INCOMPATIBLE_DISPLAY_KHR";
            break;
        case VK_ERROR_VALIDATION_FAILED_EXT:
            return "VK_ERROR_VALIDATION_FAILED_EXT";
            break;
        case VK_ERROR_INVALID_SHADER_NV:
            return "VK_ERROR_INVALID_SHADER_NV";
            break;
        case VK_ERROR_IMAGE_USAGE_NOT_SUPPORTED_KHR:
            return "VK_ERROR_IMAGE_USAGE_NOT_SUPPORTED_KHR";
            break;
        case VK_ERROR_VIDEO_PICTURE_LAYOUT_NOT_SUPPORTED_KHR:
            return "VK_ERROR_VIDEO_PICTURE_LAYOUT_NOT_SUPPORTED_KHR";
            break;
        case VK_ERROR_VIDEO_PROFILE_OPERATION_NOT_SUPPORTED_KHR:
            return "VK_ERROR_VIDEO_PROFILE_OPERATION_NOT_SUPPORTED_KHR";
            break;
        case VK_ERROR_VIDEO_PROFILE_FORMAT_NOT_SUPPORTED_KHR:
            return "VK_ERROR_VIDEO_PROFILE_FORMAT_NOT_SUPPORTED_KHR";
            break;
        case VK_ERROR_VIDEO_PROFILE_CODEC_NOT_SUPPORTED_KHR:
            return "VK_ERROR_VIDEO_PROFILE_CODEC_NOT_SUPPORTED_KHR";
            break;
        case VK_ERROR_VIDEO_STD_VERSION_NOT_SUPPORTED_KHR:
            return "VK_ERROR_VIDEO_STD_VERSION_NOT_SUPPORTED_KHR";
            break;
        case VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT:
            return "VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT";
            break;
        case VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT:
            return "VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT";
            break;
        case VK_THREAD_IDLE_KHR:
            return "VK_THREAD_IDLE_KHR";
            break;
        case VK_THREAD_DONE_KHR:
            return "VK_THREAD_DONE_KHR";
            break;
        case VK_OPERATION_DEFERRED_KHR:
            return "VK_OPERATION_DEFERRED_KHR";
            break;
        case VK_OPERATION_NOT_DEFERRED_KHR:
            return "VK_OPERATION_NOT_DEFERRED_KHR";
            break;
        case VK_ERROR_INVALID_VIDEO_STD_PARAMETERS_KHR:
            return "VK_ERROR_INVALID_VIDEO_STD_PARAMETERS_KHR";
            break;
        case VK_ERROR_COMPRESSION_EXHAUSTED_EXT:
            return "VK_ERROR_COMPRESSION_EXHAUSTED_EXT";
            break;
#ifdef VK_VERSION_1_4
        case VK_INCOMPATIBLE_SHADER_BINARY_EXT:
            return "VK_INCOMPATIBLE_SHADER_BINARY_EXT";
            break;
        case VK_PIPELINE_BINARY_MISSING_KHR:
            return "VK_PIPELINE_BINARY_MISSING_KHR";
            break;
        case VK_ERROR_NOT_ENOUGH_SPACE_KHR:
            return "VK_ERROR_NOT_ENOUGH_SPACE_KHR";
            break;
#endif
        default:
            return "UNKNOWN ERROR";
            break;
    }
}

static inline const char* getStringDeviceType(const VkPhysicalDeviceType type)
{
    switch(type)
    {
        case VK_PHYSICAL_DEVICE_TYPE_CPU:
            return "VK_PHYSICAL_DEVICE_TYPE_CPU";
            break;
        case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
            return "VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU";
            break;
        case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
            return "VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU";
            break;
        case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
            return "VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU";
            break;
        case VK_PHYSICAL_DEVICE_TYPE_OTHER:
            return "VK_PHYSICAL_DEVICE_TYPE_OTHER";
            break;
        case VK_PHYSICAL_DEVICE_TYPE_MAX_ENUM:
            return "VK_PHYSICAL_DEVICE_TYPE_MAX_ENUM";
            break;
        default:
            return "UNKNOWN DEVICE TYPE";
            break;
    }
}

// -------------------------------------------------------------------------------------------------

#define VKW_INIT_CHECK_VK(f)                                                                       \
    {                                                                                              \
        VkResult res = f;                                                                          \
        if(res != VK_SUCCESS)                                                                      \
        {                                                                                          \
            utils::Log::Error("wkw", #f ": %s", getStringResult(res));                             \
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
#define VKW_CHECK_VK_RETURN_FALSE(f)                                                               \
    {                                                                                              \
        VkResult res = f;                                                                          \
        if(res != VK_SUCCESS)                                                                      \
        {                                                                                          \
            utils::Log::Error("wkw", #f ": %s", getStringResult(res));                             \
            return false;                                                                          \
        }                                                                                          \
    }
#define VKW_CHECK_VK_THROW(f, msg)                                                                 \
    {                                                                                              \
        VkResult err = f;                                                                          \
        if(err != VK_SUCCESS)                                                                      \
        {                                                                                          \
            utils::Log::Error(LOG_TAG, #f ": %s\n", getStringResult(err));                         \
            throw std::runtime_error(msg);                                                         \
        }                                                                                          \
    }
#define VKW_CHECK_BOOL_RETURN_FALSE(f)                                                             \
    {                                                                                              \
        bool res = f;                                                                              \
        if(!res)                                                                                   \
        {                                                                                          \
            utils::Log::Error(LOG_TAG, #f ": failed");                                             \
            return false;                                                                          \
        }                                                                                          \
    }

#define VKW_CHECK_BOOL_THROW(f, msg)                                                               \
    {                                                                                              \
        bool res = f;                                                                              \
        if(!res)                                                                                   \
        {                                                                                          \
            utils::Log::Error(LOG_TAG, #f ": failed");                                             \
            throw std::runtime_error(msg);                                                         \
        }                                                                                          \
    }

#define VKW_DELETE_VK(type, name)                                                                  \
    if(name != VK_NULL_HANDLE)                                                                     \
    {                                                                                              \
        device_->vk().vkDestroy##type(device_->getHandle(), name, nullptr);                        \
        name = VK_NULL_HANDLE;                                                                     \
    }
#define VKW_FREE_VK(type, name)                                                                    \
    if(name != VK_NULL_HANDLE)                                                                     \
    {                                                                                              \
        device_->vk().vkFree##type(device_->getHandle(), name, nullptr);                           \
        name = VK_NULL_HANDLE;                                                                     \
    }

// -------------------------------------------------------------------------------------------------

namespace vkw
{
namespace utils
{
    template <typename T>
    inline T alignedSize(const T val, const T align)
    {
        const T tmp = align - 1;
        return (val + tmp) & ~(tmp);
    }

    inline uint32_t divUp(const uint32_t n, const uint32_t val) { return (n + val - 1) / val; }

    VkShaderModule createShaderModule(
        const VolkDeviceTable& vk, const VkDevice device, const std::vector<char>& src);

    uint32_t findMemoryType(
        const VkPhysicalDevice physicalDevice,
        const VkMemoryPropertyFlags requiredFlags,
        const VkMemoryPropertyFlags preferredFlags,
        const VkMemoryPropertyFlags undesiredFlags,
        const VkMemoryRequirements requirements);

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
#ifdef __ANDROID__
            __android_log_print(ANDROID_LOG_DEFAULT, LOG_TAG, format, args...);
#else
            if constexpr(LogLevel >= 0)
            {
                static thread_local char buf[lineSize];
                snprintf(buf, lineSize, format, args...);
                fprintf(stdout, "%s\n", buf);
            }
#endif
        }

        template <typename... Args>
        static inline void Time(const char* tag, const char* format, Args... args)
        {
#ifdef __ANDROID__
            __android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG, format, args...);
#else
            if constexpr(LogLevel <= LOG_LEVEL_VERBOSE)
            {
                static thread_local char buf[lineSize];
                snprintf(buf, lineSize, format, args...);
                fprintf(stdout, "\033[0;32m[T][%s]: %s\n\033[0m", tag, buf);
            }
#endif
        }

        template <typename... Args>
        static inline void Debug(const char* tag, const char* format, Args... args)
        {
#ifdef __ANDROID__
            __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, format, args...);
#else
            if constexpr(logDebug > 0)
            {
                static thread_local char buf[lineSize];
                snprintf(buf, lineSize, format, args...);
                fprintf(stdout, "\033[0;32m[D][%s]: %s\n\033[0m", tag, buf);
            }
#endif
        }
        template <typename... Args>
        static inline void Verbose(const char* tag, const char* format, Args... args)
        {
#ifdef __ANDROID__
            __android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG, format, args...);
#else
            if constexpr(LogLevel <= LOG_LEVEL_VERBOSE)
            {
                static thread_local char buf[lineSize];
                snprintf(buf, lineSize, format, args...);
                fprintf(stdout, "\033[0;34m[I][%s]: %s\n\033[0m", tag, buf);
            }
#endif
        }
        template <typename... Args>
        static inline void Info(const char* tag, const char* format, Args... args)
        {
#ifdef __ANDROID__
            __android_log_print(ANDROID_LOG_INFO, LOG_TAG, format, args...);
#else
            if constexpr(LogLevel <= LOG_LEVEL_INFO)
            {
                static thread_local char buf[lineSize];
                snprintf(buf, lineSize, format, args...);
                fprintf(stdout, "\033[0;34m[I][%s]: %s\n\033[0m", tag, buf);
            }
#endif
        }
        template <typename... Args>
        static inline void Warning(const char* tag, const char* format, Args... args)
        {
#ifdef __ANDROID__
            __android_log_print(ANDROID_LOG_WARN, LOG_TAG, format, args...);
#else
            if constexpr(LogLevel <= LOG_LEVEL_WARNING)
            {
                static thread_local char buf[lineSize];
                snprintf(buf, lineSize, format, args...);
                fprintf(stdout, "\033[0;33m[W][%s]: %s\n\033[0m", tag, buf);
                fflush(stdout);
            }
#endif
        }
        template <typename... Args>
        static inline void Error(const char* tag, const char* format, Args... args)
        {
#ifdef __ANDROID__
            __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, format, args...);
#else
            if constexpr(LogLevel <= LOG_LEVEL_ERROR)
            {
                static thread_local char buf[lineSize];
                snprintf(buf, lineSize, format, args...);
                fprintf(stdout, "\033[0;31m[E][%s]: %s\n\033[0m", tag, buf);
                fflush(stdout);
            }
#endif
        }

      private:
        static constexpr size_t lineSize = 1024;
#ifndef __ANDROID__
        static constexpr int LogLevel = LOG_LEVEL;
        static constexpr int logDebug = LOG_DEBUG_VALUE;
#endif

        Log() = default;
    };
} // namespace utils
} // namespace vkw
#ifdef __GNUC__
#    pragma GCC diagnostic pop
#endif
