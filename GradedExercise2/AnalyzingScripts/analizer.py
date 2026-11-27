import os
import re
import pandas as pd
import matplotlib.pyplot as plt

# Define the list of MPI processes
mpi_values = [1, 2, 4, 8, 16, 32, 64, 128]
directory = "../RunClean"
data = []


def extract_float(pattern, text, flags=0):
    match = re.search(pattern, text, flags)
    return float(match.group(1)) if match else None


def plot_series(ax, frame):
    ax.plot(frame["Processes"], frame["Initialization"], marker="o", label="Initialization Time")
    ax.plot(frame["Processes"], frame["Computation"], marker="s", label="Computation Time")
    ax.plot(frame["Processes"], frame["Communication"], marker="D", label="Communication Time")
    ax.plot(
        frame["Processes"],
        frame["Comp_Plus_Comm"],
        marker="P",
        linestyle="-.",
        label="Computation + Communication Time",
    )
    ax.plot(frame["Processes"], frame["Reduction"], marker="^", label="Reduction Time")
    ax.plot(
        frame["Processes"],
        frame["Total"],
        marker="d",
        linestyle="--",
        color="black",
        label="Total Time",
    )
    ax.set_xscale("log", base=2)
    ax.set_xticks(frame["Processes"])
    ax.set_xticklabels(frame["Processes"])
    ax.set_xlabel("Number of MPI Processes per Node")
    ax.grid(True, which="both", ls="--", alpha=0.7)


# Parse the files
for i in mpi_values:
    filepath = os.path.join(directory, f"out_{i}")

    if not os.path.exists(filepath):
        print(f"Warning: File {filepath} does not exist. Skipping.")
        continue

    with open(filepath, "r") as f:
        content = f.read()

    start_temp = extract_float(
        r"Start Average Temperature\s*=\s*([+-]?\d+(?:\.\d+)?(?:[eE][+-]?\d+)?)f?",
        content,
    )
    finish_temp = extract_float(
        r"Finish Average Temperature\s*=\s*([+-]?\d+(?:\.\d+)?(?:[eE][+-]?\d+)?)f?",
        content,
    )

    init_val = extract_float(
        r"Average .*inti{0,2}ialization time per rank\s*:\s*([+-]?\d+(?:\.\d+)?(?:[eE][+-]?\d+)?)",
        content,
        re.IGNORECASE,
    )
    comp_val = extract_float(
        r"Average computation time per rank\s*:\s*([+-]?\d+(?:\.\d+)?(?:[eE][+-]?\d+)?)",
        content,
    )
    comm_val = extract_float(
        r"Average communication time per rank\s*:\s*([+-]?\d+(?:\.\d+)?(?:[eE][+-]?\d+)?)",
        content,
    )
    comp_comm_val = extract_float(
        r"Average computation\s*\+\s*communication time per rank\s*:\s*([+-]?\d+(?:\.\d+)?(?:[eE][+-]?\d+)?)",
        content,
    )
    red_val = extract_float(
        r"Average reduction time per rank\s*:\s*([+-]?\d+(?:\.\d+)?(?:[eE][+-]?\d+)?)",
        content,
    )
    total_val = extract_float(
        r"Average Total time per rank\s*:\s*([+-]?\d+(?:\.\d+)?(?:[eE][+-]?\d+)?)",
        content,
    )

    data.append({
        "Processes": i,
        "Start_Temp": start_temp,
        "Finish_Temp": finish_temp,
        "Initialization": init_val,
        "Computation": comp_val,
        "Communication": comm_val,
        "Comp_Plus_Comm": comp_comm_val,
        "Reduction": red_val,
        "Total": total_val,
    })

df = pd.DataFrame(data)

if df.empty:
    print("No data was successfully parsed. Please check your directory path and file names.")
else:
    print("--- Parsed Data Summary ---")
    print(df.to_string(index=False))

    fig, (ax_lin, ax_log) = plt.subplots(1, 2, figsize=(16, 6.5))

    # Left: Linear Y-axis
    plot_series(ax_lin, df)
    ax_lin.set_ylabel("Time (seconds, linear scale)")
    ax_lin.set_title("Linear Scale")
    ax_lin.legend(loc="upper right")


    # Right: Log Y-axis to highlight relative differences across orders of magnitude
    plot_series(ax_log, df)
    ax_log.set_yscale("log")
    ax_log.set_ylabel("Time (seconds, log scale)")
    ax_log.set_title("Log Y-Scale")
    ax_log.legend(loc="upper right")

    plt.tight_layout()
    comparison_path = os.path.join(directory, "mpi_performance_comparison.png")
    plt.savefig(comparison_path, dpi=300)
    print(f"\nComparison plot successfully saved as '{comparison_path}'.")
    plt.show()