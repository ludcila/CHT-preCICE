import networkx as nx


class CouplingConfiguration:

    def __init__(self, couplings, steady_state, force_explicit, force_parallel, time_step, max_time, max_iterations):
        self.steady_state = steady_state
        self.couplings = couplings
        self.force_explicit = force_explicit
        self.force_parallel = force_parallel
        self.colors = None
        self.graph = None
        self.participants = None
        self.time_step = time_step
        self.max_time = max_time
        self.max_iterations = max_iterations

        self.create_graph()
        self.get_participants_from_coupling()

    def create_graph(self):
        self.graph = nx.Graph()
        [self.graph.add_edge(pair[0], pair[1]) for pair in self.couplings]
        self.colors = nx.coloring.greedy_color(self.graph, strategy=nx.coloring.strategy_largest_first)

    # Cycles can cause deadlocks if the coupling scheme is not configured appropriately
    # If there are cycles, parallel coupling will be used
    def has_cycles(self):
        """Determines whether the coupling dependency graph contains cycles"""
        return len(nx.cycle_basis(self.graph)) > 0

    def get_colors(self):
        """Returns the graph coloring"""
        return self.colors

    def sort_participants(self, participants):
        """Determines first and second participant in the coupling, based on the graph coloring"""
        if self.get_colors()[participants[0]] == 1:
            participants.reverse()

    def get_participants_from_coupling(self):
        self.participants = []
        for pair in self.couplings:
            self.participants.append(pair[0])
            self.participants.append(pair[1])
        self.participants = list(set(self.participants))

    # Use explicit in steady state simulations or if specified by user
    def is_explicit(self):
        return self.steady_state or self.force_explicit

    # Use implicit if not using explicit
    def is_implicit(self):
        return not self.is_explicit()

    # Use parallel or multi if there are cyclic coupling dependencies
    def is_parallel_or_multi(self):
        return self.has_cycles() or self.force_parallel

    # Use multi for parallel implicit coupling between more than two participants
    def is_multi(self):
        return self.is_parallel_or_multi() and self.is_implicit() and len(self.participants) > 2

    # Use parallel, for parallel coupling between two participants, or explicit coupling between more than two
    def is_parallel(self):
        return self.is_parallel_or_multi() and not self.is_multi()

    # Use serial if not using multi or parallel
    def is_serial(self):
        return not self.is_multi() and not self.is_parallel()
