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

#include "vkw/detail/DebugMessenger.hpp"

#include "vkw/detail/utils.hpp"

static inline const char* getStringMessageType(const VkDebugUtilsMessageTypeFlagsEXT messageType)
{
    switch(messageType)
    {
        case VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT:
            return "VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT";
            break;
        case VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT:
            return "VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT";
            break;
        case VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT:
            return "VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT";
            break;
        case VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT:
            return "VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT";
            break;
        case VK_DEBUG_UTILS_MESSAGE_TYPE_FLAG_BITS_MAX_ENUM_EXT:
            return "VK_DEBUG_UTILS_MESSAGE_TYPE_FLAG_BITS_MAX_ENUM_EXT";
            break;
        default:
            return "MESSAGE TYPE UNKNOWN";
            break;
    }
}

VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData)
{
    const auto msgType = getStringMessageType(messageType);
    switch(messageSeverity)
    {
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
            vkw::utils::Log::Verbose(
                msgType,
                "%s - %f",
                pCallbackData->pMessage,
                reinterpret_cast<const char*>(pUserData));
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
            vkw::utils::Log::Info(
                msgType,
                "%s - %f",
                pCallbackData->pMessage,
                reinterpret_cast<const char*>(pUserData));
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
            vkw::utils::Log::Warning(
                msgType,
                "%s - %f",
                pCallbackData->pMessage,
                reinterpret_cast<const char*>(pUserData));
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
            vkw::utils::Log::Error(
                msgType,
                "%s - %f",
                pCallbackData->pMessage,
                reinterpret_cast<const char*>(pUserData));
            break;
        default:
            break;
    }

    return VK_FALSE;
}

namespace vkw
{
DebugMessenger::DebugMessenger(const Instance& instance)
{
    VKW_CHECK_BOOL_FAIL(this->init(instance), "Creating debug messenger");
}

DebugMessenger::DebugMessenger(DebugMessenger&& cp) { *this = std::move(cp); }

DebugMessenger& DebugMessenger::operator=(DebugMessenger&& cp)
{
    this->clear();

    std::swap(instance_, cp.instance_);
    std::swap(messenger_, cp.messenger_);

    return *this;
}

DebugMessenger::~DebugMessenger() { this->clear(); }

bool DebugMessenger::init(const Instance& instance)
{
    VKW_ASSERT(this->initialized() == false);

    instance_ = &instance;

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
    debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    debugCreateInfo.pNext = nullptr;
    debugCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
                                      | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT
                                      | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
                                      | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    debugCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
                                  | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT
                                  | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
                                  | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    debugCreateInfo.pfnUserCallback = debugCallback;
    debugCreateInfo.pUserData = nullptr;

    VKW_CHECK_VK_RETURN_FALSE(vkCreateDebugUtilsMessengerEXT(
        instance_->getHandle(), &debugCreateInfo, nullptr, &messenger_));

    initialized_ = true;

    return true;
}

void DebugMessenger::clear()
{
    if(initialized_)
    {
        vkDestroyDebugUtilsMessengerEXT(instance_->getHandle(), messenger_, nullptr);
        initialized_ = false;
    }
}
} // namespace vkw