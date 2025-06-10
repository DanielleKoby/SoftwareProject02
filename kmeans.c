#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h> /* Required for strtok */
#include <float.h> /* For DBL_MAX */

/* Define a small epsilon for convergence check */
#define EPSILON 0.001

/* Function to handle errors and exit */
void error_and_exit(const char* message) {
    fprintf(stderr, "%s\n", message);
    exit(1);
}
int validate_integer_input(const char* s, int* out_val) {
    char* endptr;
    double val_double;
    double int_part;
    double frac_part;
    const char *start_num = s; /* To skip leading whitespace */

    if (s == NULL) return 0;

    /* Skip leading whitespace */
    while (*start_num != '\0' && (*start_num == ' ' || *start_num == '\t' || *start_num == '\n' || *start_num == '\v' || *start_num == '\f' || *start_num == '\r')) {
        start_num++;
    }

    if (*start_num == '\0') return 0; /* String was empty or all whitespace */

    val_double = strtod(start_num, &endptr);

    if (endptr == start_num) {
        /* No conversion took place, e.g., input was "abc" or non-numeric */
        return 0;
    }

    /* Check for any non-whitespace characters after the parsed number */
    while (*endptr != '\0') {
        if (!(*endptr == ' ' || *endptr == '\t' || *endptr == '\n' || *endptr == '\v' || *endptr == '\f' || *endptr == '\r')) {
            return 0; /* Found non-whitespace trailing characters */
        }
        endptr++;
    }

    /* Check if the number has a non-zero fractional part */
    frac_part = modf(val_double, &int_part);

    if (frac_part != 0.0) {
        /* Example: 3.4 -> frac_part is 0.4. This is an error.
           Example: 3.0 -> frac_part is 0.0. This is OK.
           Example: 3   -> val_double is 3.0, frac_part is 0.0. This is OK. */
        return 0;
    }

    /* Check if the integer part fits within standard int range.
       Using double comparison for INT_MIN/MAX boundaries.
       INT_MIN is -2147483648, INT_MAX is 2147483647 for typical 32-bit int.
    */
    if (int_part < -2147483648.0 || int_part > 2147483647.0) {
        return 0;
    }

    *out_val = (int)int_part;
    return 1;
}

/* Function to calculate Euclidean distance between two vectors */
double calculate_distance(double* p1, double* p2, int d) {
    double dist = 0.0;
    int i;
    for (i = 0; i < d; i++) {
        dist += pow(p1[i] - p2[i], 2);
    }
    return sqrt(dist);
}

/* Function to read data points from stdin
 * Returns 0 on success, 1 on error */
int read_datapoints(double*** datapoints, int* n, int* d) {
    char line[4096]; /* Assuming max line length for a data point */
    char line_copy[4096]; /* Added for strtok operations to preserve original line */
    int current_n = 0;
    int current_d = 0;
    int first_line = 1;
    int capacity = 100;
    char* token;
    char* rest;
    double* vector = NULL;
    int count_d;
    int temp_count_d;
    int i; /* Loop variable for freeing memory */

    /* Allocate initial memory for datapoints (will reallocate as needed) */
    *datapoints = (double**)malloc(sizeof(double*) * capacity);
    if (*datapoints == NULL) {
        error_and_exit("An Error Has Occurred");
    }

    while (fgets(line, sizeof(line), stdin) != NULL) {
        /* Skip empty lines */
        if (line[0] == '\n' || line[0] == '\r') {
            continue;
        }

        /* Make a copy of the line for strtok operations, as strtok modifies the string */
        strcpy(line_copy, line);

        count_d = 0;
        rest = line_copy; /* Use the copy for strtok */

        /* Count dimensions for the first data point */
        if (first_line) {
            token = strtok(rest, ",");
            while (token != NULL) {
                count_d++;
                token = strtok(NULL, ",");
            }
            current_d = count_d;
            *d = current_d;
            first_line = 0;
        } else {
            /* Validate dimensions for subsequent data points */
            temp_count_d = 0;
            token = strtok(rest, ","); /* Use the copy for strtok */
            while (token != NULL) {
                temp_count_d++;
                token = strtok(NULL, ",");
            }
            if (temp_count_d != current_d) {
                /* Free all previously allocated datapoints before exiting */
                for (i = 0; i < current_n; i++) {
                    free((*datapoints)[i]);
                }
                free(*datapoints);
                error_and_exit("An Error Has Occurred"); /* Inconsistent dimensions */
            }
        }

        vector = (double*)malloc(sizeof(double) * current_d);
        if (vector == NULL) {
            /* Free all previously allocated datapoints before exiting */
            for (i = 0; i < current_n; i++) {
                free((*datapoints)[i]);
            }
            free(*datapoints);
            error_and_exit("An Error Has Occurred");
        }

        count_d = 0;
        /* Re-copy the original line for actual parsing since strtok modified line_copy in dimension counting */
        strcpy(line_copy, line);
        rest = line_copy;

        token = strtok(rest, ",");
        while (token != NULL) {
            char* endptr;
            vector[count_d] = strtod(token, &endptr);
            /* Check if strtod consumed the whole token and it's a valid number */
            if (*endptr != '\0' && *endptr != '\n' && *endptr != '\r') {
                /* Handle non-numeric characters in input */
                free(vector);
                /* Free all previously allocated datapoints before exiting */
                for (i = 0; i < current_n; i++) {
                    free((*datapoints)[i]);
                }
                free(*datapoints);
                error_and_exit("An Error Has Occurred");
            }
            count_d++;
            token = strtok(NULL, ",");
        }

        if (current_n == capacity) {
            capacity *= 2;
            *datapoints = (double**)realloc(*datapoints, sizeof(double*) * capacity);
            if (*datapoints == NULL) {
                free(vector);
                /* Free all previously allocated datapoints before exiting */
                for (i = 0; i < current_n; i++) {
                    free((*datapoints)[i]);
                }
                free(*datapoints);
                error_and_exit("An Error Has Occurred");
            }
        }
        (*datapoints)[current_n++] = vector;
    }
    *n = current_n;

    if (*n == 0) {
        /* Free the initially allocated *datapoints if no data is read */
        free(*datapoints);
        error_and_exit("An Error Has Occurred"); /* No data points read */
    }

    /* Shrink to fit actual number of datapoints */
    *datapoints = (double**)realloc(*datapoints, sizeof(double*) * (*n));
    if (*datapoints == NULL) {
        /* Free all previously allocated datapoints before exiting */
        for (i = 0; i < current_n; i++) {
            free((*datapoints)[i]);
        }
        free(*datapoints);
        error_and_exit("An Error Has Occurred");
    }

    return 0;
}

int main(int argc, char* argv[]) {
    int k;
    int iter; /* Default value for iter */
    double** datapoints = NULL;
    int n, d; /* n = number of data points, d = dimension of data points */
    int k_val_parsed;
    int iter_val_parsed;
    double** centroids = NULL;
    int i, j; /* Loop variables */
    int* assignments = NULL;
    int iter_count;
    int changed_assignments;
    double min_dist;
    int closest_cluster_idx;
    double** new_centroids_sum = NULL;
    int* cluster_sizes = NULL;
    int l; /* Loop variable for freeing memory in error paths */
    int converged;
    double* old_centroid;
    double* new_centroid_val = NULL;


    /* 1. Parse command line arguments */
    if (argc < 2 || argc > 3) {
        error_and_exit("An Error Has Occurred");
    }

    /* Parse K */
    if (!validate_integer_input(argv[1], &k_val_parsed)) {
        /* This handles cases where argv[1] is not in a valid integer format
        (e.g., "3.4", "abc", "3.A") according to the new rules. */
        error_and_exit("Incorrect number of clusters!"); /* Using your original error message for K issues */
    }
    k = k_val_parsed;
    /* After successful parsing, check the semantic validity of K's value. */
    if (k <= 1) {
        error_and_exit("Incorrect number of clusters!");
    }

    /* Parse iterations (optional argument with default) */
    if (argc == 3) {
        if (!validate_integer_input(argv[2], &iter_val_parsed)) {
            /* This handles cases where argv[2] is not in a valid integer format. */
            error_and_exit("Incorrect maximum iteration!"); /* Using your original error message for iter issues */
        }
        iter = iter_val_parsed;
        /* After successful parsing, check the semantic validity of iter's value. */
        if (iter <= 1 || iter >= 1000) {
            error_and_exit("Incorrect maximum iteration!");
        }
    } else { /* argc == 2, iter is not provided, use default */
        iter = 400;
    }

    /* 2. Read data points from stdin */
    if (read_datapoints(&datapoints, &n, &d) != 0) {
        /* Error already handled and program exited by read_datapoints */
        return 1;
    }

    /* Validate K against N */
    if (k >= n) {
        /* Free datapoints before exiting */
        for (i = 0; i < n; i++) {
            free(datapoints[i]);
        }
        free(datapoints);
        error_and_exit("Incorrect number of clusters!");
    }

    /* 3. Initialize centroids */
    centroids = (double**)malloc(sizeof(double*) * k);
    if (centroids == NULL) {
        /* Free datapoints before exiting */
        for (i = 0; i < n; i++) {
            free(datapoints[i]);
        }
        free(datapoints);
        error_and_exit("An Error Has Occurred");
    }

    for (i = 0; i < k; i++) {
        centroids[i] = (double*)malloc(sizeof(double) * d);
        if (centroids[i] == NULL) {
            /* Free previously allocated centroids and all datapoints */
            for (l = 0; l < i; l++) {
                free(centroids[l]);
            }
            free(centroids);
            for (l = 0; l < n; l++) {
                free(datapoints[l]);
            }
            free(datapoints);
            error_and_exit("An Error Has Occurred");
        }
        /* Initialize centroids with the first k data points */
        for (j = 0; j < d; j++) {
            centroids[i][j] = datapoints[i][j];
        }
    }

    /* Array to store cluster assignment for each data point */
    assignments = (int*)malloc(sizeof(int) * n);
    if (assignments == NULL) {
        /* Free all allocated memory before exiting */
        for (i = 0; i < k; i++) {
            free(centroids[i]);
        }
        free(centroids);
        for (i = 0; i < n; i++) {
            free(datapoints[i]);
        }
        free(datapoints);
        error_and_exit("An Error Has Occurred");
    }

    /* K-means algorithm main loop */
    for (iter_count = 0; iter_count < iter; iter_count++) {
        changed_assignments = 0;

        /* Assign each data point to the closest centroid */
        for (i = 0; i < n; i++) {
            min_dist = DBL_MAX;
            closest_cluster_idx = -1;

            for (j = 0; j < k; j++) {
                double dist = calculate_distance(datapoints[i], centroids[j], d);
                if (dist < min_dist) {
                    min_dist = dist;
                    closest_cluster_idx = j;
                }
            }

            /* Initialize assignments on the first iteration, otherwise check for changes */
            if (iter_count == 0 || assignments[i] != closest_cluster_idx) {
                assignments[i] = closest_cluster_idx;
                changed_assignments = 1;
            }
        }

        /* If no assignments changed, the algorithm has converged by assignment stability */
        if (!changed_assignments && iter_count > 0) { /* Only converge after at least one iteration */
            break;
        }

        /* Update centroids */
        new_centroids_sum = (double**)calloc(k, sizeof(double*));
        cluster_sizes = (int*)calloc(k, sizeof(int));

        if (new_centroids_sum == NULL || cluster_sizes == NULL) {
            /* Free all allocated memory before exiting */
            if (new_centroids_sum) {
                for (i = 0; i < k; i++) {
                    if (new_centroids_sum[i]) free(new_centroids_sum[i]);
                }
                free(new_centroids_sum);
            }
            if (cluster_sizes) free(cluster_sizes);

            for (i = 0; i < k; i++) {
                free(centroids[i]);
            }
            free(centroids);
            free(assignments);
            for (i = 0; i < n; i++) {
                free(datapoints[i]);
            }
            free(datapoints);
            error_and_exit("An Error Has Occurred");
        }

        for (i = 0; i < k; i++) {
            new_centroids_sum[i] = (double*)calloc(d, sizeof(double));
            if (new_centroids_sum[i] == NULL) {
                 /* Free all allocated memory before exiting */
                for (l = 0; l < i; l++) {
                    free(new_centroids_sum[l]);
                }
                free(new_centroids_sum);
                free(cluster_sizes);
                for (l = 0; l < k; l++) {
                    free(centroids[l]);
                }
                free(centroids);
                free(assignments);
                for (l = 0; l < n; l++) {
                    free(datapoints[l]);
                }
                free(datapoints);
                error_and_exit("An Error Has Occurred");
            }
        }

        for (i = 0; i < n; i++) {
            int cluster_idx = assignments[i];
            cluster_sizes[cluster_idx]++;
            for (j = 0; j < d; j++) {
                new_centroids_sum[cluster_idx][j] += datapoints[i][j];
            }
        }

        /* Calculate new centroids and check for convergence based on centroid movement */
        converged = 1; /* Assume converged until proven otherwise */
        for (i = 0; i < k; i++) {
            if (cluster_sizes[i] > 0) {
                old_centroid = centroids[i];
                new_centroid_val = (double*)malloc(sizeof(double) * d);
                if (new_centroid_val == NULL) {
                    /* Free all allocated memory before exiting */
                    for (l = 0; l < k; l++) {
                        free(new_centroids_sum[l]);
                    }
                    free(new_centroids_sum);
                    free(cluster_sizes);
                    for (l = 0; l < k; l++) {
                        /* Only free if it's the old centroid pointer, not one that was kept due to empty cluster */
                        if (centroids[l] != NULL) free(centroids[l]);
                    }
                    free(centroids);
                    free(assignments);
                    for (l = 0; l < n; l++) {
                        free(datapoints[l]);
                    }
                    free(datapoints);
                    error_and_exit("An Error Has Occurred");
                }

                for (j = 0; j < d; j++) {
                    new_centroid_val[j] = new_centroids_sum[i][j] / cluster_sizes[i];
                }

                /* Check if the centroid moved significantly */
                if (calculate_distance(old_centroid, new_centroid_val, d) >= EPSILON) {
                    converged = 0; /* Not converged if any centroid moved too much */
                }
                free(old_centroid); /* Free the old centroid's memory */
                centroids[i] = new_centroid_val; /* Update with the new centroid */
            } else {
                /* If a cluster is empty, it means the algorithm is not fully converged by this metric.
                 * The centroid remains its old value. No need to free/reallocate its memory. */
                converged = 0;
                /* centroids[i] retains its value from the previous iteration */
            }
        }

        /* Free memory for sums and sizes for the current iteration */
        for (i = 0; i < k; i++) {
            free(new_centroids_sum[i]);
        }
        free(new_centroids_sum);
        free(cluster_sizes);

        /* If all centroids moved less than EPSILON (or no assignments changed, handled above), break */
        if (converged) {
            break;
        }
    }

    /* 4. Print final centroids */
    for (i = 0; i < k; i++) {
        for (j = 0; j < d; j++) {
            printf("%.4f", centroids[i][j]);
            if (j < d - 1) {
                printf(",");
            }
        }
        printf("\n");
    }

    /* 5. Free all allocated memory */
    for (i = 0; i < k; i++) {
        free(centroids[i]);
    }
    free(centroids);

    free(assignments);

    for (i = 0; i < n; i++) {
        free(datapoints[i]);
    }
    free(datapoints);

    return 0;
}
