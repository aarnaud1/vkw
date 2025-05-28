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

#include "vkw/detail/Buffer.hpp"
#include "vkw/detail/Common.hpp"
#include "vkw/detail/DescriptorPool.hpp"
#include "vkw/detail/Device.hpp"
#include "vkw/detail/Instance.hpp"
#include "vkw/detail/PipelineLayout.hpp"
#include "vkw/detail/utils.hpp"

#include <string>
#include <vector>

namespace vkw
{
class ComputePipeline
{
  public:
    ComputePipeline() {}
    ComputePipeline(const Device& device, const std::string& shaderSource);

    ComputePipeline(const ComputePipeline&) = delete;
    ComputePipeline(ComputePipeline&& cp);

    ComputePipeline& operator=(const ComputePipeline&) = delete;
    ComputePipeline& operator=(ComputePipeline&& cp);

    ~ComputePipeline();

    bool init(const Device& device, const std::string& shaderSource);

    void clear();

    bool initialized() const { return initialized_; }

    bool createPipeline(PipelineLayout& pipelineLayout);

    template <typename T>
    ComputePipeline& addSpec(const T value)
    {
        static constexpr size_t size = sizeof(T);
        const char* data = (char*) &value;

        for(size_t i = 0; i < size; i++)
        {
            specData_.push_back(data[i]);
        }
        specSizes_.push_back(size);

        return *this;
    }
    template <typename T, typename... Args>
    ComputePipeline& addSpec(const T value, Args&&... args)
    {
        addSpec<T>(value);
        return addSpec(std::forward<Args>(args)...);
    }

    VkPipeline& getHandle() { return pipeline_; }
    const VkPipeline& getHandle() const { return pipeline_; }

  private:
    const Device* device_{nullptr};
    std::string shaderSource_{};
    VkPipeline pipeline_{VK_NULL_HANDLE};

    bool initialized_{false};

    std::vector<char> specData_{};
    std::vector<size_t> specSizes_{};
};
} // namespace vkw