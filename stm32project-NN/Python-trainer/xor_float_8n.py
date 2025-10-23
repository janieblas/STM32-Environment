import numpy as np
from tensorflow.keras.models import Sequential
from tensorflow.keras.layers import Dense
from tensorflow.keras.optimizers import Adam
from tensorflow.keras.constraints import MinMaxNorm

# XOR training data
X = np.array([[0, 0],
              [0, 1],
              [1, 0],
              [1, 1]], dtype=np.float32)

y = np.array([[0],
              [1],
              [1],
              [0]], dtype=np.float32)

# Search for best random seed
best_accuracy = 0
best_model = None
best_seed = None

print("Searching for optimal configuration...")
print("Architecture: 2 → 8 (ReLU) → 1 (Sigmoid)")
print("=" * 60)

for seed in range(10):
    np.random.seed(seed)
    
    model = Sequential([
        Dense(8, input_dim=2, activation='relu', name='hidden',  # ← Changed from 4 to 8
              kernel_constraint=MinMaxNorm(min_value=-2.0, max_value=2.0),
              bias_constraint=MinMaxNorm(min_value=-2.0, max_value=2.0)),
        Dense(1, activation='sigmoid', name='output',
              kernel_constraint=MinMaxNorm(min_value=-2.0, max_value=2.0),
              bias_constraint=MinMaxNorm(min_value=-2.0, max_value=2.0))
    ])
    
    model.compile(optimizer=Adam(learning_rate=0.01),
                  loss='binary_crossentropy',
                  metrics=['accuracy'])
    
    history = model.fit(X, y, epochs=3000, verbose=0)
    
    accuracy = history.history['accuracy'][-1]
    if accuracy > best_accuracy:
        best_accuracy = accuracy
        best_model = model
        best_seed = seed
    
    if accuracy == 1.0:
        print(f"✓ Seed {seed}: Accuracy = {accuracy:.4f}")
        break
    else:
        print(f"  Seed {seed}: Accuracy = {accuracy:.4f}")

print(f"\nBest result with seed {best_seed}")
print(f"Final loss: {history.history['loss'][-1]:.6f}")
print(f"Final accuracy: {best_accuracy:.4f}")

# Predictions with best model
print("\n" + "="*60)
print("PREDICTIONS")
print("="*60)
predictions = best_model.predict(X, verbose=0)

print("\nFloat model predictions:")
for i in range(len(X)):
    pred_val = predictions[i][0]
    expected = y[i][0]
    result = "✓" if (pred_val < 0.5 and expected == 0) or (pred_val >= 0.5 and expected == 1) else "✗"
    margin = abs(pred_val - 0.5)
    print(f"{result} [{X[i][0]:.0f}, {X[i][1]:.0f}] -> {pred_val:.4f} (expected: {expected:.0f}) | margin: {margin:.4f}")

# Extract weights
W1, b1 = best_model.get_layer('hidden').get_weights()
W2, b2 = best_model.get_layer('output').get_weights()

print("\n" + "="*60)
print("TRAINED WEIGHTS (FLOAT)")
print("="*60)
print(f"\nW1 (input -> hidden) [{W1.shape[0]}×{W1.shape[1]}]:")
print(W1)
print(f"\nb1 (bias hidden) [{b1.shape[0]}]:")
print(b1)
print(f"\nW2 (hidden -> output) [{W2.shape[0]}×{W2.shape[1]}]:")
print(W2)
print(f"\nb2 (bias output) [{b2.shape[0]}]:")
print(b2)

print("\n" + "="*60)
print("WEIGHT RANGES (for Q7 verification)")
print("="*60)
print(f"W1: [{W1.min():.3f}, {W1.max():.3f}]")
print(f"b1: [{b1.min():.3f}, {b1.max():.3f}]")
print(f"W2: [{W2.min():.3f}, {W2.max():.3f}]")
print(f"b2: [{b2.min():.3f}, {b2.max():.3f}]")

