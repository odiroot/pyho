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
