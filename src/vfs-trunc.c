#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vfs.h"

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Uso: %s <imagen.vfs> <archivo1> [archivo2] ...\n", argv[0]);
        return 1;
    }

    const char *image_path = argv[1];

    // Leer superbloque
    struct superblock sb;
    if (read_superblock(image_path, &sb) != 0) {
        fprintf(stderr, "Error: no se pudo leer el superbloque\n");
        return 1;
    }

    if (sb.magic != MAGIC_NUMBER) {
        fprintf(stderr, "Error: imagen no válida (magic number incorrecto)\n");
        return 1;
    }

    int status = 0;

    for (int i = 2; i < argc; i++) {
        const char *filename = argv[i];

        if (!name_is_valid(filename)) {
            fprintf(stderr, "Error: nombre inválido: '%s'\n", filename);
            status = 1;
            continue;
        }

        int inode_nbr = dir_lookup(image_path, filename);
        if (inode_nbr == -1) {
            fprintf(stderr, "Error: el archivo '%s' no existe\n", filename);
            status = 1;
            continue;
        }

        struct inode file_inode;
        if (read_inode(image_path, inode_nbr, &file_inode) != 0) {
            fprintf(stderr, "Error: no se pudo leer el inodo de '%s'\n", filename);
            status = 1;
            continue;
        }

        if ((file_inode.mode & INODE_MODE_FILE) != INODE_MODE_FILE) {
            fprintf(stderr, "Error: '%s' no es un archivo regular\n", filename);
            status = 1;
            continue;
        }

        if (inode_trunc_data(image_path, &file_inode) != 0) {
            fprintf(stderr, "Error: no se pudo truncar '%s'\n", filename);
            status = 1;
            continue;
        }

        if (write_inode(image_path, inode_nbr, &file_inode) != 0) {
            fprintf(stderr, "Error: no se pudo guardar el inodo truncado de '%s'\n", filename);
            status = 1;
            continue;
        }

        printf("Archivo '%s' truncado correctamente.\n", filename);
    }

    return status;
}