
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vfs.h"

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Uso: %s <imagen.vfs> <nombre_archivo>\n", argv[0]);
        return 1;
    }

    const char *image_path = argv[1];
    const char *filename = argv[2];

    if (!name_is_valid(filename)) {
        fprintf(stderr, "Error: nombre de archivo inválido\n");
        return 1;
    }

    struct superblock sb;
    if (read_superblock(image_path, &sb) != 0) {
        fprintf(stderr, "Error: no se pudo leer el superbloque\n");
        return 1;
    }

    if (sb.magic != MAGIC_NUMBER) {
        fprintf(stderr, "Error: imagen no válida (magic number incorrecto)\n");
        return 1;
    }

    if (dir_lookup(image_path, filename) != -1) {
        fprintf(stderr, "Error: ya existe un archivo con ese nombre\n");
        return 1;
    }

    int new_inode_nbr = create_empty_file_in_free_inode(image_path, DEFAULT_PERM);
    if (new_inode_nbr < 0) {
        fprintf(stderr, "Error: no hay inodos libres para crear el archivo\n");
        return 1;
    }

    if (add_dir_entry(image_path, filename, new_inode_nbr) != 0) {
        fprintf(stderr, "Error: no se pudo agregar el archivo al directorio raíz\n");
        return 1;
    }

    printf("Archivo '%s' creado exitosamente en la imagen '%s'.\n", filename, image_path);
    return 0;
}