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

#version 450 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec3 normal;

out gl_PerVertex { vec4 gl_Position; };
layout(location = 0) out vec3 vertexColor;
layout(location = 1) out vec3 vertexNormal;

layout(binding = 0) uniform Matrices
{
    mat4 model;
    mat4 view;
    mat4 proj;
}
mvp;

void main()
{
    gl_Position
        = mvp.proj * mvp.view * mvp.model * vec4(vec3(position.x, position.y, position.z), 1.0f);
    gl_Position.y = -gl_Position.y;
    vertexColor = color;
    vertexNormal = vec3(mvp.view * mvp.model * vec4(normal, 0.0f));
}
