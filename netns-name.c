#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <sys/stat.h>
#include <dirent.h>

#define NETNS_PATH "/var/run/netns"
#define SELF_NS_PATH "/proc/self/ns/net"
#define INIT_NS_PATH "/proc/1/ns/net"

/**
 * Get the inode number of a network namespace.
 */
ino_t get_ns_inode(const char *path) {
    struct stat ns_stat;
    if (stat(path, &ns_stat) != 0) {
        perror("stat");
        return 0;
    }
    return ns_stat.st_ino;
}

int main() {
    ino_t self_inode = get_ns_inode(SELF_NS_PATH);
    ino_t init_inode = get_ns_inode(INIT_NS_PATH);

    // If we're in the root namespace, print nothing and exit
    if (self_inode == init_inode) {
        return 0;
    }

    // Open the network namespace directory
    DIR *dir = opendir(NETNS_PATH);
    if (!dir) {
        perror("opendir");
        return 1;
    }

    struct dirent *entry;
    char path[PATH_MAX];
    struct stat ns_stat;

    // Iterate through named network namespaces
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] == '.') continue; // Skip "." and ".."

        snprintf(path, sizeof(path), "%s/%s", NETNS_PATH, entry->d_name);
        if (stat(path, &ns_stat) == 0 && ns_stat.st_ino == self_inode) {
            printf("%s\n", entry->d_name);
            closedir(dir);
            return 0;
        }
    }

    // If no name was found, print nothing
    closedir(dir);
    return 0;
}
