/**
 * @file Triplet.hpp
 * @author Kobe Bergmans (kobe.bergmans@student.kuleuven.be)
 * @brief Triplet format of a sparse matrix
 * @version 0.1
 * @date 2022-10-18
 * 
 * Includes a function to import from Matrix Market Format
 * TODO: Includes a function to import from the Kronecker graph generator (https://github.com/RapidsAtHKUST/Graph500KroneckerGraphGenerator)
 */

#ifndef PWM_TRIPLET_HPP
#define PWM_TRIPLET_HPP

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

#include <boost/lexical_cast.hpp>

#include "VectorUtill.hpp"

namespace pwm {
    template<typename T, typename int_type>
    class Triplet {
        public:
            // x Coordinate array
            int_type* xcoord;

            // y coordinate array
            int_type* ycoord;

            // Data array
            T* data;

            // x size
            int_type x_size;

            // y size
            int_type y_size;

            // Amount of nonzeros
            int_type nnz;

            // Base constructor
            Triplet() {}

            // Load from matrix market format
            void loadFromMM(std::string filename) {
                std::ifstream input_file(filename);

                if (input_file.is_open()) {
                    std::string temp_line;
                    std::string word;

                    // Get comment lines
                    while (input_file.good()) {
                        std::getline(input_file, temp_line);
                        
                        if (temp_line[0] != '%') {
                            std::stringstream line(temp_line);
                            line >> word;

                            // Get matrix size and amount of entries
                            x_size = boost::lexical_cast<int_type>(word);

                            line >> word;
                            y_size = boost::lexical_cast<int_type>(word);

                            line >> word; // Get amount of entries
                            nnz  = boost::lexical_cast<int_type>(word);
                            xcoord = new int_type[nnz];
                            ycoord = new int_type[nnz];
                            data = new T[nnz];

                            break;
                        }
                    }

                    // Get data
                    int_type index = 0;
                    while (input_file.good()) {
                        std::getline(input_file, temp_line);
                        if (temp_line.size() == 0) break;

                        std::stringstream line(temp_line);

                        // x coord
                        line >> word;
                        xcoord[index] = boost::lexical_cast<int_type>(word);

                        // y coord
                        line >> word;
                        ycoord[index] = boost::lexical_cast<int_type>(word);

                        // Data
                        line >> word;
                        data[index] = boost::lexical_cast<T>(word);

                        index++;
                    }
                }
            }
    };
} // namespace pwm


#endif // PWM_TRIPLET_HPP