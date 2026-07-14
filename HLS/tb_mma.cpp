#include <iostream>
#include <cmath>
#include <ap_fixed.h>

#define TILE_SIZE 12

typedef ap_fixed<16,8> data_t;

// Prototype of the HLS accelerator
void mma_top(
    const data_t *A,
    const data_t *B,
    data_t *C);

int main()
{
    //----------------------------------------------------------
    // Allocate "DDR" Buffers
    //----------------------------------------------------------
    data_t mat_A[TILE_SIZE * TILE_SIZE];
    data_t mat_B[TILE_SIZE * TILE_SIZE];
    data_t hw_result[TILE_SIZE * TILE_SIZE];

    // Golden reference
    data_t sw_result[TILE_SIZE][TILE_SIZE];

    //----------------------------------------------------------
    // Initialize matrices
    //----------------------------------------------------------
    for(int i=0;i<TILE_SIZE;i++)
    {
        for(int j=0;j<TILE_SIZE;j++)
        {
            mat_A[i*TILE_SIZE + j] = (data_t)(i*0.5 + 1.0);
            mat_B[i*TILE_SIZE + j] = (data_t)(j*0.25 + 0.5);

            sw_result[i][j] = 0;
            hw_result[i*TILE_SIZE + j] = 0;
        }
    }

    //----------------------------------------------------------
    // Software Reference
    //----------------------------------------------------------
    for(int i=0;i<TILE_SIZE;i++)
    {
        for(int j=0;j<TILE_SIZE;j++)
        {
            for(int k=0;k<TILE_SIZE;k++)
            {
                sw_result[i][j] +=
                    mat_A[i*TILE_SIZE + k] *
                    mat_B[k*TILE_SIZE + j];
            }
        }
    }

    //----------------------------------------------------------
    // Run Accelerator
    //----------------------------------------------------------
    std::cout << "Running MMA accelerator..." << std::endl;

    mma_top(mat_A, mat_B, hw_result);

    std::cout << "Accelerator finished." << std::endl;

    //----------------------------------------------------------
    // Print Sample Results
    //----------------------------------------------------------
    std::cout << "\nSample Results (Top-left 4x4)\n";

    for(int i=0;i<4;i++)
    {
        for(int j=0;j<4;j++)
        {
            std::cout
                << "SW: "
                << sw_result[i][j].to_double()
                << "   HW: "
                << hw_result[i*TILE_SIZE + j].to_double()
                << std::endl;
        }
    }

    //----------------------------------------------------------
    // Verify Results
    //----------------------------------------------------------
    int errors = 0;

    const double tolerance = 0.05;

    for(int i=0;i<TILE_SIZE;i++)
    {
        for(int j=0;j<TILE_SIZE;j++)
        {
            double sw = sw_result[i][j].to_double();
            double hw = hw_result[i*TILE_SIZE + j].to_double();

            if(std::abs(sw-hw) > tolerance)
            {
                std::cout
                    << "Mismatch @ ("
                    << i << ","
                    << j << ") "
                    << "Expected = "
                    << sw
                    << "  Got = "
                    << hw
                    << std::endl;

                errors++;
            }
        }
    }

    //----------------------------------------------------------
    // Final Status
    //----------------------------------------------------------
    std::cout << "\n----------------------------------------" << std::endl;

    if(errors == 0)
    {
        std::cout << "TEST PASSED" << std::endl;
        return 0;
    }
    else
    {
        std::cout << "TEST FAILED (" << errors << " errors)" << std::endl;
        return 1;
    }
}
