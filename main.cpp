#include "delaunay.hpp"
#include <random>


int main()
{
    //四角形領域で実験
    std::vector<double> boundary = {-1.0, 1.0,
                                    -1.0, -1.0,
                                    1.0, -1.0,
                                    1.0, 1.0};


    std::vector<double> big_triangle = {0.0, 5.0,
                                        -5.0/sqrt(2), -5.0/sqrt(2),
                                        5.0/sqrt(2), -5.0/sqrt(2)};


    Delaunay delo(boundary, big_triangle);


    int sample_point = 1000;
    std::random_device seed;
    std::mt19937 mt(seed());

    // 一様実数分布
    // [0.0, 1.0)の値の範囲で、等確率に実数を生成する
    std::uniform_real_distribution<> dist(0.0, 1.0);
    auto rand = [&](){return dist(mt);};


    for(int i = 0; i < sample_point; ++i)
    {
        delo.add_point(2.0 * rand() - 1.0, 2.0 * rand() - 1.0);
    }




    delo.report("output.txt", {"VERTEX", "EDGE"});
}