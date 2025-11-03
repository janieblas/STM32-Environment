# 🧠 XOR Neural Network Q7 Manual para STM32F446RE

## 📋 Descripción del Proyecto
Este proyecto implementa una **red neuronal artificial completamente manual** que resuelve el problema **XOR** utilizando **cuantización Q7 (int8)** en un microcontrolador **STM32F446RE**.  

**🎯 Resultado: Inferencia en tiempo real con precisión del 100% sin usar CMSIS-NN.**

---

## 🚀 Características Principales

### ✨ Lo que hace especial este proyecto:
- ✅ **Implementación manual pura** - Sin dependencias de CMSIS-NN
- ✅ **Cuantización Q7 inteligente** - Factores de escala por capa
- ✅ **100% de precisión** - Validado contra modelo flotante
- ✅ **Educativo** - Código claro que muestra cada operación matemática
- ✅ **Optimizable** - Base para agregar CMSIS-DSP después

### 🔧 Arquitectura de Red Neuronal
```c
Entrada (2) → Capa Oculta (4, ReLU) → Salida (1, Sigmoid)
```

```
INPUT_SIZE  = 2   // Entradas: X1, X2
HIDDEN_SIZE = 4   // 4 neuronas con ReLU
OUTPUT_SIZE = 1   // Salida: resultado XOR
```

---

## 🎓 Conceptos Clave

### 📌 ¿Por qué Q7 en lugar de float?
| Aspecto | Float32 | Q7 (int8) |
|---------|---------|-----------|
| **Memoria** | 4 bytes | 1 byte (75% menos) |
| **Velocidad** | Usa FPU | Usa ALU (más rápido en loops) |
| **Consumo** | Alto | Bajo (no usa FPU) |
| **Precisión** | Alta | Suficiente para XOR |

### 🔑 ¿Qué es Q7?
**Q7 = formato de punto fijo de 8 bits con 7 bits fraccionarios**

```
Rango: -128 a 127 (int8)
Representa: -1.0 a ~0.992 (flotante)

Conversión:
float → Q7:  q7_value = round(float_value × 127)
Q7 → float:  float_value = q7_value / 127.0
```

### ⚠️ El Problema Crítico de Cuantización
**No todos los valores se pueden escalar igual.**

#### ❌ Intento #1: Factor de escala global (70.92)
```python
# Usamos el mismo factor para TODO
SCALE = 127 / max(|all_weights|)  # 127 / 1.79 = 70.92

Resultado:
- W1 cuantizados correctos ✓
- W2 cuantizados correctos ✓  
- Bias b2 = -55 ... ¡DEMASIADO GRANDE! ❌

Problema:
  Activaciones hidden: ~6,993 (Q14)
  Bias b2 escalado:    -7,040 (Q14)
  → El bias CANCELA las activaciones
  → Predicciones incorrectas
```

#### ✅ Solución: Factores de escala por capa
```python
# Cada capa usa SU propio factor óptimo
SCALE_W1 = 127 / max(|W1, b1|)  # 89.78
SCALE_W2 = 127 / max(|W2, b2|)  # 70.92

Resultado:
- Proporciones preservadas ✓
- Bias balanceado con activaciones ✓
- 100% accuracy ✓
```

---

## 🐍 Entrenamiento en Python

### 📝 Script 1: Entrenar con Constraints para Q7

**¿Por qué constraints?**  
Forzamos los pesos a estar en rango [-2, 2] para facilitar la cuantización sin saturación.

```python
import numpy as np
from tensorflow.keras.models import Sequential
from tensorflow.keras.layers import Dense
from tensorflow.keras.optimizers import Adam
from tensorflow.keras.constraints import MinMaxNorm

# Datos XOR
X = np.array([[0, 0], [0, 1], [1, 0], [1, 1]], dtype=np.float32)
y = np.array([[0], [1], [1], [0]], dtype=np.float32)

# Buscar mejor semilla random
best_accuracy = 0
best_model = None

for seed in range(10):
    np.random.seed(seed)
    
    model = Sequential([
        Dense(4, input_dim=2, activation='relu', name='hidden',
              kernel_constraint=MinMaxNorm(-2.0, 2.0),  # ← CLAVE
              bias_constraint=MinMaxNorm(-2.0, 2.0)),
        Dense(1, activation='sigmoid', name='output',
              kernel_constraint=MinMaxNorm(-2.0, 2.0),
              bias_constraint=MinMaxNorm(-2.0, 2.0))
    ])
    
    model.compile(optimizer=Adam(learning_rate=0.01),
                  loss='binary_crossentropy',
                  metrics=['accuracy'])
    
    history = model.fit(X, y, epochs=3000, verbose=0)
    accuracy = history.history['accuracy'][-1]
    
    if accuracy > best_accuracy:
        best_accuracy = accuracy
        best_model = model
    
    if accuracy == 1.0:
        break

# Extraer pesos
W1, b1 = best_model.get_layer('hidden').get_weights()
W2, b2 = best_model.get_layer('output').get_weights()
```

**🎯 Pesos obtenidos (flotante):**
```
W1: [[ 1.414,  1.414, -0.692, -0.848]
     [ 1.414,  1.415, -0.597, -0.627]]
b1: [ 0.162, -1.413,  0.0,   0.0]
W2: [[ 0.891], [-1.791], [0.0], [0.0]]
b2: [-0.768]
```

---

### 📝 Script 2: Cuantización Inteligente a Q7

**¿Por qué diferentes factores de escala?**  
Porque las activaciones hidden tienen rango diferente a los pesos originales.

```python
import numpy as np

# (Pegar aquí los pesos flotantes del script anterior)

# ============================================================
# PASO CRÍTICO: Calcular activaciones reales
# ============================================================
def relu(x):
    return np.maximum(0, x)

X = np.array([[0, 0], [0, 1], [1, 0], [1, 1]])

hidden_activations = []
for x in X:
    z1 = np.dot(x, W1) + b1
    a1 = relu(z1)
    hidden_activations.append(a1)

max_hidden = np.array(hidden_activations).max()
print(f"Max activación hidden: {max_hidden:.4f}")  # 2.99

# ============================================================
# Factores de escala POR CAPA
# ============================================================
scale_w1 = 127 / max(abs(W1.max()), abs(W1.min()), 
                     abs(b1.max()), abs(b1.min()))
scale_w2 = 127 / max(abs(W2.max()), abs(W2.min()), 
                     abs(b2.max()), abs(b2.min()))

print(f"Scale W1/b1: {scale_w1:.2f}")  # 89.78
print(f"Scale W2/b2: {scale_w2:.2f}")  # 70.92

# ============================================================
# Cuantizar
# ============================================================
W1_q7 = np.clip(np.round(W1 * scale_w1), -128, 127).astype(np.int8)
b1_q7 = np.clip(np.round(b1 * scale_w1), -128, 127).astype(np.int8)
W2_q7 = np.clip(np.round(W2 * scale_w2), -128, 127).astype(np.int8)
b2_q7 = np.clip(np.round(b2 * scale_w2), -128, 127).astype(np.int8)
```

**🎯 Pesos cuantizados Q7:**
```
W1_q7: [[ 127,  127, -62, -76]
        [ 127,  127, -54, -56]]
b1_q7: [ 15, -127,  0,  0]
W2_q7: [[ 63], [-127], [0], [0]]
b2_q7: [-55]
```

---

## 💻 Implementación en C (STM32F4)

### 🔧 Configuración del Makefile

#### Flags críticos:
```makefile
# Habilitar funciones matemáticas (para expf en sigmoid)
LDFLAGS += -lm

# Opcional si usas printf con floats para debug
LDFLAGS += -u _printf_float
```

**¿Por qué -lm?**  
La función `expf()` usada en sigmoid necesita la librería matemática.

---

### 📄 main.c - Implementación Manual

```c
#include "main.h"
#include <math.h>

/* Arquitectura */
#define INPUT_SIZE 2
#define HIDDEN_SIZE 4
#define OUTPUT_SIZE 1

/* Pesos Q7 - Del script de Python */
static const int8_t W1_q7[INPUT_SIZE][HIDDEN_SIZE] = {
    { 127,  127, -62, -76},
    { 127,  127, -54, -56}
};
static const int8_t b1_q7[HIDDEN_SIZE] = {15, -127, 0, 0};

static const int8_t W2_q7[HIDDEN_SIZE][OUTPUT_SIZE] = {
    { 63}, {-127}, {0}, {0}
};
static const int8_t b2_q7[OUTPUT_SIZE] = {-55};

/* Forward pass manual */
static float NN_Predict(const float input[2])
{
    int8_t input_q7[INPUT_SIZE];
    int8_t hidden_q7[HIDDEN_SIZE];
    int8_t output_q7;
    int32_t acc;
    
    // 1. Entrada float → Q7
    for (int i = 0; i < INPUT_SIZE; i++) {
        input_q7[i] = (int8_t)(input[i] * 127.0f);
    }
    
    // 2. Capa oculta: Input × W1 + b1 → ReLU
    for (int h = 0; h < HIDDEN_SIZE; h++) {
        acc = 0;
        
        // Multiplicación Q7 × Q7 = Q14
        for (int i = 0; i < INPUT_SIZE; i++) {
            acc += (int32_t)input_q7[i] * (int32_t)W1_q7[i][h];
        }
        
        // Bias: Q7 → Q14 (shift left 7)
        acc += (int32_t)b1_q7[h] << 7;
        
        // Q14 → Q7 (shift right 7 = dividir por 128)
        acc = acc >> 7;
        
        // Saturar y ReLU
        if (acc > 127) acc = 127;
        if (acc < -128) acc = -128;
        hidden_q7[h] = (acc < 0) ? 0 : (int8_t)acc;
    }
    
    // 3. Capa salida: Hidden × W2 + b2
    acc = 0;
    for (int h = 0; h < HIDDEN_SIZE; h++) {
        acc += (int32_t)hidden_q7[h] * (int32_t)W2_q7[h][0];
    }
    acc += (int32_t)b2_q7[0] << 7;
    acc = acc >> 7;
    
    if (acc > 127) acc = 127;
    if (acc < -128) acc = -128;
    output_q7 = (int8_t)acc;
    
    // 4. Q7 → float → Sigmoid
    float output_float = (float)output_q7 / 127.0f;
    return 1.0f / (1.0f + expf(-output_float * 4.0f));
}
```

---

## 🔬 Explicación Detallada de Operaciones Q7

### 🧮 ¿Por qué shift left 7 en el bias?

```c
// Bias está en Q7, pero el acumulador está en Q14
acc = input_q7[i] * W1_q7[i][h];  // Q7 × Q7 = Q14
acc += b1_q7[h] << 7;              // Q7 → Q14
```

**Matemática:**
```
Q7 tiene punto decimal en bit 7:  SXXXXXXX.
Q14 tiene punto decimal en bit 14: SXXXXXXXXXXXXXX.

Para sumar, ambos deben tener el mismo punto decimal:
b1_q7 = 00000000SXXXXXXX  (Q7)
   <<7 = 0SXXXXXXX0000000  (Q14) ✓
```

### 🧮 ¿Por qué shift right 7 al final?

```c
acc = acc >> 7;  // Q14 → Q7
```

**Ejemplo numérico:**
```
Input:  127 (representa 1.0 en Q7)
Peso:   100 (representa ~0.79 en Q7)
Mult:   12,700 (Q14)

12,700 >> 7 = 99 (Q7)
99 / 127 ≈ 0.78 ✓ (cercano a 1.0 × 0.79)
```

### 🧮 ¿Por qué saturar a [-128, 127]?

```c
if (acc > 127) acc = 127;
if (acc < -128) acc = -128;
```

**Previene overflow** al convertir `int32_t` → `int8_t`:
```c
// Sin saturación:
int32_t acc = 200;
int8_t result = (int8_t)acc;  // Overflow! result = -56

// Con saturación:
if (acc > 127) acc = 127;
int8_t result = (int8_t)acc;  // result = 127 ✓
```

---

## 📊 Resultados

### ✅ Output del STM32F4:
```
Network Configuration:
- Input neurons: 2
- Hidden neurons: 4 (ReLU)
- Output neurons: 1 (Sigmoid)
- Precision: Q7 (int8_t)

Testing XOR Neural Network:
----------------------------
Input: [0, 0] -> Output: 0.1807 -> Class: 0 [V]
Input: [0, 1] -> Output: 0.5549 -> Class: 1 [V]
Input: [1, 0] -> Output: 0.5549 -> Class: 1 [V]
Input: [1, 1] -> Output: 0.0245 -> Class: 0 [V]

Results Summary:
Correct predictions: 4/4
Accuracy: 100.0%
```

### 📈 Comparación con Python:
| Input | Python Float | STM32 Q7 | Error | Estado |
|-------|-------------:|---------:|------:|:------:|
| [0,0] | 0.3489 | 0.1807 | 0.168 | ✅ (ambos < 0.5) |
| [0,1] | 0.6531 | 0.5549 | 0.098 | ✅ (ambos > 0.5) |
| [1,0] | 0.6535 | 0.5549 | 0.099 | ✅ (ambos > 0.5) |
| [1,1] | 0.3454 | 0.0245 | 0.321 | ✅ (ambos < 0.5) |

**🎯 Todos los casos clasifican correctamente.**

---

## 🚀 Optimizaciones Futuras

### 1️⃣ Agregar CMSIS-DSP (Fase 2)
```c
// Reemplazar loops por:
arm_dot_prod_q7(input_q7, W1_q7[h], INPUT_SIZE, &acc);
```
**Beneficio:** 2-3x más rápido con instrucciones SIMD.

### 2️⃣ Eliminar Sigmoid con LUT (Look-Up Table)
```c
// Tabla pre-calculada en flash
const uint8_t sigmoid_lut[256] = { /* ... */ };
uint8_t result = sigmoid_lut[output_q7 + 128];
```
**Beneficio:** Sin `expf()`, ahorra 100+ ciclos.

### 3️⃣ Usar CMSIS-NN (Fase 3)
```c
arm_fully_connected_s8(/* ... */);
```
**Beneficio:** Funciones optimizadas end-to-end.

---

## 🎯 Lecciones Aprendidas

### ✅ Lo que funcionó:
1. **Constraints en entrenamiento** - Pesos dentro de [-2, 2]
2. **Factores de escala por capa** - Preservar proporciones
3. **Implementación manual primero** - Entender antes de optimizar
4. **Validación en Python** - Detectar problemas antes de C

### ❌ Lo que NO funcionó:
1. ~~Factor de escala global~~ → Bias dominó las activaciones
2. ~~Bias con `<< 24`~~ → Escala incorrecta para CMSIS-NN
3. ~~Asumir rangos uniformes~~ → Saturación en hidden layer

---

## 📚 Referencias Técnicas

### 📖 Documentación CMSIS:
- [CMSIS-NN Documentation](https://arm-software.github.io/CMSIS_5/NN/html/index.html)
- [Q Format Explanation](https://en.wikipedia.org/wiki/Q_(number_format))

### 🔬 Papers:
- "Quantization and Training of Neural Networks for Efficient Integer-Arithmetic-Only Inference" (Google, 2017)

---

## 🏆 Conclusión

Este proyecto demuestra el **flujo completo de cuantización manual**:

1. ✅ Entrenamiento con constraints en Python
2. ✅ Cuantización inteligente con factores por capa
3. ✅ Implementación manual educativa en C
4. ✅ Validación contra modelo flotante
5. ✅ 100% de precisión en STM32F4

**Base sólida para:**
- Entender cuantización a bajo nivel
- Implementar redes más complejas
- Optimizar con CMSIS-DSP/NN después

---

**👨‍💻 Autor:** Antonio Nieblas  
**🔧 Hardware:** STM32F446RE (Cortex-M4)  
**📅 Fecha:** 2025  
**📜 Licencia:** MIT