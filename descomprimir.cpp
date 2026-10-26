#include "huffman.h"
#include <cstring>

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Uso: %s <archivo.huff> [archivo_salida]\n", argv[0]);
        return 1;
    }

    FILE *in = fopen(argv[1], "rb");
    if (!in) { fprintf(stderr, "Error abriendo '%s'\n", argv[1]); return 1; }

    char magic[4];
    if (fread(magic, 1, 4, in) != 4 || memcmp(magic, "HUFF", 4) != 0) {
        fprintf(stderr, "No es .huff valido (magic)\n"); fclose(in); return 1;
    }
    uint16_t version;
    if (fread(&version, sizeof(version), 1, in) != 1 || version != HUFF_VERSION) {
        fprintf(stderr, "Version no soportada: %u\n", version); fclose(in); return 1;
    }
    uint64_t tam_orig;
    if (fread(&tam_orig, sizeof(tam_orig), 1, in) != 1) {
        fprintf(stderr, "Header truncado\n"); fclose(in); return 1;
    }

    NodoHuff *raiz = deserializarArbol(in);
    if (!raiz) { fprintf(stderr, "Arbol corrupto\n"); fclose(in); return 1; }

    std::string salida = (argc >= 3) ? argv[2]
                                     : (std::string(argv[1]) + ".out");
    FILE *out = fopen(salida.c_str(), "wb");
    if (!out) { fprintf(stderr, "Error creando '%s'\n", salida.c_str());
                delete raiz; fclose(in); return 1; }

    if (tam_orig == 0) {
        // nada
    } else if (raiz->esHoja) {
        for (uint64_t i = 0; i < tam_orig; i++) fputc(raiz->simbolo, out);
    } else {
        BitReader br(in);
        NodoHuff *actual = raiz;
        uint64_t escritos = 0;
        while (escritos < tam_orig) {
            int bit = br.getBit();
            if (bit < 0) { fprintf(stderr, "Archivo truncado\n"); break; }
            actual = (bit == 0) ? actual->izq : actual->der;
            if (actual && actual->esHoja) {
                fputc(actual->simbolo, out);
                escritos++;
                actual = raiz;
            }
        }
    }

    fclose(out);
    fclose(in);
    delete raiz;
    fprintf(stdout, "Descomprimido: %s -> %s (%llu bytes)\n",
            argv[1], salida.c_str(), (unsigned long long)tam_orig);
    return 0;
}
