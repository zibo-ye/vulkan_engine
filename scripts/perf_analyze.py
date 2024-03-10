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
    assert len(data_series) == len(
        labels), "The number of data series must match the number of labels."

    plt.figure(figsize=(10, 6))
    time_axis_set = False

    for i, data in enumerate(data_series):
        fps_data = extract_fps_data(data)

        # Calculate and print average FPS and standard deviation
        average_fps = np.mean(fps_data)
        std_dev_fps = np.std(fps_data)
        print(
            f"Average FPS for '{labels[i]}': {average_fps:.3f}, std_dev: {std_dev_fps:.3f}")

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


raw_data = {
#     "Environment": """
# MARK iteration 2 of 10 
# FPS: 4986.8, Avg Frame Time: 200.53us , P99: 524.9us , P95: 197.6us , P90: 147us , Std Dev: 4278.75
# MARK iteration 3 of 10 
# FPS: 6816.02, Avg Frame Time: 146.713us , P99: 577.5us , P95: 229.1us , P90: 163.5us , Std Dev: 102.07
# FPS: 6424.4, Avg Frame Time: 155.657us , P99: 491.8us , P95: 196.5us , P90: 147.9us , Std Dev: 1177.52
# MARK iteration 4 of 10 
# FPS: 7198.73, Avg Frame Time: 138.913us , P99: 416.3us , P95: 170.9us , P90: 146.5us , Std Dev: 64.009
# FPS: 6309.52, Avg Frame Time: 158.491us , P99: 487.7us , P95: 196us , P90: 152us , Std Dev: 1280.57
# MARK iteration 5 of 10 
# FPS: 6821.72, Avg Frame Time: 146.591us , P99: 596.1us , P95: 217.1us , P90: 153.8us , Std Dev: 117.437
# FPS: 6141.82, Avg Frame Time: 162.818us , P99: 560.1us , P95: 208.9us , P90: 150.2us , Std Dev: 1212.04
# MARK iteration 6 of 10 
# FPS: 7016.85, Avg Frame Time: 142.514us , P99: 454.7us , P95: 184.5us , P90: 150.7us , Std Dev: 74.9095
# FPS: 6307, Avg Frame Time: 158.554us , P99: 486.8us , P95: 200.3us , P90: 150.3us , Std Dev: 1445.88
# MARK iteration 7 of 10 
# FPS: 7188.89, Avg Frame Time: 139.104us , P99: 483us , P95: 198.5us , P90: 147.8us , Std Dev: 84.756
# FPS: 6307.79, Avg Frame Time: 158.534us , P99: 605.3us , P95: 220.9us , P90: 155.3us , Std Dev: 1219.89
# MARK iteration 8 of 10 
# FPS: 6492.88, Avg Frame Time: 154.015us , P99: 703.6us , P95: 238.9us , P90: 157.8us , Std Dev: 230.989
# FPS: 6431.38, Avg Frame Time: 155.488us , P99: 553.6us , P95: 198.2us , P90: 144.3us , Std Dev: 1213.65
# MARK iteration 9 of 10 
# FPS: 7011.57, Avg Frame Time: 142.621us , P99: 468.5us , P95: 193.4us , P90: 151.9us , Std Dev: 77.748
# FPS: 6560.7, Avg Frame Time: 152.423us , P99: 442.6us , P95: 184.2us , P90: 147.5us , Std Dev: 1189.16
# MARK iteration 10 of 10 
# FPS: 7003.14, Avg Frame Time: 142.793us , P99: 500us , P95: 207.5us , P90: 153.1us , Std Dev: 132.766
# FPS: 6731.84, Avg Frame Time: 148.548us , P99: 356.2us , P95: 171.9us , P90: 144.2us , Std Dev: 1140.55
# """,
#     "Mirror": """
# MARK iteration 2 of 10 
# FPS: 5939.53, Avg Frame Time: 168.363us , P99: 462.3us , P95: 186us , P90: 153us , Std Dev: 1390.9
# MARK iteration 3 of 10 
# FPS: 6665.68, Avg Frame Time: 150.022us , P99: 589.3us , P95: 195.2us , P90: 157.8us , Std Dev: 96.7118
# FPS: 5724.51, Avg Frame Time: 174.687us , P99: 503.8us , P95: 261.5us , P90: 243.1us , Std Dev: 1239.84
# MARK iteration 4 of 10 
# FPS: 6501.81, Avg Frame Time: 153.803us , P99: 479.3us , P95: 251.6us , P90: 235.1us , Std Dev: 82.7316
# FPS: 6063.99, Avg Frame Time: 164.908us , P99: 441.7us , P95: 247.2us , P90: 215.9us , Std Dev: 1197.64
# MARK iteration 5 of 10 
# FPS: 6732.15, Avg Frame Time: 148.541us , P99: 499.6us , P95: 206.6us , P90: 156.7us , Std Dev: 113.124
# FPS: 6172.71, Avg Frame Time: 162.003us , P99: 487.6us , P95: 206.7us , P90: 157.2us , Std Dev: 1227.08
# MARK iteration 6 of 10 
# FPS: 6759.77, Avg Frame Time: 147.934us , P99: 461us , P95: 194.9us , P90: 156.6us , Std Dev: 81.0689
# FPS: 5780.99, Avg Frame Time: 172.981us , P99: 431.9us , P95: 178.6us , P90: 152us , Std Dev: 1588.55
# MARK iteration 7 of 10 
# FPS: 4985.6, Avg Frame Time: 200.578us , P99: 1034.5us , P95: 359.2us , P90: 185.9us , Std Dev: 1286.68
# FPS: 5917.24, Avg Frame Time: 168.998us , P99: 708.9us , P95: 222.7us , P90: 158.8us , Std Dev: 1285.86
# MARK iteration 8 of 10 
# FPS: 6324.55, Avg Frame Time: 158.114us , P99: 666.2us , P95: 279.8us , P90: 173us , Std Dev: 114.344
# FPS: 6049.2, Avg Frame Time: 165.311us , P99: 510.8us , P95: 200.1us , P90: 157.3us , Std Dev: 1346.65
# MARK iteration 9 of 10 
# FPS: 6794.39, Avg Frame Time: 147.18us , P99: 443.4us , P95: 200.2us , P90: 158.3us , Std Dev: 74.5694
# FPS: 6116.93, Avg Frame Time: 163.481us , P99: 435.3us , P95: 184.2us , P90: 157us , Std Dev: 1288.06
# MARK iteration 10 of 10 
# FPS: 6731.27, Avg Frame Time: 148.56us , P99: 457us , P95: 200.2us , P90: 158.7us , Std Dev: 83.1127
# FPS: 5803.89, Avg Frame Time: 172.298us , P99: 468.1us , P95: 182.2us , P90: 149.5us , Std Dev: 1403.47
# """,

#     "Lambertian": """
# FPS: 5562.5, Avg Frame Time: 179.775us , P99: 485.9us , P95: 215.7us , P90: 166.2us , Std Dev: 1292.63
# MARK iteration 3 of 10 
# FPS: 6271.47, Avg Frame Time: 159.452us , P99: 528.6us , P95: 228.7us , P90: 170.9us , Std Dev: 82.2913
# FPS: 5565.21, Avg Frame Time: 179.688us , P99: 624.9us , P95: 238.6us , P90: 173.6us , Std Dev: 1252.37
# MARK iteration 4 of 10 
# FPS: 6349.4, Avg Frame Time: 157.495us , P99: 477.5us , P95: 206us , P90: 165.7us , Std Dev: 77.9967
# FPS: 5691.92, Avg Frame Time: 175.688us , P99: 452.3us , P95: 202.7us , P90: 166.6us , Std Dev: 1407.93
# MARK iteration 5 of 10 
# FPS: 6243.99, Avg Frame Time: 160.154us , P99: 498.2us , P95: 221.2us , P90: 169.3us , Std Dev: 124.088
# FPS: 5560.59, Avg Frame Time: 179.837us , P99: 544.5us , P95: 221.6us , P90: 168.3us , Std Dev: 1455.58
# MARK iteration 6 of 10 
# FPS: 6140.29, Avg Frame Time: 162.859us , P99: 589.8us , P95: 231.9us , P90: 172.7us , Std Dev: 109.856
# FPS: 5486.75, Avg Frame Time: 182.257us , P99: 507.5us , P95: 219.8us , P90: 175us , Std Dev: 1268.17
# MARK iteration 7 of 10 
# FPS: 6276.12, Avg Frame Time: 159.334us , P99: 521.2us , P95: 208.7us , P90: 165.9us , Std Dev: 86.2172
# FPS: 5684.84, Avg Frame Time: 175.907us , P99: 497us , P95: 209.9us , P90: 166.6us , Std Dev: 1360.01
# MARK iteration 8 of 10 
# FPS: 6188.71, Avg Frame Time: 161.585us , P99: 573.3us , P95: 230.1us , P90: 171.3us , Std Dev: 107.452
# FPS: 5544.67, Avg Frame Time: 180.353us , P99: 612.4us , P95: 236.9us , P90: 170.3us , Std Dev: 1259.84
# MARK iteration 9 of 10 
# FPS: 6241.4, Avg Frame Time: 160.22us , P99: 504.1us , P95: 228.6us , P90: 171.5us , Std Dev: 96.3635
# FPS: 5675.83, Avg Frame Time: 176.186us , P99: 498.8us , P95: 218.5us , P90: 168.4us , Std Dev: 1253.99
# MARK iteration 10 of 10 
# FPS: 6219.93, Avg Frame Time: 160.773us , P99: 553.5us , P95: 219.3us , P90: 168.3us , Std Dev: 89.4815
# FPS: 5296.44, Avg Frame Time: 188.806us , P99: 531.6us , P95: 224.4us , P90: 170.8us , Std Dev: 2140.02
# """,

#     "PBR": """
# FPS: 3671.85, Avg Frame Time: 272.342us , P99: 895.4us , P95: 344.5us , P90: 262.8us , Std Dev: 3863.61
# MARK iteration 3 of 10 
# FPS: 6028.3, Avg Frame Time: 165.884us , P99: 631.3us , P95: 234.3us , P90: 173.4us , Std Dev: 119.572
# FPS: 5636.86, Avg Frame Time: 177.404us , P99: 565.1us , P95: 218.3us , P90: 168.7us , Std Dev: 1273.64
# MARK iteration 4 of 10 
# FPS: 6224.65, Avg Frame Time: 160.652us , P99: 480.1us , P95: 202.3us , P90: 167.8us , Std Dev: 78.8195
# FPS: 5636.32, Avg Frame Time: 177.421us , P99: 475us , P95: 203.3us , P90: 168.6us , Std Dev: 1333.87
# MARK iteration 5 of 10 
# FPS: 6129.93, Avg Frame Time: 163.134us , P99: 583.6us , P95: 233.6us , P90: 174.5us , Std Dev: 109.12
# FPS: 5626.91, Avg Frame Time: 177.717us , P99: 475.1us , P95: 215.1us , P90: 171us , Std Dev: 1321.93
# MARK iteration 6 of 10 
# FPS: 6005.69, Avg Frame Time: 166.509us , P99: 623.7us , P95: 248.7us , P90: 177.5us , Std Dev: 103.564
# FPS: 5549.69, Avg Frame Time: 180.19us , P99: 600.8us , P95: 223.6us , P90: 176.5us , Std Dev: 1430.78
# MARK iteration 7 of 10 
# FPS: 6281.38, Avg Frame Time: 159.201us , P99: 500.1us , P95: 211.4us , P90: 168.2us , Std Dev: 77.428
# FPS: 5467.33, Avg Frame Time: 182.905us , P99: 605.9us , P95: 230.3us , P90: 177us , Std Dev: 1431.36
# MARK iteration 8 of 10 
# FPS: 6161.59, Avg Frame Time: 162.296us , P99: 559.5us , P95: 224.6us , P90: 169.7us , Std Dev: 129.133
# FPS: 4792.44, Avg Frame Time: 208.662us , P99: 634us , P95: 239.9us , P90: 177us , Std Dev: 1721.82
# MARK iteration 9 of 10 
# FPS: 6219.46, Avg Frame Time: 160.786us , P99: 538.3us , P95: 218.6us , P90: 170.8us , Std Dev: 84.4512
# FPS: 5477.93, Avg Frame Time: 182.551us , P99: 461.6us , P95: 201.9us , P90: 167.2us , Std Dev: 1760.5
# MARK iteration 10 of 10 
# FPS: 6325.25, Avg Frame Time: 158.097us , P99: 497.6us , P95: 202.8us , P90: 166.8us , Std Dev: 74.6138
# FPS: 5485.53, Avg Frame Time: 182.298us , P99: 507.1us , P95: 222.7us , P90: 172.3us , Std Dev: 1535.45
# """,

#     "Simple": """
# MARK iteration 1 of 10
# MeshInstances: 1
# MARK iteration 2 of 10 
# FPS: 6457.28, Avg Frame Time: 154.864us , P99: 451.3us , P95: 193us , P90: 147.5us , Std Dev: 1184.24
# MARK iteration 3 of 10 
# FPS: 6893.16, Avg Frame Time: 145.071us , P99: 543.3us , P95: 210us , P90: 151.9us , Std Dev: 113.83
# FPS: 6224.73, Avg Frame Time: 160.65us , P99: 441.4us , P95: 197.4us , P90: 156.2us , Std Dev: 1364.61
# MARK iteration 4 of 10 
# FPS: 6917.55, Avg Frame Time: 144.56us , P99: 502.9us , P95: 199.2us , P90: 155.8us , Std Dev: 79.6195
# FPS: 6224.76, Avg Frame Time: 160.649us , P99: 403.9us , P95: 185.8us , P90: 155.8us , Std Dev: 1199.96
# MARK iteration 5 of 10 
# FPS: 6890.84, Avg Frame Time: 145.12us , P99: 532.5us , P95: 204.9us , P90: 155.5us , Std Dev: 79.061
# FPS: 6195.94, Avg Frame Time: 161.396us , P99: 532.6us , P95: 215us , P90: 156.2us , Std Dev: 1239.47
# MARK iteration 6 of 10 
# FPS: 6810.62, Avg Frame Time: 146.83us , P99: 495.6us , P95: 204.7us , P90: 156.1us , Std Dev: 104.247
# FPS: 6140.87, Avg Frame Time: 162.843us , P99: 462.4us , P95: 185.9us , P90: 146.5us , Std Dev: 1359.11
# MARK iteration 7 of 10 
# FPS: 6787.15, Avg Frame Time: 147.337us , P99: 473.7us , P95: 204.9us , P90: 156.7us , Std Dev: 83.4791
# FPS: 6305.58, Avg Frame Time: 158.59us , P99: 495.7us , P95: 200.3us , P90: 150.9us , Std Dev: 1391.2
# MARK iteration 8 of 10 
# FPS: 6723.48, Avg Frame Time: 148.733us , P99: 604.6us , P95: 218.6us , P90: 158us , Std Dev: 122.382
# FPS: 6085.45, Avg Frame Time: 164.326us , P99: 504.6us , P95: 216.2us , P90: 161.2us , Std Dev: 1262.45
# MARK iteration 9 of 10 
# FPS: 6752.49, Avg Frame Time: 148.093us , P99: 559.5us , P95: 212.8us , P90: 155.2us , Std Dev: 171.008
# FPS: 5888.35, Avg Frame Time: 169.827us , P99: 461.5us , P95: 197.2us , P90: 150.2us , Std Dev: 1760.61
# MARK iteration 10 of 10 
# FPS: 6856.94, Avg Frame Time: 145.838us , P99: 535.7us , P95: 201.5us , P90: 153.5us , Std Dev: 94.139
# FPS: 6005.67, Avg Frame Time: 166.509us , P99: 563.4us , P95: 221.1us , P90: 159.6us , Std Dev: 1362.04
# """,
# "Lambertian (no normal map)": """
# FPS: 5485.79, Avg Frame Time: 182.289us , P99: 578.2us , P95: 210.5us , P90: 159.9us , Std Dev: 1598.58
# MARK iteration 3 of 10 
# FPS: 7438.69, Avg Frame Time: 134.432us , P99: 312.3us , P95: 158.5us , P90: 140.3us , Std Dev: 58.1195
# FPS: 6619.79, Avg Frame Time: 151.062us , P99: 427.8us , P95: 177.8us , P90: 143us , Std Dev: 1206.18
# MARK iteration 4 of 10 
# FPS: 7137.49, Avg Frame Time: 140.105us , P99: 505.8us , P95: 192.7us , P90: 147.5us , Std Dev: 111.05
# FPS: 6553.76, Avg Frame Time: 152.584us , P99: 503us , P95: 191.9us , P90: 147.9us , Std Dev: 1174.19
# MARK iteration 5 of 10 
# FPS: 7130.82, Avg Frame Time: 140.236us , P99: 496.1us , P95: 200.4us , P90: 153.2us , Std Dev: 134.885
# FPS: 6423.61, Avg Frame Time: 155.676us , P99: 552.2us , P95: 200us , P90: 148us , Std Dev: 1189.55
# MARK iteration 6 of 10 
# FPS: 7003.75, Avg Frame Time: 142.781us , P99: 565.5us , P95: 221.7us , P90: 166.8us , Std Dev: 99.6305
# FPS: 5442.14, Avg Frame Time: 183.751us , P99: 629.8us , P95: 246.2us , P90: 161.8us , Std Dev: 2663.94
# MARK iteration 7 of 10 
# FPS: 6839.17, Avg Frame Time: 146.217us , P99: 543.7us , P95: 201.9us , P90: 154us , Std Dev: 85.3367
# FPS: 6449.5, Avg Frame Time: 155.051us , P99: 478.3us , P95: 199.9us , P90: 149us , Std Dev: 1296.26
# MARK iteration 8 of 10 
# FPS: 6805, Avg Frame Time: 146.951us , P99: 639.9us , P95: 228.9us , P90: 156.5us , Std Dev: 132.434
# FPS: 6326.99, Avg Frame Time: 158.053us , P99: 525.1us , P95: 206us , P90: 149.4us , Std Dev: 1387.31
# MARK iteration 9 of 10 
# FPS: 6986.3, Avg Frame Time: 143.137us , P99: 571.8us , P95: 207.2us , P90: 147.5us , Std Dev: 128.309
# FPS: 6425.53, Avg Frame Time: 155.629us , P99: 552.6us , P95: 204.3us , P90: 146.9us , Std Dev: 1234.83
# MARK iteration 10 of 10 
# FPS: 6834.88, Avg Frame Time: 146.308us , P99: 530.6us , P95: 206.8us , P90: 151.3us , Std Dev: 504.004
# FPS: 5799.44, Avg Frame Time: 172.43us , P99: 601.9us , P95: 369.3us , P90: 185.7us , Std Dev: 1660.51
# """,
# "Simple (no normal map)": """
# FPS: 7054.85, Avg Frame Time: 141.746us , P99: 439.6us , P95: 169.9us , P90: 127.3us , Std Dev: 1712.01
# MARK iteration 3 of 10 
# FPS: 8144.68, Avg Frame Time: 122.78us , P99: 449.8us , P95: 174.2us , P90: 129.9us , Std Dev: 74.0077
# FPS: 7457.63, Avg Frame Time: 134.091us , P99: 390.5us , P95: 160.3us , P90: 129.3us , Std Dev: 1185.31
# MARK iteration 4 of 10 
# FPS: 8195.66, Avg Frame Time: 122.016us , P99: 413.9us , P95: 170.5us , P90: 130.5us , Std Dev: 69.309
# FPS: 7509.77, Avg Frame Time: 133.16us , P99: 387.5us , P95: 159.9us , P90: 128.5us , Std Dev: 1105.22
# MARK iteration 5 of 10 
# FPS: 7857.19, Avg Frame Time: 127.272us , P99: 556.6us , P95: 198.4us , P90: 138.5us , Std Dev: 97.6617
# FPS: 7053.42, Avg Frame Time: 141.775us , P99: 583.7us , P95: 202.1us , P90: 143us , Std Dev: 1170.16
# MARK iteration 6 of 10 
# FPS: 7008.6, Avg Frame Time: 142.682us , P99: 658.2us , P95: 266.5us , P90: 221us , Std Dev: 267.395
# FPS: 7526.87, Avg Frame Time: 132.857us , P99: 395us , P95: 158.5us , P90: 127.5us , Std Dev: 1081.46
# MARK iteration 7 of 10 
# FPS: 7690.79, Avg Frame Time: 130.026us , P99: 427.8us , P95: 167.9us , P90: 128.4us , Std Dev: 626.701
# FPS: 7215.71, Avg Frame Time: 138.586us , P99: 486.2us , P95: 187.7us , P90: 133.5us , Std Dev: 1112.81
# MARK iteration 8 of 10 
# FPS: 7697.38, Avg Frame Time: 129.914us , P99: 553.3us , P95: 188.5us , P90: 135.7us , Std Dev: 328.401
# FPS: 7163.55, Avg Frame Time: 139.596us , P99: 480.9us , P95: 216.9us , P90: 142.3us , Std Dev: 1120.99
# MARK iteration 9 of 10 
# FPS: 8270.97, Avg Frame Time: 120.905us , P99: 376.1us , P95: 150.3us , P90: 125.5us , Std Dev: 61.2574
# FPS: 7117.6, Avg Frame Time: 140.497us , P99: 450.4us , P95: 188.7us , P90: 133.5us , Std Dev: 1410.55
# MARK iteration 10 of 10 
# FPS: 7710.9, Avg Frame Time: 129.686us , P99: 555.4us , P95: 193.2us , P90: 134.3us , Std Dev: 160.496
# FPS: 7202.75, Avg Frame Time: 138.836us , P99: 452.7us , P95: 172.8us , P90: 133.5us , Std Dev: 1142.93
# """,
# "Mirror (no normal map)": """
# MARK iteration 2 of 10 
# FPS: 6983.22, Avg Frame Time: 143.2us , P99: 473.5us , P95: 187.4us , P90: 143.3us , Std Dev: 607.753
# FPS: 6502.65, Avg Frame Time: 153.783us , P99: 509.5us , P95: 194.3us , P90: 144.3us , Std Dev: 1275.91
# MARK iteration 3 of 10 
# FPS: 7405.85, Avg Frame Time: 135.028us , P99: 393.2us , P95: 174.9us , P90: 142.8us , Std Dev: 66.8147
# FPS: 6560.25, Avg Frame Time: 152.433us , P99: 487.3us , P95: 200.4us , P90: 147.2us , Std Dev: 1224.44
# MARK iteration 4 of 10 
# FPS: 7332.63, Avg Frame Time: 136.377us , P99: 464.1us , P95: 186.1us , P90: 142.8us , Std Dev: 75.6069
# FPS: 6682.74, Avg Frame Time: 149.639us , P99: 459.9us , P95: 187us , P90: 145us , Std Dev: 1144.12
# MARK iteration 5 of 10 
# FPS: 6950.98, Avg Frame Time: 143.865us , P99: 605us , P95: 222.9us , P90: 155.3us , Std Dev: 129.141
# FPS: 6527.52, Avg Frame Time: 153.197us , P99: 557.1us , P95: 198.8us , P90: 146.1us , Std Dev: 1235.56
# MARK iteration 6 of 10 
# FPS: 7285.78, Avg Frame Time: 137.254us , P99: 490.4us , P95: 191.2us , P90: 145us , Std Dev: 80.9349
# FPS: 6706.64, Avg Frame Time: 149.106us , P99: 418us , P95: 179.5us , P90: 141.1us , Std Dev: 1167.9
# MARK iteration 7 of 10 
# FPS: 7336.51, Avg Frame Time: 136.305us , P99: 462.8us , P95: 186.8us , P90: 143.9us , Std Dev: 71.6098
# FPS: 5931.42, Avg Frame Time: 168.594us , P99: 596.9us , P95: 220.4us , P90: 155.6us , Std Dev: 1349.62
# MARK iteration 8 of 10 
# FPS: 7099.77, Avg Frame Time: 140.85us , P99: 387.9us , P95: 163us , P90: 139.4us , Std Dev: 617.096
# FPS: 6705.86, Avg Frame Time: 149.123us , P99: 425us , P95: 176.2us , P90: 141.9us , Std Dev: 1155.61
# MARK iteration 9 of 10 
# FPS: 7276.71, Avg Frame Time: 137.425us , P99: 471.8us , P95: 197.6us , P90: 146.4us , Std Dev: 74.5912
# FPS: 6593.82, Avg Frame Time: 151.657us , P99: 466.6us , P95: 194.1us , P90: 146.7us , Std Dev: 1173.6
# MARK iteration 10 of 10 
# FPS: 6963.29, Avg Frame Time: 143.61us , P99: 567.9us , P95: 214.4us , P90: 150.9us , Std Dev: 166.543
# FPS: 6507.86, Avg Frame Time: 153.66us , P99: 502.6us , P95: 202.8us , P90: 147.7us , Std Dev: 1212.9
# """,
# "Environment (no normal map)": """
# FPS: 6119.45, Avg Frame Time: 163.413us , P99: 503us , P95: 198.4us , P90: 144.2us , Std Dev: 1619.4
# MARK iteration 3 of 10 
# FPS: 7702.32, Avg Frame Time: 129.831us , P99: 430us , P95: 172.9us , P90: 138us , Std Dev: 102.789
# FPS: 6824.64, Avg Frame Time: 146.528us , P99: 475.4us , P95: 190.6us , P90: 141.2us , Std Dev: 1310.57
# MARK iteration 4 of 10 
# FPS: 7711.66, Avg Frame Time: 129.674us , P99: 434.4us , P95: 174.4us , P90: 138.1us , Std Dev: 73.7346
# FPS: 6987.9, Avg Frame Time: 143.105us , P99: 464.9us , P95: 180.1us , P90: 137.7us , Std Dev: 1142.79
# MARK iteration 5 of 10 
# FPS: 7521.25, Avg Frame Time: 132.957us , P99: 507.1us , P95: 182.1us , P90: 136.7us , Std Dev: 111.564
# FPS: 7023.78, Avg Frame Time: 142.374us , P99: 425.8us , P95: 169.1us , P90: 135.8us , Std Dev: 1119.34
# MARK iteration 6 of 10 
# FPS: 7620.93, Avg Frame Time: 131.218us , P99: 471.7us , P95: 188.5us , P90: 140.2us , Std Dev: 80.1136
# FPS: 7037.99, Avg Frame Time: 142.086us , P99: 416.7us , P95: 168.6us , P90: 135.7us , Std Dev: 1137.74
# MARK iteration 7 of 10 
# FPS: 7714.99, Avg Frame Time: 129.618us , P99: 435.7us , P95: 176us , P90: 137us , Std Dev: 69.7259
# FPS: 6630.47, Avg Frame Time: 150.819us , P99: 563.3us , P95: 203.1us , P90: 144.1us , Std Dev: 1289.72
# MARK iteration 8 of 10 
# FPS: 7691.83, Avg Frame Time: 130.008us , P99: 467.7us , P95: 178.2us , P90: 137.8us , Std Dev: 73.092
# FPS: 6902.47, Avg Frame Time: 144.876us , P99: 453.5us , P95: 188.1us , P90: 141.8us , Std Dev: 1136.71
# MARK iteration 9 of 10 
# FPS: 7731.9, Avg Frame Time: 129.334us , P99: 424.9us , P95: 173.1us , P90: 136.7us , Std Dev: 66.2172
# FPS: 7008.62, Avg Frame Time: 142.681us , P99: 432.3us , P95: 184.8us , P90: 138.3us , Std Dev: 1118.99
# MARK iteration 10 of 10 
# FPS: 7306.97, Avg Frame Time: 136.856us , P99: 482.7us , P95: 191.3us , P90: 141.4us , Std Dev: 227.961
# FPS: 7023.66, Avg Frame Time: 142.376us , P99: 427.6us , P95: 175.3us , P90: 138.7us , Std Dev: 1113.36
# """,
# "PBR (no normal map)": """
# FPS: 5780.1, Avg Frame Time: 173.008us , P99: 516us , P95: 222.8us , P90: 165.2us , Std Dev: 1297.26
# MARK iteration 3 of 10 
# FPS: 6573.54, Avg Frame Time: 152.125us , P99: 487.6us , P95: 208.5us , P90: 160.8us , Std Dev: 105.42
# FPS: 5883.16, Avg Frame Time: 169.977us , P99: 538.2us , P95: 228.3us , P90: 163.1us , Std Dev: 1233.29
# MARK iteration 4 of 10 
# FPS: 6514.34, Avg Frame Time: 153.508us , P99: 561.1us , P95: 223.7us , P90: 165.4us , Std Dev: 98.4183
# FPS: 5800.43, Avg Frame Time: 172.401us , P99: 544.8us , P95: 222.9us , P90: 164.7us , Std Dev: 1294.17
# MARK iteration 5 of 10 
# FPS: 6609.18, Avg Frame Time: 151.305us , P99: 475.5us , P95: 211.2us , P90: 162.3us , Std Dev: 76.1369
# FPS: 5099.09, Avg Frame Time: 196.113us , P99: 510.1us , P95: 210.5us , P90: 164.6us , Std Dev: 2023.2
# MARK iteration 6 of 10 
# FPS: 6265.38, Avg Frame Time: 159.607us , P99: 620us , P95: 237.1us , P90: 170.9us , Std Dev: 167.098
# FPS: 5840.54, Avg Frame Time: 171.217us , P99: 523.1us , P95: 230.3us , P90: 167.2us , Std Dev: 1260.82
# MARK iteration 7 of 10 
# FPS: 6533.33, Avg Frame Time: 153.061us , P99: 469.9us , P95: 200.7us , P90: 161.8us , Std Dev: 115
# FPS: 5981.9, Avg Frame Time: 167.171us , P99: 491.3us , P95: 209.5us , P90: 162.6us , Std Dev: 1225.72
# MARK iteration 8 of 10 
# FPS: 6527.95, Avg Frame Time: 153.187us , P99: 481.2us , P95: 207.7us , P90: 162us , Std Dev: 73.5691
# FPS: 5767.73, Avg Frame Time: 173.379us , P99: 538.8us , P95: 256us , P90: 172.7us , Std Dev: 1309.63
# MARK iteration 9 of 10 
# FPS: 6504.48, Avg Frame Time: 153.74us , P99: 504.1us , P95: 224.7us , P90: 164.9us , Std Dev: 88.9841
# FPS: 5970.94, Avg Frame Time: 167.478us , P99: 476.2us , P95: 216.2us , P90: 162.7us , Std Dev: 1223.76
# MARK iteration 10 of 10 
# FPS: 6601.83, Avg Frame Time: 151.473us , P99: 492.2us , P95: 206.9us , P90: 160.4us , Std Dev: 75.9051
# FPS: 5958.41, Avg Frame Time: 167.83us , P99: 438.2us , P95: 193.2us , P90: 161.1us , Std Dev: 1236.87
# """
    
    "Low-res with Normal Map": """
FPS: 5870.8, Avg Frame Time: 170.334us , P99: 495.7us , P95: 223.3us , P90: 172.6us , Std Dev: 726.344
FPS: 5348.97, Avg Frame Time: 186.952us , P99: 539.9us , P95: 216.4us , P90: 167.7us , Std Dev: 1915.02
MARK iteration 5 of 10 
FPS: 6274.69, Avg Frame Time: 159.371us , P99: 504.8us , P95: 217.9us , P90: 167.8us , Std Dev: 110.017
FPS: 5570.97, Avg Frame Time: 179.502us , P99: 597.2us , P95: 229.7us , P90: 172.3us , Std Dev: 1317.93
MARK iteration 6 of 10 
FPS: 6278.93, Avg Frame Time: 159.263us , P99: 491us , P95: 204.4us , P90: 165.8us , Std Dev: 107.744
FPS: 5676.33, Avg Frame Time: 176.17us , P99: 458.3us , P95: 205.8us , P90: 166us , Std Dev: 1389.19
MARK iteration 7 of 10 
FPS: 6336.27, Avg Frame Time: 157.822us , P99: 479.8us , P95: 207.7us , P90: 165.4us , Std Dev: 78.9138
FPS: 5371.79, Avg Frame Time: 186.158us , P99: 667.5us , P95: 247us , P90: 175.4us , Std Dev: 1313.35
MARK iteration 8 of 10 
FPS: 6189.61, Avg Frame Time: 161.561us , P99: 544.1us , P95: 226.4us , P90: 171.7us , Std Dev: 101.25
FPS: 5737.19, Avg Frame Time: 174.301us , P99: 484.8us , P95: 215.8us , P90: 166.4us , Std Dev: 1247.02
MARK iteration 9 of 10 
FPS: 6299.92, Avg Frame Time: 158.732us , P99: 505.1us , P95: 220.5us , P90: 167us , Std Dev: 76.9309
FPS: 5629.63, Avg Frame Time: 177.632us , P99: 485.4us , P95: 217.6us , P90: 168.3us , Std Dev: 1497.12
MARK iteration 10 of 10 
FPS: 6168.98, Avg Frame Time: 162.101us , P99: 526.8us , P95: 236.2us , P90: 176.3us , Std Dev: 107.22
FPS: 5506.36, Avg Frame Time: 181.608us , P99: 546.2us , P95: 235.2us , P90: 175.4us , Std Dev: 1490.48
""",
    "HIgh-res without Normal Map": """
    FPS: 3836.49, Avg Frame Time: 260.655us , P99: 590.3us , P95: 285.6us , P90: 260us , Std Dev: 956.473
FPS: 3709.44, Avg Frame Time: 269.582us , P99: 564.8us , P95: 314.7us , P90: 266.1us , Std Dev: 1561.37
MARK iteration 5 of 10 
FPS: 3889.68, Avg Frame Time: 257.091us , P99: 696.7us , P95: 351.5us , P90: 272.7us , Std Dev: 191.353
FPS: 3680.16, Avg Frame Time: 271.727us , P99: 699.7us , P95: 360.7us , P90: 274.3us , Std Dev: 1565.29
MARK iteration 6 of 10 
FPS: 4107.13, Avg Frame Time: 243.479us , P99: 598.6us , P95: 307.4us , P90: 264.9us , Std Dev: 85.4755
FPS: 3595.81, Avg Frame Time: 278.101us , P99: 623us , P95: 318.1us , P90: 269.9us , Std Dev: 1600.21
MARK iteration 7 of 10 
FPS: 4017.3, Avg Frame Time: 248.923us , P99: 602us , P95: 317us , P90: 270us , Std Dev: 88.6977
FPS: 3649.46, Avg Frame Time: 274.013us , P99: 625.6us , P95: 325.9us , P90: 270.3us , Std Dev: 1581.11
MARK iteration 8 of 10 
FPS: 3989.36, Avg Frame Time: 250.666us , P99: 684.7us , P95: 360.4us , P90: 270.5us , Std Dev: 153.027
FPS: 3536.26, Avg Frame Time: 282.784us , P99: 636.9us , P95: 323.5us , P90: 274.1us , Std Dev: 1717.79
MARK iteration 9 of 10 
FPS: 3933.54, Avg Frame Time: 254.224us , P99: 701.2us , P95: 326.2us , P90: 273.4us , Std Dev: 174.262
FPS: 3764.76, Avg Frame Time: 265.621us , P99: 564.9us , P95: 297.7us , P90: 263.1us , Std Dev: 1540.01
MARK iteration 10 of 10 
FPS: 4019.53, Avg Frame Time: 248.785us , P99: 626.1us , P95: 315.6us , P90: 267.3us , Std Dev: 95.3709
FPS: 3382.44, Avg Frame Time: 295.645us , P99: 664.2us , P95: 352.2us , P90: 266.5us , Std Dev: 2988.82
"""
}
# Example usage with 3 data series and corresponding labels
data_series = [value for key, value in raw_data.items()]
labels = [key for key, value in raw_data.items()]

# Call the function with the data series and labels
analyze_and_plot_fps_data(data_series, labels)
