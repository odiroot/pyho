from core import HybridOptimizer


def main(args, unparsed):
    local = args.local_workers is not None
    optimizer = HybridOptimizer(local=local, local_workers=args.local_workers,
        remote_workers=args.remote_workers,
        custom_evaluator=args.evaluator, extra_args=unparsed,
        stop_flag=args.stopflag, seed=args.seed,
        ga_iter=args.ngen, ga_size=args.popsize, ga_allele=args.allele,
        lm_iter=args.iter)
    optimizer.run()
