#ifndef ORCAMLSMPMDEMO_PG_TRANSFER_H
#define ORCAMLSMPMDEMO_PG_TRANSFER_H

#include <Eigen/Dense>

#include "grid.h"
#include "particle_system.h"

using namespace Eigen;

void p2g_momentum_n_mass(Vector2f* _p_vel_begin,
                  std::array<Vector2f, 3>* _weights_begin,
                  Vector2i* _base_begin,
                  Vector2f* _fracture_begin,
                  grid& _g,
                  size_t _len) {
    float p_mass = particle_system::mass();
    for (int loop = 0; loop < _len; ++loop, ++_p_vel_begin, ++_weights_begin, ++_base_begin, ++_fracture_begin) {
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                _g.mass()[(*_base_begin)[0] + i][(*_base_begin)[1] + j] += (
                        (*_weights_begin)[i][0] * (*_weights_begin)[j][1] * (p_mass));
                _g.momentum()[(*_base_begin)[0] + i][(*_base_begin)[1] + j] += (
                        (*_weights_begin)[i][0] * (*_weights_begin)[j][1] * (p_mass * (*_p_vel_begin)));
            }
        }
    }
}

void p2g_affine_n_force(Matrix2f* _p_stress_begin,
                        Matrix2f* _affine_vel_begin,
                        float _time_delta,
                        std::array<Vector2f, 3>* _weights_begin,
                        Vector2i* _base_begin,
                        Vector2f* _fracture_begin,
                        grid& _g,
                        size_t _len) {
    float p_mass = particle_system::mass();
    float D = 0.25f * _g.get_cell_space() * _g.get_cell_space();
    for (int loop = 0; loop < _len; ++loop, ++_p_stress_begin, ++_affine_vel_begin, ++_weights_begin, ++_base_begin, ++_fracture_begin) {
        Matrix2f stress_term = -(_time_delta * particle_system::initial_volume()) * (*_p_stress_begin) * (1.0f / D);
        Matrix2f pure_affine_term = (p_mass) * (*_affine_vel_begin);

        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                Vector2f _r = (Vector2f(i, j) - (*_fracture_begin)) * _g.get_cell_space();
                _g.momentum()[(*_base_begin)[0] + i][(*_base_begin)[1] + j] += (
                        (*_weights_begin)[i][0] * (*_weights_begin)[j][1] * (stress_term + pure_affine_term) * _r);
            }
        }
    }
}

void g2p_velocity(const grid& _g,
                  std::array<Vector2f, 3>* _weights_begin,
                  Vector2i* _base_begin,
                  Vector2f* _fracture_begin,
                  Vector2f* _p_vel_dest,
                  size_t _len) {
    for (int loop = 0; loop < _len; ++loop, ++_weights_begin, ++_base_begin, ++_fracture_begin, ++_p_vel_dest) {
        (*_p_vel_dest) = Vector2f::Zero();
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                float weight = (*_weights_begin)[i][0] * (*_weights_begin)[j][1];
                (*_p_vel_dest) += weight * _g.velocity()[(*_base_begin)[0] + i][(*_base_begin)[1] + j];
            }
        }
    }
}

void g2p_affine_velocity(const grid& _g,
                         std::array<Vector2f, 3>* _weights_begin,
                         Vector2i* _base_begin,
                         Vector2f* _fracture_begin,
                         Matrix2f* _p_affine_dest,
                         size_t _len) {
    for (int loop = 0; loop < _len; ++loop, ++_weights_begin, ++_base_begin, ++_fracture_begin, ++_p_affine_dest) {
        (*_p_affine_dest) = Matrix2f::Zero();
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                Vector2f _r = (Vector2f(i, j) - (*_fracture_begin)) * _g.get_cell_space();
                Vector2f grid_v = _g.velocity()[(*_base_begin)[0] + i][(*_base_begin)[1] + j];
                float weight = (*_weights_begin)[i][0] * (*_weights_begin)[j][1];

                (*_p_affine_dest) += (4.0f / _g.get_cell_space()) * weight * grid_v * _r.transpose();
            }
        }
    }
}
#endif //ORCAMLSMPMDEMO_PG_TRANSFER_H
