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
    runner = SingleProcessRun()
    evaluators = [ Evaluator(name="eval" + str(i), verbose=verbose) for i in range(2) ]
    env, all_args = load_env(os.environ, num_models=2, evaluators=evaluators, runner=runner, overrides=dict(actor_only=True))

    GC = env["game"].initialize_selfplay()

    for i, (model_loader, e) in enumerate(zip(env["model_loaders"], evaluators)):
        model = model_loader.load_model(GC.params)
        mi = ModelInterface()
        mi.add_model("actor", model, copy=False)
        e.setup(sampler=env["sampler"], mi=mi)
        GC.reg_callback("actor%d" % i, e.actor)

    def summary(i):
        for e in evaluators:
            e.episode_summary(i)

    def start(i):
        for e in evaluators:
            e.episode_start(i)

    runner.setup(GC, episode_summary=summary, episode_start=start)
    runner.run()

