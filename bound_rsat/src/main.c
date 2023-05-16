#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<time.h>
#include<inttypes.h>
#include<errno.h>
#include<assert.h>
#include<math.h>
#include<omp.h>
#include<stdbool.h>

#include<z3.h>

#include "pcg_basic.h"
#include "rng.h"


typedef struct {
    char* prefix;
    uint64_t nb_v;
    uint64_t nb_cl;
    uint64_t nb_ch;
    uint64_t nb_f;
    uint64_t k;

    bool check_sat;
} Options;

Options options_default() {
    Options r = {
        .prefix = NULL,
        .nb_v = 50,
        .nb_cl = 1,
        .nb_ch = 50,
        .nb_f = 10,
        .k = 3,
        .check_sat = false,
    };
    return r;
}

void print_help_screen() {
    printf("rsat options:\n"
    "--nv uint\n"
        "\tset the number of variables (default: 50)\n"
    "--ncl uint\n"
        "\tset the minimum number of clauses (default: 100)\n"
    "--nch uint\n"
        "\tset the maximum number of clauses (default: 0 -> nch = ncl)\n"
    "--k  uint\n"
        "\tset the size of each clause (default: 3)\n"
    "--nf uint\n"
        "\tset the number of formulas to generate (default: 10)\n"
    "--prefix str\n"
        "\tset a prefix to the formula file names\n"
    "--sat\n"
        "\tonly generate sat formulas (default: false)\n"
    "--help\n"
    "-h\n"
        "\tdisplay this help screen\n"
    "\ntotal number of formulas generated: $nf * ($nvh - $nvl + 1) * ($nch - $ncl + 1)\n"
    );
}

bool is_sat(char const* path) {
    Z3_config conf = Z3_mk_config();
    Z3_context context = Z3_mk_context(conf);
    Z3_solver solver = Z3_mk_solver(context);

    Z3_solver_from_file(context, solver, path);

    bool res = Z3_solver_check(context, solver) == Z3_L_TRUE;

    Z3_del_context(context);
    Z3_del_config(conf);

    return res;
}

uint64_t parse_int(char const * restrict str) {
    errno = 0;
    int64_t n = strtol(str, NULL, 10);
    if(errno != 0 || n < 0) {
        fprintf(stderr, "error while parsing number \"%s\"\nexiting\n", str);
        exit(EXIT_FAILURE);
    }
    return n;
}

double parse_double(char const * restrict str) {
    double res;

    if(1 != sscanf(str, "%lf", &res) || res < 0) {
        fprintf(stderr, "error while parsing float number \"%s\"\nexiting\n", str);
        exit(EXIT_FAILURE);
    }
    return res;
}

Options options_parse(int argc, char** argv) {
    Options r = options_default();

    for(int i = 1; i < argc; i++) {
        if(!strcmp(argv[i], "--nv")) {
            i++;
            r.nb_v = parse_int(argv[i]);
        }
        else if(!strcmp(argv[i], "--ncl")) {
            i++;
            r.nb_cl = parse_int(argv[i]);
        }
        else if(!strcmp(argv[i], "--nch")) {
            i++;
            r.nb_ch = parse_int(argv[i]);
        }
        else if(!strcmp(argv[i], "--nf")) {
            i++;
            r.nb_f = parse_int(argv[i]);
        }
        else if(!strcmp(argv[i], "--k")) {
            i++;
            r.k = parse_int(argv[i]);
        }
        else if(!strcmp(argv[i], "--prefix")) {
            i++;
            r.prefix = argv[i];
        }
        else if(!strcmp(argv[i], "--sat")) {
            r.check_sat = true;
        }
        else if(!strcmp(argv[i], "--help") || !strcmp(argv[i], "-h")) {
            print_help_screen();
            exit(EXIT_SUCCESS);
        }
        else {
            print_help_screen();
            fprintf(stderr, "unrecognized option:\n\"%s\"\n", argv[i]);
            perror("exiting due to errors\n");
            exit(EXIT_FAILURE);
        }
    }

    // if(r.nb_ch == 0 || r.nb_ch <= r.nb_cl) {
    //     r.nb_ch = r.nb_cl;
    // }

    return r;
}

bool contains(int64_t const * st, int64_t const * const ed, int64_t const v) {
    for(;st != ed; ++st) {
        if(*st == v) {
            return true;
        }
    }
    return false;
}

/**
 * k: size of each clause
 * nb: total number of clauses
 * c: pos of new clause
 */
bool already_generated(int64_t const * cls, uint64_t k, uint64_t nb, uint64_t c) {
    for(uint64_t i = 0; i < nb; i++) {
        if(i != c && !memcmp(&cls[i * k], &cls[c * k], k * sizeof(int64_t))) {
            return true;
        }
    }
    return false;
}

/**
 * @return returns the path to the written file
 *         !!! the path is dynamically allocated and needs to be freed !!!
 */
char* writeFormula(uint64_t const id, uint64_t const nv, uint64_t const nc, uint64_t const k, char const* path_prefix, int64_t const* cls) {
    uint64_t path_len = ceil(log(id < 100 ? 100 : id))
        + ceil(log(nv < 100 ? 100 : nv))
        + ceil(log(nc < 100 ? 100 : nc))
        + 2
        + 4
        + 1
        ;
    if(path_prefix != NULL) {
        path_len += strlen(path_prefix);
    }
    // printf("%" PRIu64 ".%" PRIu64 ".%" PRIu64 ".cnf\n", nv, id, nc);
    // printf("alloc size: %" PRIu64 "\n", path_len);

    char* path = malloc(path_len * sizeof(char));

    if(path_prefix == NULL) {
        sprintf(path, "%" PRIu64 ".%" PRIu64 ".cnf", nv, id);
    }
    else {
        sprintf(path, "%s%" PRIu64 ".%" PRIu64 ".cnf", path_prefix, nv, id);
    }

    FILE* file = fopen(path, "w");
    fprintf(file, "p cnf %" PRIu64 " %" PRIu64 "\n", nv, nc);

    for(uint64_t c = 0; c < nc; c++) {
        for(uint64_t ki = 0; ki < k; ki++) {
            fprintf(file, "%" PRIi64 " ", cls[c * k + ki]);
        }
        fprintf(file, "0\n");
    }

    fclose(file);
    //free(path);
    return path;
}

void genFormula(pcg32_random_t* rng, uint64_t const id, uint64_t const nv, uint64_t ncl, uint64_t nch, uint64_t const k, char const * path_prefix, bool const check_sat) {


    int64_t * cls = malloc(sizeof(int64_t) * k * nch);
    for(uint64_t c = 0; c < nch; c++) {
         do {
             for(uint64_t i = c * k; i < (c + 1) * k; i++) {
                 do {
                     cls[i] = pcg32_boundedrand_r(rng, nv) + 1;
                 } while(contains(cls + c * k, cls + i, cls[i]));
             }

             for(uint64_t i = c * k; i < (c + 1) * k; i++) {
                 cls[i] *= pcg32_boundedrand_r(rng, 2) ? -1 : 1;
             }
         } while(already_generated(cls, k, c, c));
    }

    uint64_t c = (ncl + nch) / 2;
    uint64_t steps = 0;
    while(ncl + 1 < nch) {
        steps++;

        char* path = writeFormula(id, nv, c, k, path_prefix, cls);

        bool unsat = check_sat && !is_sat(path);
        free(path);
        if(unsat) {
            nch = c;
        }
        else {
            ncl = c;
        }
        c = (ncl + nch) / 2;
    }

    char* path = writeFormula(id, nv, nch, k, path_prefix, cls);
    bool unsat = check_sat && !is_sat(path);
    free(path);

    if(!unsat) {
        printf("nch is sat\n");
    }

    path = writeFormula(id, nv, ncl, k, path_prefix, cls);
    unsat = check_sat && !is_sat(path);
    free(path);
    if(unsat) {
        printf("ncl is unsat\n");
    }

    //printf("steps: %" PRIi64 "\n", steps);

    free(cls);
    /*printf("done %" PRIu64 "\n", nc);*/
}


/**
 * usage:
 * rsat #v #c k
 */
int main(int argc, char** argv) {
    Options options = options_parse(argc, argv);
    //pcg32_srandom(time(NULL) ^ (intptr_t)&printf, (intptr_t)&options);
    uint64_t const num_threads = omp_get_max_threads();
    pcg32_random_t* rng_state = malloc(sizeof(pcg32_random_t) * num_threads);

    for(uint64_t i = 0; i < num_threads; i++) {
        // pcg32_srandom(time(NULL) ^ rng_random64(), rng_random64());
        pcg32_srandom_r(rng_state + i, time(NULL) ^ rng_random64(), rng_random64());
    }

#pragma omp parallel for
    for(uint64_t i = 0; i < options.nb_f; i++) {
        pcg32_random_t* rng = rng_state + omp_get_thread_num();

        genFormula(rng, i, options.nb_v, options.nb_cl, options.nb_ch, options.k, options.prefix, options.check_sat);
    }
    free(rng_state);

    return EXIT_SUCCESS;
}
