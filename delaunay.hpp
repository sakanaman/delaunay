#ifndef _DELAUNAY_
#define _DELAUNAY_

#include "geometry.hpp"

#include<set>
#include<cmath>
#include <string>


class Delaunay
{
    public:

    //外部境界のリストを与えてドロネー分割の初期状態を構築する
    Delaunay(const std::vector<double>& out_boundary, const std::vector<double>& big_triangle)
    {

        // 外部三角形の頂点を頂点配列に追加し、外部三角形を三角形リストに追加する。
        int i0 = add_vertices(big_triangle[0], big_triangle[1]);
        int i1 = add_vertices(big_triangle[2], big_triangle[3]);
        int i2 = add_vertices(big_triangle[4], big_triangle[5]);
        add_triangle({i0, i1, i2});

        //外部境界を頂点配列に追加し、ドロネー分割を行う。
        for(int i = 0; i < out_boundary.size()/2; ++i)
        {
            add_point(out_boundary[2 * i], out_boundary[2 * i + 1]);
        }

        //外部頂点を持つ三角形の削除(頂点は削除しない)
        auto delfunc = [&](const Triangle& tri)
        {
            bool pattern_i0 = tri[0] == i0 || tri[1] == i0 || tri[2] == i0;
            bool pattern_i1 = tri[0] == i1 || tri[1] == i1 || tri[2] == i1;
            bool pattern_i2 = tri[0] == i2 || tri[1] == i2 || tri[2] == i2;
            
            return pattern_i0 || pattern_i1 || pattern_i2;
        };
        delete_triangle(delfunc);
    }

    // ドロネー分割を維持するように点を追加する。
    void add_point(double x, double y)
    {
        // 頂点配列に新しく加える頂点を格納する
        int index = add_vertices(x, y);

        //加えた頂点を外接円内部に含む三角形を全探索および削除
        std::vector<Triangle> circum_tris;
        for(auto it = data.begin(); it != data.end();)
        {
            if(in_circumcircle(x, y, *it))
            {
                // 消す前に保存
                circum_tris.push_back(*it);
                // 消す。イテレーターは次の要素に移る
                it = data.erase(it);
            }
            else
            {
                //イテレーターを進めるだけ
                ++it;
            }
        }


        // 加えた頂点を外接円内部に含む三角形の辺の配列をつくる(重複を許す)
        std::vector<std::array<int, 2>> edges;
        for(const auto& tri : circum_tris)
        {
            edges.push_back({tri[0], tri[1]});
            edges.push_back({tri[0], tri[2]});
            edges.push_back({tri[1], tri[2]});
        }

        // 辺の配列を適当なIDでソートする(重複する辺を見つけるため)
        int n = vertices.size();
        auto compare = [&](const std::array<int, 2>& e1, const std::array<int, 2>& e2)
        {
            assert(e1[0] < e1[1] && e2[0] < e2[1]);

            return n * e1[0] + e1[1] < n * e2[0] + e2[1];
        };
        std::sort(edges.begin(), edges.end(), compare);


        // 重複する辺＝共有辺なので重複辺以外の辺を探索、格納する。
        std::vector<std::array<int, 2>> independent_edges;
        for(int i = 0; i < edges.size(); ++i)
        {
            if(i == 0)
            {
                if(edges[i] != edges[i+1])
                {
                    independent_edges.push_back(edges[i]);
                }
            }
            else if(i == edges.size() - 1)
            {
                if(edges[i-1] != edges[i])
                {
                    independent_edges.push_back(edges[i]);
                }
            }
            else
            {
                if(edges[i-1] != edges[i] && edges[i] != edges[i+1])
                {
                    independent_edges.push_back(edges[i]);
                }
            }
        }

        // 共有辺でない辺と追加点を結んで、新たなドロネー分割を構築する
        for(const auto& e : independent_edges)
        {
            add_triangle({e[0], e[1], index});
        }
    }

    // 任意の三角形を削除
    void delete_triangle(const std::function<bool(const Triangle&)>& func)
    {
        data.remove_if(func);
    }

    //三角形の探索
    void serch_triangles(std::vector<Triangle>& findtriangles, 
                         const std::function<bool(const Triangle&)>& func)
    {
        for(const Triangle& tri : data)
        {
            if(func(tri))
            {
                findtriangles.push_back(tri);
            }
        }
    }

    void report(const std::string& filename, const std::vector<std::string> want_data)
    {
        int num_vertices = vertices.size() / 2;
        int num_triangles = data.size();

        FILE* f = fopen(filename.c_str(), "w");

        for(auto memodata : want_data)
        {
            if(memodata == "VERTEX")
            {
                fprintf(f, "[data:vertices]\n");
                fprintf(f, "num_vertices: %d\n", num_vertices);
                for(int i = 0; i < num_vertices; ++i)
                {
                    fprintf(f, "%.9f %.9f\n", vertices[2 * i], vertices[2 * i + 1]);
                }
            }
            else if(memodata == "TRIANGLE") //このデータが必要になるのは有限要素法の時くらいかな？
            {
                fprintf(f, "[data:triangles]\n");
                fprintf(f, "num_triangles: %d\n", num_triangles);
                for(auto tri : data)
                {
                    fprintf(f, "%d %d %d\n", tri[0], tri[1], tri[2]);
                }              
            }
            else if(memodata == "EDGE")
            {
                fprintf(f, "[data:edges]\n");
                std::vector<std::array<int, 2>> edges;
                for(auto tri : data)
                {
                    edges.push_back({tri[0], tri[1]});
                    edges.push_back({tri[0], tri[2]});
                    edges.push_back({tri[1], tri[2]});
                }

                int n = vertices.size();
                auto compare = [&](const std::array<int, 2>& edge1, const std::array<int, 2>& edge2)
                {
                    return (n * edge1[0] + edge1[1]) < (n * edge2[0] + edge2[1]);
                };
                std::sort(edges.begin(), edges.end(), compare);

                decltype(edges)::iterator iter = std::unique(edges.begin(), edges.end());
                edges.erase(iter, edges.end());

                fprintf(f, "num_edges: %d\n", edges.size());
                for(auto edge : edges)
                {
                    fprintf(f, "%d %d\n", edge[0], edge[1]);
                }
            }
        }
    }

    private:

    // 点(x,y)が三角形の外接円の内部かどうか判定
    bool in_circumcircle(double x, double y, const Triangle& tri)
    {
        double v0[2] = {vertices[2 * tri[0]], vertices[2 * tri[0] + 1]};
        double v1[2] = {vertices[2 * tri[1]], vertices[2 * tri[1] + 1]};
        double v2[2] = {vertices[2 * tri[2]], vertices[2 * tri[2] + 1]};

        // {v0 -> v1 -> v2 -> v0}が反時計回りor時計回りかを確認(with 外積)
        // -1 => 時計回り
        // +1 => 反時計回り
        double sign_triangle = (v1[0] - v0[0]) * (v2[1] - v0[1]) - (v2[0] - v0[0]) * (v1[1] - v0[1]);
        sign_triangle = sign_triangle/std::abs(sign_triangle);

        // 行列式(http://www.thothchildren.com/chapter/5bdedb4341f88f267247fdd6)を計算する
        double a0 = (v0[0] - x), a1 = (v0[1] - y), a2 = (a0*a0 + a1*a1);
        double b0 = (v1[0] - x), b1 = (v1[1] - y), b2 = (b0*b0 + b1*b1);
        double c0 = (v2[0] - x), c1 = (v2[1] - y), c2 = (c0*c0 + c1*c1);
        double det_p = (a0 * b1 * c2) + (b0 * c1 * a2) + (a1 * b2 * c0);
        double det_m = (c0 * b1 * a2) + (b0 * a1 * c2) + (c1 * b2 * a0);
        double det = det_p - det_m;

        return det * sign_triangle > 0;
    }

    // 要素の追加
    void add_triangle(const Triangle& tri)
    {
        data.push_back(tri);
    }

    // 頂点の追加。返り値は追加した頂点のインデックス。
    int add_vertices(double x, double y)
    {
        vertices.push_back(x);
        vertices.push_back(y);

        return vertices.size() / 2 - 1;
    }

    std::list<Triangle> data;
    std::vector<double> vertices;
};


#endif
