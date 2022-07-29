import numpy
import numpy as np
def list_to_numpy(input_split):
    data = input_split[-1]  # after space is data
    shape = input_split[-2]  # before space is shape
    shape = tuple(map(int, shape.split(",")))  # split shape into array and cast to tuple
    result = np.array(data.split(','), float)  # make np.ndarray out of data
    result = result.reshape(shape)  # reshape
    return result


def str_to_numpy(input_str: str):
    return list_to_numpy(input_str.split(" "))


def decode_message(input_split: list):
    data = input_split[-1]  # after space is data
    shape = input_split[-2]  # before space is shape
    shape = tuple(map(int, shape.split(",")))  # split shape into array and cast to tuple
    result = np.array(data.split(','), float)  # make np.ndarray out of data
    result = result.reshape(shape)  # reshape
    # print(result)
    return input_split[0], result


def decode_message_str(input: str):
    input_split = input.split(" ")
    return decode_message(input_split)


def encode_message(text_message: str, array, encode=True):

    np_arr = array.numpy() if type(array) != numpy.ndarray else array
    output = f'{text_message} {",".join(map(str, np_arr.shape))} {",".join(map(str, np_arr.reshape(-1)))}'
    if encode:
        output = output.encode('utf-8')
    return output
