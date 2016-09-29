import networkx as nx

class CouplingConfiguration:

    def __init__(self, couplings, steadyState, forceExplicit, forceParallel):
        self.steadyState = steadyState
        self.couplings = couplings
        self.forceExplicit = forceExplicit
        self.forceParallel = forceParallel
        self.createGraph()
        self.getParticipantsFromCoupling()

    def createGraph(self):
        self.graph = nx.Graph()
        [self.graph.add_edge(pair[0], pair[1]) for pair in self.couplings]
        self.colors =  nx.coloring.greedy_color(self.graph, strategy=nx.coloring.strategy_largest_first)

    def hasCycles(self):
        return len(nx.cycle_basis(self.graph)) > 0

    def getColoring(self):
        return self.colors

    def getParticipantsFromCoupling(self):
        self.participants = []
        for pair in self.couplings:
            self.participants.append(pair[0])
            self.participants.append(pair[1])
        self.participants = list(set(self.participants))

    # Use explicit in steady state simulations or if specified by user
    def explicit(self):
        return self.steadyState or self.forceExplicit

    # Use implicit if not using explicit
    def implicit(self):
        return not self.explicit()

    # Use parallel or multi if there are cyclic coupling dependecies
    def parallelOrMulti(self):
        return self.hasCycles() or self.forceParallel

    # Use multi for parallel implicit coupling between more than two participants
    def multi(self):
        return self.parallelOrMulti() and self.implicit() and len(self.participants) > 2

    # Use parallel, for parallel coupling between two participants, or explicit coupling between more than two
    def parallel(self):
        return self.parallelOrMulti() and not self.multi()

    # Use serial if not using multi or parallel
    def serial(self):
        return not self.multi() and not self.parallel()
