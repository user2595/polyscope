// Copyright 2017-2019, Nicholas Sharp and the Polyscope contributors. http://polyscope.run.
#pragma once

#include "polyscope/gl/shaders.h"

namespace polyscope {
namespace gl {

// clang-format off

static const VertShader PROJECTIVE_PARAM_SURFACE_VERT_SHADER =  {

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
        {"a_texture_coord0", GLData::Vector2Float},
        {"a_texture_coord1", GLData::Vector2Float},
        {"a_texture_coord2", GLData::Vector2Float},
    },

    // source
    POLYSCOPE_GLSL(150,
      uniform mat4 u_modelView;
      uniform mat4 u_projMatrix;
      in vec3 a_position;
      in vec3 a_normal;
      in vec3 a_scale_factor;
      in vec3 a_barycoord;
      in vec2 a_texture_coord0;
      in vec2 a_texture_coord1;
      in vec2 a_texture_coord2;
      out vec3 Normal;
      out vec3 scaleFactor;
      out vec3 baryCoord;
      out vec2 tCoord0;
      out vec2 tCoord1;
      out vec2 tCoord2;

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

static const FragShader PROJECTIVE_PARAM_CHECKER_SURFACE_FRAG_SHADER = {


    // uniforms
    {
        {"u_projectiveInterpolate", GLData::Int},
        {"u_modLen", GLData::Float},
        {"u_color1", GLData::Vector3Float},
        {"u_color2", GLData::Vector3Float},
    },

    // attributes
    {
    },

    // textures
    {
     {"t_mat_r", TextureTarget::TwoD},
     {"t_mat_g", TextureTarget::TwoD},
     {"t_mat_b", TextureTarget::TwoD},
    },

    // output location
    "outputF",

    // source
    POLYSCOPE_GLSL(150,
      uniform vec3 u_color1;
      uniform vec3 u_color2;
      uniform float u_modLen;

      uniform sampler2D t_mat_r;
      uniform sampler2D t_mat_g;
      uniform sampler2D t_mat_b;
      uniform int u_projectiveInterpolate;

      in vec3 scaleFactor;
      in vec3 baryCoord;
      in vec3 Normal;
      in vec2 tCoord0;
      in vec2 tCoord1;
      in vec2 tCoord2;
      out vec4 outputF;

      // Forward declarations of methods from <shaders/common.h>
      vec4 lightSurfaceMat(vec3 normal, vec3 color, sampler2D t_mat_r, sampler2D t_mat_g, sampler2D t_mat_b);

      void main()
      {

        vec2 pos0 = tCoord0 / baryCoord[0];
        vec2 pos1 = tCoord1 / baryCoord[1];
        vec2 pos2 = tCoord2 / baryCoord[2];

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

        vec2 Coord;
        if (u_projectiveInterpolate > 0) {
          Coord =
            s0 * pos0 + s1 * pos1 + s2 * pos2;
        } else {
          Coord =
            b0 * pos0 + b1 * pos1 + b2 * pos2;
        }

        // Apply the checkerboard effect
        float mX = mod(Coord.x, 2.0 * u_modLen) / u_modLen - 1.f; // in [-1, 1]
        float mY = mod(Coord.y, 2.0 * u_modLen) / u_modLen - 1.f;

        float minD = min( min(abs(mX), 1.0 - abs(mX)), min(abs(mY), 1.0 - abs(mY))) * 2.; // rect distace from flipping sign in [0,1]
        float p = 6;
        float minDSmooth = pow(minD, 1. / p);
        // TODO do some clever screen space derivative thing to prevent aliasing

        float v = (mX * mY); // in [-1, 1], color switches at 0
        float adjV = sign(v) * minDSmooth;

        float s = smoothstep(-1.f, 1.f, adjV);

        vec3 outColor = mix(u_color1, u_color2, s);


        outputF = lightSurfaceMat(Normal, outColor, t_mat_r, t_mat_g, t_mat_b);
      }

    )
};

static const FragShader PROJECTIVE_PARAM_GRID_SURFACE_FRAG_SHADER = {


    // uniforms
    {
        {"u_projectiveInterpolate", GLData::Int},
        {"u_modLen", GLData::Float},
        {"u_gridLineColor", GLData::Vector3Float},
        {"u_gridBackgroundColor", GLData::Vector3Float},
    },

    // attributes
    {
    },

    // textures
    {
     {"t_mat_r", TextureTarget::TwoD},
     {"t_mat_g", TextureTarget::TwoD},
     {"t_mat_b", TextureTarget::TwoD},
    },

    // output location
    "outputF",

    // source
    POLYSCOPE_GLSL(150,
      uniform vec3 u_gridLineColor;
      uniform vec3 u_gridBackgroundColor;
      uniform float u_modLen;

      uniform sampler2D t_mat_r;
      uniform sampler2D t_mat_g;
      uniform sampler2D t_mat_b;
      uniform int u_projectiveInterpolate;

      in vec3 scaleFactor;
      in vec3 baryCoord;
      in vec3 Normal;
      in vec2 tCoord0;
      in vec2 tCoord1;
      in vec2 tCoord2;
      out vec4 outputF;

      // Forward declarations of methods from <shaders/common.h>
      vec4 lightSurfaceMat(vec3 normal, vec3 color, sampler2D t_mat_r, sampler2D t_mat_g, sampler2D t_mat_b);

      void main()
      {
        vec2 pos0 = tCoord0 / baryCoord[0];
        vec2 pos1 = tCoord1 / baryCoord[1];
        vec2 pos2 = tCoord2 / baryCoord[2];

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

        vec2 Coord;
        if (u_projectiveInterpolate > 0) {
          Coord =
            s0 * pos0 + s1 * pos1 + s2 * pos2;
        } else {
          Coord =
            b0 * pos0 + b1 * pos1 + b2 * pos2;
        }

        // Apply the checkerboard effect
        float mX = mod(Coord.x, 2.0 * u_modLen) / u_modLen - 1.f; // in [-1, 1]
        float mY = mod(Coord.y, 2.0 * u_modLen) / u_modLen - 1.f;


        float minD = min(min(abs(mX), 1.0 - abs(mX)), min(abs(mY), 1.0 - abs(mY))) * 2.; // rect distace from flipping sign in [0,1]

        float width = 0.05;
        float slopeWidthPix = 5.;

        vec2 fw = fwidth(Coord);
        float scale = max(fw.x, fw.y);
        float pWidth = slopeWidthPix * scale;

        float s = smoothstep(width, width + pWidth, minD);
        vec3 outColor = mix(u_gridLineColor, u_gridBackgroundColor, s);


        outputF = lightSurfaceMat(Normal, outColor, t_mat_r, t_mat_g, t_mat_b);
      }

    )
};


static const FragShader PROJECTIVE_PARAM_LOCAL_RAD_SURFACE_FRAG_SHADER = {


    // uniforms
    {
        {"u_projectiveInterpolate", GLData::Int},
        {"u_modLen", GLData::Float},
        {"u_angle", GLData::Float},
    },

    // attributes
    {
    },

    // textures
    {
     {"t_mat_r", TextureTarget::TwoD},
     {"t_mat_g", TextureTarget::TwoD},
     {"t_mat_b", TextureTarget::TwoD},
     {"t_colormap", TextureTarget::OneD}
    },

    // output location
    "outputF",

    // source
    POLYSCOPE_GLSL(150,
      uniform float u_modLen;
      uniform float u_angle;

      uniform sampler2D t_mat_r;
      uniform sampler2D t_mat_g;
      uniform sampler2D t_mat_b;
      uniform sampler1D t_colormap;
      uniform int u_projectiveInterpolate;

      in vec3 scaleFactor;
      in vec3 baryCoord;
      in vec3 Normal;
      in vec2 tCoord0;
      in vec2 tCoord1;
      in vec2 tCoord2;
      out vec4 outputF;

      // Forward declarations of methods from <shaders/common.h>
      vec4 lightSurfaceMat(vec3 normal, vec3 color, sampler2D t_mat_r, sampler2D t_mat_g, sampler2D t_mat_b);

      void main()
      {
        vec2 pos0 = tCoord0 / baryCoord[0];
        vec2 pos1 = tCoord1 / baryCoord[1];
        vec2 pos2 = tCoord2 / baryCoord[2];

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

        vec2 Coord;
        if (u_projectiveInterpolate > 0) {
          Coord =
            s0 * pos0 + s1 * pos1 + s2 * pos2;
        } else {
          Coord =
            b0 * pos0 + b1 * pos1 + b2 * pos2;
        }


        // Get the color at this point
        float pi = 3.14159265359;
        float angle = atan(Coord.y, Coord.x) / (2. * pi) + 0.5; // in [0,1]
        float shiftedAngle = mod(angle + u_angle/(2. * pi), 1.);
        vec3 color = texture(t_colormap, shiftedAngle).rgb;
        vec3 colorDark = color * .5;

        // Apply the checkerboard effect (vert similar to rectangular checker
        float mX = mod(length(Coord), 2.0 * u_modLen) / u_modLen - 1.f; // in [-1, 1]
        float minD = min(abs(mX), 1.0 - abs(mX)) * 2.; // rect distace from flipping sign in [0,1]
        float p = 6;
        float minDSmooth = pow(minD, 1. / p);
        float v = mX; // in [-1, 1], color switches at 0
        float adjV = sign(v) * minDSmooth;
        float s = smoothstep(-1.f, 1.f, adjV);

        vec3 outColor = mix(color, colorDark, s);

        outputF = lightSurfaceMat(Normal, outColor, t_mat_r, t_mat_g, t_mat_b);
      }

    )
};

static const FragShader PROJECTIVE_PARAM_LOCAL_CHECKER_SURFACE_FRAG_SHADER = {


    // uniforms
    {
        {"u_projectiveInterpolate", GLData::Int},
        {"u_modLen", GLData::Float},
        {"u_angle", GLData::Float},
    },

    // attributes
    {
    },

    // textures
    {
     {"t_mat_r", TextureTarget::TwoD},
     {"t_mat_g", TextureTarget::TwoD},
     {"t_mat_b", TextureTarget::TwoD},
     {"t_colormap", TextureTarget::OneD}
    },

    // output location
    "outputF",

    // source
    POLYSCOPE_GLSL(150,
      uniform float u_modLen;
      uniform float u_angle;

      uniform sampler2D t_mat_r;
      uniform sampler2D t_mat_g;
      uniform sampler2D t_mat_b;
      uniform sampler1D t_colormap;
      uniform int u_projectiveInterpolate;

      in vec3 scaleFactor;
      in vec3 baryCoord;
      in vec3 Normal;
      in vec2 tCoord0;
      in vec2 tCoord1;
      in vec2 tCoord2;
      out vec4 outputF;

      // Forward declarations of methods from <shaders/common.h>
      vec4 lightSurfaceMat(vec3 normal, vec3 color, sampler2D t_mat_r, sampler2D t_mat_g, sampler2D t_mat_b);

      void main()
      {
        vec2 pos0 = tCoord0 / baryCoord[0];
        vec2 pos1 = tCoord1 / baryCoord[1];
        vec2 pos2 = tCoord2 / baryCoord[2];

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

        vec2 Coord;
        if (u_projectiveInterpolate > 0) {
          Coord =
            s0 * pos0 + s1 * pos1 + s2 * pos2;
        } else {
          Coord =
            b0 * pos0 + b1 * pos1 + b2 * pos2;
        }


        // Rotate coords
        float cosT = cos(u_angle);
        float sinT = sin(u_angle);
        vec2 rotCoord = vec2(cosT * Coord.x - sinT * Coord.y, sinT * Coord.x + cosT * Coord.y);

        // Get the color at this point
        float pi = 3.14159265359;
        float angle = atan(rotCoord.y, rotCoord.x) / (2. * pi) + 0.5; // in [0,1]
        vec3 color = texture(t_colormap, angle).rgb;
        vec3 colorDark = color * .5;

        // Apply the checkerboard effect (copied from checker above)
        float mX = mod(rotCoord.x, 2.0 * u_modLen) / u_modLen - 1.f; // in [-1, 1]
        float mY = mod(rotCoord.y, 2.0 * u_modLen) / u_modLen - 1.f;
        float minD = min( min(abs(mX), 1.0 - abs(mX)), min(abs(mY), 1.0 - abs(mY))) * 2.; // rect distace from flipping sign in [0,1]
        float p = 6;
        float minDSmooth = pow(minD, 1. / p);
        float v = (mX * mY); // in [-1, 1], color switches at 0
        float adjV = sign(v) * minDSmooth;
        float s = smoothstep(-1.f, 1.f, adjV);

        vec3 outColor = mix(color, colorDark, s);

        outputF = lightSurfaceMat(Normal, outColor, t_mat_r, t_mat_g, t_mat_b);
      }

    )
};

// clang-format on

} // namespace gl
} // namespace polyscope
