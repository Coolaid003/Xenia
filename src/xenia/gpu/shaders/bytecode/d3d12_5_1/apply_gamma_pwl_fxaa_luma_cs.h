#if 0
//
// Generated by Microsoft (R) HLSL Shader Compiler 10.1
//
//
// Buffer Definitions: 
//
// cbuffer XeApplyGammaRampConstants
// {
//
//   uint2 xe_apply_gamma_size;         // Offset:    0 Size:     8
//
// }
//
//
// Resource Bindings:
//
// Name                                 Type  Format         Dim      ID      HLSL Bind  Count
// ------------------------------ ---------- ------- ----------- ------- -------------- ------
// xe_apply_gamma_source             texture  float3          2d      T0             t0      1 
// xe_apply_gamma_ramp               texture   uint2         buf      T1             t1      1 
// xe_apply_gamma_dest                   UAV  unorm4          2d      U0             u0      1 
// XeApplyGammaRampConstants         cbuffer      NA          NA     CB0            cb0      1 
//
//
//
// Input signature:
//
// Name                 Index   Mask Register SysValue  Format   Used
// -------------------- ----- ------ -------- -------- ------- ------
// no Input
//
// Output signature:
//
// Name                 Index   Mask Register SysValue  Format   Used
// -------------------- ----- ------ -------- -------- ------- ------
// no Output
cs_5_1
dcl_globalFlags refactoringAllowed
dcl_constantbuffer CB0[0:0][1], immediateIndexed, space=0
dcl_resource_texture2d (float,float,float,float) T0[0:0], space=0
dcl_resource_buffer (uint,uint,uint,uint) T1[1:1], space=0
dcl_uav_typed_texture2d (unorm,unorm,unorm,unorm) U0[0:0], space=0
dcl_input vThreadID.xy
dcl_temps 3
dcl_thread_group 8, 8, 1
uge r0.xy, vThreadID.xyxx, CB0[0][0].xyxx
or r0.x, r0.y, r0.x
if_nz r0.x
  ret 
endif 
mov r0.xy, vThreadID.xyxx
mov r0.zw, l(0,0,0,0)
ld r0.xyz, r0.xyzw, T0[0].xyzw
mad r0.xyz, r0.xyzx, l(1023.000000, 1023.000000, 1023.000000, 0.000000), l(0.500000, 0.500000, 0.500000, 0.000000)
ftou r0.xyz, r0.xyzx
ushr r1.xyz, r0.xyzx, l(3, 3, 3, 0)
imul null, r0.w, r1.z, l(3)
imad r1.xy, r1.xyxx, l(3, 3, 0, 0), l(2, 1, 0, 0)
ld r1.xz, r1.xxxx, T1[1].xzyw
utof r1.x, r1.x
and r0.xyz, r0.xyzx, l(7, 7, 7, 0)
imul null, r0.x, r1.z, r0.x
utof r0.x, r0.x
mad r0.x, r0.x, l(0.125000), r1.x
mul r0.x, r0.x, l(0.000015)
min r2.x, r0.x, l(1.000000)
ld r1.xy, r1.yyyy, T1[1].xyzw
utof r0.x, r1.x
imul null, r0.y, r0.y, r1.y
utof r0.y, r0.y
mad r0.x, r0.y, l(0.125000), r0.x
mul r0.x, r0.x, l(0.000015)
min r2.y, r0.x, l(1.000000)
ld r0.xy, r0.wwww, T1[1].xyzw
imul null, r0.y, r0.y, r0.z
utof r0.xy, r0.xyxx
mad r0.x, r0.y, l(0.125000), r0.x
mul r0.x, r0.x, l(0.000015)
min r2.z, r0.x, l(1.000000)
dp3 r2.w, r2.xyzx, l(0.299000, 0.587000, 0.114000, 0.000000)
store_uav_typed U0[0].xyzw, vThreadID.xyyy, r2.xyzw
ret 
// Approximately 37 instruction slots used
#endif

const BYTE apply_gamma_pwl_fxaa_luma_cs[] =
{
     68,  88,  66,  67, 165, 122, 
    242,  36, 160, 218, 193,  67, 
     37,  43, 138,  45, 109, 219, 
    226, 109,   1,   0,   0,   0, 
    148,   7,   0,   0,   5,   0, 
      0,   0,  52,   0,   0,   0, 
     24,   2,   0,   0,  40,   2, 
      0,   0,  56,   2,   0,   0, 
    248,   6,   0,   0,  82,  68, 
     69,  70, 220,   1,   0,   0, 
      1,   0,   0,   0,  52,   1, 
      0,   0,   4,   0,   0,   0, 
     60,   0,   0,   0,   1,   5, 
     83,  67,   0,   5,   0,   0, 
    180,   1,   0,   0,  19,  19, 
     68,  37,  60,   0,   0,   0, 
     24,   0,   0,   0,  40,   0, 
      0,   0,  40,   0,   0,   0, 
     36,   0,   0,   0,  12,   0, 
      0,   0,   0,   0,   0,   0, 
    220,   0,   0,   0,   2,   0, 
      0,   0,   5,   0,   0,   0, 
      4,   0,   0,   0, 255, 255, 
    255, 255,   0,   0,   0,   0, 
      1,   0,   0,   0,   8,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0, 242,   0, 
      0,   0,   2,   0,   0,   0, 
      4,   0,   0,   0,   1,   0, 
      0,   0, 255, 255, 255, 255, 
      1,   0,   0,   0,   1,   0, 
      0,   0,   4,   0,   0,   0, 
      0,   0,   0,   0,   1,   0, 
      0,   0,   6,   1,   0,   0, 
      4,   0,   0,   0,   1,   0, 
      0,   0,   4,   0,   0,   0, 
    255, 255, 255, 255,   0,   0, 
      0,   0,   1,   0,   0,   0, 
     12,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
     26,   1,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      1,   0,   0,   0,   1,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0, 120, 101, 
     95,  97, 112, 112, 108, 121, 
     95, 103,  97, 109, 109,  97, 
     95, 115, 111, 117, 114,  99, 
    101,   0, 120, 101,  95,  97, 
    112, 112, 108, 121,  95, 103, 
     97, 109, 109,  97,  95, 114, 
     97, 109, 112,   0, 120, 101, 
     95,  97, 112, 112, 108, 121, 
     95, 103,  97, 109, 109,  97, 
     95, 100, 101, 115, 116,   0, 
     88, 101,  65, 112, 112, 108, 
    121,  71,  97, 109, 109,  97, 
     82,  97, 109, 112,  67, 111, 
    110, 115, 116,  97, 110, 116, 
    115,   0,  26,   1,   0,   0, 
      1,   0,   0,   0,  76,   1, 
      0,   0,  16,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0, 116,   1,   0,   0, 
      0,   0,   0,   0,   8,   0, 
      0,   0,   2,   0,   0,   0, 
    144,   1,   0,   0,   0,   0, 
      0,   0, 255, 255, 255, 255, 
      0,   0,   0,   0, 255, 255, 
    255, 255,   0,   0,   0,   0, 
    120, 101,  95,  97, 112, 112, 
    108, 121,  95, 103,  97, 109, 
    109,  97,  95, 115, 105, 122, 
    101,   0, 117, 105, 110, 116, 
     50,   0, 171, 171,   1,   0, 
     19,   0,   1,   0,   2,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
    136,   1,   0,   0,  77, 105, 
     99, 114, 111, 115, 111, 102, 
    116,  32,  40,  82,  41,  32, 
     72,  76,  83,  76,  32,  83, 
    104,  97, 100, 101, 114,  32, 
     67, 111, 109, 112, 105, 108, 
    101, 114,  32,  49,  48,  46, 
     49,   0,  73,  83,  71,  78, 
      8,   0,   0,   0,   0,   0, 
      0,   0,   8,   0,   0,   0, 
     79,  83,  71,  78,   8,   0, 
      0,   0,   0,   0,   0,   0, 
      8,   0,   0,   0,  83,  72, 
     69,  88, 184,   4,   0,   0, 
     81,   0,   5,   0,  46,   1, 
      0,   0, 106,   8,   0,   1, 
     89,   0,   0,   7,  70, 142, 
     48,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   1,   0,   0,   0, 
      0,   0,   0,   0,  88,  24, 
      0,   7,  70, 126,  48,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
     85,  85,   0,   0,   0,   0, 
      0,   0,  88,   8,   0,   7, 
     70, 126,  48,   0,   1,   0, 
      0,   0,   1,   0,   0,   0, 
      1,   0,   0,   0,  68,  68, 
      0,   0,   0,   0,   0,   0, 
    156,  24,   0,   7,  70, 238, 
     49,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,  17,  17,   0,   0, 
      0,   0,   0,   0,  95,   0, 
      0,   2,  50,   0,   2,   0, 
    104,   0,   0,   2,   3,   0, 
      0,   0, 155,   0,   0,   4, 
      8,   0,   0,   0,   8,   0, 
      0,   0,   1,   0,   0,   0, 
     80,   0,   0,   8,  50,   0, 
     16,   0,   0,   0,   0,   0, 
     70,   0,   2,   0,  70, 128, 
     48,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,  60,   0,   0,   7, 
     18,   0,  16,   0,   0,   0, 
      0,   0,  26,   0,  16,   0, 
      0,   0,   0,   0,  10,   0, 
     16,   0,   0,   0,   0,   0, 
     31,   0,   4,   3,  10,   0, 
     16,   0,   0,   0,   0,   0, 
     62,   0,   0,   1,  21,   0, 
      0,   1,  54,   0,   0,   4, 
     50,   0,  16,   0,   0,   0, 
      0,   0,  70,   0,   2,   0, 
     54,   0,   0,   8, 194,   0, 
     16,   0,   0,   0,   0,   0, 
      2,  64,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,  45,   0,   0,   8, 
    114,   0,  16,   0,   0,   0, 
      0,   0,  70,  14,  16,   0, 
      0,   0,   0,   0,  70, 126, 
     32,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,  50,   0, 
      0,  15, 114,   0,  16,   0, 
      0,   0,   0,   0,  70,   2, 
     16,   0,   0,   0,   0,   0, 
      2,  64,   0,   0,   0, 192, 
    127,  68,   0, 192, 127,  68, 
      0, 192, 127,  68,   0,   0, 
      0,   0,   2,  64,   0,   0, 
      0,   0,   0,  63,   0,   0, 
      0,  63,   0,   0,   0,  63, 
      0,   0,   0,   0,  28,   0, 
      0,   5, 114,   0,  16,   0, 
      0,   0,   0,   0,  70,   2, 
     16,   0,   0,   0,   0,   0, 
     85,   0,   0,  10, 114,   0, 
     16,   0,   1,   0,   0,   0, 
     70,   2,  16,   0,   0,   0, 
      0,   0,   2,  64,   0,   0, 
      3,   0,   0,   0,   3,   0, 
      0,   0,   3,   0,   0,   0, 
      0,   0,   0,   0,  38,   0, 
      0,   8,   0, 208,   0,   0, 
    130,   0,  16,   0,   0,   0, 
      0,   0,  42,   0,  16,   0, 
      1,   0,   0,   0,   1,  64, 
      0,   0,   3,   0,   0,   0, 
     35,   0,   0,  15,  50,   0, 
     16,   0,   1,   0,   0,   0, 
     70,   0,  16,   0,   1,   0, 
      0,   0,   2,  64,   0,   0, 
      3,   0,   0,   0,   3,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   2,  64, 
      0,   0,   2,   0,   0,   0, 
      1,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
     45,   0,   0,   8,  82,   0, 
     16,   0,   1,   0,   0,   0, 
      6,   0,  16,   0,   1,   0, 
      0,   0, 134, 125,  32,   0, 
      1,   0,   0,   0,   1,   0, 
      0,   0,  86,   0,   0,   5, 
     18,   0,  16,   0,   1,   0, 
      0,   0,  10,   0,  16,   0, 
      1,   0,   0,   0,   1,   0, 
      0,  10, 114,   0,  16,   0, 
      0,   0,   0,   0,  70,   2, 
     16,   0,   0,   0,   0,   0, 
      2,  64,   0,   0,   7,   0, 
      0,   0,   7,   0,   0,   0, 
      7,   0,   0,   0,   0,   0, 
      0,   0,  38,   0,   0,   8, 
      0, 208,   0,   0,  18,   0, 
     16,   0,   0,   0,   0,   0, 
     42,   0,  16,   0,   1,   0, 
      0,   0,  10,   0,  16,   0, 
      0,   0,   0,   0,  86,   0, 
      0,   5,  18,   0,  16,   0, 
      0,   0,   0,   0,  10,   0, 
     16,   0,   0,   0,   0,   0, 
     50,   0,   0,   9,  18,   0, 
     16,   0,   0,   0,   0,   0, 
     10,   0,  16,   0,   0,   0, 
      0,   0,   1,  64,   0,   0, 
      0,   0,   0,  62,  10,   0, 
     16,   0,   1,   0,   0,   0, 
     56,   0,   0,   7,  18,   0, 
     16,   0,   0,   0,   0,   0, 
     10,   0,  16,   0,   0,   0, 
      0,   0,   1,  64,   0,   0, 
      8,  32, 128,  55,  51,   0, 
      0,   7,  18,   0,  16,   0, 
      2,   0,   0,   0,  10,   0, 
     16,   0,   0,   0,   0,   0, 
      1,  64,   0,   0,   0,   0, 
    128,  63,  45,   0,   0,   8, 
     50,   0,  16,   0,   1,   0, 
      0,   0,  86,   5,  16,   0, 
      1,   0,   0,   0,  70, 126, 
     32,   0,   1,   0,   0,   0, 
      1,   0,   0,   0,  86,   0, 
      0,   5,  18,   0,  16,   0, 
      0,   0,   0,   0,  10,   0, 
     16,   0,   1,   0,   0,   0, 
     38,   0,   0,   8,   0, 208, 
      0,   0,  34,   0,  16,   0, 
      0,   0,   0,   0,  26,   0, 
     16,   0,   0,   0,   0,   0, 
     26,   0,  16,   0,   1,   0, 
      0,   0,  86,   0,   0,   5, 
     34,   0,  16,   0,   0,   0, 
      0,   0,  26,   0,  16,   0, 
      0,   0,   0,   0,  50,   0, 
      0,   9,  18,   0,  16,   0, 
      0,   0,   0,   0,  26,   0, 
     16,   0,   0,   0,   0,   0, 
      1,  64,   0,   0,   0,   0, 
      0,  62,  10,   0,  16,   0, 
      0,   0,   0,   0,  56,   0, 
      0,   7,  18,   0,  16,   0, 
      0,   0,   0,   0,  10,   0, 
     16,   0,   0,   0,   0,   0, 
      1,  64,   0,   0,   8,  32, 
    128,  55,  51,   0,   0,   7, 
     34,   0,  16,   0,   2,   0, 
      0,   0,  10,   0,  16,   0, 
      0,   0,   0,   0,   1,  64, 
      0,   0,   0,   0, 128,  63, 
     45,   0,   0,   8,  50,   0, 
     16,   0,   0,   0,   0,   0, 
    246,  15,  16,   0,   0,   0, 
      0,   0,  70, 126,  32,   0, 
      1,   0,   0,   0,   1,   0, 
      0,   0,  38,   0,   0,   8, 
      0, 208,   0,   0,  34,   0, 
     16,   0,   0,   0,   0,   0, 
     26,   0,  16,   0,   0,   0, 
      0,   0,  42,   0,  16,   0, 
      0,   0,   0,   0,  86,   0, 
      0,   5,  50,   0,  16,   0, 
      0,   0,   0,   0,  70,   0, 
     16,   0,   0,   0,   0,   0, 
     50,   0,   0,   9,  18,   0, 
     16,   0,   0,   0,   0,   0, 
     26,   0,  16,   0,   0,   0, 
      0,   0,   1,  64,   0,   0, 
      0,   0,   0,  62,  10,   0, 
     16,   0,   0,   0,   0,   0, 
     56,   0,   0,   7,  18,   0, 
     16,   0,   0,   0,   0,   0, 
     10,   0,  16,   0,   0,   0, 
      0,   0,   1,  64,   0,   0, 
      8,  32, 128,  55,  51,   0, 
      0,   7,  66,   0,  16,   0, 
      2,   0,   0,   0,  10,   0, 
     16,   0,   0,   0,   0,   0, 
      1,  64,   0,   0,   0,   0, 
    128,  63,  16,   0,   0,  10, 
    130,   0,  16,   0,   2,   0, 
      0,   0,  70,   2,  16,   0, 
      2,   0,   0,   0,   2,  64, 
      0,   0, 135,  22, 153,  62, 
    162,  69,  22,  63, 213, 120, 
    233,  61,   0,   0,   0,   0, 
    164,   0,   0,   7, 242, 224, 
     33,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,  70,   5, 
      2,   0,  70,  14,  16,   0, 
      2,   0,   0,   0,  62,   0, 
      0,   1,  83,  84,  65,  84, 
    148,   0,   0,   0,  37,   0, 
      0,   0,   3,   0,   0,   0, 
      0,   0,   0,   0,   1,   0, 
      0,   0,  11,   0,   0,   0, 
      5,   0,   0,   0,   4,   0, 
      0,   0,   2,   0,   0,   0, 
      1,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   4,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   2,   0,   0,   0, 
      0,   0,   0,   0,   6,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   1,   0, 
      0,   0
};