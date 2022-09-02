#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

typedef struct {
    int size;
    double* data;
} vector;

vector vec(int size) {
    vector v;
    v.data = (double*)malloc(size * sizeof(double));
    v.size = size;
    return v;
}

void vec_mul(vector* v, double s) {
    for (int i = 0; i < v->size; i++){
        v->data[i] *= s;
    }
}

void vec_add(vector* a, vector* b) {
    if (a->size != b->size) {
        fprintf(stderr, "Attempt to add a vector of dimension %i to a vector with dimension %i", b->size, a->size);
        exit(-1);
    }
    for (int i = 0; i < a->size; i++){
        a->data[i] += b->data[i];
    }
}

void vec_copy(vector* src, vector* dst) {
    *dst = vec(src->size);
    memcpy(dst->data, src->data, sizeof(double) * src->size);
}

void freevec(vector* v) {
    free(v->data);
}

void print_vec(vector* v) {
    printf("[");
    for (int i = 0; i < v->size; i++){
        printf("%f", v->data[i]);
        if (i + 1 < v->size)
            printf(", ");
    }
    printf("]\n");
}

typedef struct {
    int columns;
    int rows;
    vector* data;
} matrix;

matrix mat(int r, int c) {
    matrix m;
    m.columns = c;
    m.rows = r;
    m.data = (vector*)malloc(sizeof(vector) * r);
    for (int i = 0; i < r; i++)
        m.data[i] = vec(c);
    return m;
}

void freemat(matrix* m) {
    for (int i = 0; i < m->rows; i++)
        freevec(&m->data[i]);
    free(m->data);
}

void print_matrix(matrix m) {
    for (int i = 0; i < m.rows; i++) {
        for (int j = 0; j < m.columns; j++) {
            printf("|%8.4f ", m.data[i].data[j]);
        }
        printf("\n");
    }
}

typedef struct {
    matrix dic;
    unsigned char* vars;
    int varc;
    int state;
} dictionary;

#define SIMPLEX_STATE_SUCCESS 0
#define SIMPLEX_STATE_INFEASIBLE -1
#define SIMPLEX_STATE_UNBOUNDED -2
#define SIMPLEX_STATE_FEASIBLE 1

dictionary create_dic(vector* c, vector* b, matrix* a) {

    dictionary d;
    d.varc = c->size;
    d.dic = mat(b->size + 1, c->size + 1);
    d.state = SIMPLEX_STATE_FEASIBLE;

    // Init basics
    d.dic.data[0].data[0] = 0;

    // Init objective row
    for (int i = 0; i < c->size; i++){
        d.dic.data[0].data[i+1] = c->data[i];
    }

    // Init nonbasic
    for (int i = 0; i < b->size; i++){
        d.dic.data[i + 1].data[0] = b->data[i];
        for (int j = 0; j < c->size; j++) {
            d.dic.data[i + 1].data[j + 1] = -a->data[i].data[j];
        }
    }

    // alloc name
    d.vars = (unsigned char*)malloc(sizeof(unsigned char) * (c->size + b->size));
    for (unsigned char i = 0; i < c->size + b->size; i++) {
        d.vars[i] = i + 1;
    }

    // Return the created dictionary
    return d;

}

void get_var(dictionary* dic, char n[5], int v) {
    if (v <= dic->varc)
        sprintf(n, "x%i", v);
    else 
        sprintf(n, "w%i", v - dic->varc);
}

void print_dictionary(dictionary* dic) {

    // Print header (names of non-basics)
    printf("%18s  ", "");
    for (int i = 0; i < dic->varc; i++) {
        char n[5];
        get_var(dic, n, dic->vars[i]);
        printf("%8s  ", n);
    }
    printf("\n");

    // Print basics
    for (int i = 0; i < dic->dic.rows; i++) {
        if (i > 0) {
            char n[5];
            get_var(dic, n, dic->vars[dic->varc - 1 + i]);
            printf("%8s =", n);
        } else {
            printf("%8s =", "Zeta");
        }
        for (int j = 0; j < dic->dic.columns; j++) {
            double v = dic->dic.data[i].data[j];
            if (j > 0) {
                if (v <= 0) {
                    printf("-");
                    v = -v;
                } else {
                    printf("+");
                }
            }
            printf("%8.4f ", v);
        }
        printf("\n");
    }

}

void eliminate_column(dictionary* dic, int column) {
    
    // Bail if no column is defined
    if (column == -1)
        return;

    // TODO: Implement

}

int index_of(unsigned char* arr, unsigned char val, int min, int max) {
    for (int i = min; i < max; i++){
        if (arr[i] == val)
            return arr[i];
    }
    return -1;
}

int find_pivot(matrix* dic, int* enter, int* leave) {

    // Find largest positive coefficient (entering)
    *enter = -1;
    for (int i = 1; i < dic->data[0].size; i++){
        if (dic->data[0].data[i] > 0){
            if (*enter == -1)
                *enter = i;
            else
                *enter = dic->data[0].data[i] > dic->data[0].data[*enter] ? i : *enter;
        }
    }

    // Return if none is found
    if (*enter == -1)
        return SIMPLEX_STATE_SUCCESS; // No positive coefficent, we're done

    // Find smallest constraint (leaving)
    *leave = -1;
    double minRatio = INFINITY;
    for (int i = 1; i < dic->rows; i++){
        double num = dic->data[i].data[0];
        double den = dic->data[i].data[*enter];
        if (den == 0 && num != 0)
            continue; // Avoid a division by 0
        double ratio = num == 0 && den == 0 ? 0 : (num / -den);
        if (ratio < minRatio && ratio >= 0) {
            minRatio = ratio;
            *leave = i;
        }
    }

    // Verify
    if (*leave == -1)
        return SIMPLEX_STATE_UNBOUNDED; // Unbounded

    // Return 1 ==> Pivot found
    return SIMPLEX_STATE_FEASIBLE;

}

int most_infeasible(matrix* dic, int enter) {
    int leave = 1;
    double maxRatio = -INFINITY;
    for (int i = 1; i < dic->rows; i++){
        double num = dic->data[i].data[0];
        double den = dic->data[i].data[enter];
        if (den == 0 && num != 0)
            continue; // Avoid a division by 0
        double ratio = num == 0 && den == 0 ? 0 : (num / -den);
        if (ratio > maxRatio && ratio >= 0) {
            maxRatio = ratio;
            leave = i;
        }
    }
    return leave;
}

dictionary pivot(dictionary d, int enter, int leaving) {

    // Grab pivot
    double pivot = d.dic.data[leaving].data[enter];

    // Correct Remaining rows
    for (int i = 0; i < d.dic.rows; i++)
        if (i != leaving) {
            double ratio = -(d.dic.data[i].data[enter] / pivot);
            for (int j = 0; j < d.dic.columns; j++) {
                if (j == enter)
                    d.dic.data[i].data[j] = -ratio;
                else
                    d.dic.data[i].data[j] += ratio * d.dic.data[leaving].data[j];
            }
        }

    // Correct leaving row
    for (int i = 0; i < d.dic.data[leaving].size; i++)
        if (i == enter)
            d.dic.data[leaving].data[i] = 1.0 / pivot;
        else
            d.dic.data[leaving].data[i] /= -pivot;

    // Swap out vars
    unsigned char tmp = d.vars[enter - 1];
    d.vars[enter-1] = d.vars[d.varc + leaving - 1];
    d.vars[d.varc + leaving - 1] = tmp;

    // Return updated dictionary
    return d;

}

// Performs phase two of the simplex method.
dictionary phase_two(dictionary dic);

// Performs phase one of the simplex method which brings negative bounds into a feasible dictionary.
dictionary phase_one(dictionary initial) {

    // Check if there's reason for doing phase one
    int skip = 1;
    for (int i = 1; i < initial.dic.rows; i++)
        if (initial.dic.data[i].data[0] < 0)
            skip = 0;

    // Bail if phase one is not required
    if (skip) {
        printf("Skipping Phase One\n");
        return initial;
    }

    // Log we're solving the auxiliary problem
    printf("--- Solving Auxiliary Problem ---\n");

    // Create new coefficient matrix
    vector c = vec(initial.varc + 1);
    c.data[initial.varc] = -1;
    memset(c.data, 0, sizeof(double) * initial.varc);

    // Copy bounds matrix
    vector b = vec(initial.dic.rows - 1);
    for (int i = 1; i < initial.dic.rows; i++)
        b.data[i - 1] = initial.dic.data[i].data[0];

    // Create new constraint matrix
    matrix a = mat(initial.dic.rows - 1, initial.dic.columns);
    for (int i = 1; i < initial.dic.rows; i++) {
        for (int j = 1; j < initial.dic.columns; j++) {
            a.data[i - 1].data[j - 1] = -initial.dic.data[i].data[j];
        }
        a.data[i - 1].data[initial.varc] = -1;
    }

    // Construct the auxiliary problem dictionary
    dictionary aux = create_dic(&c, &b, &a);
    aux.vars[c.size - 1] = 0;

    // Debug aux
    printf("Auxiliary dictionary:\n");
    print_dictionary(&aux);

    // Pick enter and leave
    int enter = aux.varc;
    int leave = most_infeasible(&aux.dic, enter);

    // Pivot towards feasibility
    char e[5];
    char l[5];
    get_var(&aux, e, aux.vars[enter - 1]);
    get_var(&aux, l, aux.vars[aux.varc + leave - 1]);
    printf("%s entering and %s leaving:\n\n", e, l);
    aux = pivot(aux, enter, leave);

    // Print feasible dictionary
    print_dictionary(&aux);

    // Apply simplex on this dictionary
    aux = phase_two(aux);
    if (aux.dic.data[0].data[0] < 0)
        aux.state = SIMPLEX_STATE_INFEASIBLE;
    if (aux.state == SIMPLEX_STATE_SUCCESS)
        printf("--- Auxiliary Problem Solved ---\n");
    else
        return aux;

    // Eliminate column
    eliminate_column(&aux, index_of(aux.vars, 0, 0, aux.varc));

    // Reintroduce objective function and remove x0
    vector obj = vec(aux.dic.columns);
    for (int i = 0; i < initial.varc; i++) {

        // Find the value in 
        int k = -1;
        for (int j = 1; j < aux.dic.rows; j++) {
            if (aux.vars[j - 1 + aux.varc] == initial.vars[i]) {
                k = j;
                break;
            }
        }
        double scalar = initial.dic.data[0].data[i + 1];
        vector v;
        if (k != -1) {
            vec_copy(&aux.dic.data[k], &v);
        } else {
            // TODO: Implement
        }
        // Multiply and add to updated objective function
        vec_mul(&v, scalar);
        vec_add(&obj, &v);
    }

    // free current
    print_vec(&obj);
    freevec(&aux.dic.data[0]);
    aux.dic.data[0] = obj;
    
    // Log main problem
    printf("---   Solving Main Problem   ---\n\n");
    print_dictionary(&aux);

    // Do an exit (Debug)
    exit(0);

    // Return the dictionary
    return aux;

}

// Performs phase two of the simplex method.
dictionary phase_two(dictionary dic) {

    // Enter and leaving variable
    int e, l;

    // Make space for enter and leave names
    char ev[5];
    char lv[5];

    // While feasible
    while (dic.state) {
        
        // Find pivot location
        dic.state = find_pivot(&dic.dic, &e, &l);
        switch (dic.state) {
        case SIMPLEX_STATE_SUCCESS:
            printf("--- Simplex Terminating (Success) ---\n\n");
            return dic;
        case SIMPLEX_STATE_INFEASIBLE:
            printf("--- Simplex Terminating (Infeasible) ---\n\n");
            return dic;
        case SIMPLEX_STATE_UNBOUNDED:
            printf("--- Simplex Terminating (Unbounded) ---\n\n");
            return dic;
        default:
            get_var(&dic, ev, dic.vars[e - 1]);
            get_var(&dic, lv, dic.vars[dic.varc + l - 1]);
            printf("%s entering and %s leaving:\n\n", ev, lv);
            // pivot
            dic = pivot(dic, e, l);
            print_dictionary(&dic);
            printf("\n");
            
            break;
        }

    }

}

dictionary simplex(vector* c, vector* b, matrix* a) {
    
    // Prepare
    dictionary dic = create_dic(c,b,a);
    
    // print inital
    printf("Initial Dictionary:\n");
    print_dictionary(&dic);
    printf("\n");

    // Do phase one
    dic = phase_one(dic);
    if (dic.state != SIMPLEX_STATE_FEASIBLE) {
        return dic; // Return immediately
    }

    // Do phase two and return result
    return phase_two(dic);
    
}

// Print solution
void print_solution(dictionary* dic) {

    // Only print if success
    if (dic->state == SIMPLEX_STATE_SUCCESS) {
        printf("Maximum Value: %f\nVariables: ", dic->dic.data[0].data[0]);
        for (int i = 1; i < dic->dic.rows; i++) {
            char n[5];
            get_var(dic, n, dic->vars[dic->varc - 1 + i]);
            printf("%s = %.4f", n, dic->dic.data[i].data[0]);
            if (i + 1 < dic->dic.rows)
                printf(", ");
        }
        printf("\n\n");
    } else if (dic->state == SIMPLEX_STATE_INFEASIBLE) {
        printf("Problem is infeasible and has no solution");
    } else if (dic->state == SIMPLEX_STATE_UNBOUNDED) {
        printf("Problem is unbounded and thus has no optimal solution");
    }

}

typedef struct {
    int vars;
    char max;
    vector c; // Coefficients
    vector b; // Bounds
    matrix a; // Constraints
} linprog;

// Read a problem from file
linprog read_problem(const char* pFilePath) {
    
    // Define the program to be read
    linprog prog;
    prog.max = -1;
    
    // Open file
    FILE* pFile = fopen(pFilePath, "r");

    // Read over 'variables '
    fseek(pFile, 10, SEEK_SET);
    if (fscanf(pFile, "%d", &prog.vars) <= 0){
        fprintf(stderr, "Problem variable count expected but none found.\n");
        return prog;
    }

    // Read minmax mode
    char minmax[4];
    if (fscanf(pFile, "%3s", minmax) <= 0) {
        fprintf(stderr, "Objective goal expected but was not valid. (allowed: min, max)\n");
        return prog;
    }

    // Read coefficients
    prog.c = vec(prog.vars);
    for (int i = 0; i < prog.vars; i++){
        float f;
        if (!fscanf(pFile, "%f", &f)) {
            printf("Failed to read coefficient %i.\n", i + 1);
            return prog;
        }
        prog.c.data[i] = f;
    }

    // Read constraints
    char cons[12];
    if (fscanf(pFile, "%11s", cons) <= 0) {
        fprintf(stderr, "'constraints' keyword expected following objective function definition.\n");
        return prog;
    }

    // Read constraints
    int constraints;
    if (!fscanf(pFile, "%i", &constraints)){
        printf("Failed to read constraint count.\n");
        return prog;
    }

    // Write how many constraints we have
    prog.a = mat(constraints, prog.vars);
    prog.b = vec(constraints);
    for (int i = 0; i < constraints; i++) {

        // Read constraints
        float f;
        for (int j = 0; j < prog.vars; j++) {
            if (!fscanf(pFile, "%f", &f)) {
                printf("Failed to read constraint coefficient a[%i,%i].\n", i,j);
                return prog;
            }
            prog.a.data[i].data[j] = f;
        }

        // Read mode
        char constrainType[3];
        if (!fscanf(pFile, "%2s", constrainType)){
            printf("Failed to read constraint type.\n");
            return prog;
        }

        // Read bounds
        if (!fscanf(pFile, "%f", &f)) {
            printf("Failed to read constraint bound b[%i].\n", i);
            return prog;
        }

        // Set bound
        prog.b.data[i] = f;

        // Correct constraint
        if (strcmp(constrainType, ">=") == 0) {
            prog.b.data[i] *= -1;
            for (int j = 0; j < prog.vars; j++)
                prog.a.data[i].data[j] *= -1;
        } else if (strcmp(constrainType, "=") == 0) {
            // TODO: Implement
        } else if (strcmp(constrainType, "<=") != 0) {
            fprintf(stderr, "Invalid constraint type '%s'\n", constrainType);
            return prog;
        }

    }

    // Close file
    fclose(pFile);

    // Set mode
    if (strcmp(minmax, "max") == 0){
        prog.max = 0;
    } // TODO: Rewrite if 'min'

    // Return problem
    return prog;

}

int main(int argc, char** args) {
    
    // Print usage if no arguments are provided
    if (argc == 1) {
        printf("No problem file supplied in arguments.\n");
        return 0;
    }

    // Read over all inputs
    for (int i = 1; i < argc; i++) {

        // Log
        printf("Solving problem: %s\n", args[i]);

        // Read simplex
        linprog p = read_problem(args[i]);
        if (p.max == -1) {
            printf("Failed to read program file: %s\n", args[i]);
            continue;
        }

        // Find the optimal solution and exit
        dictionary optimal = simplex(&p.c, &p.b, &p.a);        
        print_solution(&optimal);

        // Cleanup
        freemat(&p.a);
        freevec(&p.c);
        freevec(&p.b);
        
    }

    // Return OK
    return 0;

}
