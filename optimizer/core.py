import subprocess
import atexit
import sys
import time

# Insert path to libs directory.
from common.utils import libs_to_path
libs_to_path()

from common.utils import Timer, printf, check_stop_flag
from common.communication import LocalClientComm, NetworkClientComm
from genetic import CustomG1DList, CustomGSimpleGA, stats_step_callback
from genetic import AlleleG1DList
from objective import GeneticObjective
from misc import default_evaluator_path, spawn_workers


def main(args, unknown):
    if args.local_workers:  # Local mode.
        printf("Starting optimization with local workers (%d)" %
            args.local_workers)

        # Prepare the ZeroMQ communication layer.
        cc = LocalClientComm()

        # Arguments to be passed to evaluator processes.
        evaluator_args = unknown + ["-local-mode", "-local-pull-address",
            cc.push_addr, "-local-publish-address", cc.sub_addr]
        # Launch worker processes.
        command = args.evaluator or default_evaluator_path()
        spawn_workers(args.local_workers, command, evaluator_args)

    else:  # Network mode.
        printf("Starting optimization with network workers")
        if not args.remote_workers:
            raise RuntimeError("You have to specify a list of remote"
                " worker addresses")

        # Parse workers addresses.
        hosts = args.remote_workers.split(",")
        workers = []
        for host in hosts:
            parts = host.split(":")
            if len(parts) == 3:  # Full format.
                workers.append(tuple(parts))
            else:  # Hostname only or wrong format.
                workers.append((parts[0], "5558", "5559"))

        # Connect to workers with ZeroMQ.
        cc = NetworkClientComm(addresses=workers)

    # Wait until (presumably) all workers are awake and ready
    # to avoid unfair distribution of tasks.
    time.sleep(1)
    printf("Waiting for initial connection")

    # Fetch constraints from any worker.
    _sent = False
    while not _sent:
        _sent = cc.get_options(0, wait=False)
        if check_stop_flag(args.stopflag):
            sys.exit(-1)
    resp = cc.resp_options(0, wait=True)

    no_vars = resp["num_params"]
    my_min = resp["min_constr"]
    my_max = resp["max_constr"]
    printf("Received initial data from workers")

    # Prepare the GA engine.
    # Initialize genome with constraints.
    if args.allele:  # Built-in allele version.
        genome = AlleleG1DList(no_vars, constr_min=my_min, constr_max=my_max)
    else:  # Custom, ported genetic operators.
        genome = CustomG1DList(no_vars)
        genome.setParams(min_constr=my_min, max_constr=my_max)
    genome.evaluator.set(GeneticObjective(cc))
    # Set GA engine parameters.
    ga = CustomGSimpleGA(genome, args.seed)
    ga.setPopulationSize(args.popsize or 200)
    ga.setGenerations(args.ngen or 100)
    ga.setParams(stop_file=args.stopflag)

    # Fire the Genetic Algorithm Engine.
    printf("Starting GA: %d generations of %d individuals" % (ga.nGenerations,
        ga.getPopulation().popSize))
    timer = Timer().start()
    ga.setParams(timer=timer)
    ga.evolve()
    stats_step_callback(ga)  # Display final statistics.

    # Evolution is stoped.
    run_time = timer.stop()
    print "GA finished in %g s." % run_time

    ## TODO: End of evaluation output and files. ##
    # bridge.coil_to_print(ga.bestIndividual().getInternalList())

    # Save optimization results CBlock output.
    cc.save_output(ga.bestIndividual().getInternalList(), 1)
    resp = cc.resp_save(1, wait=True)
    if resp["status"] == "":
        print "Saved files: %s" % ', '.join(resp["files"])

    # Close communicator.
    cc.close()


__all__ = ["main"]
