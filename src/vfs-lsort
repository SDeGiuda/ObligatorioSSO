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

    int num_entries = root_inode.size / sizeof(struct dir_entry);
    if (num_entries == 0) {
        printf("El directorio está vacío.\n");
        return 0;
    }

    struct dir_entry entries[num_entries];
    if (inode_read_data(image_path, ROOTDIR_INODE, entries, root_inode.size, 0) != 0) {
        fprintf(stderr, "Error: no se pudo leer el contenido del directorio raíz\n");
        return 1;
    }

    struct file_info archivos[num_entries];

    for (int i = 0; i < num_entries; i++) {
        archivos[i].inode_number = entries[i].inode;
        strncpy(archivos[i].name, entries[i].name, FILENAME_MAX_LEN);
        archivos[i].name[FILENAME_MAX_LEN - 1] = '\0';

        if (read_inode(image_path, archivos[i].inode_number, &archivos[i].in) != 0) {
            fprintf(stderr, "Advertencia: no se pudo leer el inodo %u\n", archivos[i].inode_number);
            archivos[i].inode_number = 0; // marcar inválido
        }
    }

    qsort(archivos, num_entries, sizeof(struct file_info), cmp_name);

    for (int i = 0; i < num_entries; i++) {
        if (archivos[i].inode_number != 0) {
            print_inode(&archivos[i].in, archivos[i].inode_number, archivos[i].name);
        }
    }

    return 0;
}