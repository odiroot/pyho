from core import HybridOptimizer


def main(args, unparsed):
    local = args.local_workers is not None
    optimizer = HybridOptimizer(local=local, local_workers=args.local_workers,
        remote_workers=args.remote_workers,
        custom_evaluator=args.evaluator, unknown_args=unparsed,
        stop_flag=args.stopflag, ga_seed=args.seed,
        ga_iter=args.ngen, ga_size=args.popsize, ga_allele=args.allele,
        lm_iter=args.iter, lm_central=args.central)
    optimizer.run()
