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

void freevec(vector* v) {
    free(v->data);
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
    char** vars;
    int basic;
    int state;
} dictionary;

#define SIMPLEX_STATE_SUCCESS 0
#define SIMPLEX_STATE_INFEASIBLE -1
#define SIMPLEX_STATE_UNBOUNDED -2
#define SIMPLEX_STATE_FEASIBLE 1

dictionary create_dic(vector* c, vector* b, matrix* a) {

    dictionary d;
    d.basic = c->size;
    d.dic = mat(a->rows + 1, a->columns + 1);
    d.state = SIMPLEX_STATE_FEASIBLE;

    // Init basics
    for (int i = 0; i < c->size; i++){
        d.dic.data[0].data[i+1] = c->data[i];
        for (int j = 0; j < b->size; j++) {
            d.dic.data[i+1].data[j+1] = -a->data[i].data[j];
            if (i == 0)
                d.dic.data[j+1].data[0] = b->data[j];
        }
    }

    // alloc name
    d.vars = (char**)malloc(sizeof(char*) * (c->size + b->size));
    for (int i = 0; i < c->size; i++) {
        d.vars[i] = malloc(sizeof(char) * 4);
        sprintf(d.vars[i], "x%i", (i+1));}
    for (int i = 0; i < b->size; i++) {        
        d.vars[c->size + i] = malloc(sizeof(char) * 4);
        sprintf(d.vars[c->size + i], "w%i", (i+1));
    }

    return d;

}

void print_dictionary(dictionary* dic) {

    // Print header (names of non-basics)
    printf("%18s  ", "");
    for (int i = 0; i < dic->basic; i++) {
        printf("%8s  ", dic->vars[i]);
    }
    printf("\n");

    // Print basics
    for (int i = 0; i < dic->dic.rows; i++) {
        if (i > 0) {
            printf("%8s =", dic->vars[dic->basic - 1 + i]);
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
        double den = -dic->data[i].data[*enter];
        double ratio = num == 0 && den == 0 ? 0 : (num / den);
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
    char* tmp = d.vars[enter - 1];
    d.vars[enter-1] = d.vars[d.basic + leaving - 1];
    d.vars[d.basic + leaving - 1] = tmp;

    // Return updated dictionary
    return d;

}

dictionary simplex(vector* c, vector* b, matrix* a) {
    
    // Prepare
    dictionary dic = create_dic(c,b,a);
    
    // print inital
    printf("Initial Dictionary:\n");
    print_dictionary(&dic);
    printf("\n--- Now Solving ---\n");

    int e, l;
    while (dic.state) {
        
        // Find pivot location
        //print_matrix(dic);
        dic.state = find_pivot(&dic.dic, &e, &l);
        switch (dic.state) {
        case SIMPLEX_STATE_SUCCESS:
            printf("--- Simplex Terminating (Success) ---\n\n");
            return dic;
        case SIMPLEX_STATE_UNBOUNDED:
            printf("--- Simplex Terminating (Unbounded) ---\n\n");
            return dic;
        default:
            printf("%s entering and %s leaving:\n\n", dic.vars[e - 1], dic.vars[c->size + l - 1]);
            // pivot
            dic = pivot(dic, e, l);
            print_dictionary(&dic);
            printf("\n");
            
            break;
        }

    }
    
}

// Print solution
void print_solution(dictionary* dic) {

    // Only print if success
    if (dic->state == SIMPLEX_STATE_SUCCESS) {
        printf("Maximum Value: %f\nVariables: ", dic->dic.data[0].data[0]);
        for (int i = 1; i < dic->dic.rows; i++) {
            printf("%s = %.4f", dic->vars[dic->basic - 1 + i], dic->dic.data[i].data[0]);
            if (i + 1 < dic->dic.rows)
                printf(", ");
        }
        printf("\n\n");
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
    fscanf(pFile, "%d", &prog.vars);

    // Read minmax mode
    char minmax[4];
    fscanf(pFile, "%3s", &minmax);

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
    fscanf(pFile, "%11s", &cons);

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
        if (!fscanf(pFile, "%2s", &constrainType)){
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

        // TODO: Rewrite consraint if not <=

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
