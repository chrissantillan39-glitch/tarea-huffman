#ifndef HUFFMAN_H
#define HUFFMAN_H

#include <cstdio>
#include <cstdint>
#include <string>

#define HUFF_VERSION 1

struct NodoHuff {
    unsigned char simbolo;
    unsigned long frecuencia;
    bool esHoja;
    NodoHuff *izq, *der;

    NodoHuff(unsigned char s = 0, unsigned long f = 0, bool hoja = false)
        : simbolo(s), frecuencia(f), esHoja(hoja), izq(nullptr), der(nullptr) {}

    ~NodoHuff() { delete izq; delete der; }
};

struct BitWriter {
    FILE *fp;
    uint8_t buffer;
    int nbits;
    explicit BitWriter(FILE *f);
    void putBit(int bit);
    void flush();
};

struct BitReader {
    FILE *fp;
    uint8_t buffer;
    int nbits;
    explicit BitReader(FILE *f);
    int getBit();
};

NodoHuff* construirArbol(const unsigned long frec[256]);
void generarCodigos(NodoHuff *raiz, const std::string &prefijo,
                    std::string codigos[256]);
void serializarArbol(NodoHuff *raiz, FILE *fp);
NodoHuff* deserializarArbol(FILE *fp);

#endif
