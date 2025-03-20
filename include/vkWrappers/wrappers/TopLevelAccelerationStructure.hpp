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

#include "vkWrappers/wrappers/BaseAccelerationStructure.hpp"
#include "vkWrappers/wrappers/BottomLevelAccelerationStructure.hpp"
#include "vkWrappers/wrappers/Common.hpp"

namespace vkw
{
class TopLevelAccelerationStructure final : public BaseAccelerationStructure
{
  public:
    TopLevelAccelerationStructure() : BaseAccelerationStructure{} {}
    TopLevelAccelerationStructure(Device& device, const bool buildOnHost);

    TopLevelAccelerationStructure(const TopLevelAccelerationStructure&) = delete;
    TopLevelAccelerationStructure(TopLevelAccelerationStructure&& rhs) { *this = std::move(rhs); }

    TopLevelAccelerationStructure& operator=(const TopLevelAccelerationStructure&) = delete;
    TopLevelAccelerationStructure& operator=(TopLevelAccelerationStructure&& rhs);

    ~TopLevelAccelerationStructure() { this->clear(); }

    bool isInitialized() const { return initialized_; }

    bool init(Device& device, const bool buildOnHost);

    void create(const VkBuildAccelerationStructureFlagBitsKHR buildFlags = {});

    void clear();

    inline VkAccelerationStructureTypeKHR type() const override
    {
        return VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
    }

    // ---------------------------------------------------------------------------------------------

    TopLevelAccelerationStructure& addGeometry(
        const BottomLevelAccelerationStructure& geometry,
        const VkTransformMatrixKHR& transform = asIdentityMatrix);

    // ---------------------------------------------------------------------------------------------

    void build(void* scratchData, const VkBuildAccelerationStructureFlagsKHR buildFlags);

    ///@todo Not implemented yet
    template <typename ScratchBufferType>
    void update(const ScratchBufferType& scratchBuffer);

    ///@todo Not implemented yet
    void update();

    ///@todo Not implemented yet
    void copy();

  private:
    std::vector<const VkAccelerationStructureGeometryKHR*> ppGeometries_;
    std::vector<const VkAccelerationStructureBuildRangeInfoKHR*> ppBuildRanges_;

    bool initialized_{false};
};
} // namespace vkw