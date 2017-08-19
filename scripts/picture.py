import numpy as np
from matplotlib.path import Path
from matplotlib.patches import PathPatch
from matplotlib.patches import Circle
import matplotlib.pyplot as plt

s_rects = open("out_rects.txt").read()
s_bound = open("out_segs.txt").read()


r_vertices = []
r_codes = []

b_vertices = []
b_codes = []

rects = map(lambda x: map(int, x.split()), s_rects.split("\n")[:-1])

for x, y, width, height in rects:
    r_codes += [Path.MOVETO] + 3*[Path.LINETO] + [Path.CLOSEPOLY]
    r_vertices += [(x, y), (x, y+height), (x+width, y+height), (x+width, y), (0, 0)]
for x_0, y_0, x_1, y_1 in map(lambda x: map(int, x.split()), s_bound.split("\n")[:-1]):
    b_codes += [Path.MOVETO] + [Path.LINETO]
    b_vertices += [(x_0, y_0), (x_1, y_1)]
    
    
#vertices = np.array(vertices, float)
r_path = Path(r_vertices, r_codes)
r_pathpatch = PathPatch(r_path, facecolor='yellow', edgecolor='green')

b_path = Path(b_vertices, b_codes)
b_pathpatch = PathPatch(b_path, facecolor='None', edgecolor='red')

fig, ax = plt.subplots()
ax.add_patch(r_pathpatch)
ax.add_patch(b_pathpatch)

#ax.add_patch(Circle((0, 0), radius=np.sqrt(2)*np.sqrt(len(rects)/np.pi)*500, facecolor=None, fill=False))


ax.dataLim.update_from_data_xy(r_vertices, b_vertices)
#ax.dataLim.update_from_data_xy(b_vertices)
ax.autoscale_view()

plt.show()
