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

    
    if (root_inode.blocks == 0) {
        printf("El directorio está vacío.\n");
        return 0;
    }

    int encontrado = 0;

    // Recorre todos los bloques del directorio raíz
    for (int b = 0; b < root_inode.blocks; b++) {
        int block_num = get_block_number_at(image_path, &root_inode, b);
        if (block_num <= 0) {
            fprintf(stderr, "Error: bloque inválido (%d) en el directorio raíz (posición lógica %d)\n", block_num, b);
            continue;
        }

        uint8_t data_buf[BLOCK_SIZE];
        if (read_block(image_path, block_num, data_buf) != 0) {
            fprintf(stderr, "Error: no se pudo leer el bloque %d\n", block_num);
            continue;
        }

        struct dir_entry *entries = (struct dir_entry *)data_buf;

        for (int i = 0; i < DIR_ENTRIES_PER_BLOCK; i++) {
            if (entries[i].inode == 0) continue;

            struct inode file_inode;
            if (read_inode(image_path, entries[i].inode, &file_inode) != 0) {
                fprintf(stderr, "Advertencia: no se pudo leer el inodo %u\n", entries[i].inode);
                continue;
            }

            print_inode(&file_inode, entries[i].inode, entries[i].name);
            encontrado = 1;
        }
    }

    if (!encontrado) {
        printf("El directorio está vacío.\n");
    }

    return 0;
}