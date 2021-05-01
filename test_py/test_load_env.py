import config

import os
import argparse

import pytest


def init_for_test():
    config.config_for_test()
    # 已经将父目录添加到系统，可以直接使用项目目录
    os.environ.setdefault("game", "rts/game_MC/game")
    os.environ.setdefault("model", "actor_critic")
    os.environ.setdefault("model_file", "rts/game_MC/model")


def test_load_module():
    init_for_test()
    from rlpytorch import model_loader
    game = model_loader.load_module(os.environ["game"]).Loader()
    assert repr(type(game)) == "<class 'game.Loader'>"


def test_mock_load_env():
    init_for_test()
    from rlpytorch import (Trainer,
                           SingleProcessRun,
                           ArgsProvider,
                           ModelLoader,
                           model_loader,
                           Sampler,
                           ModelInterface)
    envs = os.environ
    load_module = model_loader.load_module
    defaults = dict()
    overrides = dict()
    num_models = None
    kwargs = {}

    trainer = Trainer()
    runner = SingleProcessRun()

    game = load_module(envs["game"]).Loader()
    model_file = load_module(envs["model_file"])

    if len(model_file.Models[envs["model"]]) == 2:
        model_class, method_class = model_file.Models[envs["model"]]
        sampler_class = Sampler
    else:
        model_class, method_class, sampler_class = model_file.Models[envs["model"]]

    defaults.update(getattr(model_file, "Defaults", dict()))
    overrides.update(getattr(model_file, "Overrides", dict()))

    method = method_class()
    sampler = sampler_class()
    mi = ModelInterface()

    # You might want multiple models loaded.
    if num_models is None:
        model_loaders = [ModelLoader(model_class)]
    else:
        model_loaders = [ModelLoader(model_class, model_idx=i)
                         for i in range(num_models)]

    env = dict(game=game, method=method, sampler=sampler,
               model_loaders=model_loaders, mi=mi)
    env.update(kwargs)

    parser = argparse.ArgumentParser()
    # 模拟命令行
    cmd_key = 'save_replay_prefix'
    cmd_v = '~/log/elf/'
    cmd_line = [f'--{cmd_key}', cmd_v]
    all_args = ArgsProvider.Load(
        parser, env, cmd_line=cmd_line, global_defaults=defaults, global_overrides=overrides)
    assert all_args[cmd_key] == cmd_v
    assert 'game' in env.keys()

