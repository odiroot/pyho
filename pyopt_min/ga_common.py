import os
from itertools import izip
from random import randint, uniform, random
from pyevolve.Consts import minimaxType
from pyevolve.G1DList import G1DList
from pyevolve.GSimpleGA import GSimpleGA
from pyevolve import Util


def stop_flag_criteria(ga_engine):
    u"GA Engine stop criteria depending on stop flag file."
    stop_file = ga_engine.getParam("stop_file", None)
    if stop_file:
        return os.path.exists(stop_file)
    else:
        return False


def stats_step_callback(ga):
    u"Function run every generation printing erorr and time stats"
    pstr = "Step %d out of %d: " % (ga.currentGeneration, ga.nGenerations)
    pstr += "err [%%] = %g" % (ga.bestIndividual().score * 100.)

    timer = ga.getParam("timer")
    timer.lap()
    if ga.currentGeneration > 0:
        gens_left = ga.nGenerations - ga.currentGeneration
        mean_time = timer.time_so_far / ga.currentGeneration
        pstr += "\t(est. time to end=%d s.)" % (gens_left * mean_time)
    print pstr


## Custom Genetic operators (and other classes). ##
def custom_initializer(genome, **kwargs):
    u"Custom genome initializer with constraints ported from C++."
    mins = genome.getParam("min_constr")
    maxes = genome.getParam("max_constr")
    genome.genomeList = [uniform(mins[i], maxes[i])
        for i in xrange(genome.getListSize())]


def custom_mutator(genome, **kwargs):
    u"Custom mutation operator adhering to the constraints."
    if kwargs["pmut"] <= 0.0:
        return 0
    size = len(genome) - 1
    mutations = kwargs["pmut"] * (size + 1)
    mins = genome.getParam("min_constr")
    maxes = genome.getParam("max_constr")
    
    if mutations < 1.0:
        mutations = 0
        for it in xrange(size + 1):
            if Util.randomFlipCoin(kwargs["pmut"]):
                genome[it] = uniform(mins[it], maxes[it])
                mutations += 1
    else:
        assert False
        for it in xrange(int(round(mutations))):
            which_gene = randint(0, size)
            genome[which_gene] = uniform(mins[which_gene], maxes[which_gene])
    return int(mutations)


def custom_crossover(genome, **kwargs):
    u"Custom crossover operator adhering to the constraints."    
    mom = kwargs["mom"]
    dad = kwargs["dad"]    
    mins = mom.getParam("min_constr")
    maxes = mom.getParam("max_constr")        
    count = kwargs["count"]
    sister = None
    brother = None

    if count >= 1:
        sister = mom.clone()
        sister.resetStats()
        for i, pair in enumerate(izip(mom, dad)):
            midpoint = (pair[0] + pair[1]) / 2
            distance = abs(pair[0] - pair[1])
            sister[i] = midpoint + distance * (random() - random())
            if sister[i] < mins[i]:
                sister[i] = mins[i]
            elif sister[i] > maxes[i]:
                sister[i] = maxes[i]
    if count == 2:
        brother = dad.clone()
        brother.resetStats()
        for i, pair in enumerate(izip(mom, dad)):
            midpoint = (pair[0] + pair[1]) / 2
            distance = abs(pair[0] - pair[1])
            brother[i] = midpoint + distance * (random() - random())
        if brother[i] < mins[i]:
            brother[i] = mins[i]
        elif brother[i] > maxes[i]:
            brother[i] = maxes[i]

    return (sister, brother)
        

class CustomG1DList(G1DList):
    def __init__(self, *args, **kwargs):
        G1DList.__init__(self, *args, **kwargs)
        self.initializator.set(custom_initializer)
        self.mutator.set(custom_mutator)
        self.crossover.set(custom_crossover)


class CustomGSimpleGA(GSimpleGA):
    def __init__(self, *args, **kwargs):
        # Initialize original GA engine.
        GSimpleGA.__init__(self, *args, **kwargs)
        # Set custom evolution parameters.
        self.setMinimax(minimaxType["minimize"])
        self.setMutationRate(0.05)
        self.setCrossoverRate(1.0)
        # Useful callbacks (stats, stopping evolution).
        self.stepCallback.add(stats_step_callback)
        self.terminationCriteria.add(stop_flag_criteria)
        # Clean previous stop flag.
        stop_file = self.getParam("stop_file", None)
        if stop_file and os.path.isfile(stop_file):
            os.remove(stop_file)
