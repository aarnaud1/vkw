/*
 * Copyright (c) 2025 Adrien ARNAUD
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
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

#define ERROR_SEVERITY_SILENT 0 // Silent error
#define ERROR_SEVERITY_PRINT  1 // Print an error message
#define ERROR_SEVERITY_THROW  2 // Throws an exception

#ifndef ERROR_SEVERITY
#    define ERROR_SEVERITY ERROR_SEVERITY_THROW
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

#ifdef DEBUG
#    define VKW_ASSERT(cond)                                                                                 \
        if(!(cond))                                                                                          \
        {                                                                                                    \
            vkw::utils::Log::Warning(LOG_TAG, "[%s:%d] Assertion failed: " #cond, __FILE__, __LINE__);       \
        }
#else
#    define VKW_ASSERT(cond)
#endif

#if (ERROR_SEVERITY == ERROR_SEVERITY_SILENT)
#    define VKW_ERROR(msg)
#elif (ERROR_SEVERITY == ERROR_SEVERITY_PRINT)
#    define VKW_ERROR(msg) vkw::utils::Log::Error(LOG_TAG, msg);
#elif (ERROR_SEVERITY == ERROR_SEVERITY_THROW)
#    define VKW_ERROR(msg) throw std::runtime_error(msg);
#endif

#define VKW_INIT_CHECK_VK(f)                                                                                 \
    {                                                                                                        \
        VkResult res = f;                                                                                    \
        if(res != VK_SUCCESS)                                                                                \
        {                                                                                                    \
            vkw::utils::Log::Error(LOG_TAG, #f ": %s", getStringResult(res));                                \
            clear();                                                                                         \
            return false;                                                                                    \
        }                                                                                                    \
    }
#define VKW_INIT_CHECK_BOOL(f)                                                                               \
    {                                                                                                        \
        if(!f)                                                                                               \
        {                                                                                                    \
            vkw::utils::Log::Error(LOG_TAG, #f ": failed");                                                  \
            clear();                                                                                         \
            return false;                                                                                    \
        }                                                                                                    \
    }
#define VKW_CHECK_VK_RETURN_FALSE(f)                                                                         \
    {                                                                                                        \
        VkResult res = f;                                                                                    \
        if(res != VK_SUCCESS)                                                                                \
        {                                                                                                    \
            vkw::utils::Log::Error(LOG_TAG, #f ": %s", getStringResult(res));                                \
            return false;                                                                                    \
        }                                                                                                    \
    }
#define VKW_CHECK_VK_FAIL(f, msg)                                                                            \
    {                                                                                                        \
        VkResult res = f;                                                                                    \
        if(res != VK_SUCCESS)                                                                                \
        {                                                                                                    \
            vkw::utils::Log::Error(LOG_TAG, #f ": %s", getStringResult(res));                                \
            VKW_ERROR(msg);                                                                                  \
        }                                                                                                    \
    }

#define VKW_CHECK_VK_CLEAR(f, obj, msg)                                                                      \
    {                                                                                                        \
        VkResult res = f;                                                                                    \
        if(res != VK_SUCCESS)                                                                                \
        {                                                                                                    \
            vkw::utils::Log::Error(LOG_TAG, #f ": %s", getStringResult(res));                                \
            vkw::utils::Log::Error(LOG_TAG, msg);                                                            \
            obj.clear();                                                                                     \
        }                                                                                                    \
    }

#define VKW_CHECK_BOOL_RETURN_FALSE(f)                                                                       \
    {                                                                                                        \
        bool res = f;                                                                                        \
        if(!res)                                                                                             \
        {                                                                                                    \
            vkw::utils::Log::Error(LOG_TAG, #f " failed");                                                   \
            return false;                                                                                    \
        }                                                                                                    \
    }

#define VKW_CHECK_BOOL_FAIL(f, msg)                                                                          \
    {                                                                                                        \
        bool res = f;                                                                                        \
        if(!res)                                                                                             \
        {                                                                                                    \
            vkw::utils::Log::Error(LOG_TAG, #f " failed");                                                   \
            VKW_ERROR(msg);                                                                                  \
        }                                                                                                    \
    }

#define VKW_CHECK_BOOL_CLEAR(f, obj, msg)                                                                    \
    {                                                                                                        \
        bool res = f;                                                                                        \
        if(!res)                                                                                             \
        {                                                                                                    \
            vkw::utils::Log::Error(LOG_TAG, #f " failed");                                                   \
            VKW_ERROR(msg);                                                                                  \
            obj.clear();                                                                                     \
        }                                                                                                    \
    }

#define VKW_DELETE_VK(type, name)                                                                            \
    if(name != VK_NULL_HANDLE)                                                                               \
    {                                                                                                        \
        device_->vk().vkDestroy##type(device_->getHandle(), name, nullptr);                                  \
        name = VK_NULL_HANDLE;                                                                               \
    }
#define VKW_FREE_VK(type, name)                                                                              \
    if(name != VK_NULL_HANDLE)                                                                               \
    {                                                                                                        \
        device_->vk().vkFree##type(device_->getHandle(), name, nullptr);                                     \
        name = VK_NULL_HANDLE;                                                                               \
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
            __android_log_print(ANDROID_LOG_VERBOSE, tag, format, args...);
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
            __android_log_print(ANDROID_LOG_DEBUG, tag, format, args...);
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
            __android_log_print(ANDROID_LOG_VERBOSE, tag, format, args...);
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
            __android_log_print(ANDROID_LOG_INFO, tag, format, args...);
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
            __android_log_print(ANDROID_LOG_WARN, tag, format, args...);
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
            __android_log_print(ANDROID_LOG_ERROR, tag, format, args...);
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
