#include "huffman.h"
#include <cstring>
#include <string>

static bool leerArchivo(const char *path, std::string &out) {
    FILE *fp = fopen(path, "rb");
    if (!fp) return false;
    fseek(fp, 0, SEEK_END);
    long tam = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    if (tam < 0) { fclose(fp); return false; }
    out.resize((size_t)tam);
    if (tam > 0 && fread(&out[0], 1, (size_t)tam, fp) != (size_t)tam) {
        fclose(fp); return false;
    }
    fclose(fp);
    return true;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Uso: %s <archivo_entrada> [archivo_salida]\n", argv[0]);
        return 1;
    }

    std::string original;
    if (!leerArchivo(argv[1], original)) {
        fprintf(stderr, "Error leyendo '%s'\n", argv[1]);
        return 1;
    }

    std::string salida = (argc >= 3) ? argv[2]
                                     : (std::string(argv[1]) + ".huff");
    FILE *out = fopen(salida.c_str(), "wb");
    if (!out) { fprintf(stderr, "Error creando '%s'\n", salida.c_str()); return 1; }

    unsigned long frec[256] = {0};
    for (unsigned char c : original) frec[c]++;

    NodoHuff *raiz = construirArbol(frec);

    std::string codigos[256];
    generarCodigos(raiz, "", codigos);

    fwrite("HUFF", 1, 4, out);
    uint16_t version = HUFF_VERSION;
    fwrite(&version, sizeof(version), 1, out);
    uint64_t tam_orig = (uint64_t)original.size();
    fwrite(&tam_orig, sizeof(tam_orig), 1, out);

    if (raiz == nullptr) {
        uint8_t marker = 0x01, dummy = 0;
        fwrite(&marker, 1, 1, out);
        fwrite(&dummy, 1, 1, out);
    } else {
        serializarArbol(raiz, out);
    }

    BitWriter bw(out);
    if (raiz && !raiz->esHoja) {
        for (unsigned char c : original)
            for (char bit : codigos[c])
                bw.putBit(bit == '1');
    } else if (raiz && raiz->esHoja) {
        for (size_t i = 0; i < original.size(); i++) bw.putBit(0);
    }
    bw.flush();
    fclose(out);
    delete raiz;

    FILE *chk = fopen(salida.c_str(), "rb");
    fseek(chk, 0, SEEK_END);
    long tam_comp = ftell(chk);
    fclose(chk);
    fprintf(stdout, "Comprimido: %s -> %s\n", argv[1], salida.c_str());
    fprintf(stdout, "  Original:   %zu bytes\n", original.size());
    fprintf(stdout, "  Comprimido: %ld bytes\n", tam_comp);
    return 0;
}
