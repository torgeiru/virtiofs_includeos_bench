#include <os>
#include <hal/machine.hpp>
#include <hw/vfs_device.hpp>
#include <fcntl.h>
#include <chrono>
#include <string>
#include <format>

#define FILE_SIZE (1024 * 1024)  // 1MiB
#define SMALL_CHUNK 100
#define MEDIUM_CHUNK 1024
#define LARGE_CHUNK (64 * 1024)

struct BenchmarkResult {
    int chunk_size;
    int run_number;
    double read_time_ms;
    double throughput_mbps;
};

void write_result(hw::VFS_device& vfs_device, auto data_fh, const BenchmarkResult& result) {
    std::string results_str  = std::format("{},{},{:.3f},{:.3f}\n",
        result.chunk_size,
        result.run_number,
        result.read_time_ms,
        result.throughput_mbps
    );


    vfs_device.write(data_fh, results_str.data(), results_str.size());
}

void benchmark_chunk_size(hw::VFS_device& vfs_device, int chunk_size, auto data_fh) {
    void* buffer = malloc(chunk_size);
    if (buffer == NULL) {
        os::panic("Could not allocate chunk buffer!");
    }

    for (int run = 0; run < 30; run++) {
        // Open test file and get file handle
        auto test_fh = vfs_device.open("test_file.bin", O_RDONLY, 0);
        printf("File handle is %zu\n", test_fh);

        auto start = std::chrono::high_resolution_clock::now();

        // Read entire file in chunks
        int total_read = 0;
        int bytes_read;
        while (total_read < FILE_SIZE) {
            int to_read = (FILE_SIZE - total_read > chunk_size) ? chunk_size : (FILE_SIZE - total_read);
            bytes_read = vfs_device.read(test_fh, buffer, to_read);
            if (bytes_read <= 0) break;
            total_read += bytes_read;
        }

        auto stop = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> start_stop_diff = stop - start;

        vfs_device.close(test_fh);

        printf("Read %zu bytes\n", total_read);

        if (total_read != FILE_SIZE) {
            os::panic("Could not read entire file!");
        }

        // Calculate metrics
        double read_time_ms = start_stop_diff.count() * 1000.0;
        double throughput_mbps = (FILE_SIZE / (1024.0 * 1024.0)) / (read_time_ms / 1000.0);

        // Write result
        BenchmarkResult result = {chunk_size, run + 1, read_time_ms, throughput_mbps};
        write_result(vfs_device, data_fh, result);
    }

    free(buffer);
}

int main() {
    auto& vfs_device = os::machine().get<hw::VFS_device>(0);

    // Open data output file and get file handle
    auto data_fh = vfs_device.open("benchmark_results.csv", O_WRONLY | O_TRUNC, 0644);

    // Write CSV header
    const char* header = "chunk_size,run_number,read_time_ms,throughput_mbps\n";
    vfs_device.write(data_fh, (void*)header, strlen(header));

    printf("Starting sequential read benchmark...\n");

    // Benchmark small reads (100 bytes)
    printf("Testing small reads (100 bytes)...\n");
    benchmark_chunk_size(vfs_device, SMALL_CHUNK, data_fh);

    // Benchmark medium reads (1K)
    printf("Testing medium reads (1K)...\n");
    benchmark_chunk_size(vfs_device, MEDIUM_CHUNK, data_fh);

    // Benchmark large reads (64K)
    printf("Testing large reads (64K)...\n");
    benchmark_chunk_size(vfs_device, LARGE_CHUNK, data_fh);

    vfs_device.close(data_fh);

    printf("Benchmark completed. Results saved to benchmark_results.csv\n");
    os::shutdown();
}
