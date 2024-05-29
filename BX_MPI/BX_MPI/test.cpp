//test
/*#include <mpi.h>
#include <iostream>

int main(int argc, char* argv[]) {
    int rank, size;

    // 初始化 MPI 环境
    MPI_Init(&argc, &argv);

    // 获取当前进程的秩
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    // 获取总进程数
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // 输出当前进程的秩和总进程数
    std::cout << "Greetings from process " << rank << " of " << size << std::endl;

    // 结束 MPI 环境
    MPI_Finalize();

    return 0;
}
*/

#include <mpi.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>
#include <algorithm>
#include <cstdint>
using namespace std;
// 读取小端格式的四字节无符号整数
uint32_t read_uint32_le(std::ifstream& stream) {
    uint32_t value;
    char bytes[4];
    stream.read(bytes, 4);
    value = (static_cast<uint32_t>(static_cast<unsigned char>(bytes[3])) << 24) |
        (static_cast<uint32_t>(static_cast<unsigned char>(bytes[2])) << 16) |
        (static_cast<uint32_t>(static_cast<unsigned char>(bytes[1])) << 8) |
        static_cast<uint32_t>(static_cast<unsigned char>(bytes[0]));
    return value;
}

// 读取一个整数数组
std::vector<uint32_t> read_array(std::ifstream& stream) {
    uint32_t length = read_uint32_le(stream);
    std::vector<uint32_t> array(length);
    for (uint32_t i = 0; i < length; ++i) {
        array[i] = read_uint32_le(stream);
    }
    return array;
}

// Find the intersection of two sorted arrays
vector<uint32_t> findIntersection(const vector<uint32_t>& array1, const vector<uint32_t>& array2) {
    vector<uint32_t> result;
    for (uint32_t element : array1) {
        if (binary_search(array2.begin(), array2.end(), element)) {
            result.push_back(element);
        }
    }
    return result;
}
int main(int argc, char** argv) {
    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    vector<uint32_t> local_array1, local_array2;
    ifstream file;
    if (rank == 0) {
        file.open("D:/MyVS/BX_LW/ExpIndex", ios::binary);
        if (!file) {
            cerr << "Cannot open file\n";
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
        file.seekg(32832, ios::beg);
        local_array1 = read_array(file);
        local_array2 = read_array(file);
        file.close();
    }

    vector<uint32_t> local_result = findIntersection(local_array1, local_array2);
    vector<uint32_t> global_result;
    int local_size = local_result.size();
    vector<int> sizes(size);
    vector<int> displs(size);

    MPI_Gather(&local_size, 1, MPI_INT, sizes.data(), 1, MPI_INT, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        int total_size = 0;
        for (int i = 0; i < size; ++i) {
            displs[i] = total_size;
            total_size += sizes[i];
        }
        global_result.resize(total_size);
    }

    MPI_Gatherv(local_result.data(), local_size, MPI_UINT32_T,
        global_result.data(), sizes.data(), displs.data(), MPI_UINT32_T, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        ofstream resultFile("D:/MyVS/BX_MPI/result.txt", ios::app);
        if (!resultFile.is_open()) {
            cerr << "Cannot open file to write results\n";
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
        for (uint32_t value : global_result) {
            resultFile << value << ' ';
        }
        resultFile.close();
    }

    MPI_Finalize();
    return 0;
}


/*int main() {
    // double arg = 0;
    for (int j = 0; j < 1000; j++) {
        std::ifstream file("D:/MyVS/BX_MPI/ExpIndex", std::ios::binary);
        if (!file) {
            std::cerr << "无法打开文件" << std::endl;
            return 1;
        }
        file.seekg(32832, std::ios::beg);
        std::vector<uint32_t> array = read_array(file);
        std::vector<float> floatArray;
        // 转换整数数组到浮点数组
        for (int value : array) {
            floatArray.push_back(static_cast<float>(value));
        }
        uint32_t length = floatArray.size();
        std::vector<float> compress(length);
        compress[0] = floatArray[0];
        //auto beforeTime = std::chrono::steady_clock::now();
        for (uint32_t i = 1; i < length; i++)
        {
            compress[i] = array[i] - array[i - 1];
        }
        // auto afterTime = std::chrono::steady_clock::now();
         //double time = std::chrono::duration<double>(afterTime - beforeTime).count();
        // arg += time;
        // std::cout << " time=" << time << "seconds" << std::endl;
         /*std::ofstream f("D:/MyVS/BXFOR/compress.txt", std::ios::app);
         if (!f.is_open()) {
             std::cerr << "无法打开文件" << std::endl;
             return 0;
         }
         for (float value : compress) {
             f << value << ' ';
         }
         f.close();
         std::ofstream f2("D:/MyVS/BXFOR/array.txt", std::ios::app);
         if (!f2.is_open()) {
             std::cerr << "无法打开文件" << std::endl;
             return 0;
         }
         for (uint32_t value : array) {
             f2 << value << ' ';
         }
         f2.close();

         file.close();
    }
    // std::cout << " time=" << arg/100 << "seconds" << std::endl;

    return 0;
}*/