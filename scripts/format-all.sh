# Copyright (C) 2024 Adrien ARNAUD
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <https://www.gnu.org/licenses/>.

#!/usr/bin/bash

find ./include -regex '.*\.\(cpp\|hpp\|cc\|cxx\|cuh\|cu\|inl\|comp\)' -exec clang-format -style=file -i {} \;
find ./src -regex '.*\.\(cpp\|hpp\|cc\|cxx\|cuh\|cu\|inl\|comp\)' -exec clang-format -style=file -i {} \;
find ./samples -regex '.*\.\(cpp\|hpp\|cc\|cxx\|cuh\|cu\|inl\|comp\)' -exec clang-format -style=file -i {} \;
