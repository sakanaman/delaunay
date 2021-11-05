#ifndef _GEOM_
#define _GEOM_


#include<list>
#include<vector>
#include<array>
#include<algorithm>
#include<cassert>


class Triangle
{
    public:

    Triangle(int a, int b, int c)
    {
        v_indices = {a, b, c};

        // 昇順で格納する
        std::sort(v_indices.begin(), v_indices.end());
    }

    const int& operator[](const int i) const
    {
        return v_indices[i];
    }

    private:

    std::array<int, 3> v_indices;
};


#endif