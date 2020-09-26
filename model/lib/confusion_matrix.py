import io
import itertools
import matplotlib.pyplot as plt
import tensorflow as tf
import numpy as np


def plot_to_image(figure):
    """Converts the matplotlib plot specified by 'figure' to a PNG image and
  returns it. The supplied figure is closed and inaccessible after this call."""
    # Save the plot to a PNG in memory.
    buf = io.BytesIO()
    plt.savefig(buf, format="png")
    # Closing the figure prevents it from being displayed directly inside
    # the notebook.
    plt.close(figure)
    buf.seek(0)
    # Convert PNG buffer to TF image
    image = tf.image.decode_png(buf.getvalue(), channels=4)
    # Add the batch dimension
    image = tf.expand_dims(image, 0)
    return image


def plot_confusion_matrix(cm, class_names):
    """
  Returns a matplotlib figure containing the plotted confusion matrix.

  Args:
    cm (array, shape = [n, n]): a confusion matrix of integer classes
    class_names (array, shape = [n]): String names of the integer classes
  """
    figure = plt.figure(figsize=(8, 8))
    plt.imshow(cm, interpolation="nearest", cmap=plt.cm.Blues)
    plt.title("Confusion matrix")
    plt.colorbar()
    tick_marks = np.arange(len(class_names))
    plt.xticks(tick_marks, class_names, rotation=45)
    plt.yticks(tick_marks, class_names)

    cm = cm.numpy()
    # Normalize the confusion matrix.
    cm = np.around(cm.astype("float") / cm.sum(axis=1)[:, np.newaxis], decimals=2)

    # Use white text if squares are dark; otherwise black.
    threshold = cm.max() / 2.0
    for i, j in itertools.product(range(cm.shape[0]), range(cm.shape[1])):
        color = "white" if cm[i, j] > threshold else "black"
        plt.text(j, i, cm[i, j], horizontalalignment="center", color=color)

    plt.tight_layout()
    plt.ylabel("True label")
    plt.xlabel("Predicted label")
    return figure


def log_confusion_matrix(epoch, model, test_images, test_labels, file_writer_cm):
    # Use the model to predict the values from the validation dataset.
    test_pred = model.predict(test_images)

    # Calculate the confusion matrix.
    cm = tf.math.confusion_matrix(
        labels=tf.argmax(test_labels, 1), predictions=tf.argmax(test_pred, 1)
    )
    # Log the confusion matrix as an image summary.
    figure = plot_confusion_matrix(
        cm, class_names=["1", "2", "3", "4", "5", "6", "7", "8", "9"]
    )
    cm_image = plot_to_image(figure)

    # Log the confusion matrix as an image summary.
    with file_writer_cm.as_default():
        tf.summary.image("Confusion Matrix", cm_image, step=epoch)


def create_confusion_matrix_callback(model, test_data, log_dir):
    test_images, test_labels = next(test_data)
    file_writer_cm = tf.summary.create_file_writer(log_dir + "/cm")

    def _log_confusion_matrix(epoch, logs):
        log_confusion_matrix(epoch, model, test_images, test_labels, file_writer_cm)

    return _log_confusion_matrix
