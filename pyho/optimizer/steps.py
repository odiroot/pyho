# Insert path to libs directory.
from common.utils import libs_to_path
libs_to_path()
from common.utils import Timer, printf, check_stop_flag
from genetic import CustomG1DList, CustomGSimpleGA, stats_step_callback
from genetic import AlleleG1DList
from objective import GeneticObjective, LevmarObjective
# LM imports
import pyximport
pyximport.install()
from levmar import levmar
import numpy as np


class OptimizationStep(object):
    def __init__(self, no_vars, mins, maxes, comm, stop_flag):
        self.no_vars = no_vars
        self.mins = mins
        self.maxes = maxes
        self.comm = comm
        self.stop_flag = stop_flag

    def prepare(self):
        pass

    def run(self):
        return None


class GeneticOptimization(OptimizationStep):
    "Genetic optimization step."
    def __init__(self, no_vars, mins, maxes, comm, stop_flag, seed=False,
            allele=False, size=200, generations=100):
        super(GeneticOptimization, self).__init__(no_vars, mins, maxes, comm,
            stop_flag)
        self.seed = seed
        self.allele = allele
        self.size = size
        self.timer = Timer()
        self.generations = generations
        self.prepare()

    def __prepare_genome(self):
        u"Initialize genome with constraints."
        if self.allele:  # Built-in allele version.
            self.genome = AlleleG1DList(self.no_vars, constr_min=self.mins,
                constr_max=self.maxes)
        else:  # Custom, ported genetic operators.
            self.genome = CustomG1DList(self.no_vars)
            self.genome.setParams(min_constr=self.mins, max_constr=self.maxes)
        self.genome.evaluator.set(GeneticObjective(self.comm))

    def __prepare_engine(self):
        u"Prepare the GA engine."
        # Set GA engine parameters.
        self.ga = CustomGSimpleGA(self.genome, self.seed)
        self.ga.setPopulationSize(self.size)
        self.ga.setGenerations(self.generations)
        self.ga.setParams(stop_file=self.stop_flag)
        self.ga.setParams(timer=self.timer)

    def prepare(self):
        self.__prepare_genome()
        self.__prepare_engine()

    def run(self):
        u"Fire the Genetic Algorithm Engine."
        printf("Starting GA: %d generations of %d individuals" % (
            self.generations, self.size))
        self.timer.start()
        self.ga.evolve()
        self.__post_run()
        # Return the best parameters found.
        return list(self.ga.bestIndividual().getInternalList())

    def __post_run(self):
        stats_step_callback(self.ga)  # Display final statistics.
        # Evolution is stoped.
        run_time = self.timer.stop()
        print "GA finished in %g s." % run_time


class LevmarOptimization(OptimizationStep):
    "Levenberg-Marquardt optimization step."
    def __init__(self, no_vars, mins, maxes, comm, stop_flag, p0=None,
            max_iter=5, central=False):
        super(LevmarOptimization, self).__init__(no_vars, mins, maxes, comm,
            stop_flag)
        self.timer = Timer()
        # If starting vector is not specified, start with min values.
        self.p0 = np.array(p0 or mins)
        self.max_iter = max_iter
        self.central = central
        self.prepare()

    def prepare(self):
        # Starting residuals (or observed values).
        self.y = np.array([0] * self.no_vars)
        self.bounds = zip(self.mins, self.maxes)

    def run(self):
        printf("Starting LM: %d iterations" % self.max_iter)
        objective = LevmarObjective(self.comm)
        self.timer.start()
        output = levmar(objective, p0=self.p0, y=self.y, bounds=self.bounds,
            maxit=self.max_iter, cdif=self.central, eps1=1e-15, eps2=1e-15,
            eps3=1e-20, full_output=True, breakf=self.__breakf)
        self.__post_run(output)
        return list(output.p)

    def __post_run(self, lm_output):
        run_time = self.timer.stop()
        info = lm_output.info
        printf("Iteration %d of %d, ||error|| %g" % (info[2], self.max_iter,
            info[1][0]))
        print "LM finished in %g s, with reason: %s" % (run_time, info[3])

    def __breakf(self, i, maxit, p, error):
        pstr = "Iteration %d of %d, ||error|| %g" % (i, maxit, error)
        self.timer.lap()
        if i > 0:
            iter_left = maxit - i
            mean_time = self.timer.time_so_far / i
            pstr += "\t(est. time to end=%d s.)" % (iter_left * mean_time)
        printf(pstr)
        if check_stop_flag(self.stop_flag):
            return 1
