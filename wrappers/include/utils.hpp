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

#include <cstdio>
#include <cstdlib>

#define CHECK_VK(f, msg)                                                                           \
  if(f != VK_SUCCESS)                                                                              \
  {                                                                                                \
    fprintf(stderr, "%s : error\\n", msg);                                                         \
    exit(1);                                                                                       \
  }

namespace vk
{
inline uint32_t divUp(const uint32_t n, const uint32_t val) { return (n + val - 1) / val; }
} // namespace vk
