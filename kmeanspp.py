import sys
import numpy as np

# Helper function to validate integer-like input (e.g., "3", "03", "3.0")
def validate_integer_like_input(s_val):
    try:
        f_val = float(s_val)
        if f_val.is_integer(): # Checks if the float is a whole number (e.g., 3.0, 4.0)
            return int(f_val)
        else:
            return None # It's a float with a non-zero fractional part (e.g., 3.4)
    except ValueError:
        return None # Not a valid number at all (e.g., "abc")

# data file
def read_data_from_files(input_1, input_2):
    datapoints = []
    for line in sys.stdin:  # Reads line by line from stdin
        line = line.strip()  # Remove leading/trailing whitespace, including newline
        if not line:  # Skip empty lines
            continue
        try:
            vals = line.split(',')
            vector = [float(x) for x in vals]
            datapoints.append(vector)
        except ValueError:
            print("An Error Has Occurred") # Specific error for malformed data
            sys.exit(1)
    return datapoints


# K value
def get_k(number_of_vectors, k_user_input_str): # Renamed input to clarify it's a string
    k_val = validate_integer_like_input(k_user_input_str)

    if k_val is not None:
        # In K-means, K must be at least 1 (or 2, depending on interpretation)
        # and less than the number of vectors.
        # The original check was > 1. Let's keep that.
        if 1 < k_val < number_of_vectors:
            return k_val

    # If validation failed or conditions not met
    print("Incorrect number of clusters!") # Output to stderr
    sys.exit(1) # Exit with error code 1

# iteration num
def get_iteration_num(iter_user_input_str, default_iter=300): # Renamed input and default
    # If iter_user_input_str is empty (meaning iter was not provided by user), return default
    if not iter_user_input_str: 
        return default_iter

    iter_val = validate_integer_like_input(iter_user_input_str)

    if iter_val is not None:
        # Iterations must be > 1 and < 1000
        if 1 < iter_val < 1000:
            return iter_val

    print("Incorrect maximum iteration!") 
    sys.exit(1) # Exit with error code 1

def get_eps(eps_user_input_str):
    try:
        eps = float(eps_user_input_str)
        if eps >= 0:
            return eps
        else:
            print("Invalid epsilon!")
            sys.exit(1) # Exit with error code 1

    except (ValueError, TypeError):
        print("Invalid epsilon!")
        sys.exit(1) # Exit with error code 1



def main():
    datapoints =[]
    k_user = sys.argv[1]
    ITERATION_NUM = 300
    EPSILON = 0
    # get user's info 
    # if no iteration number was entered:
    if len(sys.argv) == 6:
        iteration_number_user = sys.argv[2]
        ITERATION_NUM = get_iteration_num(iteration_number_user)
        eps_number_user = sys.argv[3]
        EPSILON = get_eps(eps_number_user)
        datapoints = read_data_from_files(sys.argv[4], sys.argv[5])
    
    else: # No iteration value was sent by the user
        eps_number_user = sys.argv[2]
        EPSILON = get_eps(eps_number_user)
        datapoints = read_data_from_files(sys.argv[3], sys.argv[4])

    if not datapoints: # Should have been caught by read_data_from_stdin, but double check
        print("An Error Has Occurred")
        sys.exit(1)

    d = len(datapoints[0][1])
    n = len(datapoints)
    k = get_k(n, k_user)
    clusters = [[[], [], []] for i in range(k)]
    vec_new_position_array = n*[0]
    vec_prev_position_array = n*[None]
    best_cluster_index = 0
    new_cluster_index = 0
    prev_cluster_index = 0

