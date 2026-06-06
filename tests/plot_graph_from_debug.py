#
# Tool to plot debug graphs from REBL - Library for handling and analysis of
# REliability BLock diagrams.
# This specific tool in this file is AI generated and is not part of the C++ codebase. It only serves a purpose during development.
# Author: Bartek Musiol (bmusiol@proton.me)
# See LICENSE
#

import re
import networkx as nx
import matplotlib.pyplot as plt

# ============================================================
# PASTE YOUR DEBUG OUTPUT HERE
# ============================================================

DEBUG_TEXT = r"""
61 (Actor_(m_p,p)) -> [ 43 (§virtual§)] (H: 0.021; T: 0.000163234; P: 3.42792e-06)
60 (ACC_(m_p,p)->Actor_(m_p,p)) -> [ 61 (Actor_(m_p,p))] (H: 0.022; T: 0.013698; P: 0.000301356)
28 (§virtual§) -> [ 30 (Sensor_1), 34 (Sensor_n)] (H: 0; T: 0; P: 0)
29 (§virtual§) -> [ 38 (Actor_(1,2)->MGCC)] (H: 0; T: 0; P: 0)
27 (Actor_(1,1)) -> [ 9 (§virtual§)] (H: 0.021; T: 0.000163234; P: 3.42792e-06)
26 (Sensor_n->Actor_(m_1,1)) -> [ 22 (§virtual§)] (H: 0.022; T: 0.013698; P: 0.000301356)
25 (Sensor_n) -> [ 26 (Sensor_n->Actor_(m_1,1))] (H: 0.001; T: 0.0004566; P: 4.566e-07)
24 (Sensor_1->Actor_(m_1,1)) -> [ 22 (§virtual§)] (H: 0.022; T: 0.013698; P: 0.000301356)
23 (Sensor_1) -> [ 24 (Sensor_1->Actor_(m_1,1))] (H: 0.1; T: 0.00057075; P: 5.7075e-05)
21 (§virtual§) -> [ 23 (Sensor_1), 25 (Sensor_n)] (H: 0; T: 0; P: 0)
22 (§virtual§) -> [ 20 (§virtual§)] (H: 0; T: 0; P: 0)
19 (§virtual§) -> [ 21 (§virtual§)] (H: 0; T: 0; P: 0)
20 (§virtual§) -> [ 27 (Actor_(1,1))] (H: 0; T: 0; P: 0)
18 (§virtual§) -> [ 19 (§virtual§)] (H: 0; T: 0; P: 0)
17 (Sensor_n->Actor_(1,1)) -> [ 13 (§virtual§)] (H: 0.022; T: 0.013698; P: 0.000301356)
16 (Sensor_n) -> [ 17 (Sensor_n->Actor_(1,1))] (H: 0.001; T: 0.0004566; P: 4.566e-07)
15 (Sensor_1->Actor_(1,1)) -> [ 13 (§virtual§)] (H: 4.88; T: 0.0079905; P: 0.0389936)
14 (Sensor_1) -> [ 15 (Sensor_1->Actor_(1,1))] (H: 0.1; T: 0.00057075; P: 5.7075e-05)
1 (§virtual§) -> [ 3 (MGCC)] (H: 0; T: 0; P: 0)
2 (§virtual§) -> [ ] (H: 0; T: 0; P: 0)
3 (MGCC) -> [ 4 (Actor_(1,1)->MGCC)] (H: 0; T: 0; P: 0)
4 (Actor_(1,1)->MGCC) -> [ 5 (Actor_(m_1,1)->MGCC)] (H: 0.022; T: 0.013698; P: 0.000301356)
5 (Actor_(m_1,1)->MGCC) -> [ 6 (§virtual§)] (H: 0; T: 0; P: 0)
7 (§virtual§) -> [ 2 (§virtual§)] (H: 0; T: 0; P: 0)
6 (§virtual§) -> [ 8 (§virtual§)] (H: 0; T: 0; P: 0)
9 (§virtual§) -> [ 7 (§virtual§)] (H: 0; T: 0; P: 0)
8 (§virtual§) -> [ 10 (§virtual§), 28 (§virtual§)] (H: 0; T: 0; P: 0)
11 (§virtual§) -> [ 18 (§virtual§)] (H: 0; T: 0; P: 0)
10 (§virtual§) -> [ 12 (§virtual§)] (H: 0; T: 0; P: 0)
13 (§virtual§) -> [ 11 (§virtual§)] (H: 0; T: 0; P: 0)
12 (§virtual§) -> [ 14 (Sensor_1), 16 (Sensor_n)] (H: 0; T: 0; P: 0)
30 (Sensor_1) -> [ 31 (Sensor_1->SCC_1)] (H: 0.1; T: 0.00057075; P: 5.7075e-05)
31 (Sensor_1->SCC_1) -> [ 32 (SCC_1)] (H: 0.022; T: 0.013698; P: 0.000301356)
32 (SCC_1) -> [ 33 (SCC_1->MGCC)] (H: 0; T: 0; P: 0)
33 (SCC_1->MGCC) -> [ 29 (§virtual§)] (H: 0.022; T: 0.013698; P: 0.000301356)
34 (Sensor_n) -> [ 35 (Sensor_n->SCC_n)] (H: 0.001; T: 0.0004566; P: 4.566e-07)
35 (Sensor_n->SCC_n) -> [ 36 (SCC_n)] (H: 0.022; T: 0.013698; P: 0.000301356)
36 (SCC_n) -> [ 37 (SCC_n->MGCC)] (H: 0; T: 0; P: 0)
37 (SCC_n->MGCC) -> [ 29 (§virtual§)] (H: 0.022; T: 0.013698; P: 0.000301356)
38 (Actor_(1,2)->MGCC) -> [ 39 (Actor_(m_1,2)->MGCC)] (H: 0.022; T: 0.013698; P: 0.000301356)
39 (Actor_(m_1,2)->MGCC) -> [ 40 (§virtual§)] (H: 0.022; T: 0.013698; P: 0.000301356)
41 (§virtual§) -> [ 9 (§virtual§)] (H: 0; T: 0; P: 0)
40 (§virtual§) -> [ 42 (§virtual§)] (H: 0; T: 0; P: 0)
43 (§virtual§) -> [ 41 (§virtual§)] (H: 0; T: 0; P: 0)
42 (§virtual§) -> [ 44 (ACC_(1,2)), 52 (Actor_(1,p)->MGCC)] (H: 0; T: 0; P: 0)
44 (ACC_(1,2)) -> [ 45 (MGCC->ACC_(1,2))] (H: 0; T: 0; P: 0)
45 (MGCC->ACC_(1,2)) -> [ 46 (ACC_(1,2)->Actor_(1,2))] (H: 0.022; T: 0.013698; P: 0.000301356)
46 (ACC_(1,2)->Actor_(1,2)) -> [ 47 (Actor_(1,2))] (H: 0.022; T: 0.013698; P: 0.000301356)
47 (Actor_(1,2)) -> [ 48 (ACC_(m_2,2))] (H: 0.021; T: 0.000163234; P: 3.42792e-06)
48 (ACC_(m_2,2)) -> [ 49 (MGCC->ACC_(m_2,2))] (H: 0; T: 0; P: 0)
49 (MGCC->ACC_(m_2,2)) -> [ 50 (ACC_(m_2,2)->Actor_(m_2,2))] (H: 0; T: 0; P: 0)
50 (ACC_(m_2,2)->Actor_(m_2,2)) -> [ 51 (Actor_(m_2,2))] (H: 0.022; T: 0.013698; P: 0.000301356)
51 (Actor_(m_2,2)) -> [ 43 (§virtual§)] (H: 0.021; T: 0.000163234; P: 3.42792e-06)
52 (Actor_(1,p)->MGCC) -> [ 53 (Actor_(m_p,p)->MGCC)] (H: 0.022; T: 0.013698; P: 0.000301356)
53 (Actor_(m_p,p)->MGCC) -> [ 54 (ACC_(1,p))] (H: 0.022; T: 0.013698; P: 0.000301356)
54 (ACC_(1,p)) -> [ 55 (MGCC->ACC_(1,p))] (H: 0; T: 0; P: 0)
55 (MGCC->ACC_(1,p)) -> [ 56 (ACC_(1,p)->Actor_(1,p))] (H: 0.022; T: 0.013698; P: 0.000301356)
56 (ACC_(1,p)->Actor_(1,p)) -> [ 57 (Actor_(1,p))] (H: 0.022; T: 0.013698; P: 0.000301356)
57 (Actor_(1,p)) -> [ 58 (ACC_(m_p,p))] (H: 0.021; T: 0.000163234; P: 3.42792e-06)
58 (ACC_(m_p,p)) -> [ 59 (MGCC->ACC_(m_p,p))] (H: 0; T: 0; P: 0)
59 (MGCC->ACC_(m_p,p)) -> [ 60 (ACC_(m_p,p)->Actor_(m_p,p))] (H: 0.022; T: 0.013698; P: 0.000301356)
"""


def is_virtual_node(label: str) -> bool:
    """
    Returns True if the node name contains §...§
    """
    return re.search(r"§.*?§", label) is not None


# ============================================================
# GRAPH LAYOUT CONFIGURATION
# ============================================================
#
# Available layouts:
#
# "spring"      -> force-directed (good default)
# "shell"       -> concentric shells
# "circular"    -> circular layout
# "kamada"      -> Kamada-Kawai layout
# "spectral"    -> spectral layout
# "random"      -> random positions
# "planar"      -> planar layout (fails if graph non-planar)
#
# BFS-style hierarchical layout:
# "bfs"
#
# ============================================================

GRAPH_LAYOUT = "bfs"

# Root node for BFS layout
# Change this if desired.
BFS_ROOT = 1

# Figure size
FIGSIZE = (16, 10)

# Node size
NODE_SIZE = 800

# Font size
FONT_SIZE = 8


# ============================================================
# PARSER
# ============================================================

line_pattern = re.compile(r"^\s*(\d+)\s*\((.*?)\)\s*->\s*\[(.*?)\]\s*\(.*\)\s*$")

neighbor_pattern = re.compile(r"(\d+)\s*\((.*?)\)")

G = nx.DiGraph()

for line in DEBUG_TEXT.strip().splitlines():
    line = line.strip()

    if not line:
        continue

    match = line_pattern.match(line)

    if not match:
        print(f"Could not parse line:\n{line}\n")
        continue

    curr_id = int(match.group(1))
    curr_name = match.group(2)
    neighbors_raw = match.group(3)

    curr_label = f"{curr_id} ({curr_name})"

    G.add_node(curr_id, label=curr_label)

    neighbors = neighbor_pattern.findall(neighbors_raw)

    for neighbor_id_str, neighbor_name in neighbors:
        neighbor_id = int(neighbor_id_str)

        neighbor_label = f"{neighbor_id} ({neighbor_name})"

        G.add_node(neighbor_id, label=neighbor_label)

        G.add_edge(curr_id, neighbor_id)


# ============================================================
# LAYOUT SELECTION
# ============================================================


def bfs_hierarchy_pos(graph, root, level_gap=2.0, width_gap=2.0):
    """
    Simple BFS hierarchical layout.
    """

    levels = {}
    visited = set()

    queue = [(root, 0)]

    while queue:
        node, level = queue.pop(0)

        if node in visited:
            continue

        visited.add(node)

        levels.setdefault(level, []).append(node)

        for neighbor in graph.successors(node):
            if neighbor not in visited:
                queue.append((neighbor, level + 1))

    pos = {}

    for level, nodes in levels.items():
        count = len(nodes)

        for i, node in enumerate(nodes):
            x = (i - (count - 1) / 2) * width_gap
            y = -level * level_gap

            pos[node] = (x, y)

    # Any disconnected nodes
    remaining = set(graph.nodes()) - set(pos.keys())

    if remaining:
        y = -(max(levels.keys()) + 2) * level_gap

        for i, node in enumerate(remaining):
            pos[node] = (i * width_gap, y)

    return pos


if GRAPH_LAYOUT == "spring":
    pos = nx.spring_layout(G, seed=42)

elif GRAPH_LAYOUT == "shell":
    pos = nx.shell_layout(G)

elif GRAPH_LAYOUT == "circular":
    pos = nx.circular_layout(G)

elif GRAPH_LAYOUT == "kamada":
    pos = nx.kamada_kawai_layout(G)

elif GRAPH_LAYOUT == "spectral":
    pos = nx.spectral_layout(G)

elif GRAPH_LAYOUT == "random":
    pos = nx.random_layout(G)

elif GRAPH_LAYOUT == "planar":
    pos = nx.planar_layout(G)

elif GRAPH_LAYOUT == "bfs":
    pos = bfs_hierarchy_pos(G, BFS_ROOT)

else:
    raise ValueError(f"Unknown GRAPH_LAYOUT: {GRAPH_LAYOUT}")


# ============================================================
# DRAW GRAPH
# ============================================================

plt.figure(figsize=FIGSIZE)

labels = nx.get_node_attributes(G, "label")

# ------------------------------------------------------------
# NODE COLORS
# ------------------------------------------------------------

node_colors = []

for node in G.nodes():
    label = labels[node]

    if is_virtual_node(label):
        node_colors.append("#BBBBBB")  # gray for virtual nodes
    else:
        node_colors.append("#1f78b4")  # default matplotlib/networkx blue


nx.draw_networkx_nodes(
    G,
    pos,
    node_size=NODE_SIZE,
    node_color=node_colors,
)

nx.draw_networkx_edges(
    G,
    pos,
    arrows=True,
    arrowstyle="-|>",
    arrowsize=20,
)

nx.draw_networkx_labels(
    G,
    pos,
    labels=labels,
    font_size=FONT_SIZE,
)

plt.axis("off")
plt.tight_layout()
plt.show()
