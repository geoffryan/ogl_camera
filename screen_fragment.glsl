#version 400

#define PI 3.14159265359f

in vec2 TexUV;

uniform sampler2D sceneTexture;
uniform sampler2D depthTexture;

out vec4 FragColor;

float rand(vec2 xy)
{
    return fract(sin(dot(xy, vec2(12.9898, 78.233))) * 43758.5453);
}

vec4 blur2x2(vec2 xy, float sigma, float sx, float sy)
{
    vec4 val = vec4(0.f, 0.f, 0.f, 0.f);

    vec2 dx = vec2(sigma*sx, 0.0f);
    vec2 dy = vec2(0.0f, sigma*sy);

    float k[4] = float[](0.25f, 0.25f, 0.25f, 0.25f);

    int i, j;
    for(i=0; i<2; i++)
        for(j=0; j<2; j++)
            val += k[2*i+j] * texture(sceneTexture,
                                      xy + (i-0.5)*dx + (j-0.5)*dy);
    
    return val;
}

vec4 blur3x3(vec2 xy, float sigma, float sx, float sy)
{
    vec4 val = vec4(0.f, 0.f, 0.f, 0.f);

    vec2 dx = vec2(sigma*sx, 0.0f);
    vec2 dy = vec2(0.0f, sigma*sy);

    float k[9] = float[](7.511361e-02, 1.238414e-01, 7.511361e-02,
                         1.238414e-01, 2.041800e-01, 1.238414e-01,
                         7.511361e-02, 1.238414e-01, 7.511361e-02);

    int i, j;
    for(i=0; i<3; i++)
        for(j=0; j<3; j++)
            val += k[3*i+j] * texture(sceneTexture,
                                      xy + (i-1)*dx + (j-1)*dy);
    
    return val;
}

vec4 blur5x5(vec2 xy, float sigma, float sx, float sy)
{
    vec4 val = vec4(0.f, 0.f, 0.f, 0.f);

    vec2 dx = vec2(sigma*sx, 0.0f);
    vec2 dy = vec2(0.0f, sigma*sy);

    float k[25] = float[](
        2.969017e-03, 1.330621e-02, 2.193823e-02, 1.330621e-02, 2.969017e-03,
        1.330621e-02, 5.963430e-02, 9.832033e-02, 5.963430e-02, 1.330621e-02, 
        2.193823e-02, 9.832033e-02, 1.621028e-01, 9.832033e-02, 2.193823e-02, 
        1.330621e-02, 5.963430e-02, 9.832033e-02, 5.963430e-02, 1.330621e-02, 
        2.969017e-03, 1.330621e-02, 2.193823e-02, 1.330621e-02, 2.969017e-03);

    int i, j;
    for(i=0; i<5; i++)
        for(j=0; j<5; j++)
            val += k[5*i+j] * texture(sceneTexture,
                                      xy + (i-2)*dx + (j-2)*dy);
    
    return val;
}

vec4 blur7x7(vec2 xy, float sigma, float sx, float sy)
{
    vec4 val = vec4(0.f, 0.f, 0.f, 0.f);

    vec2 dx = vec2(sigma*sx, 0.0f);
    vec2 dy = vec2(0.0f, sigma*sy);

    float k[49] = float[](
        1.965192e-05, 2.394093e-04, 1.072958e-03, 1.769009e-03,
        1.072958e-03, 2.394093e-04, 1.965192e-05, 
        2.394093e-04, 2.916603e-03, 1.307131e-02, 2.155094e-02,
        1.307131e-02, 2.916603e-03, 2.394093e-04, 
        1.072958e-03, 1.307131e-02, 5.858154e-02, 9.658463e-02,
        5.858154e-02, 1.307131e-02, 1.072958e-03, 
        1.769009e-03, 2.155094e-02, 9.658463e-02, 1.592411e-01,
        9.658463e-02, 2.155094e-02, 1.769009e-03, 
        1.072958e-03, 1.307131e-02, 5.858154e-02, 9.658463e-02,
        5.858154e-02, 1.307131e-02, 1.072958e-03, 
        2.394093e-04, 2.916603e-03, 1.307131e-02, 2.155094e-02,
        1.307131e-02, 2.916603e-03, 2.394093e-04, 
        1.965192e-05, 2.394093e-04, 1.072958e-03, 1.769009e-03,
        1.072958e-03, 2.394093e-04, 1.965192e-05
    );

    int i, j;
    for(i=0; i<7; i++)
        for(j=0; j<7; j++)
            val += k[7*i+j] * texture(sceneTexture,
                                      xy + (i-2)*dx + (j-2)*dy);
    
    return val;
}

void main()
{
    float maxT = 50.0f;
    float depth = texture(depthTexture, TexUV).r * (maxT + 1.0f);
    
    float f = 0.055f;  // 55mm focal length
    float N = 0.001f;    // f-stop = 5, so aperture diameter is: 11mm
    float sensor_height = 0.016f;  // 35mm film, academy format, 16mm tall
    float sensor_width = 0.022f;  // 35mm film, academy format, 22mm wide

    float s = 15.0f;



    //vec3 fv = vec3(1.0025f, 1.0f, 0.9975f) * f;
    vec3 fv = vec3(1.f, 1.f, 1.f) * f;
    vec3 Nv = (N/f) * fv;
    float s2 = f*s / (s - f);
    vec3 sv = fv*s2 / (s2 - fv);

    //Assume focused at infinity for now
    vec3 cv;
    if(s > maxT)
        cv = 0.5f*fv*fv/(Nv * depth);
    else
        cv = 0.5f*fv*fv * abs(depth-sv) / (Nv * depth * (sv - fv));

    float dxi = 1.f/800.f;
    float dyi = 1.f/600.f;

    float fov = 1.0;

    // re-doing to make consistent with painting
    sensor_height = f * fov;

    vec4 color = vec4(blur7x7(TexUV, cv.r/sensor_height, dxi, dyi).r,
                      blur7x7(TexUV, cv.g/sensor_height, dxi, dyi).g,
                      blur7x7(TexUV, cv.b/sensor_height, dxi, dyi).b, 1.0f);

    // some hacky chromatic aberration
    float mag = 0.02f;

    vec2 center = vec2(0.5f, 0.5f);

    vec2 dxR = (TexUV - center) * 0.3f * mag;
    vec2 dxG = (TexUV - center) * 0.6f * mag;
    vec2 dxB = (TexUV - center) * 1.0f * mag;

    /*
    vec4 color = vec4(texture(sceneTexture, TexUV + dxR).r,
                      texture(sceneTexture, TexUV + dxG).g,
                      texture(sceneTexture, TexUV + dxB).b, 1.0f);
    */

    float noiseR = clamp(
                        sqrt(-2*log(rand(dxR*1000.0f)*0.999f + 0.001f))
                            * cos(2*PI * rand(-dxR*1000.0f)),
                        -5, 5);
    float noiseG = clamp(
                        sqrt(-2*log(rand(dxG*1000.0f)*0.999f + 0.001f))
                            * cos(2*PI * rand(-dxG*1000.0f)),
                        -5, 5);
    float noiseB = clamp(
                        sqrt(-2*log(rand(dxB*1000.0f)*0.999f + 0.001f))
                            * cos(2*PI * rand(-dxB*1000.0f)),
                        -5, 5);

    float noiseMagR = 0.01f;
    float noiseMagG = 0.01f;
    float noiseMagB = 0.01f;

    FragColor = vec4(color.r + noiseMagR*noiseR, color.g + noiseMagG*noiseG,
                     color.b + noiseMagB*noiseB, 1.0f);

}
