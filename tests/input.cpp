/**
 * @file input.cpp
 * @author Kobe Bergmans (kobe.bergmans@student.kuleuven.be)
 * @brief Test file for input matrices in different formats
 * @version 0.1
 * @date 2022-10-24
 * 
 * Requires following files in "/input" folder:
 *   - arc130.mtx
 */

#define BOOST_TEST_MODULE test_input
#include <boost/test/included/unit_test.hpp>

#include <vector>
#include <cmath>

#include "../SparseMatrix.hpp"
#include "../Triplet.hpp"
#include "GetMatrices.hpp"

#include "omp.h"
#include "oneapi/tbb.h"

BOOST_AUTO_TEST_SUITE(input_mv)

BOOST_AUTO_TEST_CASE(mv_arc130, * boost::unit_test::tolerance(std::pow(10, -14))) {
    pwm::Triplet<double, int> input_mat;
    input_mat.loadFromMM("test_input/arc130.mtx");
    int mat_size = input_mat.col_size;

    // Precomputed solution using matlab
    double real_sol[] = {7.833242759536130e+00, -6.993735475365190e+00, 1.936709185814659e+00, 2.503290850938932e+00, 3.339283448756583e-01, 5.869714739734910e-01, 9.986911518391108e-01, 1.003101641972662e+00, 1.004218398789794e+00, 1.002013153286248e+00, 9.391520197023548e-01, 9.718098842587097e-01, 1.004218398789794e+00, 1.028811178984129e+00, 9.906930076896483e-01, 1.000000000000000e+00, 1.003101641972661e+00, 1.000041660912518e+00, 1.000896397214176e+00,-6.816589735179274e+02,-1.084595375000000e+06,-1.053428656250000e+06,-6.898380781250000e+05,-9.646252226562500e+05,-9.248353828125000e+05, 1.740456342697133e+00, 9.420697018500712e-01, 1.049086261540483e+00, 1.151831865310630e+00, 1.070025980472519e+00, 2.215560913085917e+00, 8.621966830369048e-01, 1.039153825288474e+00, 1.231180489063188e+00, 1.099351406097324e+00, 2.367364883422827e+00, 8.088930876815595e-01, 1.006792400395448e+00, 1.252006113529105e+00, 1.108648717403296e+00, 2.239842414855931e+00, 7.948492078628132e-01, 9.537848090230652e-01, 1.225186288356672e+00, 1.100826203822965e+00, 1.955817461013772e+00, 8.174166444430848e-01, 9.434605348760020e-01, 1.173709630966085e+00, 1.083329916000251e+00, 1.642910003662092e+00, 8.625847668101152e-01, 9.487569629474911e-01, 1.119902253150857e+00, 1.063922047614959e+00, 1.385215580463398e+00, 9.132438301851473e-01, 9.612678530668286e-01, 1.077387571334780e+00, 1.047769121825630e+00, 1.210645496845238e+00, 9.568538181480447e-01, 9.798460863306148e-01, 1.049969587475024e+00, 1.036741930991371e+00, 1.110036253929134e+00, 9.880154319107257e-01, 9.973230699540945e-01, 1.035176958888749e+00, 1.030361384153345e+00, 1.059764284640550e+00, 1.007092889398330e+00, 1.010153356939538e+00, 1.028476625680914e+00, 1.027193285524834e+00, 1.037754297256469e+00, 1.017277095466850e+00, 1.017999485135074e+00, 1.025970183312889e+00, 1.025839652866120e+00, 1.029255405068398e+00, 1.022067971527575e+00, 1.022117994725703e+00, 1.025238625705240e+00, 1.025346063077448e+00, 1.026350062340498e+00, 1.024067506194115e+00, 1.024003818631172e+00, 1.025104723870754e+00, 1.025196127593516e+00, 1.025468096137047e+00, 1.024810910224915e+00, 1.024764768779278e+00, 1.025115087628365e+00, 1.025160629302263e+00, 1.025229826569557e+00, 1.025058060884476e+00, 1.025037329643965e+00, 1.025137882679701e+00, 1.025155592709780e+00, 1.025172512978315e+00, 1.025131687521935e+00, 1.025124348700047e+00, 1.025150395929813e+00, 1.025156177580357e+00, 1.025160226970911e+00, 1.025151398032904e+00, 1.025149203836918e+00, 1.025155294686556e+00, 1.025156926363707e+00, 1.025157883763313e+00, 1.025156144052744e+00, 1.025155574083328e+00, 1.025156859308481e+00, 1.025157265365124e+00, 1.025157485157251e+00, 1.025157172232866e+00, 1.025157041847706e+00, 1.025157287716866e+00, 1.025157377123833e+00, 1.025157421827316e+00, 1.025157373398542e+00, 1.025157347321510e+00, 1.025157388299704e+00, 1.025157406926155e+00, 1.025157414376736e+00, 1.025157406926155e+00, 1.025157403200865e+00, 1.025157410651445e+00,
     1.025157410651445e+00};


    // Get datastructures
    std::vector<pwm::SparseMatrix<double, int>*> matrices = pwm::get_all_matrices<double, int>();
    double* x = new double[mat_size];
    double* y = new double[mat_size];

    // Run test on all the matrices
    for (size_t mat_index = 0; mat_index < matrices.size(); ++mat_index) {
        pwm::SparseMatrix<double, int>* mat = matrices[mat_index];

        // Get omp max threads
        int max_threads = omp_get_max_threads();

        // If we have a TBB implementation set a global limiter to overwrite other limits
        tbb::global_control global_limit(tbb::global_control::max_allowed_parallelism, pwm::get_threads_for_matrix(mat_index));
        
        for (int partitions = 1; partitions <= std::min(max_threads*2, mat_size); ++partitions) {
            mat->loadFromTriplets(input_mat, partitions);
            std::fill(x, x+mat_size, 1.);
            mat->mv(x,y);

            // Check solution
            for (int i = 0; i < mat_size; ++i) {
                BOOST_TEST(y[i] == real_sol[i]);
            }

            // If matrix is an omp or TBB matrix break because all executions are the same
            if ((mat_index-1) % 6 == 0 || (mat_index-2) % 6 == 0) {
                break;
            }
        }

        // Reset omp threads
        omp_set_num_threads(max_threads);
    }
}

BOOST_AUTO_TEST_CASE(mv_gre_1107, * boost::unit_test::tolerance(std::pow(10, -14))) {
    pwm::Triplet<double, int> input_mat;
    input_mat.loadFromMM("test_input/gre_1107.mtx");
    int mat_size = input_mat.col_size;

    // Precomputed solution using matlab
    double real_sol[] = {1.272700000000000e+00, 1.636310000000000e+00,1.121110000000000e+00,1.151410000000000e+00,1.363610000000000e+00,1.121110000000000e+00,9.999201787999998e-01,9.999000000000000e-01,1.272710000000000e+00,1.151410000000000e+00,9.090000000000000e-01,1.363610000000000e+00,1.363610000000000e+00,1.121110000000000e+00,9.999201787999998e-01,1.121110000000000e+00,9.999099999999999e-01,9.999201787999998e-01,8.787101788000000e-01,7.575101787999999e-01,9.999000000000000e-01,1.272710000000000e+00,1.272710000000000e+00,1.151410000000000e+00,1.151410000000000e+00,9.090000000000000e-01,1.363610000000000e+00,1.363610000000000e+00,1.363610000000000e+00,1.121110000000000e+00,9.999201787999998e-01,1.121110000000000e+00,9.999099999999999e-01,9.999201787999998e-01,9.999201788000001e-01,7.575101787999999e-01,1.121110000000000e+00,9.999099999999999e-01,9.999099999999999e-01,9.999099999999999e-01,9.999201787999998e-01,8.787101788000000e-01,9.999201788000001e-01,8.787101788000000e-01,6.363001788000000e-01,7.575101787999999e-01,1.272710000000000e+00,1.272710000000000e+00,1.272710000000000e+00,1.151410000000000e+00,1.151410000000000e+00,1.151410000000000e+00,9.090000000000000e-01,1.363610000000000e+00,1.363610000000000e+00,1.363610000000000e+00,1.121110000000000e+00,9.999201787999998e-01,1.121110000000000e+00,9.999099999999999e-01,9.999201787999998e-01,9.999201788000001e-01,7.575101787999999e-01,1.121110000000000e+00,9.999099999999999e-01,9.999099999999999e-01,9.999099999999999e-01,9.999201787999998e-01,9.999201788000001e-01,9.999201788000001e-01,9.999201788000001e-01,7.575101788000000e-01,7.575101787999999e-01,9.999099999999999e-01,9.999100000000001e-01,9.999099999999999e-01,9.999099999999999e-01,9.999099999999999e-01,9.999099999999999e-01,9.999201787999998e-01,8.787101788000000e-01,9.999201788000001e-01,8.787101788000000e-01,8.787101787999999e-01,9.999201788000001e-01,6.363001788000000e-01,6.363001788000000e-01,7.575101787999999e-01,1.272710000000000e+00,1.272710000000000e+00,1.272710000000000e+00,1.272710000000000e+00,1.151410000000000e+00,1.151410000000000e+00,1.151410000000000e+00,1.151410000000000e+00,1.363610000000000e+00,1.363610000000000e+00,1.363610000000000e+00,1.121110000000000e+00,9.999201787999998e-01,1.121110000000000e+00,9.999099999999999e-01,9.999201787999998e-01,9.999201788000001e-01,7.575101787999999e-01,1.121110000000000e+00,9.999099999999999e-01,9.999099999999999e-01,9.999099999999999e-01,9.999201787999998e-01,9.999201788000001e-01,9.999201788000001e-01,9.999201788000001e-01,7.575101788000000e-01,7.575101787999999e-01,9.999099999999999e-01,9.999100000000001e-01,9.999099999999999e-01,9.999099999999999e-01,9.999099999999999e-01,9.999099999999999e-01,9.999201787999998e-01,9.999201788000001e-01,9.999201788000001e-01,9.999201788000001e-01,9.999201787999999e-01,9.999201788000001e-01,7.575101788000000e-01,7.575101788000000e-01,7.575101787999999e-01,9.999100000000001e-01,9.999100000000001e-01,9.999099999999999e-01,9.999099999999999e-01,9.999100000000001e-01,9.999099999999999e-01,9.999099999999999e-01,9.999099999999999e-01,9.999099999999999e-01,9.999201787999998e-01,8.787101788000000e-01,9.999201788000001e-01,8.787101788000000e-01,8.787101787999999e-01,9.999201788000001e-01,8.787101787999999e-01,8.787101787999999e-01,9.999201788000001e-01,6.363001788000000e-01,6.363001788000000e-01,1.272710000000000e+00,1.272710000000000e+00,1.272710000000000e+00,1.272710000000000e+00,1.151410000000000e+00,1.151410000000000e+00,1.151410000000000e+00,1.151410000000000e+00,1.363610000000000e+00,1.363610000000000e+00,1.363610000000000e+00,1.121110000000000e+00,9.999201787999998e-01,1.121110000000000e+00,9.999099999999999e-01,9.999201787999998e-01,9.999201788000001e-01,7.575101787999999e-01,1.121110000000000e+00,9.999099999999999e-01,9.999099999999999e-01,9.999099999999999e-01,9.999201787999998e-01,9.999201788000001e-01,9.999201788000001e-01,9.999201788000001e-01,7.575101788000000e-01,7.575101787999999e-01,9.999099999999999e-01,9.999100000000001e-01,9.999099999999999e-01,9.999099999999999e-01,9.999099999999999e-01,9.999099999999999e-01,9.999201787999998e-01,9.999201788000001e-01,9.999201788000001e-01,9.999201788000001e-01,9.999201787999999e-01,9.999201788000001e-01,7.575101788000000e-01,7.575101788000000e-01,7.575101787999999e-01,9.999100000000001e-01,9.999100000000001e-01,9.999099999999999e-01,9.999099999999999e-01,9.999100000000001e-01,9.999099999999999e-01,9.999099999999999e-01,9.999099999999999e-01,9.999099999999999e-01,9.999201787999998e-01,9.999201788000001e-01,9.999201788000001e-01,9.999201788000001e-01,9.999201787999999e-01,9.999201788000001e-01,9.999201787999999e-01,9.999201787999999e-01,9.999201788000001e-01,7.575101788000000e-01,7.575101788000000e-01,9.999100000000001e-01,9.999100000000001e-01,9.999100000000001e-01,9.999100000000001e-01,9.999099999999999e-01,9.999099999999999e-01,9.999100000000001e-01,9.999099999999999e-01,9.999099999999999e-01,9.999099999999999e-01,9.999099999999999e-01,9.999201787999998e-01,8.787101788000000e-01,9.999201788000001e-01,8.787101788000000e-01,8.787101787999999e-01,9.999201788000001e-01,8.787101787999999e-01,8.787101787999999e-01,9.999201788000001e-01,8.787101787999999e-01,8.787101787999999e-01,6.363001788000000e-01,1.272710000000000e+00,1.272710000000000e+00,1.272710000000000e+00,1.272710000000000e+00,1.151410000000000e+00,1.151410000000000e+00,1.151410000000000e+00,1.151410000000000e+00,1.363610000000000e+00,1.363610000000000e+00,1.363610000000000e+00,1.121110000000000e+00,9.999201787999998e-01,1.121110000000000e+00,9.999099999999999e-01,9.999201787999998e-01,9.999201788000001e-01,7.575101787999999e-01,1.121110000000000e+00,9.999099999999999e-01,9.999099999999999e-01,9.999099999999999e-01,9.999201787999998e-01,9.999201788000001e-01,9.999201788000001e-01,9.999201788000001e-01,7.575101788000000e-01,7.575101787999999e-01,9.999099999999999e-01,9.999100000000001e-01,9.999099999999999e-01,9.999099999999999e-01,9.999099999999999e-01,9.999099999999999e-01,9.999201787999998e-01,9.999201788000001e-01,9.999201788000001e-01,9.999201788000001e-01,9.999201787999999e-01,9.999201788000001e-01,7.575101788000000e-01,7.575101788000000e-01,7.575101787999999e-01,9.999100000000001e-01,9.999100000000001e-01,9.999099999999999e-01,9.999099999999999e-01,9.999100000000001e-01,9.999099999999999e-01,9.999099999999999e-01,9.999099999999999e-01,9.999099999999999e-01,9.999201787999998e-01,9.999201788000001e-01,9.999201788000001e-01,9.999201788000001e-01,9.999201787999999e-01,9.999201788000001e-01,9.999201787999999e-01,9.999201787999999e-01,9.999201788000001e-01,7.575101788000000e-01,7.575101788000000e-01,9.999100000000001e-01,9.999100000000001e-01,9.999100000000001e-01,9.999100000000001e-01,9.999099999999999e-01,9.999099999999999e-01,9.999100000000001e-01,9.999099999999999e-01,9.999099999999999e-01,9.999099999999999e-01,9.999099999999999e-01,9.999201787999998e-01,9.999201788000001e-01,9.999201788000001e-01,9.999201788000001e-01,9.999201787999999e-01,9.999201788000001e-01,9.999201787999999e-01,9.999201787999999e-01,9.999201788000001e-01,9.999201787999999e-01,9.999201787999999e-01,7.575101788000000e-01,9.999100000000001e-01,9.999100000000001e-01,9.999100000000001e-01,9.999100000000001e-01,9.999100000000001e-01,9.999099999999999e-01,9.999099999999999e-01,9.999100000000001e-01,9.999099999999999e-01,9.999099999999999e-01,9.999099999999999e-01,9.999099999999999e-01,9.999201787999998e-01,8.787101788000000e-01,9.999201788000001e-01,8.787101788000000e-01,8.787101787999999e-01,9.999201788000001e-01,8.787101787999999e-01,8.787101787999999e-01,9.999201788000001e-01,8.787101787999999e-01,8.787101787999999e-01,8.787101787999999e-01,1.272710000000000e+00,1.272710000000000e+00,1.272710000000000e+00,1.272710000000000e+00,1.151410000000000e+00,1.151410000000000e+00,1.151410000000000e+00,1.151410000000000e+00,1.363610000000000e+00,1.363610000000000e+00,1.363610000000000e+00,1.121110000000000e+00,9.999201787999998e-01,1.121110000000000e+00,9.999099999999999e-01,9.999201787999998e-01,9.999201788000001e-01,7.575101787999999e-01,1.121110000000000e+00,9.999099999999999e-01,9.999099999999999e-01,9.999099999999999e-01,9.999201787999998e-01,9.999201788000001e-01,9.999201788000001e-01,9.999201788000001e-01,7.575101788000000e-01,7.575101787999999e-01,9.999099999999999e-01,9.999100000000001e-01,9.999099999999999e-01,9.999099999999999e-01,9.999099999999999e-01,9.999099999999999e-01,9.999201787999998e-01,9.999201788000001e-01,9.999201788000001e-01,9.999201788000001e-01,9.999201787999999e-01,9.999201788000001e-01,7.575101788000000e-01,7.575101788000000e-01,7.575101787999999e-01,9.999100000000001e-01,9.999100000000001e-01,9.999099999999999e-01,9.999099999999999e-01,9.999100000000001e-01,9.999099999999999e-01,9.999099999999999e-01,9.999099999999999e-01,9.999099999999999e-01,9.999201787999998e-01,9.999201788000001e-01,9.999201788000001e-01,9.999201788000001e-01,9.999201787999999e-01,9.999201788000001e-01,9.999201787999999e-01,9.999201787999999e-01,9.999201788000001e-01,7.575101788000000e-01,7.575101788000000e-01,9.999100000000001e-01,9.999100000000001e-01,9.999100000000001e-01,9.999100000000001e-01,9.999099999999999e-01,9.999099999999999e-01,9.999100000000001e-01,9.999099999999999e-01,9.999099999999999e-01,9.999099999999999e-01,9.999099999999999e-01,9.999201787999998e-01,9.999201788000001e-01,9.999201788000001e-01,9.999201788000001e-01,9.999201787999999e-01,9.999201788000001e-01,9.999201787999999e-01,9.999201787999999e-01,9.999201788000001e-01,9.999201787999999e-01,9.999201787999999e-01,7.575101788000000e-01,9.999100000000001e-01,9.999100000000001e-01,9.999100000000001e-01,9.999100000000001e-01,9.999100000000001e-01,9.999099999999999e-01,9.999099999999999e-01,9.999100000000001e-01,9.999099999999999e-01,9.999099999999999e-01,9.999099999999999e-01,9.999099999999999e-01,9.999201787999998e-01,9.999201788000001e-01,9.999201788000001e-01,9.999201788000001e-01,9.999201787999999e-01,9.999201788000001e-01,9.999201787999999e-01,9.999201787999999e-01,9.999201788000001e-01,9.999201787999999e-01,9.999201787999999e-01,9.999201787999999e-01,9.999100000000001e-01,9.999100000000001e-01,9.999100000000001e-01,9.999100000000001e-01,9.999100000000001e-01,9.999099999999999e-01,9.999099999999999e-01,9.999100000000001e-01,9.999099999999999e-01,9.999099999999999e-01,9.999099999999999e-01,9.999099999999999e-01,9.999201787999998e-01,8.787101788000000e-01,9.999201788000001e-01,8.787101788000000e-01,8.787101787999999e-01,9.999201788000001e-01,8.787101787999999e-01,8.787101787999999e-01,9.999201788000001e-01,8.787101787999999e-01,8.787101787999999e-01,8.787101787999999e-01,1.272710000000000e+00,1.272710000000000e+00,1.272710000000000e+00,1.272710000000000e+00,1.151410000000000e+00,1.151410000000000e+00,1.151410000000000e+00,1.151410000000000e+00,1.363610000000000e+00,1.363610000000000e+00,7.273100000000000e-01,8.485100000000000e-01,7.272200000000000e-01,1.121110000000000e+00,3.636100000000000e-01,7.272200000000000e-01,9.999201788000001e-01,7.575101787999999e-01,1.121110000000000e+00,9.999099999999999e-01,9.999099999999999e-01,3.636100000000000e-01,7.272200000000000e-01,9.999201788000001e-01,9.999201788000001e-01,9.999201788000001e-01,7.575101788000000e-01,7.575101787999999e-01,9.999099999999999e-01,9.999100000000001e-01,9.999099999999999e-01,9.999099999999999e-01,9.999099999999999e-01,3.636100000000000e-01,7.272200000000000e-01,9.999201788000001e-01,9.999201788000001e-01,9.999201788000001e-01,9.999201787999999e-01,9.999201788000001e-01,7.575101788000000e-01,7.575101788000000e-01,7.575101787999999e-01,9.999100000000001e-01,9.999100000000001e-01,9.999099999999999e-01,9.999099999999999e-01,9.999100000000001e-01,9.999099999999999e-01,9.999099999999999e-01,9.999099999999999e-01,3.636100000000000e-01,7.272200000000000e-01,9.999201788000001e-01,9.999201788000001e-01,9.999201788000001e-01,9.999201787999999e-01,9.999201788000001e-01,9.999201787999999e-01,9.999201787999999e-01,9.999201788000001e-01,7.575101788000000e-01,7.575101788000000e-01,9.999100000000001e-01,9.999100000000001e-01,9.999100000000001e-01,9.999100000000001e-01,9.999099999999999e-01,9.999099999999999e-01,9.999100000000001e-01,9.999099999999999e-01,9.999099999999999e-01,9.999099999999999e-01,3.636100000000000e-01,7.272200000000000e-01,9.999201788000001e-01,9.999201788000001e-01,9.999201788000001e-01,9.999201787999999e-01,9.999201788000001e-01,9.999201787999999e-01,9.999201787999999e-01,9.999201788000001e-01,9.999201787999999e-01,9.999201787999999e-01,7.575101788000000e-01,9.999100000000001e-01,9.999100000000001e-01,9.999100000000001e-01,9.999100000000001e-01,9.999100000000001e-01,9.999099999999999e-01,9.999099999999999e-01,9.999100000000001e-01,9.999099999999999e-01,9.999099999999999e-01,9.999099999999999e-01,3.636100000000000e-01,7.272200000000000e-01,9.999201788000001e-01,9.999201788000001e-01,9.999201788000001e-01,9.999201787999999e-01,9.999201788000001e-01,9.999201787999999e-01,9.999201787999999e-01,9.999201788000001e-01,9.999201787999999e-01,9.999201787999999e-01,9.999201787999999e-01,9.999100000000001e-01,9.999100000000001e-01,9.999100000000001e-01,9.999100000000001e-01,9.999100000000001e-01,9.999099999999999e-01,9.999099999999999e-01,9.999100000000001e-01,9.999099999999999e-01,9.999099999999999e-01,9.999099999999999e-01,3.636100000000000e-01,7.272200000000000e-01,9.999201788000001e-01,9.999201788000001e-01,9.999201788000001e-01,9.999201787999999e-01,9.999201788000001e-01,9.999201787999999e-01,9.999201787999999e-01,9.999201788000001e-01,9.999201787999999e-01,9.999201787999999e-01,9.999201787999999e-01,9.999100000000001e-01,9.999100000000001e-01,9.999100000000001e-01,9.999100000000001e-01,9.999100000000001e-01,9.999099999999999e-01,9.999099999999999e-01,9.999100000000001e-01,9.999099999999999e-01,9.999099999999999e-01,9.999099999999999e-01,3.636100000000000e-01,7.272200000000000e-01,8.787101788000000e-01,9.999201788000001e-01,8.787101788000000e-01,8.787101787999999e-01,9.999201788000001e-01,8.787101787999999e-01,8.787101787999999e-01,9.999201788000001e-01,8.787101787999999e-01,8.787101787999999e-01,8.787101787999999e-01,1.272710000000000e+00,1.272710000000000e+00,1.272710000000000e+00,6.364100000000000e-01,8.788100000000000e-01,1.151410000000000e+00,1.151410000000000e+00,1.151410000000000e+00,1.363610000000000e+00,1.000010000000000e+00,1.121210000000000e+00,9.999199999999999e-01,8.484099999999999e-01,1.121110000000000e+00,6.363099999999999e-01,7.272099999999999e-01,9.999199999999999e-01,1.090820000000000e+00,9.999201788000001e-01,7.575101788000000e-01,7.575101787999999e-01,9.999099999999999e-01,9.999100000000001e-01,9.999099999999999e-01,6.363099999999999e-01,7.272099999999999e-01,9.999199999999999e-01,1.090820000000000e+00,9.999201788000001e-01,9.999201787999999e-01,9.999201788000001e-01,7.575101788000000e-01,7.575101788000000e-01,7.575101787999999e-01,9.999100000000001e-01,9.999100000000001e-01,9.999099999999999e-01,9.999099999999999e-01,9.999100000000001e-01,9.999099999999999e-01,6.363099999999999e-01,7.272099999999999e-01,9.999199999999999e-01,1.090820000000000e+00,9.999201788000001e-01,9.999201787999999e-01,9.999201788000001e-01,9.999201787999999e-01,9.999201787999999e-01,9.999201788000001e-01,7.575101788000000e-01,7.575101788000000e-01,9.999100000000001e-01,9.999100000000001e-01,9.999100000000001e-01,9.999100000000001e-01,9.999099999999999e-01,9.999099999999999e-01,9.999100000000001e-01,9.999099999999999e-01,6.363099999999999e-01,7.272099999999999e-01,9.999199999999999e-01,1.090820000000000e+00,9.999201788000001e-01,9.999201787999999e-01,9.999201788000001e-01,9.999201787999999e-01,9.999201787999999e-01,9.999201788000001e-01,9.999201787999999e-01,9.999201787999999e-01,7.575101788000000e-01,9.999100000000001e-01,9.999100000000001e-01,9.999100000000001e-01,9.999100000000001e-01,9.999100000000001e-01,9.999099999999999e-01,9.999099999999999e-01,9.999100000000001e-01,9.999099999999999e-01,6.363099999999999e-01,7.272099999999999e-01,9.999199999999999e-01,1.090820000000000e+00,9.999201788000001e-01,9.999201787999999e-01,9.999201788000001e-01,9.999201787999999e-01,9.999201787999999e-01,9.999201788000001e-01,9.999201787999999e-01,9.999201787999999e-01,9.999201787999999e-01,9.999100000000001e-01,9.999100000000001e-01,9.999100000000001e-01,9.999100000000001e-01,9.999100000000001e-01,9.999099999999999e-01,9.999099999999999e-01,9.999100000000001e-01,9.999099999999999e-01,6.363099999999999e-01,7.272099999999999e-01,9.999199999999999e-01,1.090820000000000e+00,9.999201788000001e-01,9.999201787999999e-01,9.999201788000001e-01,9.999201787999999e-01,9.999201787999999e-01,9.999201788000001e-01,9.999201787999999e-01,9.999201787999999e-01,9.999201787999999e-01,9.999100000000001e-01,9.999100000000001e-01,9.999100000000001e-01,9.999100000000001e-01,9.999100000000001e-01,9.999099999999999e-01,9.999099999999999e-01,9.999100000000001e-01,9.999099999999999e-01,6.363099999999999e-01,7.272099999999999e-01,9.999199999999999e-01,1.090820000000000e+00,9.999201788000001e-01,9.999201787999999e-01,9.999201788000001e-01,9.999201787999999e-01,9.999201787999999e-01,9.999201788000001e-01,9.999201787999999e-01,9.999201787999999e-01,9.999201787999999e-01,9.999100000000001e-01,9.999100000000001e-01,9.999100000000001e-01,9.999100000000001e-01,9.999100000000001e-01,9.999099999999999e-01,9.999099999999999e-01,9.999100000000001e-01,9.999099999999999e-01,6.363099999999999e-01,7.272099999999999e-01,8.787099999999999e-01,1.090820000000000e+00,8.787101788000000e-01,8.787101787999999e-01,9.999201788000001e-01,8.787101787999999e-01,8.787101787999999e-01,9.999201788000001e-01,8.787101787999999e-01,8.787101787999999e-01,8.787101787999999e-01,1.272710000000000e+00,1.272710000000000e+00,1.000010000000000e+00,1.242410000000000e+00,1.151410000000000e+00,1.151410000000000e+00,1.000010000000000e+00,1.121210000000000e+00,9.999199999999999e-01,1.121110000000000e+00,8.484099999999999e-01,6.363099999999999e-01,9.999100000000001e-01,7.272099999999999e-01,9.999199999999999e-01,1.363520000000000e+00,1.090820000000000e+00,7.575101788000000e-01,7.575101788000000e-01,7.575101787999999e-01,9.999100000000001e-01,9.999100000000001e-01,9.999099999999999e-01,6.363099999999999e-01,9.999100000000001e-01,7.272099999999999e-01,9.999199999999999e-01,1.363520000000000e+00,1.090820000000000e+00,9.999201787999999e-01,9.999201787999999e-01,9.999201788000001e-01,7.575101788000000e-01,7.575101788000000e-01,9.999100000000001e-01,9.999100000000001e-01,9.999100000000001e-01,9.999100000000001e-01,9.999099999999999e-01,6.363099999999999e-01,9.999100000000001e-01,7.272099999999999e-01,9.999199999999999e-01,1.363520000000000e+00,1.090820000000000e+00,9.999201787999999e-01,9.999201787999999e-01,9.999201788000001e-01,9.999201787999999e-01,9.999201787999999e-01,7.575101788000000e-01,9.999100000000001e-01,9.999100000000001e-01,9.999100000000001e-01,9.999100000000001e-01,9.999100000000001e-01,9.999099999999999e-01,6.363099999999999e-01,9.999100000000001e-01,7.272099999999999e-01,9.999199999999999e-01,1.363520000000000e+00,1.090820000000000e+00,9.999201787999999e-01,9.999201787999999e-01,9.999201788000001e-01,9.999201787999999e-01,9.999201787999999e-01,9.999201787999999e-01,9.999100000000001e-01,9.999100000000001e-01,9.999100000000001e-01,9.999100000000001e-01,9.999100000000001e-01,9.999099999999999e-01,6.363099999999999e-01,9.999100000000001e-01,7.272099999999999e-01,9.999199999999999e-01,1.363520000000000e+00,1.090820000000000e+00,9.999201787999999e-01,9.999201787999999e-01,9.999201788000001e-01,9.999201787999999e-01,9.999201787999999e-01,9.999201787999999e-01,9.999100000000001e-01,9.999100000000001e-01,9.999100000000001e-01,9.999100000000001e-01,9.999100000000001e-01,9.999099999999999e-01,6.363099999999999e-01,9.999100000000001e-01,7.272099999999999e-01,9.999199999999999e-01,1.363520000000000e+00,1.090820000000000e+00,9.999201787999999e-01,9.999201787999999e-01,9.999201788000001e-01,9.999201787999999e-01,9.999201787999999e-01,9.999201787999999e-01,9.999100000000001e-01,9.999100000000001e-01,9.999100000000001e-01,9.999100000000001e-01,9.999100000000001e-01,9.999099999999999e-01,6.363099999999999e-01,9.999100000000001e-01,7.272099999999999e-01,9.999199999999999e-01,1.363520000000000e+00,1.090820000000000e+00,9.999201787999999e-01,9.999201787999999e-01,9.999201788000001e-01,9.999201787999999e-01,9.999201787999999e-01,9.999201787999999e-01,9.999100000000001e-01,9.999100000000001e-01,9.999100000000001e-01,9.999100000000001e-01,9.999100000000001e-01,9.999099999999999e-01,6.363099999999999e-01,9.999100000000001e-01,7.272099999999999e-01,8.787099999999999e-01,1.242310000000000e+00,1.090820000000000e+00,8.787101787999999e-01,8.787101787999999e-01,9.999201788000001e-01,8.787101787999999e-01,8.787101787999999e-01,8.787101787999999e-01,1.272710000000000e+00,1.000010000000000e+00,1.242410000000000e+00,1.151410000000000e+00,1.121110000000000e+00,1.121110000000000e+00,8.484099999999999e-01,9.999100000000001e-01,9.999100000000001e-01,7.272099999999999e-01,1.363520000000000e+00,1.363520000000000e+00,1.090820000000000e+00,7.575101788000000e-01,7.575101788000000e-01,9.999100000000001e-01,9.999100000000001e-01,9.999100000000001e-01,9.999100000000001e-01,7.272099999999999e-01,1.363520000000000e+00,1.363520000000000e+00,1.090820000000000e+00,9.999201787999999e-01,9.999201787999999e-01,7.575101788000000e-01,9.999100000000001e-01,9.999100000000001e-01,9.999100000000001e-01,9.999100000000001e-01,9.999100000000001e-01,7.272099999999999e-01,1.363520000000000e+00,1.363520000000000e+00,1.090820000000000e+00,9.999201787999999e-01,9.999201787999999e-01,9.999201787999999e-01,9.999100000000001e-01,9.999100000000001e-01,9.999100000000001e-01,9.999100000000001e-01,9.999100000000001e-01,7.272099999999999e-01,1.363520000000000e+00,1.363520000000000e+00,1.090820000000000e+00,9.999201787999999e-01,9.999201787999999e-01,9.999201787999999e-01,9.999100000000001e-01,9.999100000000001e-01,9.999100000000001e-01,9.999100000000001e-01,9.999100000000001e-01,7.272099999999999e-01,1.363520000000000e+00,1.363520000000000e+00,1.090820000000000e+00,9.999201787999999e-01,9.999201787999999e-01,9.999201787999999e-01,9.999100000000001e-01,9.999100000000001e-01,9.999100000000001e-01,9.999100000000001e-01,9.999100000000001e-01,7.272099999999999e-01,1.363520000000000e+00,1.363520000000000e+00,1.090820000000000e+00,9.999201787999999e-01,9.999201787999999e-01,9.999201787999999e-01,9.999100000000001e-01,9.999100000000001e-01,9.999100000000001e-01,9.999100000000001e-01,9.999100000000001e-01,7.272099999999999e-01,1.363520000000000e+00,1.363520000000000e+00,1.090820000000000e+00,9.999201787999999e-01,9.999201787999999e-01,9.999201787999999e-01,9.999100000000001e-01,9.999100000000001e-01,9.999100000000001e-01,9.999100000000001e-01,9.999100000000001e-01,7.272099999999999e-01,1.242310000000000e+00,1.242310000000000e+00,1.090820000000000e+00,8.787101787999999e-01,8.787101787999999e-01,8.787101787999999e-01,1.000010000000000e+00,1.242410000000000e+00,1.121110000000000e+00,1.121110000000000e+00,9.999100000000001e-01,9.999100000000001e-01,1.363520000000000e+00,1.363520000000000e+00,7.575101788000000e-01,9.999100000000001e-01,9.999100000000001e-01,9.999100000000001e-01,1.363520000000000e+00,1.363520000000000e+00,9.999201787999999e-01,9.999100000000001e-01,9.999100000000001e-01,9.999100000000001e-01,1.363520000000000e+00,1.363520000000000e+00,9.999201787999999e-01,9.999100000000001e-01,9.999100000000001e-01,9.999100000000001e-01,1.363520000000000e+00,1.363520000000000e+00,9.999201787999999e-01,9.999100000000001e-01,9.999100000000001e-01,9.999100000000001e-01,1.363520000000000e+00,1.363520000000000e+00,9.999201787999999e-01,9.999100000000001e-01,9.999100000000001e-01,9.999100000000001e-01,1.363520000000000e+00,1.363520000000000e+00,9.999201787999999e-01,9.999100000000001e-01,9.999100000000001e-01,9.999100000000001e-01,1.363520000000000e+00,1.363520000000000e+00,9.999201787999999e-01,9.999100000000001e-01,9.999100000000001e-01,9.999100000000001e-01,1.242310000000000e+00,1.242310000000000e+00,8.787101787999999e-01,1.121110000000000e+00,9.999100000000001e-01,1.363520000000000e+00,9.999100000000001e-01,1.363520000000000e+00,9.999100000000001e-01,1.363520000000000e+00,9.999100000000001e-01,1.363520000000000e+00,9.999100000000001e-01,1.363520000000000e+00,9.999100000000001e-01,1.363520000000000e+00,9.999100000000001e-01,1.363520000000000e+00,9.999100000000001e-01,1.242310000000000e+00};


    // Get datastructures
    std::vector<pwm::SparseMatrix<double, int>*> matrices = pwm::get_all_matrices<double, int>();
    double* x = new double[mat_size];
    double* y = new double[mat_size];

    // Run test on all the matrices
    for (size_t mat_index = 0; mat_index < matrices.size(); ++mat_index) {
        pwm::SparseMatrix<double, int>* mat = matrices[mat_index];

        // Get omp max threads
        int max_threads = omp_get_max_threads();

        // If we have a TBB implementation set a global limiter to overwrite other limits
        tbb::global_control global_limit(tbb::global_control::max_allowed_parallelism, pwm::get_threads_for_matrix(mat_index));
        
        for (int partitions = 1; partitions <= std::min(max_threads*2, mat_size); ++partitions) {
            mat->loadFromTriplets(input_mat, partitions);
            std::fill(x, x+mat_size, 1.);
            mat->mv(x,y);

            // Check solution
            for (int i = 0; i < mat_size; ++i) {
                BOOST_TEST(y[i] == real_sol[i]);
            }

            // If matrix is an omp or TBB matrix break because all executions are the same
            if ((mat_index-1) % 6 == 0 || (mat_index-2) % 6 == 0) {
                break;
            }
        }

        // Reset omp threads
        omp_set_num_threads(max_threads);
    }
}

BOOST_AUTO_TEST_CASE(mv_8_4_bin_no_rand, * boost::unit_test::tolerance(std::pow(10, -14))) {
    pwm::Triplet<double, int> input_mat;
    input_mat.loadFromKronecker("test_input/test_mat_8_4.bin", std::pow(2, 8), false);
    int mat_size = input_mat.col_size;

    // Precomputed solution using matlab
    double real_sol[] = {0.,   0.,   1.,   0.,   2.,   0.,   0.,  12.,   0.,  10.,   0.,   2.,   0.,   9.,   0.,   1.,   0.,  17.,   0.,   2.,   0.,   0.,   0.,   3.,   2.,   0.,   3.,   1.,   1.,  71.,   0.,   0.,   0.,   4.,   0.,   3.,   0.,   3.,  55.,   1.,   1.,   1.,   0.,   0.,   0.,  54.,   0.,  10.,   1.,   0.,   0.,   0.,   0.,   0.,   0.,   0.,   3.,   0.,   0.,   0.,   0.,  45.,   0.,   4.,   0.,  17.,   1.,   6.,   0.,   0.,   0.,   0.,   4.,   3.,   0.,   0.,  10.,   0.,   0.,   0.,   0.,   5.,   0.,   0.,   2.,   0.,   1.,   2.,  17.,   0.,   1.,   0.,   2.,   0.,   0.,   0.,   0.,  10.,   3.,   0., 252.,   0.,   8.,   1.,   2.,   0.,   4.,   0.,   0.,   1.,   0.,   0.,   0.,   0.,   0.,   0.,   0.,   3.,   0.,   0.,   0.,   0.,   1.,   0.,   1.,   0.,   4.,   5.,   0.,   0.,   0.,   3.,   1.,   2.,  18.,   1.,   0.,   0.,   0.,  13.,   0.,   0.,   0.,   0.,   1.,   0.,   0.,   0.,   0.,  30.,   1.,   1.,   0.,   5.,   0.,   1.,   0.,   0.,  11.,   0.,   0.,   0.,   0.,   1.,   0.,   0.,  59.,   0.,   0.,   0.,   0.,  11.,   0.,   0.,   0.,   0.,  12.,   5.,   0.,  13.,   0.,   0.,   2.,   0.,   0.,   2.,   0.,   0.,   0.,   1.,  13.,   0.,   0.,   0.,   0.,   0.,   2.,   3.,   4.,   0.,   2.,   0.,   0.,   0.,   1.,   0.,   1.,  15.,   2.,   0.,   2.,   3.,   0.,   0.,   0.,   0.,   0.,   4.,   1.,   0.,   9.,   0.,   0.,  11.,   0.,   1.,  15.,   0.,   4.,   0.,   5.,   0.,   0.,   0.,   0.,   1.,   2.,   0.,   0.,   5.,   2.,   0.,   0.,  10.,   0.,   0.,   1.,   0.,   3.,   2.,   0.,   0.,  18.,   0.,   0.,0.};

    // Get datastructures
    std::vector<pwm::SparseMatrix<double, int>*> matrices = pwm::get_all_matrices<double, int>();
    double* x = new double[mat_size];
    double* y = new double[mat_size];

    // Run test on all the matrices
    for (size_t mat_index = 0; mat_index < matrices.size(); ++mat_index) {
        pwm::SparseMatrix<double, int>* mat = matrices[mat_index];

        // Get omp max threads
        int max_threads = omp_get_max_threads();

        // If we have a TBB implementation set a global limiter to overwrite other limits
        tbb::global_control global_limit(tbb::global_control::max_allowed_parallelism, pwm::get_threads_for_matrix(mat_index));
        
        for (int partitions = 1; partitions <= std::min(max_threads*2, mat_size); ++partitions) {
            mat->loadFromTriplets(input_mat, partitions);
            std::fill(x, x+mat_size, 1.);
            mat->mv(x,y);

            // Check solution
            for (int i = 0; i < mat_size; ++i) {
                BOOST_TEST(y[i] == real_sol[i]);
            }

            // If matrix is an omp or TBB matrix break because all executions are the same
            if ((mat_index-1) % 6 == 0 || (mat_index-2) % 6 == 0) {
                break;
            }
        }

        // Reset omp threads
        omp_set_num_threads(max_threads);
    }
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(powermethod_input)

BOOST_AUTO_TEST_CASE(powermethod_arc130, * boost::unit_test::tolerance(std::pow(10, -12))) {
    pwm::Triplet<double, int> input_mat;
    input_mat.loadFromMM("test_input/arc130.mtx");
    int mat_size = input_mat.col_size;

    // Precomputed solution using matlab
    double real_sol[] = {2.492724633018974e-05, -2.390029079751346e-05,  9.572440154285601e-11, -1.109479628632532e-11,  1.715107887135334e-13,  3.217882304082448e-13, -1.103385548634800e-14,  3.804972004156662e-15,  1.882468120622790e-14,  1.140532528010935e-14, -4.594905476337396e-09, -2.320827044723591e-13,  1.882468120622791e-14,  3.063003439297170e-14, -7.616237553357624e-14,  9.144373928721132e-43,  3.804972004156651e-15, -2.552141537821988e-12, -3.614444536730160e-15,  2.345394485375163e-05, -9.999999988276607e-01,  2.413916021070737e-06, -1.502618542048214e-07,  6.580273154716977e-20,  6.631828979854100e-21,  1.505217311193943e-18, -2.908108657714851e-17,  2.023037754317203e-22, -1.083417245222760e-25, -1.179250234161811e-26,  3.232387725635387e-08, -4.253028950103695e-14,  3.500727758463204e-18, -2.461158769088564e-25, -2.371662405296508e-26,  2.441597623040829e-05, -9.709691231986644e-12,  3.158497694918372e-14, -3.405279088750418e-25, -3.170354596220193e-26,  9.613918117342152e-08, -4.725999046775580e-11,  3.068545850069518e-12, -3.572140791950370e-25, -3.379215405504973e-26,  1.241625888841511e-13, -4.874748886745859e-12,  9.112121257184283e-13, -3.069491249112258e-25, -3.049410847276142e-26,  5.983033908266379e-19, -4.417361316865787e-14,  3.067671212294626e-15, -2.232127347602557e-25, -2.392621564417478e-26,  2.906737113546671e-19, -2.383116132109114e-16,  1.569211620952665e-18, -1.375234202085984e-25, -1.652061731301824e-26,  1.446550408766568e-19, -3.363870499144852e-18,  1.423922154806218e-21, -6.985855466232769e-26, -1.012985515704650e-26,  6.906566290005820e-20, -1.775352932370874e-19,  9.989737052140664e-24, -2.797138906980411e-26, -5.598427441933988e-27,  3.043238914248902e-20, -1.863352857380007e-20,  5.058331373195670e-25, -8.822788286630944e-27, -2.835705943917218e-27,  1.215604059972376e-20, -2.306664397097668e-21,  6.387263749122422e-26, -2.492666863481531e-27, -1.325242311424826e-27,  4.421046919925564e-21, -2.708778096203366e-22,  1.191369106005722e-26, -7.641908724116118e-28, -5.702200070880650e-28,  1.470438915185142e-21, -2.762550553171631e-23,  2.826480477014610e-27, -2.666214528287778e-28, -2.248519241544640e-28,  4.450576621929338e-22, -2.358631566905962e-24,  7.512050477594792e-28, -9.591616969540117e-29, -8.099239148449887e-29,  1.219136530295899e-22, -1.664052789972254e-25,  1.988899583820249e-28, -3.266229613456442e-29, -2.660296808107736e-29,  3.016669594564926e-23, -9.809816746503891e-27,  4.917391644914019e-29, -1.016476204363154e-29, -7.963685200252959e-30,  6.737259576708041e-24, -5.198926435409626e-28,  1.106465704700631e-29, -2.862493410156643e-30, -2.171746248176351e-30,  1.358528192832975e-24, -3.286162656589790e-29,  2.245307575584594e-30, -7.283612954709367e-31, -5.397250090223691e-31,  2.474209551260810e-25, -3.556879472566343e-30,  4.094532802665354e-31, -1.675059552087905e-31, -1.222350384316561e-31,  4.069920466191034e-26, -5.371192394150234e-31,  6.692700063432779e-32, -3.484628829395268e-32, -2.523009568753440e-32,  6.051792861149525e-27, -8.243952012206658e-32,  9.774577391985403e-33, -6.561899876147630e-33, -4.747971448455573e-33};


    // Get datastructures
    std::vector<pwm::SparseMatrix<double, int>*> matrices = pwm::get_all_matrices<double, int>();
    double* x = new double[mat_size];
    double* y = new double[mat_size];

    // Run test on all the matrices
    for (size_t mat_index = 0; mat_index < matrices.size(); ++mat_index) {
        pwm::SparseMatrix<double, int>* mat = matrices[mat_index];

        // Get omp max threads
        int max_threads = omp_get_max_threads();

        // If we have a TBB implementation set a global limiter to overwrite other limits
        tbb::global_control global_limit(tbb::global_control::max_allowed_parallelism, pwm::get_threads_for_matrix(mat_index));
        
        for (int partitions = 1; partitions <= std::min(max_threads*2, mat_size); ++partitions) {
            mat->loadFromTriplets(input_mat, partitions);
            std::fill(x, x+mat_size, 1.);
            mat->powerMethod(x, y, 100);

            // Check solution
            for (int i = 0; i < mat_size; ++i) {
                BOOST_TEST(x[i] == real_sol[i]);
            }

            // If matrix is an omp or TBB matrix break because all executions are the same
            if ((mat_index-1) % 6 == 0 || (mat_index-2) % 6 == 0) {
                break;
            }
        }

        // Reset omp threads
        omp_set_num_threads(max_threads);
    }
}

BOOST_AUTO_TEST_SUITE_END()