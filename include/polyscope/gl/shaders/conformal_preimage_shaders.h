// Copyright 2017-2019, Nicholas Sharp and the Polyscope contributors. http://polyscope.run.
#pragma once

#include "polyscope/gl/shaders.h"

// clang-format off

namespace polyscope {
namespace gl {

static const VertShader CONFORMAL_PREIMAGE_VERT_SHADER =  {

    // uniforms
    {
        {"u_modelView", GLData::Matrix44Float},
        {"u_projMatrix", GLData::Matrix44Float},
    },

    // attributes
    {
        {"a_position", GLData::Vector3Float},
        {"a_normal", GLData::Vector3Float},
        {"a_scale_factor", GLData::Vector3Float},
        {"a_barycoord", GLData::Vector3Float},
        {"a_texture_coord0", GLData::Vector3Float},
        {"a_texture_coord1", GLData::Vector3Float},
        {"a_texture_coord2", GLData::Vector3Float},
    },

    // source
    POLYSCOPE_GLSL(150,
      uniform mat4 u_modelView;
      uniform mat4 u_projMatrix;
      in vec3 a_position;
      in vec3 a_normal;
      in vec3 a_scale_factor;
      in vec3 a_barycoord;
      in vec3 a_texture_coord0;
      in vec3 a_texture_coord1;
      in vec3 a_texture_coord2;
      out vec3 Normal;
      out vec3 scaleFactor;
      out vec3 baryCoord;
      out vec3 tCoord0;
      out vec3 tCoord1;
      out vec3 tCoord2;

      void main()
      {
          Normal = mat3(u_modelView) * a_normal;
          gl_Position = u_projMatrix * u_modelView * vec4(a_position,1.);
          baryCoord = a_barycoord;
          scaleFactor = a_scale_factor;
          tCoord0 = a_texture_coord0;
          tCoord1 = a_texture_coord1;
          tCoord2 = a_texture_coord2;
      }
    )
};

static const FragShader CONFORMAL_PREIMAGE_FRAG_SHADER = {

    // uniforms
    {
     {"u_projectiveInterpolate", GLData::Int},
    },

    // attributes
    {
    },

    // textures
    {
     {"t_earth", TextureTarget::Cube},
     {"t_mat_r", TextureTarget::TwoD},
     {"t_mat_g", TextureTarget::TwoD},
     {"t_mat_b", TextureTarget::TwoD},
    },

    // output location
    "outputF",

    // source
    POLYSCOPE_GLSL(150,
      uniform samplerCube t_earth;
      uniform sampler2D t_mat_r;
      uniform sampler2D t_mat_g;
      uniform sampler2D t_mat_b;
      uniform int u_projectiveInterpolate;
      in vec3 scaleFactor;
      in vec3 baryCoord;
      in vec3 Normal;
      in vec3 tCoord0;
      in vec3 tCoord1;
      in vec3 tCoord2;
      out vec4 outputF;

      vec4 myLightSurfaceMat(vec3 normal, vec3 color, sampler2D t_mat_r, sampler2D t_mat_g, sampler2D t_mat_b) {
        normal.y = -normal.y;
        vec2 matUV = normal.xy/2.0 + vec2(.5, .5);

        vec3 mat_r = texture(t_mat_r, matUV).rgb;
        vec3 mat_g = texture(t_mat_g, matUV).rgb;
        vec3 mat_b = texture(t_mat_b, matUV).rgb;
        vec3 colorCombined = color.r * mat_r + color.g * mat_g + color.b * mat_b;

        return vec4(colorCombined, 1.0);
      }


                   // method implemented in common.h
      vec4 lightSurfaceMat(vec3 normal, vec3 color, sampler2D t_mat_r, sampler2D t_mat_g, sampler2D t_mat_b) ;

      void main()
      {

        vec3 pos0 = tCoord0 / baryCoord[0];
        vec3 pos1 = tCoord1 / baryCoord[1];
        vec3 pos2 = tCoord2 / baryCoord[2];

        float b0 = baryCoord[0];
        float b1 = baryCoord[1];
        float b2 = baryCoord[2];

        float s0 = exp(-scaleFactor[0] / baryCoord[0]) * baryCoord[0];
        float s1 = exp(-scaleFactor[1] / baryCoord[1]) * baryCoord[1];
        float s2 = exp(-scaleFactor[2] / baryCoord[2]) * baryCoord[2];

        float sTotal = s0 + s1 + s2;
        s0 /= sTotal;
        s1 /= sTotal;
        s2 /= sTotal;

        vec3 projectiveCoord;
        if (u_projectiveInterpolate > 0) {
          projectiveCoord =
            s0 * pos0 + s1 * pos1 + s2 * pos2;
        } else {
          projectiveCoord =
            b0 * pos0 + b1 * pos1 + b2 * pos2;
        }
        vec3 color = texture(t_earth, projectiveCoord).rgb;
        outputF = myLightSurfaceMat(Normal, color, t_mat_r, t_mat_g, t_mat_b);
      }

    )
};

// clang-format on

} // namespace gl
} // namespace polyscope
