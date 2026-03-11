import tensorflow as tf
import numpy as np
import matplotlib.pyplot as plt
from tensorflow.keras import layers, models
from tensorflow.keras.datasets import cifar10
from tensorflow.keras.utils import to_categorical

# ==================== CONFIGURACIÓN ====================
print("🚀 Creando CNN para CIFAR-10 con TensorFlow...")

# Parámetros del modelo
INPUT_SHAPE = (32, 32, 3)
NUM_CLASSES = 10
BATCH_SIZE = 32
EPOCHS = 25

# ==================== CARGA Y PREPARACIÓN DE DATOS ====================
print("📦 Cargando dataset CIFAR-10...")
(x_train, y_train), (x_test, y_test) = cifar10.load_data()

# Normalización de imágenes (0-255 -> 0-1)
x_train = x_train.astype('float32') / 255.0
x_test = x_test.astype('float32') / 255.0

# One-hot encoding para las etiquetas
y_train = to_categorical(y_train, NUM_CLASSES)
y_test = to_categorical(y_test, NUM_CLASSES)

print(f"📊 Datos de entrenamiento: {x_train.shape[0]} imágenes")
print(f"📊 Datos de prueba: {x_test.shape[0]} imágenes")

# ==================== ARQUITECTURA DEL MODELO ====================
print("🧠 Construyendo arquitectura CNN...")

model = models.Sequential([
    # Bloque Convolucional 1
    layers.Conv2D(32, (3, 3), activation='relu', padding='same', input_shape=INPUT_SHAPE),
    layers.BatchNormalization(),
    layers.Conv2D(32, (3, 3), activation='relu', padding='same'),
    layers.MaxPooling2D((2, 2)),
    layers.Dropout(0.25),
    
    # Bloque Convolucional 2
    layers.Conv2D(64, (3, 3), activation='relu', padding='same'),
    layers.BatchNormalization(),
    layers.Conv2D(64, (3, 3), activation='relu', padding='same'),
    layers.MaxPooling2D((2, 2)),
    layers.Dropout(0.25),
    
    # Bloque Convolucional 3
    layers.Conv2D(128, (3, 3), activation='relu', padding='same'),
    layers.BatchNormalization(),
    layers.Conv2D(128, (3, 3), activation='relu', padding='same'),
    layers.MaxPooling2D((2, 2)),
    layers.Dropout(0.25),
    
    # Capas Fully Connected
    layers.Flatten(),
    layers.Dense(256, activation='relu'),
    layers.BatchNormalization(),
    layers.Dropout(0.5),
    layers.Dense(NUM_CLASSES, activation='softmax')
])

# ==================== COMPILACIÓN ====================
model.compile(
    optimizer='adam',
    loss='categorical_crossentropy',
    metrics=['accuracy']
)

print("✅ Modelo compilado exitosamente!")
model.summary()

# ==================== ENTRENAMIENTO ====================
print("🎯 Iniciando entrenamiento...")

# Callbacks para mejor entrenamiento
callbacks = [
    tf.keras.callbacks.EarlyStopping(patience=5, restore_best_weights=True),
    tf.keras.callbacks.ReduceLROnPlateau(factor=0.5, patience=3)
]

# Entrenamiento del modelo
history = model.fit(
    x_train, y_train,
    batch_size=BATCH_SIZE,
    epochs=EPOCHS,
    validation_data=(x_test, y_test),
    callbacks=callbacks,
    verbose=1
)

# ==================== EVALUACIÓN ====================
print("📈 Evaluando modelo...")
test_loss, test_acc = model.evaluate(x_test, y_test, verbose=0)
print(f"🎯 Precisión en prueba: {test_acc:.4f}")

# ==================== GUARDAR MODELO ====================
print("💾 Guardando modelo...")
model.save('cifar10_cnn_model.h5')
print("✅ Modelo guardado como 'cifar10_cnn_model.h5'")

# ==================== CONVERSIÓN PARA TFLITE ====================
print("🔧 Convirtiendo a TensorFlow Lite...")

# Crear representante dataset para cuantización
def representative_dataset():
    for i in range(100):
        yield [x_test[i:i+1]]

# Conversión a TFLite con cuantización INT8
converter = tf.lite.TFLiteConverter.from_keras_model(model)
converter.optimizations = [tf.lite.Optimize.DEFAULT]
converter.representative_dataset = representative_dataset
converter.target_spec.supported_ops = [tf.lite.OpsSet.TFLITE_BUILTINS_INT8]
converter.inference_input_type = tf.int8
converter.inference_output_type = tf.int8

tflite_model = converter.convert()

# Guardar modelo TFLite
with open('cifar10_cnn_quantized.tflite', 'wb') as f:
    f.write(tflite_model)

print("✅ Modelo TFLite cuantizado guardado como 'cifar10_cnn_quantized.tflite'")

# ==================== VISUALIZACIÓN ====================
# Gráficas de entrenamiento
plt.figure(figsize=(12, 4))

plt.subplot(1, 2, 1)
plt.plot(history.history['accuracy'], label='Precisión Entrenamiento')
plt.plot(history.history['val_accuracy'], label='Precisión Validación')
plt.title('Precisión del Modelo')
plt.xlabel('Época')
plt.ylabel('Precisión')
plt.legend()

plt.subplot(1, 2, 2)
plt.plot(history.history['loss'], label='Pérdida Entrenamiento')
plt.plot(history.history['val_loss'], label='Pérdida Validación')
plt.title('Pérdida del Modelo')
plt.xlabel('Época')
plt.ylabel('Pérdida')
plt.legend()

plt.tight_layout()
plt.savefig('training_history.png')
plt.show()

# ==================== PREDICCIÓN DE EJEMPLO ====================
print("\n🔍 Probando predicción con ejemplo...")

# Nombres de las clases
class_names = ['Avión', 'Automóvil', 'Pájaro', 'Gato', 'Ciervo', 
               'Perro', 'Rana', 'Caballo', 'Barco', 'Camión']

# Probar con una imagen de ejemplo
sample_idx = 0  # Cambiar este índice para probar diferentes imágenes
sample_image = x_test[sample_idx]
sample_label = np.argmax(y_test[sample_idx])

# Preparar imagen para predicción (añadir dimensión batch)
input_image = np.expand_dims(sample_image, axis=0)

# Predicción
prediction = model.predict(input_image, verbose=0)
predicted_class = np.argmax(prediction)
confidence = np.max(prediction)

print(f"📸 Imagen de prueba: Clase real '{class_names[sample_label]}'")
print(f"🎯 Predicción: '{class_names[predicted_class]}' con {confidence:.2%} de confianza")

# Mostrar imagen de ejemplo
plt.figure(figsize=(6, 6))
plt.imshow(sample_image)
plt.title(f'Real: {class_names[sample_label]} | Pred: {class_names[predicted_class]} ({confidence:.2%})')
plt.axis('off')
plt.show()

print("\n🎉 ¡Proceso completado! El modelo está listo para convertir a CMSIS-NN.")