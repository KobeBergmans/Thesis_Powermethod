/**
 * @file driver_input.cpp
 * @author Kobe Bergmans (kobe.bergmans@student.kuleuven.be)
 * @brief Main executable for power method comparison with MM input files
 * @version 0.1
 * @date 2022-10-18
 */

#include <iostream>
#include <algorithm>
#include <time.h>
#include <string>
#include <cmath>

#include "CRS.hpp"
#include "CRSOMP.hpp"
#include "CRSTBB.hpp"
#include "CRSTBBGraph.hpp"
#include "CRSTBBGraphPinned.hpp"
#include "CRSThreadPool.hpp"
#include "CRSThreadPoolPinned.hpp"
#include "Utill/VectorUtill.hpp"
#include "Utill/TripletToCRS.hpp"
#include "Triplet.hpp"

#include <boost/algorithm/string/predicate.hpp>

#include "omp.h"
#include "oneapi/tbb.h"

void printErrorMsg() {
    std::cout << "You need to provide the correct command line arguments:" << std::endl;
    std::cout << "  1° Filename of Matrix market / Kronecker graph input file";
    std::cout << " (Kronecker input file must start with matrix size in power of 2 and boolean indicating the fill in separated by an underscore)" << std::endl;
    std::cout << "  2° Amount of times the power algorithm is executed" << std::endl;
    std::cout << "  3° Amount of warm up runs for the power algorithm (not timed)" << std::endl;
    std::cout << "  4° Amount of iterations in the power method algorithm" << std::endl;
    std::cout << "  5° Method to use:" << std::endl;
    std::cout << "     1) Standard CRS (sequential)" << std::endl;
    std::cout << "     2) CRS parallelized using OpenMP" << std::endl;
    std::cout << "     3) CRS parallelized using TBB" << std::endl;
    std::cout << "     4) CRS parallelized using TBB graphs" << std::endl;
    std::cout << "     5) CRS parallelized using TBB graphs with each node pinned to a CPU" << std::endl;
    std::cout << "     6) CRS parallelized using Boost Thread Pool" << std::endl;
    std::cout << "     7) CRS parallelized using Boost Thread Pool with functions pinned to a CPU" << std::endl;
    std::cout << "  6° Amount of threads (only for a parallel method).";
    std::cout << " -1 lets the program choose the amount of threads arbitrarily" << std::endl;
    std::cout << "  7° Amount of partitions the matrix is split up into (only for method 4, 5, 6 and 7)" << std::endl;
}

template<typename T, typename int_type>
pwm::SparseMatrix<T, int_type>* selectType(int method, int threads) {
    switch (method) {
        case 1:
            return new pwm::CRS<T, int_type>(threads);

        case 2:
            return new pwm::CRSOMP<T, int_type>(threads);

        case 3:
            return new pwm::CRSTBB<T, int_type>(threads);

        case 4:
            return new pwm::CRSTBBGraph<T, int_type>(threads);

        case 5:
            return new pwm::CRSTBBGraphPinned<T, int_type>(threads);

        case 6:
            return new pwm::CRSThreadPool<T, int_type>(threads);

        case 7:
            return new pwm::CRSThreadPoolPinned<T, int_type>(threads);
        
        default:
            return NULL;
    }
}

int main(int argc, char** argv) {
    double start, stop, time; 

    if (argc < 6) {
        printErrorMsg();
        return -1;
    }

    std::string input_file = argv[1];
    int iter = std::stoi(argv[2]);
    int warm_up = std::stoi(argv[3]);
    int pwm_iter = std::stoi(argv[4]);

    int method = std::stoi(argv[5]);
    int threads = 0;
    int partitions = 0;
    if (method > 1 && argc < 6) {
        // No amount of threads specified
        printErrorMsg();
        return -1;
    } else if (method > 1) {
        threads = std::stoi(argv[6]);
    }

    if ((method == 4 || method == 5 || method == 6 || method == 7) && argc < 7) {
        printErrorMsg();
        return -1;
    } else if (method == 4 || method == 5 || method == 6 || method == 7) {
        partitions = std::stoi(argv[7]);
    }
    
    // Select method
    pwm::SparseMatrix<double, int>* test_mat = selectType<double, int>(method, threads);

    if (test_mat == NULL) {
        printErrorMsg();
        return -1;
    }

    // Input matrix & initialize vectors
    start = omp_get_wtime();
    pwm::Triplet<double, int> input_mat;

    if (boost::algorithm::ends_with(input_file, ".mtx")) {
        input_mat.loadFromMM(input_file);
    } else if (boost::algorithm::ends_with(input_file, ".bin")) {
        int start = input_file.find("/");
        int first_ = input_file.find("_");
        int mat_size = std::pow(2, std::stoi(input_file.substr(start+1, first_-start-1)));
        bool fill_in = std::stoi(input_file.substr(first_+1, 1));
        input_mat.loadFromKronecker(input_file, mat_size, fill_in);
    }
    int mat_size = input_mat.row_size;

    test_mat->loadFromTriplets(input_mat, partitions);
    
    double* x = new double[mat_size];
    double* y = new double[mat_size];
    std::fill(x, x+mat_size, 1.);

    stop = omp_get_wtime();
    time = (stop - start) * 1000;
    std::cout << "Time to set up datastructures: " << time << "ms" << std::endl;

    // Do warm up iterations
    for (int i = 0; i < warm_up; ++i) {
        std::fill(x, x+mat_size, 1.);
        test_mat->powerMethod(x, y, pwm_iter);
    }

    // Solve power method an amount of time
    start = omp_get_wtime();
    for (int i = 0; i < iter; ++i) {
        std::fill(x, x+mat_size, 1.);
        test_mat->powerMethod(x, y, pwm_iter);
    }
    stop = omp_get_wtime();
    time = (stop - start) * 1000;
    std::cout << "Time (ms) to get " << iter << " executions: " << time << "ms" << std::endl;

#ifndef NDEBUG
    std::cout << "Result for checking measures: " << std::endl;
    if (pwm_iter % 2 == 0) {
        pwm::printVector(x, mat_size);
    } else {
        pwm::printVector(y, mat_size);
    }
    
#endif
    
    return 0;
}