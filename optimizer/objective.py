class RemoteObjective(object):
    u"Common base for all objectives working on network."
    def __init__(self, comm):
        self.comm = comm


class MemoObjectiveMixin(object):
    u"Evaluation memory mixin for objective."
    memo = {}

    def store(self, params, score):
        key = tuple(params)
        self.memo[key] = score

    def fetch(self, params):
        key = tuple(params)
        return self.memo.get(key)


class GeneticObjective(RemoteObjective, MemoObjectiveMixin):
    u"Objective class used for Genetic Algorithm optimization."
    def __call__(self, chromosome):
        p = chromosome.getInternalList()

        score = self.fetch(p)
        if score is not None:  # We already had this task.
            yield score

        # We have to compute a score.
        uid = id(chromosome)  # Unique id for messaging.
        sent = False  # Whether evaluation request is sent.
        while True:
            if not sent:
                # Send asynchronous request for evaluation.
                self.comm.evaluate(p, uid)
                sent = True
            # Try to retrieve evaluation score from the transport.
            response = self.comm.resp_score(uid)
            if response is not None:
                score = response["score"]
                self.store(p, score)
            yield score


class LevmarObjective(RemoteObjective, MemoObjectiveMixin):
    u"Objective class used for Levenberg-Marquardt optimization."
    def __call__(self, vector, *args, **kwargs):
        score = self.fetch(vector)
        if score is None:
            uid = id(vector)
            self.comm.evaluate(list(vector), uid)
            score = self.comm.resp_score(uid, wait=True)["score"]
            self.store(vector, score)
        return [score] * len(vector)


__all__ = ["GeneticObjective", "LevmarObjective"]
