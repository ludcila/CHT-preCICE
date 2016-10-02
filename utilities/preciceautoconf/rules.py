import networkx as nx


class CouplingConfiguration:

    def __init__(self, couplings, steady_state, force_explicit, force_parallel):
        self.steady_state = steady_state
        self.couplings = couplings
        self.force_explicit = force_explicit
        self.force_parallel = force_parallel
        self.colors = None
        self.graph = None
        self.participants = None
        self.create_graph()
        self.get_participants_from_coupling()

    def create_graph(self):
        self.graph = nx.Graph()
        [self.graph.add_edge(pair[0], pair[1]) for pair in self.couplings]
        self.colors = nx.coloring.greedy_color(self.graph, strategy=nx.coloring.strategy_largest_first)

    def has_cycles(self):
        return len(nx.cycle_basis(self.graph)) > 0

    def get_colors(self):
        return self.colors

    def sort_participants(self, participants):
        if self.get_colors()[participants[0]] == 1:
            participants.reverse()

    def get_participants_from_coupling(self):
        self.participants = []
        for pair in self.couplings:
            self.participants.append(pair[0])
            self.participants.append(pair[1])
        self.participants = list(set(self.participants))

    # Use explicit in steady state simulations or if specified by user
    def explicit(self):
        return self.steady_state or self.force_explicit

    # Use implicit if not using explicit
    def implicit(self):
        return not self.explicit()

    # Use parallel or multi if there are cyclic coupling dependencies
    def parallel_or_multi(self):
        return self.has_cycles() or self.force_parallel

    # Use multi for parallel implicit coupling between more than two participants
    def multi(self):
        return self.parallel_or_multi() and self.implicit() and len(self.participants) > 2

    # Use parallel, for parallel coupling between two participants, or explicit coupling between more than two
    def parallel(self):
        return self.parallel_or_multi() and not self.multi()

    # Use serial if not using multi or parallel
    def serial(self):
        return not self.multi() and not self.parallel()
