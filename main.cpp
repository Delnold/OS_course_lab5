#include <iostream>
    #include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>

// Function prototypes
int f(int x);
int g(int x);

int main() {
    // Open shared memory file
    int fd = open("shared_memory_file", O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    if (fd == -1) {
        std::cerr << "Error opening shared memory file" << std::endl;
        return 1;
    }

    // Resize the file to hold two integers
    if (ftruncate(fd, sizeof(int) * 2) == -1) {
        std::cerr << "Error resizing shared memory file" << std::endl;
        close(fd);
        return 1;
    }

    // Memory map the file
    int* shared_memory = static_cast<int*>(mmap(NULL, sizeof(int) * 2, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0));
    if (shared_memory == MAP_FAILED) {
        std::cerr << "Error mapping shared memory" << std::endl;
        close(fd);
        return 1;
    }

    // Create child process for f(x)
    pid_t pid_f = fork();
    if (pid_f == -1) {
        std::cerr << "Error forking process for f(x)" << std::endl;
        munmap(shared_memory, sizeof(int) * 2);
        close(fd);
        return 1;
    } else if (pid_f == 0) {
        // Child process for f(x)
        shared_memory[0] = f(10); // Replace 10 with the desired value of x
        _exit(0);
    }

    // Create child process for g(x)
    pid_t pid_g = fork();
    if (pid_g == -1) {
        std::cerr << "Error forking process for g(x)" << std::endl;
        munmap(shared_memory, sizeof(int) * 2);
        close(fd);
        return 1;
    } else if (pid_g == 0) {
        // Child process for g(x)
        shared_memory[1] = g(-5); // Replace -5 with the desired value of x
        _exit(0);
    }

    // Wait for both child processes to finish
    waitpid(pid_f, NULL, 0);
    waitpid(pid_g, NULL, 0);

    // Compute f(x) * g(x)
    int result = shared_memory[0] * shared_memory[1];

    // Print the result
    std::cout << "Result: " << result << std::endl;

    // Unmap and close shared memory
    munmap(shared_memory, sizeof(int) * 2);
    close(fd);

    return 0;
}
