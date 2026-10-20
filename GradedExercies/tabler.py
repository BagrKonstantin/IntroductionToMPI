import os
import re
import csv

# Define the list of MPI processes and file paths
mpi_values = [1, 2, 4, 8, 16, 32, 64, 128]
directory = "run"
csv_filename = "mpi_results.csv"

# Define CSV columns
headers = [
    "Processes",
    "Output_Norm",
    "Initialization_Time",
    "Calculation_Time",
    "Reduction_Time",
    "Total_Time"
]

rows = []

# Parse the files
for i in mpi_values:
    filepath = os.path.join(directory, f"out_{i}")

    if not os.path.exists(filepath):
        rows.append([i, "N/A", "N/A", "N/A", "N/A", "N/A"])
        continue

    with open(filepath, "r") as f:
        content = f.read()

    # Extract metrics using regular expressions
    norm_match = re.search(r"Norm of the output vector is\s*:\s*([-\d.]+)", content)
    init_match = re.search(r"Average .*inti{0,2}ialization time per rank:\s*([-\d.]+)", content, re.IGNORECASE)
    calc_match = re.search(r"Average calculation time per rank\s*:\s*([-\d.]+)", content)
    red_match = re.search(r"Average reduction time per rank\s*:\s*([-\d.]+)", content)
    total_match = re.search(r"Average Total time per rank\s*:\s*([-\d.]+)", content)

    norm = float(norm_match.group(1)) if norm_match else "N/A"
    init = float(init_match.group(1)) if init_match else "N/A"
    calc = float(calc_match.group(1)) if calc_match else "N/A"
    red = float(red_match.group(1)) if red_match else "N/A"
    total = float(total_match.group(1)) if total_match else "N/A"

    rows.append([i, norm, init, calc, red, total])

# Write data to a CSV file
with open(csv_filename, mode="w", newline="") as csv_file:
    writer = csv.writer(csv_file)
    writer.writerow(headers)
    writer.writerows(rows)

print(f"CSV file successfully created: '{csv_filename}'\n")

# Print the CSV contents to the console
with open(csv_filename, mode="r") as csv_file:
    print(csv_file.read())