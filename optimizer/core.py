import sys
import time
import warnings
# Insert path to libs directory.
from common.utils import libs_to_path, check_stop_flag
libs_to_path()
from common.utils import printf
from common.communication import LocalClientComm, NetworkClientComm
from misc import default_evaluator_path, spawn_workers, parse_worker_addresses
from steps import GeneticOptimization, LevmarOptimization


class HybridOptimizer(object):
    "The hybrid (two-step) optimization engine."
    def __init__(self, local=True, local_workers=None, remote_workers=None,
            custom_evaluator=None, extra_args=None, stop_flag=None, seed=None,
            ga_iter=None, ga_size=None, ga_allele=None, lm_iter=None):
        self.cc = None  # Client communicator.
        self.stop_flag = stop_flag
        self.seed = seed
        self.ga_iter = ga_iter
        self.ga_size = ga_size
        self.ga_allele = ga_allele
        self.lm_iter = lm_iter
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
        ga_results = self.run_genetic(args)
        lm_results = self.run_levmar(args, ga_results)
        self.save_output(lm_results)

    def run_genetic(self, common_args):
        u"Prepare and run genetic step."
        ga_args = dict(common_args)
        if self.ga_iter:
            ga_args["generations"] = self.ga_iter
        if self.ga_size:
            ga_args["size"] = self.ga_size
        if self.ga_allele is not None:
            ga_args["allele"] = self.ga_allele
        ga_opt = GeneticOptimization(**ga_args)
        return ga_opt.run()

    def run_levmar(self, common_args, start_vector):
        u"Prepare and run levmar step."
        lm_args = dict(common_args)
        lm_args["p0"] = start_vector
        if self.lm_iter:
            lm_args["max_iter"] = self.lm_iter
        lm_opt = LevmarOptimization(**lm_args)
        with warnings.catch_warnings():
            warnings.filterwarnings("ignore")
            return lm_opt.run()

    def save_output(self, parameters):
        u"Save optimization results"
        self.cc.save_output(parameters, 1)
        response = self.cc.resp_save(1, wait=True)
        if response["status"] == "" and response["files"]:
            print "Saved files: %s" % ', '.join(response["files"])


__all__ = ["HybridOptimizer"]
