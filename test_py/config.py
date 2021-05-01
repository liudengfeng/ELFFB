import sys
import os


def config_for_test():
    # 添加父目录
    p_path = os.path.join(os.path.dirname(__file__), '..')
    if p_path not in sys.path:
        sys.path.insert(0, p_path)
