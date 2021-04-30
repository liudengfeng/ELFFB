# Copyright (c) 2017-present, Facebook, Inc.
# All rights reserved.
#
# This source code is licensed under the BSD-style license found in the
# LICENSE file in the root directory of this source tree.

#!/usr/bin/env python
# -*- coding: utf-8 -*-

import argparse
from datetime import datetime

import sys
import os

from rlpytorch import *

if __name__ == '__main__':
    verbose = False

    trainer = Trainer(verbose=verbose)
    runner = SingleProcessRun()
    evaluator = Evaluator(stats=False, verbose=verbose)
    env, all_args = load_env(os.environ, trainer=trainer, runner=runner, evaluator=evaluator)

    GC = env["game"].initialize_selfplay()

    model = env["model_loaders"][0].load_model(GC.params)
    env["mi"].add_model("model", model, opt=True)
    env["mi"].add_model("actor", model, copy=True, cuda=all_args.gpu is not None, gpu_id=all_args.gpu)

    trainer.setup(sampler=env["sampler"], mi=env["mi"], rl_method=env["method"])
    evaluator.setup(sampler=env["sampler"], mi=env["mi"].clone(gpu=all_args.gpu))

    if not all_args.actor_only:
        GC.reg_callback("train1", trainer.train)
    GC.reg_callback("actor1", trainer.actor)
    GC.reg_callback("actor0", evaluator.actor)

    def summary(i):
        trainer.episode_summary(i)
        evaluator.episode_summary(i)

    def start(i):
        trainer.episode_start(i)
        evaluator.episode_start(i)

    runner.setup(GC, episode_summary=summary, episode_start=start)
    runner.run()

