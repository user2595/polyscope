// Copyright 2017-2019, Nicholas Sharp and the Polyscope contributors. http://polyscope.run.

#include "polyscope/render/opengl/gl_shaders.h"

namespace polyscope {
namespace render {

// clang-format off

const ShaderStageSpecification PASSTHRU_CUBE_VERT_SHADER = {

    ShaderStageType::Vertex,

    {
        {"u_modelView", DataType::Matrix44Float},
    }, // uniforms

    // attributes
    {
        {"a_position", DataType::Vector3Float},
        {"a_vector_x", DataType::Vector3Float},
        {"a_vector_y", DataType::Vector3Float},
        {"a_vector_z", DataType::Vector3Float},
    },

    {}, // textures

    // source
    POLYSCOPE_GLSL(150,
        uniform mat4 u_modelView;
        in vec3 a_position;
        in vec3 a_vector_x;
        in vec3 a_vector_y;
        in vec3 a_vector_z;

        out vec4 vectorX;
        out vec4 vectorY;
        out vec4 vectorZ;

        void main()
        {
            gl_Position = u_modelView * vec4(a_position,1.0);
            vectorX = u_modelView * vec4(a_vector_x, 0.0);
            vectorY = u_modelView * vec4(a_vector_y, 0.0);
            vectorZ = u_modelView * vec4(a_vector_z, 0.0);
        }
    )
};



const ShaderStageSpecification CUBE_GEOM_SHADER = {

    ShaderStageType::Geometry,

    // uniforms
    {
        {"u_projMatrix", DataType::Matrix44Float},
        {"u_lengthMult", DataType::Float},
        {"u_baseColorX", DataType::Vector3Float},
        {"u_baseColorY", DataType::Vector3Float},
        {"u_baseColorZ", DataType::Vector3Float},
    },

    { }, // attributes

    {}, // textures

    // source
    POLYSCOPE_GLSL(150,
        layout(points) in;
        layout(triangle_strip, max_vertices=24) out;
        in vec4 vectorX[];
        in vec4 vectorY[];
        in vec4 vectorZ[];
        uniform mat4 u_projMatrix;
        uniform float u_lengthMult;
        uniform vec3 u_baseColorX;
        uniform vec3 u_baseColorY;
        uniform vec3 u_baseColorZ;
        out vec3 color;
        out vec3 Normal;

        void main()   {

            vec3 centerView = gl_in[0].gl_Position.xyz / gl_in[0].gl_Position.w;
            vec3 xViewVal = vectorX[0].xyz * u_lengthMult;
            vec3 yViewVal = vectorY[0].xyz * u_lengthMult;
            vec3 zViewVal = vectorZ[0].xyz * u_lengthMult;

            vec4 c1 = u_projMatrix * vec4(centerView + xViewVal + yViewVal + zViewVal, 1.0);
            vec4 c2 = u_projMatrix * vec4(centerView - xViewVal + yViewVal + zViewVal, 1.0);
            vec4 c3 = u_projMatrix * vec4(centerView - xViewVal - yViewVal + zViewVal, 1.0);
            vec4 c4 = u_projMatrix * vec4(centerView + xViewVal - yViewVal + zViewVal, 1.0);
            vec4 c5 = u_projMatrix * vec4(centerView + xViewVal + yViewVal - zViewVal, 1.0);
            vec4 c6 = u_projMatrix * vec4(centerView - xViewVal + yViewVal - zViewVal, 1.0);
            vec4 c7 = u_projMatrix * vec4(centerView - xViewVal - yViewVal - zViewVal, 1.0);
            vec4 c8 = u_projMatrix * vec4(centerView + xViewVal - yViewVal - zViewVal, 1.0);

            // X Axis Faces
            color = u_baseColorX; Normal =  xViewVal; gl_Position = c1; EmitVertex();
            color = u_baseColorX; Normal =  xViewVal; gl_Position = c4; EmitVertex();
            color = u_baseColorX; Normal =  xViewVal; gl_Position = c5; EmitVertex();
            color = u_baseColorX; Normal =  xViewVal; gl_Position = c8; EmitVertex();
            EndPrimitive();
            color = u_baseColorX; Normal = -xViewVal; gl_Position = c2; EmitVertex();
            color = u_baseColorX; Normal = -xViewVal; gl_Position = c6; EmitVertex();
            color = u_baseColorX; Normal = -xViewVal; gl_Position = c3; EmitVertex();
            color = u_baseColorX; Normal = -xViewVal; gl_Position = c7; EmitVertex();
            EndPrimitive();

            // Y Axis Faces
            color = u_baseColorY; Normal =  yViewVal; gl_Position = c1; EmitVertex();
            color = u_baseColorY; Normal =  yViewVal; gl_Position = c5; EmitVertex();
            color = u_baseColorY; Normal =  yViewVal; gl_Position = c2; EmitVertex();
            color = u_baseColorY; Normal =  yViewVal; gl_Position = c6; EmitVertex();
            EndPrimitive();
            color = u_baseColorY; Normal = -yViewVal; gl_Position = c4; EmitVertex();
            color = u_baseColorY; Normal = -yViewVal; gl_Position = c3; EmitVertex();
            color = u_baseColorY; Normal = -yViewVal; gl_Position = c8; EmitVertex();
            color = u_baseColorY; Normal = -yViewVal; gl_Position = c7; EmitVertex();
            EndPrimitive();

            // Z Axis Faces
            color = u_baseColorZ; Normal =  zViewVal; gl_Position = c1; EmitVertex();
            color = u_baseColorZ; Normal =  zViewVal; gl_Position = c2; EmitVertex();
            color = u_baseColorZ; Normal =  zViewVal; gl_Position = c4; EmitVertex();
            color = u_baseColorZ; Normal =  zViewVal; gl_Position = c3; EmitVertex();
            EndPrimitive();
            color = u_baseColorZ; Normal = -zViewVal; gl_Position = c5; EmitVertex();
            color = u_baseColorZ; Normal = -zViewVal; gl_Position = c8; EmitVertex();
            color = u_baseColorZ; Normal = -zViewVal; gl_Position = c6; EmitVertex();
            color = u_baseColorZ; Normal = -zViewVal; gl_Position = c7; EmitVertex();
            EndPrimitive();
        }
    )
};



const ShaderStageSpecification CUBE_FRAG_SHADER = {

    ShaderStageType::Fragment,

    // uniforms
    {
    },

    { }, // attributes

    // textures
    {
        {"t_mat_r", 2},
        {"t_mat_g", 2},
        {"t_mat_b", 2},
        {"t_mat_k", 2},
    },

    // source
    POLYSCOPE_GLSL(330 core,
        in vec3 Normal;
        in vec3 color;
        uniform sampler2D t_mat_r;
        uniform sampler2D t_mat_g;
        uniform sampler2D t_mat_b;
        uniform sampler2D t_mat_k;
        layout(location = 0) out vec4 outputF;


        vec3 lightSurfaceMat(vec3 normal, vec3 color, sampler2D t_mat_r, sampler2D t_mat_g, sampler2D t_mat_b, sampler2D t_mat_k);

        void main()
        {
          outputF = vec4(lightSurfaceMat(normalize(Normal), color, t_mat_r, t_mat_g, t_mat_b, t_mat_k), 1.);
        }

    )
};

// clang-format on

} // namespace render
} // namespace polyscope
