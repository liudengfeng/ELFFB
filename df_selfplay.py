# Copyright (c) 2017-present, Facebook, Inc.
# All rights reserved.
#
# This source code is licensed under the BSD-style license found in the
# LICENSE file in the root directory of this source tree.

# Console for DarkForest
import sys
import os
from rlpytorch import load_env, Evaluator, ModelInterface, ArgsProvider, EvalIters

if __name__ == '__main__':
    evaluator = Evaluator(stats=False)
    # Set game to online model.
    env, args = load_env(os.environ, evaluator=evaluator, overrides=dict(mode="selfplay", T=1))

    GC = env["game"].initialize()
    model = env["model_loaders"][0].load_model(GC.params)
    mi = ModelInterface()
    mi.add_model("model", model)
    mi.add_model("actor", model, copy=True, cuda=args.gpu is not None, gpu_id=args.gpu)
    mi["model"].eval()
    mi["actor"].eval()

    evaluator.setup(mi=mi)

    total_batchsize = 0
    total_sel_batchsize = 0

    def actor(batch):
        global total_batchsize, total_sel_batchsize
        reply = evaluator.actor(batch)
        total_sel_batchsize += batch.batchsize
        total_batchsize += batch.max_batchsize

        if total_sel_batchsize >= 5000:
            print("Batch usage: %d/%d (%.2f%%)" %
                  (total_sel_batchsize, total_batchsize, 100.0 * total_sel_batchsize / total_batchsize))
            total_sel_batchsize = 0
            total_batchsize = 0
        # import pdb
        # pdb.set_trace()
        return reply

    GC.reg_callback_if_exists("actor", actor)

    GC.Start()

    evaluator.episode_start(0)

    while True:
        GC.Run()

    GC.Stop()
