#include <stdio.h>
#include <string.h>
#include "vfs.h"

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Uso: %s <imagen.vfs> <archivo> <texto>\n", argv[0]);
        return 1;
    }

    const char *image_path = argv[1];
    const char *filename = argv[2];
    const char *texto = argv[3];

    int inode_nbr = dir_lookup(image_path, filename);
    if (inode_nbr == -1) {
        fprintf(stderr, "Error: el archivo '%s' no existe\n", filename);
        return 1;
    }

    if (inode_write_data(image_path, inode_nbr, (void *)texto, strlen(texto), 0) != 0) {
        fprintf(stderr, "Error al escribir en '%s'\n", filename);
        return 1;
    }

    printf("Texto escrito en '%s' exitosamente.\n", filename);
    return 0;
}