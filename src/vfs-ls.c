#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vfs.h"


int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Uso: %s <imagen.vfs>\n", argv[0]);
        return 1;
    }

    const char *image_path = argv[1];

    struct superblock sb;
    if (read_superblock(image_path, &sb) != 0) {
        fprintf(stderr, "Error: no se pudo leer el superbloque\n");
        return 1;
    }

    if (sb.magic != MAGIC_NUMBER) {
        fprintf(stderr, "Error: imagen no válida (magic number incorrecto)\n");
        return 1;
    }

    struct inode root_inode;
    if (read_inode(image_path, ROOTDIR_INODE, &root_inode) != 0) {
        fprintf(stderr, "Error: no se pudo leer el inodo del directorio raíz\n");
        return 1;
    }

    int num_entries = root_inode.size / sizeof(struct dir_entry);
    if (num_entries == 0) {
        printf("El directorio está vacío.\n");
        return 0;
    }

    struct dir_entry *entries = malloc(root_inode.size);
    if (!entries) {
        fprintf(stderr, "Error: no se pudo asignar memoria para las entradas\n");
        return 1;
    }


    for (int i = 1; i < num_entries; i++) {
        struct inode file_inode;
        
        if (entries[i].inode == 0) continue;
        if (read_inode(image_path, entries[i].inode, &file_inode) != 0) {
            fprintf(stderr, "Advertencia: no se pudo leer el inodo %u\n", entries[i].inode);
            continue;
        }

        print_inode(&file_inode, entries[i].inode, entries[i].name);
    }

    return 0;
}