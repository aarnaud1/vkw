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

#include "vkWrappers/wrappers/Validation.hpp"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

namespace vkw
{
// -----------------------------------------------------------------------------

static void printDebug(FILE *fp, const char *info, const char *msg, const char *pUserData);

// -----------------------------------------------------------------------------

VkResult CreateDebugUtilsMessengerEXT(
    VkInstance instance,
    const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
    const VkAllocationCallbacks *pAllocator,
    VkDebugUtilsMessengerEXT *pDebugMessenger)
{
    PFN_vkCreateDebugUtilsMessengerEXT func
        = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(
            instance, "vkCreateDebugUtilsMessengerEXT");
    if(func != nullptr)
    {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    }
    else
    {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void DestroyDebugUtilsMessengerEXT(
    VkInstance instance,
    VkDebugUtilsMessengerEXT debugMessenger,
    const VkAllocationCallbacks *pAllocator)
{
    PFN_vkDestroyDebugUtilsMessengerEXT func
        = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(
            instance, "vkDestroyDebugUtilsMessengerEXT");
    if(func != nullptr)
    {
        func(instance, debugMessenger, pAllocator);
    }
}

VkResult CreateDebugReportCallbackEXT(
    VkInstance instance,
    const VkDebugReportCallbackCreateInfoEXT *pCreateInfo,
    const VkAllocationCallbacks *pAllocator,
    VkDebugReportCallbackEXT *pReportCallback)
{
    PFN_vkCreateDebugReportCallbackEXT func
        = (PFN_vkCreateDebugReportCallbackEXT) vkGetInstanceProcAddr(
            instance, "vkCreateDebugReportCallbackEXT");
    if(func != nullptr)
    {
        return func(instance, pCreateInfo, pAllocator, pReportCallback);
    }
    else
    {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void DestroyDebugReportCallbackEXT(
    VkInstance instance,
    VkDebugReportCallbackEXT reportCallback,
    const VkAllocationCallbacks *pAllocator)
{
    PFN_vkDestroyDebugReportCallbackEXT func
        = (PFN_vkDestroyDebugReportCallbackEXT) vkGetInstanceProcAddr(
            instance, "vkDestroyDebugReportCallbackEXT");
    if(func != nullptr)
    {
        func(instance, reportCallback, pAllocator);
    }
}

static void printDebug(FILE *fp, const char *info, const char *msg, const char *pUserData)
{
    if(pUserData != NULL)
        fprintf(fp, "%s : %s from %s\n\n", info, msg, pUserData);
    else
        fprintf(fp, "%s : %s -\n\n", info, msg);
}

// -----------------------------------------------------------------------------

VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
    void *pUserData)
{
    switch(messageSeverity)
    {
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
#if(LOG_FILTER == LOG_LEVEL_VERBOSE)
            printDebug(
                stderr,
                "[Verbose] Validation layer",
                pCallbackData->pMessage,
                reinterpret_cast<const char *>(pUserData));
#endif
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
#if(LOG_FILTER <= LOG_LEVEL_INFO)
            printDebug(
                stderr,
                "[Info] Validation layer",
                pCallbackData->pMessage,
                reinterpret_cast<const char *>(pUserData));
#endif
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
#if(LOG_FILTER <= LOG_LEVEL_WARNING)
            printDebug(
                stderr,
                "[Warning] Validation layer",
                pCallbackData->pMessage,
                reinterpret_cast<const char *>(pUserData));
#endif
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
            printDebug(
                stderr,
                "[Error] Validation layer",
                pCallbackData->pMessage,
                reinterpret_cast<const char *>(pUserData));
            break;
        default:
            break;
    }

    return VK_FALSE;
}

VKAPI_ATTR VkBool32 debugReportCallback VKAPI_CALL(
    VkDebugReportFlagsEXT flags,
    VkDebugReportObjectTypeEXT objectType,
    uint64_t object,
    size_t location,
    int32_t messageCode,
    const char *pLayerPrefix,
    const char *pMessage,
    void *pUserData)
{
    if(flags & VK_DEBUG_REPORT_ERROR_BIT_EXT)
    {
        printf("debugPrintfEXT: %s", pMessage);
    }

    return false;
}
} // namespace vkw
#pragma GCC diagnostic pop
