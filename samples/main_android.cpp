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

#include "IGraphicsSample.hpp"
#include "RayQueryTriangle.hpp"
#include "SimpleTriangle.hpp"

#include <android/native_window.h>
#include <android/native_window_jni.h>
#include <atomic>
#include <jni.h>
#include <memory>
#include <thread>
#include <vkw/vkw.hpp>

#ifndef LOG_TAG
#    define LOG_TAG "vkw-samples"
#endif

static const uint32_t fboWidth = 1440;
static const uint32_t fboHeight = 2560;

struct AppInfo
{
    std::unique_ptr<IGraphicsSample> pSample = nullptr;
    ANativeWindow* pNativeWindow = nullptr;
    std::atomic_bool running = false;
    std::atomic_bool requestResize = false;
    std::mutex surfaceMutex;
};
static AppInfo appInfo{};
static std::thread mainThread{};

static void app_main_loop();

extern "C" {
JNIEXPORT jboolean JNICALL
Java_com_aarnaud_vkwsamples_SampleActivity_InitSample(JNIEnv*, jclass, jint sampleId)
{
    const std::vector<const char*> instanceExtensions = {"VK_KHR_surface", "VK_KHR_android_surface"};
    switch(static_cast<int>(sampleId))
    {
        case 0:
            appInfo.pSample.reset(new SimpleTriangle(fboWidth, fboHeight, {}, instanceExtensions));
            break;
        case 1:
            appInfo.pSample.reset(new RayQueryTriangle(fboWidth, fboHeight, {}, instanceExtensions));
            break;
        default:
            break;
    }

    if(!appInfo.pSample)
    {
        vkw::utils::Log::Error(LOG_TAG, "Error : wrong sample ID");
        return JNI_FALSE;
    }

    if(!appInfo.pSample->initSample())
    {
        vkw::utils::Log::Error(LOG_TAG, "Error initializing sample");
        appInfo.pSample.reset(nullptr);
        return JNI_FALSE;
    }

    return JNI_TRUE;
}

JNIEXPORT jboolean JNICALL Java_com_aarnaud_vkwsamples_SampleActivity_DestroySample(JNIEnv*, jclass)
{
    appInfo.pSample.reset(nullptr);
    return JNI_TRUE;
}

JNIEXPORT jboolean JNICALL Java_com_aarnaud_vkwsamples_SampleActivity_StartSample(JNIEnv*, jclass)
{
    appInfo.running = true;
    mainThread = std::thread([]() { app_main_loop(); });

    vkw::utils::Log::Debug(LOG_TAG, "Main loop started");
    return JNI_TRUE;
}

JNIEXPORT jboolean JNICALL Java_com_aarnaud_vkwsamples_SampleActivity_StopSample(JNIEnv*, jclass)
{
    appInfo.running = false;
    mainThread.join();

    vkw::utils::Log::Debug(LOG_TAG, "Main loop stopped");
    return JNI_TRUE;
}

JNIEXPORT jboolean JNICALL
Java_com_aarnaud_vkwsamples_SampleActivity_InitNativeWindow(JNIEnv* env, jclass, jobject surface)
{
    std::lock_guard<std::mutex> lock(appInfo.surfaceMutex);

    ANativeWindow* pWindow = ANativeWindow_fromSurface(env, surface);
    if(!pWindow)
    {
        vkw::utils::Log::Error(LOG_TAG, "Error initializing native window");
        return JNI_FALSE;
    }
    appInfo.pNativeWindow = pWindow;

    const auto windowWidth = ANativeWindow_getWidth(pWindow);
    const auto windowHeight = ANativeWindow_getHeight(pWindow);
    vkw::utils::Log::Debug(LOG_TAG, "Native window initialized: w=%d, h=%d", windowWidth, windowHeight);

    auto& pSample = appInfo.pSample;
    if(!pSample)
    {
        vkw::utils::Log::Error(LOG_TAG, "Error sample not initialized");
        return JNI_FALSE;
    }

    VkSurfaceKHR vkSurf = VK_NULL_HANDLE;
    VkAndroidSurfaceCreateInfoKHR surfaceCreateInfo = {};
    surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR;
    surfaceCreateInfo.pNext = nullptr;
    surfaceCreateInfo.flags = 0;
    surfaceCreateInfo.window = pWindow;
    if(vkCreateAndroidSurfaceKHR(
           appInfo.pSample->instance().getHandle(), &surfaceCreateInfo, nullptr, &vkSurf)
       != VK_SUCCESS)
    {
        vkw::utils::Log::Error(LOG_TAG, "Error creating Android surface");
        return JNI_FALSE;
    }

    if(!pSample->setSurface(std::move(vkSurf)))
    {
        vkw::utils::Log::Error(LOG_TAG, "Error initializing sample surface");
        return JNI_FALSE;
    }

    return JNI_TRUE;
}

JNIEXPORT jboolean JNICALL Java_com_aarnaud_vkwsamples_SampleActivity_ResizeNativeWindow(JNIEnv* env, jclass)
{
    appInfo.requestResize = true;
    return JNI_TRUE;
}

JNIEXPORT jboolean JNICALL Java_com_aarnaud_vkwsamples_SampleActivity_DestroyNativeWindow(JNIEnv* env, jclass)
{
    std::lock_guard<std::mutex> lock(appInfo.surfaceMutex);
    appInfo.pNativeWindow = nullptr;
    if(appInfo.pSample)
    {
        appInfo.pSample->clearSurface();
    }
    return JNI_TRUE;
}
}

void app_main_loop()
{
    auto& pSample = appInfo.pSample;
    if(!pSample)
    {
        return;
    }
    while(appInfo.running)
    {
        std::lock_guard<std::mutex> lock(appInfo.surfaceMutex);
        if(pSample->hasSurface())
        {
            if(appInfo.requestResize)
            {
                auto* pWindow = appInfo.pNativeWindow;
                const auto windowWidth = ANativeWindow_getWidth(pWindow);
                const auto windowHeight = ANativeWindow_getHeight(pWindow);
                pSample->resize(windowWidth, windowHeight);

                appInfo.requestResize = false;
            }
            pSample->render();
        }
    }
}