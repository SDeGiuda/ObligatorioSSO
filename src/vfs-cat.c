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

        // Validar nombre
        if (!name_is_valid(filename)) {
            fprintf(stderr, "Error: nombre de archivo inválido: '%s'\n", filename);
            status = 1;
            continue;
        }

        // Buscar inodo
        int inode_nbr = dir_lookup(image_path, filename);
        if (inode_nbr == -1) {
            fprintf(stderr, "Error: el archivo '%s' no existe\n", filename);
            status = 1;
            continue;
        }

        // Leer inodo
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

        if (file_inode.size == 0) {
            continue; // archivo vacío, no imprime nada
        }

        // Leer datos
        char *buffer = malloc(file_inode.size + 1);
        if (!buffer) {
            fprintf(stderr, "Error: no se pudo asignar memoria para '%s'\n", filename);
            status = 1;
            continue;
        }
    

        if (inode_read_data(image_path, inode_nbr, buffer, file_inode.size, 0) <= 0) {
            fprintf(stderr, "Error: no se pudo leer el contenido de '%s'\n", filename);
            free(buffer);
            status = 1;
            continue;
        }

        buffer[file_inode.size] = '\0'; // aseguramos fin de string
        printf("%s", buffer);           // mostramos el contenido
        free(buffer);
    }

    return status;
}