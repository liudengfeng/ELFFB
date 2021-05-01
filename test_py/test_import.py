import pytest
import sys
import os


def test_raises():
    """没有正确配置触发导入异常"""
    with pytest.raises(ModuleNotFoundError):
        from rlpytorch import Trainer


def test_no_raises():
    """正确导入"""
    import config
    config.config_for_test()
    from rlpytorch import Trainer
    # 测试顺序不定，需要移除路径，确保上一个测试正确
    sys.path.remove(os.path.join(os.path.dirname(__file__), '..'))
