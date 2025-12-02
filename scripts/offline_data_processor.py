import os
import csv
import glob

def classify_csv_files_by_hash_streaming(root_dir, output_dir="classified_output"):
    """
    Traverses all subdirectories under 'root_dir' to find and process CSV files.
    The process is memory-efficient (streaming): it reads and writes data line by line,
    making it suitable for very large datasets.

    Data is classified based on the parity (even/odd) of the last digit of the 'HASH' field:
    - Even parity (0, 2, 4, 6, 8, c, e) -> shard_0_data.csv
    - Odd parity (1, 3, 5, 7, 9, b, d, f) -> shard_1_data.csv

    Args:
        root_dir (str): The root directory containing subfolders and CSV files.
        output_dir (str): The directory where the classified output files will be saved.
    """

    # 1. Ensure the output directory exists
    if not os.path.exists(output_dir):
        os.makedirs(output_dir)
        print(f"Created output directory: {output_dir}")

    # 2. Define file paths and expected header fields
    output_0_path = os.path.join(output_dir, "shard_0_data.csv")
    output_1_path = os.path.join(output_dir, "shard_1_data.csv")
    FIELDNAMES = ['LON', 'LAT', 'NUMBER', 'STREET', 'UNIT', 'CITY', 'DISTRICT', 'REGION', 'POSTCODE', 'ID', 'HASH']

    # 3. Streaming Write Setup: Open output files once
    try:
        with open(output_0_path, 'w', newline='', encoding='utf-8') as outfile_0, \
             open(output_1_path, 'w', newline='', encoding='utf-8') as outfile_1:

            # Create DictWriter objects for easy dictionary-based writing
            writer_0 = csv.DictWriter(outfile_0, fieldnames=FIELDNAMES)
            writer_1 = csv.DictWriter(outfile_1, fieldnames=FIELDNAMES)

            # Write the header to both output files
            writer_0.writeheader()
            writer_1.writeheader()

            print(f"Starting to process directory: {root_dir}...")

            # Use glob to recursively find all CSV files in all subdirectories
            csv_files = glob.glob(os.path.join(root_dir, '**', '*.csv'), recursive=True)

            if not csv_files:
                print("No CSV files found. Please check the root directory path.")
                return

            total_files = len(csv_files)
            processed_rows = 0

            # 4. Streaming Read: Process files one by one, line by line
            for i, file_path in enumerate(csv_files):
                print(f"[{i+1}/{total_files}] Processing file: {file_path}")

                try:
                    # Open input file for reading
                    with open(file_path, 'r', newline='', encoding='utf-8') as infile:
                        # DictReader loads data as dictionaries {header: value}
                        reader = csv.DictReader(infile)

                        # Header validation
                        if reader.fieldnames != FIELDNAMES:
                            print(f"Warning: Header in file {file_path} does not match expected fields. Skipping.")
                            continue

                        # The reader acts as an iterator, yielding one row (dictionary) at a time.
                        # This prevents the entire file content from being loaded into memory.
                        for row in reader:
                            hash_value = row.get('HASH', '')
                            if not hash_value:
                                continue

                            last_char = hash_value[-1].lower() # Get the last character and normalize case

                            # Determine parity by converting the hex character to its decimal value
                            try:
                                # int(string, base=16) handles '0'-'9' and 'a'-'f'
                                decimal_value = int(last_char, 16)
                                is_even = (decimal_value % 2 == 0)
                            except ValueError:
                                # Skip row if the character is not a valid hexadecimal digit
                                continue

                            # 5. Streaming Write: Write the current row immediately
                            if is_even:
                                writer_0.writerow(row) # Even hash goes to file_a
                            else:
                                writer_1.writerow(row) # Odd hash goes to file_b

                            processed_rows += 1

                except Exception as e:
                    print(f"An error occurred while processing file {file_path}: {e}")

    except IOError as e:
        print(f"Could not open or write to output file(s): {e}")
        return

    print("\n--- Task Complete ---")
    print(f"Total rows written: {processed_rows}.")
    print(f"Results saved to directory: {os.path.abspath(output_dir)}")


# ----------------- Example Usage / Execution -----------------

# IMPORTANT: Replace 'path/to/your/root_folder' with the actual path to your data.
# ROOT_FOLDER_PATH = 'path/to/your/root_folder'
# classify_csv_files_by_hash_streaming(ROOT_FOLDER_PATH, output_dir='output_results')
