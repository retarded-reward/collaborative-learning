import keras
from keras import layers
from keras.datasets import mnist
import numpy as np

'''
This is a simple example of a Keras model.
It only serves the purpose to show that a ML framework use in a custom module
can be imported in a cpp program
'''
def hello_world_keras():

    print(keras.__version__)
    
    # Load the data and split it between train and test sets
    (x_train, y_train), (x_test, y_test) = keras.datasets.mnist.load_data()

    # Scale images to the [0, 1] range
    x_train = x_train.astype("float32") / 255
    x_test = x_test.astype("float32") / 255
    # Make sure images have shape (28, 28, 1)
    x_train = np.expand_dims(x_train, -1)
    x_test = np.expand_dims(x_test, -1)
    print("x_train shape:", x_train.shape)
    print("y_train shape:", y_train.shape)
    print(x_train.shape[0], "train samples")
    print(x_test.shape[0], "test samples")
