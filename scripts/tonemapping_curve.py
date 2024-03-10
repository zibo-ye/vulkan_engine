import numpy as np
import matplotlib.pyplot as plt

def aces_tonemap(color):
    m1 = np.array([
        [0.59719, 0.07600, 0.02840],
        [0.35458, 0.90834, 0.13383],
        [0.04823, 0.01566, 0.83777]
    ])
    
    color = np.dot(m1, color)
    
    a = color * (color + 0.0245786) - 0.000090537
    b = color * (0.983729 * color + 0.4329510) + 0.238081
    color = a / b
    
    m2 = np.array([
        [1.60475, -0.10208, -0.00327],
        [-0.53108, 1.10813, -0.07276],
        [-0.07367, -0.00605, 1.07602]
    ])
    
    color = np.dot(m2, color)
    
    return np.clip(color, 0.0, 1.0)

# Prepare input values in the range 10^-2 to 10, with logarithmic spacing
input_values = np.logspace(-2, 1, 256)
output_values = []

for value in input_values:
    color = np.array([value, value, value])
    tonemapped_color = aces_tonemap(color)
    output_values.append(tonemapped_color[0])  # Assuming uniform response for R, G, B

# Plotting the response curve
plt.figure(figsize=(8, 6))
plt.plot(input_values, output_values, label='Tonemapped Response')
plt.xscale('log')
plt.title('Tonemapping Response Curve (Log Scale)')
plt.xlabel('Input Intensity (Log Scale)')
plt.ylabel('Output Intensity')
plt.grid(True, which="both", ls="--")  # Grid for both major and minor ticks
plt.legend()
plt.show()
