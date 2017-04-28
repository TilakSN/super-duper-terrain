#pragma once

#include <bits/stdc++.h>
#include "textureGL.h"
using namespace std;

class Mesh2D {
    protected:
        vector<float> position;
        vector<float> normal;
        vector<float> texture_coordinates;
        int num_vertices;

        void generate_vertex(float t_x, float t_y, float p_x, float p_y, TextureGL &texture) {
            texture_coordinates.push_back(t_x);
            texture_coordinates.push_back(t_y);
            position.push_back(p_x);
            position.push_back(p_y);
            position.push_back(texture.get_greyscale(t_x, t_y));
            normal.push_back(0);
            normal.push_back(0);
            normal.push_back(1);
            num_vertices++;
        }

    public:
        Mesh2D(int n, TextureGL &texture) {
            num_vertices = 0;
            // Generate mesh from -1 to +1 in X-Y plane
            float p_x = -1, p_y, t_x = 0, t_y, p_i = 2.0 / n, t_i = 1.0 / n;
            for (int i = -1; ++i < n;) {
                p_y = -1;
                t_y = 0;
                for (int j = -1; ++j < n;) {
                    generate_vertex(t_x, t_y, p_x, p_y, texture);
                    generate_vertex(t_x + t_i, t_y, p_x + p_i, p_y, texture);
                    generate_vertex(t_x + t_i, t_y + t_i, p_x + p_i, p_y + p_i, texture);

                    generate_vertex(t_x, t_y, p_x, p_y, texture);
                    generate_vertex(t_x + t_i, t_y + t_i, p_x + p_i, p_y + p_i, texture);
                    generate_vertex(t_x, t_y + t_i, p_x, p_y + p_i, texture);

                    p_y += p_i;
                    t_y += t_i;
                }
                p_x += p_i;
                t_x += t_i;
            }
        }

        vector<float> get_position() const {
            return position;
        }

        vector<float> get_normal() const {
            return normal;
        }

        vector<float> get_texture_coordinates() const {
            return texture_coordinates;
        }

        int get_num_vertices() const {
            return num_vertices;
        }
};