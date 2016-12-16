import matplotlib.pyplot as plt
import networkx as nx

print "Start"

G = nx.Graph()
G.add_edge("P1","P2")
G.add_edge("P1","P2")
G.add_edge("P2","P3")
G.add_edge("P3","P4")
G.add_edge("P4","P5")


nx.draw(G)

print "Compute coloring"

print nx.coloring.greedy_color(G, strategy=nx.coloring.strategy_largest_first)

plt.show()
