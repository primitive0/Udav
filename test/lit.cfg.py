import os
import lit

config.name = "Udav tests"
config.test_format = lit.formats.ShTest()

config.test_source_root = os.path.dirname(__file__)
config.test_exec_root = os.path.join(config.my_build_root, "test")
config.suffixes = [".udav"]

config.substitutions.append(
    ("%udav", os.path.join(config.my_build_root, "UdavMain")))
