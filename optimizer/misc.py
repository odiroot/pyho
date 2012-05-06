import os
import subprocess
import atexit


def default_evaluator_path():
    common_dir = os.path.dirname(os.path.abspath(__file__))
    return os.path.join(os.path.dirname(common_dir), "run_bc_evaluator")


def spawn_workers(number, command, arguments):
    workers = []
    # Launch desired number of worker processes.
    for i in range(number):
        p = subprocess.Popen([command] + arguments,
            stdout=subprocess.PIPE, stdin=subprocess.PIPE,
            #stderr=subprocess.PIPE
            )
        workers.append(p)

    # Kill children workers at exit.
    @atexit.register
    def stop_workers():
        for proc in workers:
            proc.kill()


def parse_worker_addresses(source):
    hosts = source.remote_workers.split(",")
    workers = []
    for host in hosts:
        parts = host.split(":")
        if len(parts) == 3:  # Full format.
            workers.append(tuple(parts))
        else:  # Hostname only or wrong format.
            workers.append((parts[0], "5558", "5559"))
    return workers
