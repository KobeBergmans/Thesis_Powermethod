/**
 * @file CRS.hpp
 * @author Kobe Bergmans (kobe.bergmans@student.kuleuven.be)
 * @brief Compressed Row Storage matrix class
 * @version 0.1
 * @date 2022-09-29
 * 
 * Includes method to generate CRS matrix obtained from discrete 2D poisson equation
 */

#ifndef PWM_CRS_HPP
#define PWM_CRS_HPP

#include <vector>
#include <iostream>
#include <cassert>
#include <algorithm>

#include "SparseMatrix.hpp"
#include "VectorUtill.hpp"

namespace pwm {
    template<typename T, typename int_type>
    class CRS: public pwm::SparseMatrix<T, int_type> {
        protected:
            // Row start array for the CRS format
            int_type* row_start;
            
            // Column index array for the CRS format
            int_type* col_ind;

            // Data array which stores the actual nonzeros
            T* data_arr;

        public:
            // Base constructor
            CRS() {}

            /**
             * @brief Base constructor which initializes internal arrays
             * 
             * @param nnz Number of nonzeros
             * @param nor Number of rows
             * @param noc Number of columns
             */
            CRS(const int_type non_z, const int_type nb_r, const int_type nb_c) {
                this->nnz = non_z;
                this->nor = nb_r;
                this->noc = nb_c;

                row_start = new int_type[this->nor+1];
                col_ind = new int_type[this->nnz];
                data_arr = new T[this->nnz];
            }

            /**
             * @brief Fill the given matrix as a 2D discritizised poisson matrix with equal discritization steplength in x and y
             * 
             * https://en.wikipedia.org/wiki/Discrete_Poisson_equation
             * 
             * @param m The amount of discritization steps in the x direction
             * @param n The amount of discritization steps in the y direction
             */
            void generatePoissonMatrix(const int_type m, const int_type n) {
                this->noc = m*n;
                this->nor = m*n;

                this->nnz = n*(m+2*(m-1)) + 2*(n-1)*m;

                row_start = new int_type[this->nor+1];
                col_ind = new int_type[this->nnz];
                data_arr = new T[this->nnz];

                // Fill data rows
                // TODO: Can this be more efficient?
                int_type nnz_index = 0;
                for (int_type row = 0; row < m*n; ++row) {

                    // Check for identity before D
                    if (row >= m) {
                        // Diagonal of I
                        data_arr[nnz_index] = -1;
                        col_ind[nnz_index] = row - m;
                        nnz_index++;
                    }

                    // Check if we are not on the first row of D
                    if (row % m != 0) {
                        // Subdiagonal of D
                        data_arr[nnz_index] = -1;
                        col_ind[nnz_index] = row-1;
                        nnz_index++;
                    } 

                    // Diagonal of D
                    data_arr[nnz_index] = 4;
                    col_ind[nnz_index] = row;
                    nnz_index++;

                    // Check if we are not on the last row of D
                    if (row % m != m-1) {
                        // Superdiagonal of D
                        data_arr[nnz_index] = -1;
                        col_ind[nnz_index] = row+1;
                        nnz_index++;
                    }

                    // Check for identity after D
                    if (row < m*n - n) {
                        data_arr[nnz_index] = -1;
                        col_ind[nnz_index] = m+row;
                        nnz_index++;
                    }

                    row_start[row+1] = nnz_index;
                }

#ifndef NDEBUG
                assert(this->nnz == nnz_index);
                assert(row_start[0] == 0);
                assert(row_start[this->nor] == this->nnz);

                std::cout << "nnz and index: " << this->nnz << ", " << nnz_index << std::endl;

                std::cout << "Row start: " << std::endl;
                pwm::printVector(row_start, this->nor+1);

                std::cout << "Col ind: " << std::endl;
                pwm::printVector(col_ind, this->nnz);

                std::cout << "Data: " << std::endl;
                pwm::printVector(data_arr, this->nnz);
#endif
            }

            void mv(const T* x, T* y) {
                std::fill(y, y+this->nor, 0.);

                int_type j;
                for (int_type i = 0; i < this->nor; ++i) {
                    for (int_type k = row_start[i]; k < row_start[i+1]; ++k) {
                        j = col_ind[k];
                        y[i] = y[i] + data_arr[k]*x[j];
                    }
                }
            }

            void powerMethod(T* x, T* y, const int_type it) {
                assert(this->nor == this->noc); //Power method only works on square matrices
                
                for (int i = 0; i < it; ++i) {
                    this->mv(x, y);
                    pwm::normalize(y, this->nor);

                    // TODO: Not completely efficient on the last iteration
                    std::copy(y, y+this->nor, x);
                }
            }
    };
} // namespace pwm

#endif // PWM_CRS_HPP