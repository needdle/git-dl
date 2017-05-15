import os
import sys

import tensorflow as tf
from tensorflow_fold.util import proto_tools
from git_pb2 import Log
from google.protobuf import text_format

proto_tools.map_proto_source_tree_path("", os.getcwd())
proto_tools.import_proto_file("git.proto")

log = Log()
with open(sys.argv[1], 'rb') as f:
           log.ParseFromString(f.read())
           f.close()
