#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vfs.h"

struct file_info {
    struct inode in;
    uint32_t inode_number;
    char name[FILENAME_MAX_LEN];
};

int cmp_name(const void *a, const void *b) {
    const struct file_info *fa = a, *fb = b;
    return strcmp(fa->name, fb->name);
}

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

     // Capacidad máxima basada en todos los bloques del directorio raíz
    int max_entries = root_inode.blocks * DIR_ENTRIES_PER_BLOCK;
    struct file_info *archivos = calloc(max_entries, sizeof(struct file_info));
    if (!archivos) {
        fprintf(stderr, "Error: no se pudo asignar memoria\n");
        return 1;
    }

    int count = 0;

    // Leer todas las entradas desde los bloques del directorio raíz
    for (int b = 0; b < root_inode.blocks; b++) {
        int block_num = get_block_number_at(image_path, &root_inode, b);
        if (block_num <= 0) continue;

        uint8_t data_buf[BLOCK_SIZE];
        if (read_block(image_path, block_num, data_buf) != 0) continue;

        struct dir_entry *entries = (struct dir_entry *)data_buf;

        for (int i = 0; i < DIR_ENTRIES_PER_BLOCK; i++) {
            if (entries[i].inode == 0) continue;

            archivos[count].inode_number = entries[i].inode;
            strncpy(archivos[count].name, entries[i].name, FILENAME_MAX_LEN);
            archivos[count].name[FILENAME_MAX_LEN - 1] = '\0';

            if (read_inode(image_path, archivos[count].inode_number, &archivos[count].in) != 0) {
                archivos[count].inode_number = 0; // marcar inválido
            }

            count++;
        }
    }

    if (count == 0) {
        printf("El directorio está vacío.\n");
        free(archivos);
        return 0;
    }

    qsort(archivos, count, sizeof(struct file_info), cmp_name);

    for (int i = 0; i < count; i++) {
        if (archivos[i].inode_number != 0) {
            print_inode(&archivos[i].in, archivos[i].inode_number, archivos[i].name);
        }
    }

    free(archivos);
    return 0;
}