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

#include "vkWrappers/wrappers/TopLevelAccelerationStructure.hpp"

namespace vkw
{
TopLevelAccelerationStructure& TopLevelAccelerationStructure::addGeometry(
    const BottomLevelAccelerationStructure& geometry, const VkTransformMatrixKHR& transform)
{
    VKW_CHECK_BOOL_THROW(
        this->buildOnHost() == geometry.buildOnHost(),
        "Error all structures must be build at the same place: device or host");

    VkAccelerationStructureInstanceKHR geometryInstance = {};
    geometryInstance.transform = transform;
    geometryInstance.instanceCustomIndex = 0;                    ///@todo: Add actual value here
    geometryInstance.mask = 0xff;                                ///@todo: Add actual value here
    geometryInstance.instanceShaderBindingTableRecordOffset = 0; ///@todo: Add actual value here
    geometryInstance.accelerationStructureReference
        = geometry.buildOnHost() ? reinterpret_cast<uint64_t>(geometry.getHandle())
                                 : static_cast<uint64_t>(geometry.getDeviceAddress());
}
} // namespace vkw