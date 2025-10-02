__kernel void rasterCube(__global uchar4* pixels, __global float* depth, int width, int height, float t)
{
    int x = get_global_id(0);
    int y = get_global_id(1);
    float s = sin(t), c = cos(t);
    float sX = sin(t*0.7f), cX = cos(t*0.7f);
    float3 verts[8];
    verts[0]= (float3)(-0.5f,-0.5f,-0.5f); verts[1]= (float3)(0.5f,-0.5f,-0.5f);
    verts[2]= (float3)(0.5f,0.5f,-0.5f); verts[3]= (float3)(-0.5f,0.5f,-0.5f);
    verts[4]= (float3)(-0.5f,-0.5f,0.5f); verts[5]= (float3)(0.5f,-0.5f,0.5f);
    verts[6]= (float3)(0.5f,0.5f,0.5f); verts[7]= (float3)(-0.5f,0.5f,0.5f);
    int tris[12][3] = {{0,1,2},{0,2,3},{4,5,6},{4,6,7},{0,1,5},{0,5,4},
                       {2,3,7},{2,7,6},{0,3,7},{0,7,4},{1,2,6},{1,6,5}};
    uchar colors_r[6] = {255,0,0,255,255,0};
    uchar colors_g[6] = {0,255,0,255,0,255};
    uchar colors_b[6] = {0,0,255,0,255,255};
    float3 proj[8];
    for(int i=0;i<8;i++){
        float y0 = verts[i].y*cX - verts[i].z*sX;
        float z0 = verts[i].y*sX + verts[i].z*cX;
        float x0 = verts[i].x;
        float x1 = x0*c - z0*s;
        float z1 = x0*s + z0*c;
        float f = 1.0f/(z1+2.0f);
        proj[i] = (float3)((x1*f+0.5f)*(float)width,(y0*f+0.5f)*(float)height,z1);
    }
    float mindepth = 1e10f;
    uchar4 color = (uchar4)(0,0,0,255);
    for(int i=0;i<12;i++){
        int faceID = i/2;
        uchar r = colors_r[faceID]; uchar g = colors_g[faceID]; uchar b = colors_b[faceID];
        float2 v0 = (float2)(proj[tris[i][0]].x, proj[tris[i][0]].y);
        float2 v1 = (float2)(proj[tris[i][1]].x, proj[tris[i][1]].y);
        float2 v2 = (float2)(proj[tris[i][2]].x, proj[tris[i][2]].y);
        float px = (float)x; float py = (float)y;
        float den = (v1.x-v0.x)*(v2.y-v0.y)-(v2.x-v0.x)*(v1.y-v0.y);
        if(fabs(den)<1e-6f) continue;
        float a = ((v2.y-v0.y)*(px-v0.x)-(v2.x-v0.x)*(py-v0.y))/den;
        float b_coef = (-(v1.y-v0.y)*(px-v0.x)+(v1.x-v0.x)*(py-v0.y))/den;
        float g_coef = 1.0f-a-b_coef;
        if(a>=0 && b_coef>=0 && g_coef>=0){
            float z = a*proj[tris[i][1]].z + b_coef*proj[tris[i][2]].z + g_coef*proj[tris[i][0]].z;
            if(z<mindepth){ mindepth=z; color = (uchar4)(r,g,b,255);}
        }
    }
    pixels[y*width+x]=color;
    depth[y*width+x]=mindepth;
}
