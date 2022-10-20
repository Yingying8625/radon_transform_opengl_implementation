#version 450
layout(location = 0)  uniform sampler2D	A;
layout(location = 0)  out vec4 O;

// rotate txc by ang radians about cx
vec2 rotate(vec2 txc, float ang, vec2 cx)
{
    return vec2(
        cos(ang) * (txc.x - cx.x) - sin(ang) * (txc.y - cx.y) + cx.x,
        cos(ang) * (txc.y - cx.y) + sin(ang) * (txc.x - cx.x) + cx.y
    );
}

#define M_PI 3.1415926535897932384626433832795

void main()
{
    ivec2 Asz = textureSize(A, 0);
    ivec2 coord = ivec2(gl_FragCoord.yx);
    vec4 accum = vec4(0.0);

    // get the angle for this y location
    float angle = float(coord.y) / float(Asz.y);
    //  float angle = float(100.0)/float(Asz.y);

    for (int y = 0; y < Asz.y; y++)
    {
        // rotate the xy
        vec2 txc = rotate(
            vec2(coord.x, y),
            -angle * 1.0 * M_PI,
            vec2(Asz) / 2.0
        );

        accum += texelFetch(A, ivec2(txc), 0) / float(Asz.y);
    }

    // output
  //  O = vec4(vec3(angle), 1.0);
  //  O = vec4(vec3(txc, 0.0), 1.0);
  //  O = texelFetch(A, ivec2(txc), 0);
    O = accum;
}
