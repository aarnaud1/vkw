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

layout(location = 0) in vec3 vertexColor;
layout(location = 1) in vec3 vertexNormal;

layout(location = 0) out vec4 fragColor;

const vec3 lightDir = vec3(1.0f, 1.0f, 0.0f);

void main()
{
    const float alpha = 0.5f * clamp(dot(vertexNormal, normalize(lightDir)), 0.0f, 1.0f);
    fragColor = vec4((alpha + 0.5f) * vertexColor, 1.0f);
}