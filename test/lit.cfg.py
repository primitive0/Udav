import os
import lit

config.name = "Udav tests"
config.test_format = lit.formats.ShTest()

config.test_source_root = os.path.dirname(__file__)
config.test_exec_root = os.path.join(config.test_source_root, "lit_output")
config.suffixes = [".udav"]

udav = os.path.join(config.test_source_root, "..", "build", "Debug", "UdavMain")
config.substitutions.append(("%udav", udav))
