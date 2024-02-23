import matplotlib.pyplot as plt
import numpy as np
import re

def extract_fps_data(data_str):
    """Extracts FPS values from a given string containing FPS data."""
    fps_data = re.findall(r'FPS: (\d+)', data_str)
    return [int(fps) for fps in fps_data]

def analyze_and_plot_fps_data(data_series, labels):
    """
    Analyzes multiple FPS data series, calculates average FPS and standard deviation,
    and plots a comparison graph with custom labels.
    
    Parameters:
    - data_series: List of strings, each containing FPS data.
    - labels: List of strings for labels corresponding to each data series.
    """
    # Ensure the number of data series matches the number of labels provided
    assert len(data_series) == len(labels), "The number of data series must match the number of labels."
    
    plt.figure(figsize=(10, 6))
    time_axis_set = False
    
    for i, data in enumerate(data_series):
        fps_data = extract_fps_data(data)
        
        # Calculate and print average FPS and standard deviation
        average_fps = np.mean(fps_data)
        std_dev_fps = np.std(fps_data)
        print(f"Average FPS for '{labels[i]}': {average_fps:.3f}, std_dev: {std_dev_fps:.3f}")
        
        # Create a time axis starting from 1s if not set
        if not time_axis_set:
            time_axis = range(1, len(fps_data) + 1)
            time_axis_set = True
        
        # Plotting
        plt.plot(time_axis, fps_data, label=labels[i], marker='o')
    
    plt.xlabel('Time (s)')
    plt.ylabel('FPS')
    plt.title('FPS Comparison Over Time')
    plt.legend()
    plt.grid(True)
    plt.ylim(bottom=0)  # Start y-axis from 0
    plt.show()

# Example usage with 3 data series and corresponding labels
data_series = [
"""
MARK iteration 1 of 10
FPS: 517
MARK iteration 2 of 10 
FPS: 533
FPS: 545
MARK iteration 3 of 10 
FPS: 532
FPS: 520
MARK iteration 4 of 10 
FPS: 531
FPS: 537
MARK iteration 5 of 10 
FPS: 528
FPS: 535
MARK iteration 6 of 10 
FPS: 530
FPS: 519
MARK iteration 7 of 10 
FPS: 540
FPS: 538
MARK iteration 8 of 10 
FPS: 543
FPS: 538
MARK iteration 9 of 10 
FPS: 532
FPS: 532
MARK iteration 10 of 10 
FPS: 539
FPS: 543
""","""
MARK iteration 1 of 10 
FPS: 402
MARK iteration 2 of 10 
FPS: 419
FPS: 421
MARK iteration 3 of 10 
FPS: 419
FPS: 420
MARK iteration 4 of 10 
FPS: 414
FPS: 416
MARK iteration 5 of 10 
FPS: 411
FPS: 423
MARK iteration 6 of 10 
FPS: 412
FPS: 423
MARK iteration 7 of 10 
FPS: 425
FPS: 415
MARK iteration 8 of 10 
FPS: 418
FPS: 415
MARK iteration 9 of 10 
FPS: 419
FPS: 428
MARK iteration 10 of 10 
FPS: 418
FPS: 417
""",
]

labels = ["Non-indexed vertices","Indexed vertices"]

# Call the function with the data series and labels
analyze_and_plot_fps_data(data_series, labels)
