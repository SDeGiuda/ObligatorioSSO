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

    // Leer y validar superbloque
    struct superblock sb;
    if (read_superblock(image_path, &sb) != 0) {
        fprintf(stderr, "Error: no se pudo leer el superbloque\n");
        return 1;
    }

    if (sb.magic != MAGIC_NUMBER) {
        fprintf(stderr, "Error: imagen no válida (magic number incorrecto)\n");
        return 1;
    }

    // Iterar sobre cada archivo dado como argumento
    for (int i = 2; i < argc; i++) {
        const char *filename = argv[i];

        // Validar nombre
        if (!name_is_valid(filename)) {
            fprintf(stderr, "Error: nombre inválido '%s'\n", filename);
            continue;
        }

        // Buscar inodo
        int inode_nbr = dir_lookup(image_path, filename);
        if (inode_nbr == -1) {
            fprintf(stderr, "Error: el archivo '%s' no existe en la imagen\n", filename);
            continue;
        }

        // Leer inodo
        struct inode file_inode;
        if (read_inode(image_path, inode_nbr, &file_inode) != 0) {
            fprintf(stderr, "Error: no se pudo leer el inodo de '%s'\n", filename);
            continue;
        }

        // Verificar que sea un archivo
        if ((file_inode.mode & INODE_MODE_FILE) != INODE_MODE_FILE) {
            fprintf(stderr, "Error: '%s' no es un archivo regular\n", filename);
            continue;
        }

        // Truncar su contenido
        if (inode_trunc_data(image_path, &file_inode) != 0) {
            fprintf(stderr, "Error: no se pudo truncar '%s'\n", filename);
            continue;
        }

        // Liberar inodo
        if (free_inode(image_path, inode_nbr) != 0) {
            fprintf(stderr, "Error: no se pudo liberar el inodo de '%s'\n", filename);
            continue;
        }

        // Eliminar entrada del directorio
        if (remove_dir_entry(image_path, filename) != 0) {
            fprintf(stderr, "Error: no se pudo eliminar '%s' del directorio\n", filename);
            continue;
        }

        printf("Archivo '%s' eliminado correctamente.\n", filename);
    }

    return 0;
}