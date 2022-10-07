/**
 * @file CRSTBBGraphPinned.hpp
 * @author Kobe Bergmans (kobe.bergmans@student.kuleuven.be)
 * @brief Compressed Row Storage matrix class using TBB Graph where each node is pinned to a CPU
 * @version 0.1
 * @date 2022-10-07
 * 
 * Includes method to generate CRS matrix obtained from discrete 2D poisson equation
 */

#ifndef PWM_CRSTBBGRAPHPINNED_HPP
#define PWM_CRSTBBGRAPHPINNED_HPP

#include <vector>
#include <iostream>
#include <cassert>
#include <algorithm>
#include <tuple>
#include <thread>

#include "SparseMatrix.hpp"
#include "VectorUtill.hpp"

#include "oneapi/tbb.h"

namespace tbb = oneapi::tbb;
namespace flow = oneapi::tbb::flow;

namespace pwm {
    template<typename T, typename int_type>
    class CRSTBBGraphPinned: public pwm::SparseMatrix<T, int_type> {
        protected:
            // Array of row start arrays for the CRS format. 1 for each thread.
            int_type** row_start;
            
            // Array of column index array for the CRS format. 1 for each thread
            int_type** col_ind;

            // Array of data array which stores the actual nonzeros. 1 for each thread.
            T** data_arr;

            // Threads
            int threads;

            // Amount of partitions
            int partitions;

            // Graph for TBB nodes
            flow::graph g;

            // TBB mv nodes list
            std::vector<flow::function_node<std::tuple<const T*, T*>, int>> mv_func_list;

            // TBB normalize nodes list
            std::vector<flow::function_node<std::tuple<T*, T*, T>, int>> norm_func_list;

            // Global threads limit
            tbb::global_control global_limit;

        public:
            // Base constructor
            CRSTBBGraphPinned() {}

            // Base constructor
            CRSTBBGraphPinned(int threads):
            threads(threads),
            global_limit(tbb::global_control::max_allowed_parallelism, threads) {}

            /**
             * @brief Fill the given matrix as a 2D discretized poisson matrix with equal discretization steplength in x and y
             * 
             * The matrix is partitioned for each thread 
             * This is done by splitting the rows equally, this is only optimal because every row has approximately the same elements.
             * 
             * Then each Matrix part gets its own TBB function_node which is used to calculate the matrix vector product of the given partition.
             * 
             * @param m The amount of discretization steps in the x direction
             * @param n The amount of discretization steps in the y direction
             * @param partitions_am The amount of partitions the matrix is partitioned in
             */
            void generatePoissonMatrix(const int_type m, const int_type n, const int partitions_am) {
                this->noc = m*n;
                this->nor = m*n;

                this->nnz = n*(m+2*(m-1)) + 2*(n-1)*m;

                partitions = partitions_am;

                row_start = new int_type*[partitions];
                col_ind = new int_type*[partitions];
                data_arr = new T*[partitions];

                // Generate data for each thread
                int_type am_rows = std::round(m*n/partitions);
                int_type first_row = 0;
                int_type last_row = 0;
                int_type thread_rows;
                int cpu_count = std::thread::hardware_concurrency();
                int max_threads = std::min(threads, cpu_count);
                for (int i = 0; i < partitions; ++i) {
                    first_row = last_row;

                    if (i == partitions - 1) last_row = m*n;
                    else last_row = first_row + am_rows;
                    thread_rows = last_row - first_row;

                    // Generate datastructures for this thread CRS
                    // TODO: The size of data_arr & col_ind is not exactly right
                    data_arr[i] = new T[5*thread_rows];
                    row_start[i] = new int_type[thread_rows+1];
                    col_ind[i] = new int_type[5*thread_rows];
                    
                    // Fill CRS matrix for given thread
                    pwm::fillPoisson(data_arr[i], row_start[i], col_ind [i], m, n, first_row, last_row);

                    // Create mv node for this thread
                    flow::function_node<std::tuple<const T*, T*>, int> mv_node(g, 1, [=](std::tuple<const T*, T*> input) -> int {
                        // Put the current thread on the right cpu
                        cpu_set_t *mask;
                        mask = CPU_ALLOC(1);
                        auto mask_size = CPU_ALLOC_SIZE(1);
                        CPU_ZERO_S(mask_size, mask);
                        CPU_SET_S(i % max_threads, mask_size, mask);
                        if (sched_setaffinity(0, mask_size, mask)) {
                            std::cout << "Error in setAffinity" << std::endl;
                        }

                        const T* x = std::get<0>(input);
                        T* y = std::get<1>(input);

                        int_type j;
                        for (int_type l = 0; l < thread_rows; ++l) {
                            T sum = 0;
                            for (int_type k = row_start[i][l]; k < row_start[i][l+1]; ++k) {
                                j = col_ind[i][k];
                                sum += data_arr[i][k]*x[j];
                            }
                            y[l+first_row] = sum;
                        }

                        return 0;
                    });

                    mv_func_list.push_back(mv_node);

                    // Create normalize node for this thread
                    flow::function_node<std::tuple<T*, T*, T>, int> norm_node(g, 1, [=](std::tuple<T*, T*, T> input) -> int {
                        // Put the current thread on the right cpu
                        cpu_set_t *mask;
                        mask = CPU_ALLOC(1);
                        auto mask_size = CPU_ALLOC_SIZE(1);
                        CPU_ZERO_S(mask_size, mask);
                        CPU_SET_S(i % max_threads, mask_size, mask);
                        if (sched_setaffinity(0, mask_size, mask)) {
                            std::cout << "Error in setAffinity" << std::endl;
                        }

                        T* x = std::get<0>(input);
                        T* y = std::get<1>(input);
                        T norm = std::get<2>(input);

                        for (int_type l = 0; l < thread_rows; ++l) {
                            y[l+first_row] /= norm;
                            x[l+first_row] = y[l+first_row];
                        }

                        return 0;
                    });

                    norm_func_list.push_back(norm_node);
                }
            }

            /**
             * @brief Matrix vector product Ax = y
             * 
             * The loop is parallelized using different graph nodes from TBB for each thread.
             * 
             * @param x Input vector
             * @param y Output vector
             */
            void mv(const T* x, T* y) {   
                for (int i = 0; i < partitions; ++i) {
                    mv_func_list[i].try_put(std::make_tuple(x,y));
                }
                
                g.wait_for_all();
            }

            /**
             * @brief Power method: Executes matrix vector product repeatedly to get the dominant eigenvector.
             * 
             * Loop is parallelized using parallel_for from TBB
             * 
             * @param x Input vector to start calculation, contains the output at the end of the algorithm
             * @param y Temporary vector to store calculations
             * @param it Amount of iterations for the algorithm
             */
            void powerMethod(T* x, T* y, const int_type it) {
                assert(this->nor == this->noc); //Power method only works on square matrices
                
                for (int i = 0; i < it; ++i) {
                    this->mv(x, y);

                    T norm = pwm::norm2(y, this->nor);
                    
                    for (int i = 0; i < partitions; ++i) {
                        norm_func_list[i].try_put(std::make_tuple(x,y,norm));
                    }
                    
                    g.wait_for_all();
                }
            }
    };
} // namespace pwm

#endif // PWM_CRSTBBGRAPHPINNED_HPP