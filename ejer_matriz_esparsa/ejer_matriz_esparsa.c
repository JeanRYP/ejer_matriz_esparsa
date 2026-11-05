#include <stdio.h>
#include <stdlib.h>
#include <time.h>

typedef struct {
    int fila;
    int col;
    double valor;
} ElementoEsparso;

void generar_texto(const char *nombre_archivo, int filas, int cols) {
    FILE *f = fopen(nombre_archivo, "w");
    if (!f) return;

    fprintf(f, "%d %d\n", filas, cols);

    int total_elementos = filas * cols;
    int no_ceros = (int)(total_elementos * 0.05);

    for (int k = 0; k < no_ceros; k++) {
        int i = rand() % filas;
        int j = rand() % cols;
        double v = (rand() % 100) + 1;
        fprintf(f, "%d %d %.2f\n", i, j, v);
    }

    fclose(f);
}

void texto_a_binario(const char *txt, const char *bin) {
    FILE *ftxt = fopen(txt, "r");
    FILE *fbin = fopen(bin, "wb");
    if (!ftxt || !fbin) return;

    int filas, cols;
    fscanf(ftxt, "%d %d", &filas, &cols);

    fwrite(&filas, sizeof(int), 1, fbin);
    fwrite(&cols, sizeof(int), 1, fbin);

    ElementoEsparso elem;
    while (fscanf(ftxt, "%d %d %lf", &elem.fila, &elem.col, &elem.valor) == 3) {
        fwrite(&elem, sizeof(ElementoEsparso), 1, fbin);
    }

    fclose(ftxt);
    fclose(fbin);
}

void multiplicar_binarios(const char *bin_a, const char *bin_b, const char *bin_temp) {
    FILE *fa = fopen(bin_a, "rb");
    FILE *fb = fopen(bin_b, "rb");
    FILE *ftemp = fopen(bin_temp, "wb");
    if (!fa || !fb || !ftemp) return;

    int filas_a, cols_a, filas_b, cols_b;
    fread(&filas_a, sizeof(int), 1, fa);
    fread(&cols_a, sizeof(int), 1, fa);
    fread(&filas_b, sizeof(int), 1, fb);
    fread(&cols_b, sizeof(int), 1, fb);

    fwrite(&filas_a, sizeof(int), 1, ftemp);
    fwrite(&cols_b, sizeof(int), 1, ftemp);

    ElementoEsparso elem_a, elem_b, res;

    while (fread(&elem_a, sizeof(ElementoEsparso), 1, fa) == 1) {
        fseek(fb, sizeof(int) * 2, SEEK_SET);
        while (fread(&elem_b, sizeof(ElementoEsparso), 1, fb) == 1) {
            if (elem_a.col == elem_b.fila) {
                res.fila = elem_a.fila;
                res.col = elem_b.col;
                res.valor = elem_a.valor * elem_b.valor;
                fwrite(&res, sizeof(ElementoEsparso), 1, ftemp);
            }
        }
    }

    fclose(fa);
    fclose(fb);
    fclose(ftemp);
}

void consolidar_y_exportar(const char *bin_temp, const char *txt_out) {
    FILE *fbin = fopen(bin_temp, "rb");
    FILE *ftxt = fopen(txt_out, "w");
    if (!fbin || !ftxt) return;

    int filas, cols;
    fread(&filas, sizeof(int), 1, fbin);
    fread(&cols, sizeof(int), 1, fbin);

    fprintf(ftxt, "%d %d\n", filas, cols);

    long pos_inicio = ftell(fbin);
    ElementoEsparso elem1, elem2;

    while (fread(&elem1, sizeof(ElementoEsparso), 1, fbin) == 1) {
        if (elem1.valor == 0) continue;

        long pos_actual = ftell(fbin);
        double suma = elem1.valor;

        while (fread(&elem2, sizeof(ElementoEsparso), 1, fbin) == 1) {
            if (elem2.valor != 0 && elem1.fila == elem2.fila && elem1.col == elem2.col) {
                suma += elem2.valor;

                long pos_marca = ftell(fbin);
                fseek(fbin, -(long)sizeof(ElementoEsparso), SEEK_CUR);
                ElementoEsparso borrado = {elem2.fila, elem2.col, 0.0};
                fwrite(&borrado, sizeof(ElementoEsparso), 1, fbin);
                fseek(fbin, pos_marca, SEEK_SET);
            }
        }

        fprintf(ftxt, "%d %d %.2f\n", elem1.fila, elem1.col, suma);
        fseek(fbin, pos_actual, SEEK_SET);
    }

    fclose(fbin);
    fclose(ftxt);
}

int main() {
    srand(time(NULL));

    int M = 100, N = 100, P = 100;

    generar_texto("matriz_a.txt", M, N);
    generar_texto("matriz_b.txt", N, P);

    texto_a_binario("matriz_a.txt", "matriz_a.bin");
    texto_a_binario("matriz_b.txt", "matriz_b.bin");

    multiplicar_binarios("matriz_a.bin", "matriz_b.bin", "acumulado.bin");

    consolidar_y_exportar("acumulado.bin", "matriz_axb.txt");

    return 0;
}