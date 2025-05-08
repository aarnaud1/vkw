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

#include "Common.hpp"

#include <vector>

class IComputeSample
{
  public:
    IComputeSample() = default;

    virtual ~IComputeSample() = default;

    bool init() { return true; }
    bool runSample() { return true; }

    virtual std::vector<const char*> requiredDeviceExtensions() = 0;
    virtual void* getAdditionalFeatures() = 0;

    virtual uint32_t sampleCount() const { return 1; }

  protected:
    vkw::Instance instance_{};
    vkw::Device device_{};

    virtual bool initPipeline() = 0;
    virtual bool initResources() = 0;

    virtual bool recordInitCommands(vkw::CommandBuffer& initCmdBuffer) = 0;
    virtual bool recordPrepareRunCommands(vkw::CommandBuffer& cmdBuffer, const uint32_t id) = 0;
    virtual bool recordRunCommands(vkw::CommandBuffer& cmdBuffer, const uint32_t id) = 0;
    virtual bool recordFinalizeRunCommands(vkw::CommandBuffer& cmdBuffer, const uint32_t id) = 0;

    virtual void processResult(const uint32_t i) = 0;

  private:
    std::vector<vkw::Queue> computeQueues_{};

    vkw::CommandPool computeCmdPool_{};
};