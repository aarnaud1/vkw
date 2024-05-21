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

import sys
import open3d as o3d

if __name__ == '__main__':
    filename = sys.argv[1]

    print('Reading input mesh...')
    mesh = o3d.io.read_triangle_mesh(filename)
    print('Import completed');

    bbox = mesh.get_axis_aligned_bounding_box()
    print(bbox)

    # mesh.purge()
    mesh.compute_vertex_normals()
    o3d.io.write_triangle_mesh('mesh_output.ply', mesh)

