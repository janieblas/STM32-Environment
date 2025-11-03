import numpy as np

# ============================================================
# PASTE YOUR TRAINING WEIGHTS HERE
# ============================================================

# Hidden layer weights
W1 = np.array([
    [ 1.4140395,  -0.32317525, -0.5892663,   1.4139926,   1.4144512,  -0.5058538,   1.4895502,  -1.4142237 ],
    [-1.4143873,  -0.5707602,  -0.63660157, -1.4144346,  -1.413976,   -0.25045705, -1.3346311,   1.4142034 ]
])

b1 = np.array([ 1.8288612e-05,  0.0000000e+00,  0.0000000e+00, -5.9220994e-05,
                3.4028068e-04,  0.0000000e+00,  1.3401138e+00, -7.6320133e-04])

# Output layer weights
W2 = np.array([
    [ 9.0511572e-01],
    [ 5.7357344e-16],
    [-6.5643264e-16],
    [ 9.0668517e-01],
    [ 9.0706390e-01],
    [ 2.4115623e-15],
    [-8.5447931e-01],
    [ 8.9765680e-01]
])

b2 = np.array([-0.0746395])

# ============================================================
# CALCULATE ACTIVATIONS TO KNOW REAL RANGE
# ============================================================

def relu(x):
    return np.maximum(0, x)

# XOR test data
X = np.array([[0, 0], [0, 1], [1, 0], [1, 1]], dtype=np.float32)

# Forward pass to get activation ranges
hidden_activations = []
for x in X:
    z1 = np.dot(x, W1) + b1
    a1 = relu(z1)
    hidden_activations.append(a1)

hidden_activations = np.array(hidden_activations)
max_hidden_activation = hidden_activations.max()

print("="*60)
print("RANGE ANALYSIS")
print("="*60)
print(f"Max hidden activation: {max_hidden_activation:.4f}")
print(f"Hidden activations per case:")
for i, x in enumerate(X):
    print(f"  {x} -> {hidden_activations[i]}")

# ============================================================
# STRATEGY: PER-LAYER QUANTIZATION
# ============================================================

print("\n" + "="*60)
print("QUANTIZATION WITH PER-LAYER FACTORS")
print("="*60)

# Factor for W1 and b1 (based on absolute maximum)
max_w1 = max(abs(W1.min()), abs(W1.max()))
max_b1 = max(abs(b1.min()), abs(b1.max()))
scale_layer1 = 127.0 / max(max_w1, max_b1)

print(f"\nLayer 1 (Input -> Hidden):")
print(f"  Max |W1|: {max_w1:.4f}")
print(f"  Max |b1|: {max_b1:.4f}")
print(f"  Scale factor: {scale_layer1:.4f}")

# Factor for hidden activations
scale_hidden = 127.0 / max_hidden_activation
print(f"\nHidden Activations:")
print(f"  Max activation: {max_hidden_activation:.4f}")
print(f"  Scale factor: {scale_hidden:.4f}")

# Factor for W2 and b2
max_w2 = max(abs(W2.min()), abs(W2.max()))
max_b2 = max(abs(b2.min()), abs(b2.max()))

scale_layer2 = 127.0 / max(max_w2, max_b2)

print(f"\nLayer 2 (Hidden -> Output):")
print(f"  Max |W2|: {max_w2:.4f}")
print(f"  Max |b2|: {max_b2:.4f}")
print(f"  Scale factor: {scale_layer2:.4f}")

# ============================================================
# QUANTIZE WEIGHTS
# ============================================================

def quantize_q7(value, scale):
    return np.clip(np.round(value * scale), -128, 127).astype(np.int8)

W1_q7 = quantize_q7(W1, scale_layer1)
b1_q7 = quantize_q7(b1, scale_layer1)
W2_q7 = quantize_q7(W2, scale_layer2)
b2_q7 = quantize_q7(b2, scale_layer2)

print("\n" + "="*60)
print("QUANTIZED WEIGHTS Q7")
print("="*60)

print("\nW1_q7 (input -> hidden) [2×8]:")
print(W1_q7)
print(f"\nb1_q7 (bias hidden) [8]:")
print(b1_q7)
print(f"\nW2_q7 (hidden -> output) [8×1]:")
print(W2_q7)
print(f"\nb2_q7 (bias output) [1]:")
print(b2_q7)

# ============================================================
# VERIFICATION WITH Q7 FORWARD PASS
# ============================================================

print("\n" + "="*60)
print("VERIFICATION")
print("="*60)

def sigmoid(x):
    return 1 / (1 + np.exp(-np.clip(x, -10, 10)))

HIDDEN_SIZE = 8  # Updated for 8 neurons

for i, x in enumerate(X):
    # Float
    z1_float = np.dot(x, W1) + b1
    a1_float = relu(z1_float)
    z2_float = np.dot(a1_float, W2) + b2
    out_float = sigmoid(z2_float[0])
    
    # Q7 simulated
    x_q7 = quantize_q7(x, 127)
    
    # Hidden layer
    z1_q14 = np.zeros(HIDDEN_SIZE, dtype=np.int32)
    for h in range(HIDDEN_SIZE):
        acc = 0
        for j in range(2):
            acc += int(x_q7[j]) * int(W1_q7[j, h])
        acc += int(b1_q7[h]) << 7
        z1_q14[h] = acc >> 7
    
    a1_q7 = np.clip(z1_q14, 0, 127).astype(np.int8)
    
    # Output layer
    z2_q14 = 0
    for h in range(HIDDEN_SIZE):
        z2_q14 += int(a1_q7[h]) * int(W2_q7[h, 0])
    z2_q14 += int(b2_q7[0]) << 7
    z2_q7 = np.clip(z2_q14 >> 7, -128, 127)
    
    # To float
    z2_q7_float = z2_q7 / 127.0
    out_q7 = sigmoid(z2_q7_float * 4)
    
    expected = 1 if (x[0] != x[1]) else 0
    
    print(f"\nInput {x}:")
    print(f"  Expected: {expected}")
    print(f"  Float:    {out_float:.4f}")
    print(f"  Q7:       {out_q7:.4f} (output_q7: {z2_q7})")
    print(f"  Error:    {abs(out_float - out_q7):.4f}")
    print(f"  Margin:   {abs(out_q7 - 0.5):.4f}")

