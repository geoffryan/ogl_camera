#version 400

#define PI 3.14159265359f

in vec2 UV;

out vec3 color;

uniform vec2 sph_xy;
uniform ivec2 iresolution;

float sdf(in vec3 r)
{
    vec3 r0 = vec3(sph_xy.x, sph_xy.y, 1.0);
    vec3 dr = r - r0;

    return sqrt(dot(dr, dr)) - 1.0;
}

vec3 norm_sph(in vec3 r)
{
    vec3 r0 = vec3(sph_xy.x, sph_xy.y, 1.0);
    vec3 dr = r - r0;
    return normalize(dr);
}

float noise_ptval(float i, float j)
{
    float u = 50*fract(i/PI);
    float v = 50*fract(j/PI);
    return 2*fract(u*v*(u+v))-1;
}

float noise_octave(in vec2 r)
{
    float i = floor(r.x);
    float j = floor(r.y);

    float a = noise_ptval(i, j);
    float b = noise_ptval(i+1, j);
    float c = noise_ptval(i, j+1);
    float d = noise_ptval(i+1, j+1);

    float sx = smoothstep(0, 1, r.x-i);
    float sy = smoothstep(0, 1, r.y-j);

    float z = a + (b-a)*sx + (c-a)*sy + (d - b - c + a)*sx*sy;

    z = 0.5*(z + 1);

    z = smoothstep(0, 1, z);
    z = smoothstep(0, 1, z);
    z = smoothstep(0, 1, z);

    z = 2*z - 1;

    return z;
}

float noise(in vec2 r, float gfactor, int Nlayers)
{
    vec2 s = r;
    mat2 R = mat2(0.6, 0.8, -0.8, 0.6);
    float g = gfactor;
    
    float n = g*noise_octave(s);

    for(int i = 0; i<Nlayers; i++)
    {
        s = 2*R*s;
        g *= gfactor;
        n += g*noise_octave(s);
    }

    return n;

}

float height(in vec2 r)
{
    float scale = 3;
    return scale*noise(r/scale, 0.3, 10);
}

vec3 norm_grnd(in vec2 r)
{
    float d = 1.0e-3f;
    vec2 dx = vec2(d, 0);
    vec2 dy = vec2(0, d);
    float dhdx = (height(r + dx) - height(r - dx)) / (2*d);
    float dhdy = (height(r + dy) - height(r - dy)) / (2*d);
    return normalize(vec3(-dhdx, -dhdy, 1));
}

float march(in vec3 x, in vec3 v, float maxT, out uint who, out vec3 y)
{
    float t = 0.0;
    who = 0;
    float h = maxT;
    float maxStep = 0.2;

    while(t < maxT)
    {
        float h1 = sdf(x);
        float h2 = x.z - height(x.xy);
        
        if(h1 < 1.0e-2 && h1 > 0.0 && h2 > 0.0)
        {
            who = 1;
            break;
        }
        else if(h2 < 1.0e-2 && h1 > 0.0 && h2 > 0.0)
        {
            who = 2;
            break;
        }

        if(h1 <= 0.0 || h2 <= 0.0)
        {
            h = 0.5*h;
            x -= h*v;
            t -=h;
        }
        else
        {
            h = min(h1, h2);
            h = min(h, maxStep);
            x += h*v;
            t += h;
        }
    }
    y = x;
    float h1 = sdf(x);
    float h2 = x.z - height(x.xy);
    if(h1 <= 0 || h2 <= 0.0)
    {
        who = 4;
        return t;
    }
    if(t > maxT)
        return maxT;

    return t;
}

float shadow_march(in vec3 x, in vec3 v, float maxT, out uint who)
{
    float t = 0.0;
    who = 0;
    float maxStep = 0.1;

    float h1 = sdf(x);
    float h2 = x.z - height(x.xy);
    float h = min(h1, h2);
    h = min(h, maxStep);
    float hmin = h;

    /*

    if(h < 0)
    {
        who = 4;
        return 0.0;
    }

    if(h == 0)
    {
        who = 5;
        return 0.0;
    }
    */

    x += h*v;
    t += h;

    while(t < maxT)
    {
        h1 = sdf(x);
        if(h1 < 1e-3 && h1 < h)
        {
            who = 1;
            break;
        }

        h2 = x.z - height(x.xy);
        if(h2 < 1e-3 && h2 < h)
        {
            who = 2;
            break;
        }
        if(h1 <= 0.0 || h2 <= 0.0)
        {
            h = 0.5*h;
            x -= h*v;
            t -=h;
        }
        
        h = min(h1, h2);
        h = min(h, maxStep);

        x += h*v;
        t += h;
    }
    if(t > maxT)
        return maxT;

    return t;
}


void main()
{
    vec3 colorA = vec3(0.8, 0.4, 0.2);
    vec3 colorB = vec3(0.1, 0.6, 0.5);
    vec3 colorC = vec3(0.3, 0.2, 0.7);
    vec3 colorD = vec3(0.5, 0.8, 0.9);

    float fov = 1.0;

    float thDown = 0.0f; //0.8f;
    float cam_height = 1.0f; //6.0f;

    vec3 ro = vec3(0, 0, cam_height);
    vec3 rd = normalize(vec3(fov*((UV.x-0.5)*iresolution.x)/iresolution.y,
                             1.0, fov*(UV.y-0.5) -0.1));

    mat3 rot_look = mat3(1, 0, 0,
                         0, cos(thDown), -sin(thDown),
                         0, sin(thDown), cos(thDown));
    rd = rot_look * rd;

    //vec3 ro = vec3(2*(UV.x-0.5), 2*(UV.y-0.5), 0.0);
    //vec3 rd = vec3(0, 0, -1);

    float maxT = 50;
    vec3 r;
    uint who;

    float t = march(ro, rd, maxT, who, r);

    vec3 lA = vec3(1000, -500, 1000);
    vec3 lB = vec3(-10, -3, 10);
    vec3 lC = vec3(-10, -3, -10);
    vec3 lD = vec3(10, -3, -10);
    
    vec3 dlA = normalize(lA - r);
    vec3 dlB = normalize(lB - r);
    vec3 dlC = normalize(lC - r);
    vec3 dlD = normalize(lD - r);

    uint whoA;
    float tA = shadow_march(r, dlA, maxT, whoA);
    
    vec3 sky_color = 0.7*colorD;
    sky_color = UV.y * sky_color + (1-UV.y) * (0.2*sky_color + 0.8*vec3(1, 1, 1));

    if (t >= maxT)
    {
        color = sky_color;
    }
    else if(who == 1)
    {
        vec3 n = norm_sph(r);

        vec3 surfColor = clamp(dot(n, dlA), 0, 1) * colorB
                    + 0*clamp(dot(n, dlB), 0, 1) * colorB
                    + 0*clamp(dot(n, dlC), 0, 1) * colorC
                    + 0*clamp(dot(n, dlD), 0, 1) * colorD;

        color = surfColor;
    }
    else if(who == 2)
    {
        vec3 n = norm_grnd(r.xy);

        float flat_cut_a = 0.87;
        float flat_cut_b = 0.95;
        float w_flat = smoothstep(flat_cut_a, flat_cut_b, n.z);

        vec3 grnd_color = 0.8*vec3(0.6, 0.2, 0);  //colorB;
        vec3 flat_color = vec3(1, 1, 1);

        vec3 unlit_color = w_flat*flat_color + (1-w_flat)*grnd_color;

        color = clamp(dot(n, dlA), 0, 1) * unlit_color;
    }
    else if(who == 3)
        color = vec3(1, 0, 1);
    else if(who == 4)
    {
        color = vec3(1, 1, 0);
        return;
    }

    if(whoA == 4)
        color = vec3(1, 0, 0);
    else if(whoA == 5)
        color = vec3(0.0, 1.0, 0);
    else if(whoA == 6)
        color = vec3(0.0, 0.0, 1.0);
    else if(whoA != 0)
        color = vec3(0, 0, 0);

    float atm_scale = 50;

    vec3 optical_depths = vec3(1.0, 0.8, 0.6) * atm_scale;
    vec3 w_atm = exp(-t / optical_depths);

    color = w_atm*color + (1-w_atm)*sky_color;
    gl_FragDepth = t/(maxT+1.0f);
}
