import os
import re
import pandas as pd
import matplotlib.pyplot as plt

# Define the list of MPI processes
mpi_values = [1, 2, 4, 8, 16, 32, 64, 128]
directory = "run"
data = []

# Parse the files
for i in mpi_values:
    filepath = os.path.join(directory, f"out_{i}")

    if not os.path.exists(filepath):
        print(f"Warning: File {filepath} does not exist. Skipping.")
        continue

    with open(filepath, "r") as f:
        content = f.read()

    # Extract metrics using regular expressions (handling potential typos like 'intiialization')
    norm_match = re.search(r"Norm of the output vector is\s*:\s*([-\d.]+)", content)
    init_match = re.search(r"Average .*inti{0,2}ialization time per rank:\s*([-\d.]+)", content, re.IGNORECASE)
    calc_match = re.search(r"Average calculation time per rank\s*:\s*([-\d.]+)", content)
    red_match = re.search(r"Average reduction time per rank\s*:\s*([-\d.]+)", content)
    total_match = re.search(r"Average Total time per rank\s*:\s*([-\d.]+)", content)

    data.append({
        "Processes": i,
        "Norm": float(norm_match.group(1)) if norm_match else None,
        "Initialization": float(init_match.group(1)) if init_match else None,
        "Calculation": float(calc_match.group(1)) if calc_match else None,
        "Reduction": float(red_match.group(1)) if red_match else None,
        "Total": float(total_match.group(1)) if total_match else None
    })

# Convert data into a DataFrame for easy handling
df = pd.DataFrame(data)

if df.empty:
    print("No data was successfully parsed. Please check your directory path and file names.")
else:
    print("--- Parsed Data Summary ---")
    print(df.to_string(index=False))

    # Plotting the results
    plt.figure(figsize=(10, 6))

    plt.plot(df["Processes"], df["Initialization"], marker='o', label='Initialization Time')
    plt.plot(df["Processes"], df["Calculation"], marker='s', label='Calculation Time')
    plt.plot(df["Processes"], df["Reduction"], marker='^', label='Reduction Time')
    plt.plot(df["Processes"], df["Total"], marker='d', linestyle='--', color='black', label='Total Time')

    # Use log scale for x-axis since processes double each step (1, 2, 4, 8...)
    plt.xscale('log', base=2)
    plt.xticks(df["Processes"], df["Processes"]) # Show explicit tick marks for each value

    plt.xlabel('Number of MPI Processes per Node ($i$)')
    plt.ylabel('Time (seconds)')
    plt.title('MPI Performance Analysis per Rank')
    plt.legend()
    plt.grid(True, which="both", ls="--", alpha=0.7)

    # Save and display the plot
    plt.tight_layout()
    plt.savefig('mpi_performance_plot.png', dpi=300)
    print("\nPlot successfully saved as 'mpi_performance_plot.png'.")
    plt.show()