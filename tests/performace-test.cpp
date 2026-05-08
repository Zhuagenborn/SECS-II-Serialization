#include "secs2/secs2.h"
#include <gtest/gtest.h>
#include <chrono>

using namespace secs2;
TEST(Secs2PerformanceTest, LargeBinaryToSml) {
    Binary bigBin(10000000, static_cast<std::byte>(0xFF)); // 10 million bytes
    Message msg {bigBin};

    auto start = std::chrono::high_resolution_clock::now();
    std::string sml = msg.ToSml();
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;

    // Output the time taken for the conversion
    std::cout << "Time taken to convert large binary(" << bigBin.size() << " bytes) to SML: " << elapsed.count() << " seconds\n";
}