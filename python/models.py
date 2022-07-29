import keras.models
from tensorflow.keras import layers
import pandas as pd
import numpy as np
import tensorflow as tf
import parse_utils
all_models=dict()
def load_model(name:str,directory:str):
    all_models[name] = keras.models.load_model(directory)


def predict(args):
    model_name, input_val = parse_utils.decode_message(args)
    result=all_models[model_name](input_val, training=False)
    return parse_utils.encode_message('predict', result)