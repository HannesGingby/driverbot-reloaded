import tensorflow as tf
from tensorflow.keras.applications import MobileNetV2
from tensorflow.keras.layers import Dense, GlobalAveragePooling2D
from tensorflow.keras.models import Model
from tensorflow.keras.preprocessing.image import ImageDataGenerator
import matplotlib.pyplot as plt

# Load a pretrained MobileNetV2 model without the top layers
base_model = MobileNetV2(weights='imagenet', include_top=False, input_shape=(224, 224, 3))

x = base_model.output
x = GlobalAveragePooling2D()(x)
x = Dense(128, activation='relu')(x)

# Define the number of classes (make sure this matches your dataset)
NUM_CLASSES = 7  # Update this based on the actual number of class folders in your data

# Load a pretrained MobileNetV2 model without the top layers
base_model = MobileNetV2(weights='imagenet', include_top=False, input_shape=(224, 224, 3))

# Add custom layers on top of the pretrained model
x = base_model.output
x = GlobalAveragePooling2D()(x)
x = Dense(128, activation='relu')(x)

# Adjust the number of output classes based on your dataset
predictions = Dense(NUM_CLASSES, activation='softmax')(x)

# Create the final model
model = Model(inputs=base_model.input, outputs=predictions)

# Freeze the base model layers (optional, if you want to fine-tune later, unfreeze them)
for layer in base_model.layers:
    layer.trainable = False

# Prepare data generators
train_datagen = ImageDataGenerator(
    rescale=1./255,       # Normalize pixel values to [0, 1]
    horizontal_flip=True  # Randomly flip images horizontally
)

train_generator = train_datagen.flow_from_directory(
    './data/preprocessed',  # Directory containing the training data
    target_size=(224, 224),
    batch_size=32,
    class_mode='categorical',
    shuffle=True
)

validation_generator = train_datagen.flow_from_directory(
    './data/preprocessed',  # Directory containing the validation data
    target_size=(224, 224),
    batch_size=32,
    class_mode='categorical',
    shuffle=False  # Do not shuffle validation data
)

# Compile the model
model.compile(optimizer='adam', loss='categorical_crossentropy', metrics=['accuracy'])

# Train the model
history = model.fit(
    train_generator,
    epochs=10,
    validation_data=validation_generator
)


# Print accuracy and validation accuracy for each epoch
for epoch in range(10):  # assuming 10 epochs as in the example
    print(f"Epoch {epoch+1}/{10}")
    print(f"Training Accuracy: {history.history['accuracy'][epoch]:.4f}")
    print(f"Validation Accuracy: {history.history['val_accuracy'][epoch]:.4f}")
    print('-' * 30)


# Plot accuracy and validation accuracy
plt.plot(history.history['accuracy'], label='Training Accuracy')
plt.plot(history.history['val_accuracy'], label='Validation Accuracy')
plt.title('Training and Validation Accuracy')
plt.xlabel('Epochs')
plt.ylabel('Accuracy')
plt.legend()
plt.show()
