#include "huffman.h"
#include <queue>
#include <vector>

BitWriter::BitWriter(FILE *f) : fp(f), buffer(0), nbits(0) {}

void BitWriter::putBit(int bit) {
    buffer = (uint8_t)((buffer << 1) | (bit & 1));
    nbits++;
    if (nbits == 8) {
        fwrite(&buffer, 1, 1, fp);
        buffer = 0;
        nbits = 0;
    }
}

void BitWriter::flush() {
    if (nbits > 0) {
        buffer = (uint8_t)(buffer << (8 - nbits));
        fwrite(&buffer, 1, 1, fp);
        buffer = 0;
        nbits = 0;
    }
}

BitReader::BitReader(FILE *f) : fp(f), buffer(0), nbits(0) {}

int BitReader::getBit() {
    if (nbits == 0) {
        if (fread(&buffer, 1, 1, fp) != 1) return -1;
        nbits = 8;
    }
    int bit = (buffer >> 7) & 1;
    buffer <<= 1;
    nbits--;
    return bit;
}

struct Comparar {
    bool operator()(const NodoHuff *l, const NodoHuff *r) const {
        return l->frecuencia > r->frecuencia;
    }
};

NodoHuff* construirArbol(const unsigned long frec[256]) {
    std::priority_queue<NodoHuff*, std::vector<NodoHuff*>, Comparar> heap;

    for (int i = 0; i < 256; i++) {
        if (frec[i] > 0)
            heap.push(new NodoHuff((unsigned char)i, frec[i], true));
    }
    if (heap.empty()) return nullptr;

    while (heap.size() > 1) {
        NodoHuff *izq = heap.top(); heap.pop();
        NodoHuff *der = heap.top(); heap.pop();

        NodoHuff *padre = new NodoHuff(0,
            izq->frecuencia + der->frecuencia, false);
        padre->izq = izq;
        padre->der = der;
        heap.push(padre);
    }
    return heap.top();
}

void generarCodigos(NodoHuff *raiz, const std::string &prefijo,
                    std::string codigos[256]) {
    if (!raiz) return;
    if (raiz->esHoja) {
        codigos[raiz->simbolo] = prefijo.empty() ? "0" : prefijo;
        return;
    }
    generarCodigos(raiz->izq, prefijo + "0", codigos);
    generarCodigos(raiz->der, prefijo + "1", codigos);
}

void serializarArbol(NodoHuff *raiz, FILE *fp) {
    if (!raiz) return;
    uint8_t marker;
    if (raiz->esHoja) {
        marker = 0x01;
        fwrite(&marker, 1, 1, fp);
        fwrite(&raiz->simbolo, 1, 1, fp);
    } else {
        marker = 0x00;
        fwrite(&marker, 1, 1, fp);
        serializarArbol(raiz->izq, fp);
        serializarArbol(raiz->der, fp);
    }
}

NodoHuff* deserializarArbol(FILE *fp) {
    uint8_t marker;
    if (fread(&marker, 1, 1, fp) != 1) return nullptr;

    if (marker == 0x01) {
        unsigned char s;
        if (fread(&s, 1, 1, fp) != 1) return nullptr;
        return new NodoHuff(s, 0, true);
    } else if (marker == 0x00) {
        NodoHuff *n = new NodoHuff(0, 0, false);
        n->izq = deserializarArbol(fp);
        n->der = deserializarArbol(fp);
        return n;
    }
    return nullptr;
}
