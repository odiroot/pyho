import sys
import time

# Insert path to libs directory.
from common.utils import libs_to_path
libs_to_path()

from common.utils import Timer, printf, check_stop_flag
from common.communication import LocalClientComm, NetworkClientComm
from genetic import CustomG1DList, CustomGSimpleGA, stats_step_callback
from genetic import AlleleG1DList
from objective import GeneticObjective, LevmarObjective
from misc import default_evaluator_path, spawn_workers, parse_worker_addresses
import pyximport
pyximport.install()
from levmar import levmar


class OptimizationStep(object):
    def __init__(self, no_vars, mins, maxes, comm, seed, stop_flag):
        self.no_vars = no_vars
        self.mins = mins
        self.maxes = maxes
        self.comm = comm
        self.seed = seed
        self.stop_flag = stop_flag

    def prepare(self):
        pass

    def run(self):
        return None


class GeneticOptimization(OptimizationStep):
    "Genetic optimization step."
    def __init__(self, no_vars, mins, maxes, comm, seed, stop_flag,
            allele=False, size=200, generations=100):
        super(GeneticOptimization, self).__init__(no_vars, mins, maxes, comm,
            seed, stop_flag)
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
    def __init__(self, no_vars, mins, maxes, comm, seed, stop_flag, p0=None,
            max_iter=10):
        super(LevmarOptimization, self).__init__(no_vars, mins, maxes, comm,
            seed, stop_flag)
        self.timer = Timer()
        # If starting vector is not specified, start with min values.
        self.p0 = p0 or mins
        self.max_iter = max_iter
        self.prepare()

    def prepare(self):
        # Starting residuals (or observed values).
        self.y = [0.]
        self.bounds = zip(self.mins, self.maxes)

    def run(self):
        printf("Starting LM: %d iterations" % self.max_iter)
        objective = LevmarObjective(self.comm)
        self.timer.start()
        output = levmar(objective, p0=self.p0, y=self.y, bounds=self.bounds,
            maxit=self.max_iter, cdif=False, eps1=1e-15, eps2=1e-15, eps3=1e-20,
            full_output=True, breakf=self.__breakf)
        self.__post_run(output)
        return list(output.p)

    def __post_run(self, lm_output):
        run_time = self.timer.stop()
        print "LM finished in %g s." % run_time
        print lm_output.info

    def __breakf(self, i, maxit, p, error):
        if check_stop_flag(self.stop_flag):
            return 1
        printf("Iteration %d of %d, ||error|| %g" % (i, maxit, error))


class HybridOptimizer(object):
    "The hybrid (two-step) optimization engine."
    def __init__(self, local=True, local_workers=None, remote_workers=None,
            custom_evaluator=None, extra_args=None, stop_flag=None, seed=None,
            ga_iter=None, ga_size=None):
        self.cc = None  # Client communicator.
        self.stop_flag = stop_flag
        self.seed = seed
        self.ga_iter = ga_iter
        self.ga_size = ga_size
        # Initialize relation with workers.
        if local:
            self.__local_mode(local_workers, extra_args, custom_evaluator)
        else:
            self.__network_mode(remote_workers)

    def __del__(self):
        # Close communicator.
        if self.cc:
            self.cc.close()

    def __local_mode(self, workers, xargs, evaluator):
        printf("Starting optimization with local workers (%d)" % workers)
        # Prepare the ZeroMQ communication layer.
        self.cc = LocalClientComm()

        # Arguments to be passed to evaluator processes.
        evaluator_args = xargs + ["-local-mode", "-local-pull-address",
            self.cc.push_addr, "-local-publish-address", self.cc.sub_addr]
        # Launch worker processes.
        command = evaluator or default_evaluator_path()
        spawn_workers(workers, command, evaluator_args)

    def __network_mode(self, workers):
        printf("Starting optimization with network workers")
        if not workers:
            raise RuntimeError("You have to specify a list of remote"
                " worker addresses")
        # Connect to workers with ZeroMQ.
        self.cc = NetworkClientComm(addresses=parse_worker_addresses(workers))

    def __fetch_constraints(self):
        u"Fetch constraints from any worker."
        _sent = False
        while not _sent:
            _sent = self.cc.get_options(0, wait=False)
            if check_stop_flag(self.stop_flag):
                sys.exit(-1)
        resp = self.cc.resp_options(0, wait=True)
        self.no_vars = resp["num_params"]
        self.mins = resp["min_constr"]
        self.maxes = resp["max_constr"]

    def prepare_optimization(self):
        # Wait until (presumably) all workers are awake and ready
        # to avoid unfair distribution of tasks.
        time.sleep(1)
        printf("Waiting for initial connection")
        self.__fetch_constraints()
        printf("Received initial data from workers")

    def run(self):
        self.prepare_optimization()
        args = dict(no_vars=self.no_vars, mins=self.mins, maxes=self.maxes,
            comm=self.cc, seed=self.seed, stop_flag=self.stop_flag)

        # Prepare and run Genetic step.
        ga_args = dict(args)
        if self.ga_iter:
            ga_args["generations"] = self.ga_iter
        if self.ga_size:
            ga_args["size"] = self.ga_size
        ga_opt = GeneticOptimization(**ga_args)
        ga_results = ga_opt.run()

        # Prepare and run Levmar step.
        lm_args = dict(args)
        # Pass result vector from previous step.
        lm_args["p0"] = ga_results
        lm_opt = LevmarOptimization(**lm_args)
        lm_results = lm_opt.run()
        self.save_output(lm_results)

    def save_output(self, parameters):
        u"Save optimization results"
        self.cc.save_output(parameters, 1)
        response = self.cc.resp_save(1, wait=True)
        if response["status"] == "":
            print "Saved files: %s" % ', '.join(response["files"])


__all__ = ["HybridOptimizer"]
