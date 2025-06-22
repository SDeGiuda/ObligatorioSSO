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

    // Leer el superbloque
    struct superblock sb;
    if (read_superblock(image_path, &sb) != 0) {
        fprintf(stderr, "Error: no se pudo leer el superbloque\n");
        return 1;
    }

    // Verificar el número mágico
    if (sb.magic != MAGIC_NUMBER) {
        fprintf(stderr, "Error: imagen no válida (magic number incorrecto)\n");
        return 1;
    }

    int status = 0; // 0 = éxito total, 1 = hubo errores parciales

    // Procesar cada archivo
    for (int i = 2; i < argc; i++) {
        const char *filename = argv[i];

        if (!name_is_valid(filename)) {
            fprintf(stderr, "Error: nombre de archivo inválido: '%s'\n", filename);
            status = 1;
            continue;
        }

        if (dir_lookup(image_path, filename) != 0) {
            fprintf(stderr, "Error: ya existe un archivo con ese nombre: '%s'\n", filename);
            status = 1;
            continue;
        }

        int new_inode_nbr = create_empty_file_in_free_inode(image_path, DEFAULT_PERM);
        if (new_inode_nbr < 0) {
            fprintf(stderr, "Error: no hay inodos libres para crear el archivo '%s'\n", filename);
            status = 1;
            continue;
        }

        if (add_dir_entry(image_path, filename, new_inode_nbr) != 0) {
            fprintf(stderr, "Error: no se pudo agregar el archivo '%s' al directorio raíz\n", filename);
            free_inode(image_path,new_inode_nbr);
            status = 1;
            continue;
        }

        if (dir_lookup(image_path, filename) != 0) {
             printf("Archivo '%s' creado exitosamente en la imagen '%s'.\n", filename, image_path);
        } else {
            printf("❌ Verificación: archivo '%s' NO aparece en el directorio (algo falló).\n", filename);
}
    }
    return status;
}