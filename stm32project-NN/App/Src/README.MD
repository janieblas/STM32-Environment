# CMSIS-NN Fully Connected Layer Guide

🎯 **Dimensiones Correctas para `arm_fully_connected_s8()`**  
Después de extensivas pruebas con CMSIS-NN v6.0.0 y main, se confirmó la interpretación correcta de las dimensiones.

---

## ✅ Configuración Comprobada

```c
#include "arm_nnfunctions.h"

// Para una capa Fully Connected con:
// - Batch size = 1
// - 2 características de entrada  
// - 1 neurona de salida

cmsis_nn_dims input_dims = {
    .n = 1,      // Batch size
    .h = 1,      // Altura (siempre 1 para FC)
    .w = 1,      // Ancho (siempre 1 para FC)
    .c = 2       // Número de características de entrada
};

cmsis_nn_dims filter_dims = {
    .n = 2,      // Número de inputs (características de entrada) ← CLAVE
    .h = 1,      // Siempre 1 para FC
    .w = 1,      // Siempre 1 para FC  
    .c = 1       // Número de outputs (neuronas de salida) ← CLAVE
};

cmsis_nn_dims bias_dims = {
    .n = 1,      // Siempre 1
    .h = 1,      // Siempre 1
    .w = 1,      // Siempre 1
    .c = 1       // Número de outputs (debe coincidir con filter_dims.c)
};

cmsis_nn_dims output_dims = {
    .n = 1,      // Batch size (debe coincidir con input_dims.n)
    .h = 1,      // Siempre 1 para FC
    .w = 1,      // Siempre 1 para FC
    .c = 1       // Número de outputs (debe coincidir con filter_dims.c)
};
```

---

## 🧮 Ejemplo Práctico: 2 inputs → 1 output

```c
// Configuración
cmsis_nn_context ctx = {0};
cmsis_nn_fc_params fc_params = {0};
cmsis_nn_per_tensor_quant_params quant_params = {0};

// Sin cuantización (para pruebas)
fc_params.input_offset = 0;
fc_params.filter_offset = 0; 
fc_params.output_offset = 0;
quant_params.multiplier = 0x7FFFFFFF;  // 1.0 en Q31
quant_params.shift = 0;
fc_params.activation.min = -128;
fc_params.activation.max = 127;

// Kernel sum (requerido en CMSIS-NN main)
int32_t kernel_sum_buffer[1] = {0};  // Un elemento por output
ctx.buf = kernel_sum_buffer;
ctx.size = sizeof(kernel_sum_buffer);

// Datos de ejemplo: [2, 3] × [1, 1] + 0 = 5
int8_t input_data[2] = {2, 3};
int8_t filter_data[2] = {1, 1};      // Layout: [input_channels]
int32_t bias_data[1] = {0};
int8_t output_data[1] = {0};

// Llamada a la función
arm_fully_connected_s8(
    &ctx, &fc_params, &quant_params,
    &input_dims, input_data,
    &filter_dims, filter_data, 
    &bias_dims, bias_data,
    &output_dims, output_data
);

// Resultado: output_data[0] = 5 ✅
```

---

## 🔄 Para Múltiples Outputs

```c
// 2 inputs → 2 outputs
cmsis_nn_dims filter_dims_multi = {2, 1, 1, 2};  // 2 inputs, 2 outputs
cmsis_nn_dims bias_dims_multi = {1, 1, 1, 2};
cmsis_nn_dims output_dims_multi = {1, 1, 1, 2};

int8_t filter_data_multi[4] = {1, 1, 0, 0};  // [in1, in2] para output1, [in1, in2] para output2
int32_t bias_data_multi[2] = {0, 0};
int8_t output_data_multi[2] = {0};

// Kernel sum para 2 outputs
int32_t kernel_sum_buffer_multi[2] = {0, 0};
ctx.buf = kernel_sum_buffer_multi;
ctx.size = sizeof(kernel_sum_buffer_multi);
```

---

## ❌ Configuraciones que NO Funcionan

```c
// ❌ INCORRECTO - Da resultado 2 en lugar de 5
cmsis_nn_dims filter_dims_wrong = {1, 2, 1, 1};  // n=outputs, h=inputs

// ❌ INCORRECTO  
cmsis_nn_dims filter_dims_wrong2 = {1, 1, 2, 1}; // n=outputs, w=inputs
```

---

## 📋 Reglas Memorables

- `filter_dims.n = input_channels` → número de características de entrada  
- `filter_dims.c = output_channels` → número de neuronas de salida  
- `filter_dims.h` y `.w` siempre son **1** para capas Fully Connected  
- Los weights se organizan como `[input_channels × output_channels]`  
- `bias_dims.c` debe coincidir con `filter_dims.c`  
- `output_dims.c` debe coincidir con `filter_dims.c`  

---

## 🧪 Plantilla para Tests

```c
static void test_fully_connected_basic(void)
{
    cmsis_nn_context ctx = {0};
    cmsis_nn_fc_params fc_params = {0};
    cmsis_nn_per_tensor_quant_params quant_params = {0};
    
    // Configuración básica sin cuantización
    fc_params.input_offset = 0;
    fc_params.filter_offset = 0;
    fc_params.output_offset = 0;
    quant_params.multiplier = 0x7FFFFFFF;
    quant_params.shift = 0;
    fc_params.activation.min = -128;
    fc_params.activation.max = 127;
    
    // Dimensiones CORRECTAS
    cmsis_nn_dims input_dims = {1, 1, 1, 2};     // batch=1, 2 inputs
    cmsis_nn_dims filter_dims = {2, 1, 1, 1};    // 2 inputs, 1 output ← CLAVE
    cmsis_nn_dims bias_dims = {1, 1, 1, 1};
    cmsis_nn_dims output_dims = {1, 1, 1, 1};
    
    // Kernel sum
    int32_t kernel_sum[1] = {0};
    ctx.buf = kernel_sum;
    ctx.size = sizeof(kernel_sum);
    
    // ¡Implementa tu test aquí!
}
```

---

✅ **Confirmado en:** CMSIS-NN v6.0.0 y main branch  
📅 **Fecha:** Confirmado experimentalmente  
🎯 **Comportamiento:** Consistente entre versiones
