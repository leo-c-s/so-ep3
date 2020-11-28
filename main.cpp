#include "fs_sim.hpp"

int main() {
    Filesystem *f = new Filesystem("filesystem.txt");

    delete f;

    return 0;
}
