import tkinter as tk
from tkinter import scrolledtext, messagebox
import subprocess
import os

# Create the GUI window
window = tk.Tk()
window.title("Execution Metrics Viewer")

# Set window size
window.geometry("600x400")

# Add a label
label = tk.Label(window, text="Branch Prediction and Execution Metrics", font=("Arial", 14))
label.pack(pady=10)

# Create a scrolled text area to display the content from the output.txt file
output_text = scrolledtext.ScrolledText(window, width=80, height=20, wrap=tk.WORD)
output_text.pack(pady=10)

# Function to read and display the content of the output file
def display_output():
    try:
        with open("output.txt", "r") as file:
            content = file.read()
            output_text.insert(tk.END, content)
    except FileNotFoundError:
        output_text.insert(tk.END, "Error: output.txt file not found.")

# Function to run the C++ program and generate the output.txt file
def run_simulation():
    # Compile the C++ code
    try:
        # Run the C++ program using subprocess (assumes src.cpp is in the same folder)
        compile_process = subprocess.run(["g++", "src.cpp", "-o", "src.out"], capture_output=True, text=True)
        
        # Check for compilation errors
        if compile_process.returncode != 0:
            messagebox.showerror("Compilation Error", compile_process.stderr)
            return

        # Run the compiled executable to generate output.txt
        run_process = subprocess.run(["./src.out"], capture_output=True, text=True)
        
        # Check if the execution was successful
        if run_process.returncode == 0:
            display_output()  # Display the output in the GUI
        else:
            messagebox.showerror("Runtime Error", run_process.stderr)
        
    except FileNotFoundError:
        messagebox.showerror("File Not Found", "The required files (src.cpp or output.txt) are missing.")
    except Exception as e:
        messagebox.showerror("Error", f"An error occurred: {str(e)}")

# Add a Start Simulation button
start_button = tk.Button(window, text="Start Simulation", font=("Arial", 12), command=run_simulation)
start_button.pack(pady=10)

# Run the Tkinter event loop
window.mainloop()
