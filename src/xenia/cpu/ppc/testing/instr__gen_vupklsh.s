test_vupklsh_1_GEN:
  #_ REGISTER_IN v1 [00000000, 00000000, 00000000, 00000000]
  vupklsh v2, v1
  blr
  #_ REGISTER_OUT v1 [00000000, 00000000, 00000000, 00000000]
  #_ REGISTER_OUT v2 [00000000, 00000000, 00000000, 00000000]

test_vupklsh_2_GEN:
  #_ REGISTER_IN v1 [00000001, 00000001, 00000001, 00000001]
  vupklsh v2, v1
  blr
  #_ REGISTER_OUT v1 [00000001, 00000001, 00000001, 00000001]
  #_ REGISTER_OUT v2 [00000000, 00000001, 00000000, 00000001]

test_vupklsh_3_GEN:
  #_ REGISTER_IN v1 [0000FFFF, FFFF0000, 00000000, FFFF0000]
  vupklsh v2, v1
  blr
  #_ REGISTER_OUT v1 [0000FFFF, FFFF0000, 00000000, FFFF0000]
  #_ REGISTER_OUT v2 [00000000, 00000000, FFFFFFFF, 00000000]

test_vupklsh_4_GEN:
  #_ REGISTER_IN v1 [00010203, 04050607, 08090A0B, 0C0D0E0F]
  vupklsh v2, v1
  blr
  #_ REGISTER_OUT v1 [00010203, 04050607, 08090A0B, 0C0D0E0F]
  #_ REGISTER_OUT v2 [00000809, 00000A0B, 00000C0D, 00000E0F]

test_vupklsh_5_GEN:
  #_ REGISTER_IN v1 [000D000D, 000D000D, 000D000D, 000D000D]
  vupklsh v2, v1
  blr
  #_ REGISTER_OUT v1 [000D000D, 000D000D, 000D000D, 000D000D]
  #_ REGISTER_OUT v2 [0000000D, 0000000D, 0000000D, 0000000D]

test_vupklsh_6_GEN:
  #_ REGISTER_IN v1 [00112233, 44556677, 8899AABB, CCDDEEFF]
  vupklsh v2, v1
  blr
  #_ REGISTER_OUT v1 [00112233, 44556677, 8899AABB, CCDDEEFF]
  #_ REGISTER_OUT v2 [FFFF8899, FFFFAABB, FFFFCCDD, FFFFEEFF]

test_vupklsh_7_GEN:
  #_ REGISTER_IN v1 [00FFFF00, 0000FF00, FF0000FF, FFFF00FF]
  vupklsh v2, v1
  blr
  #_ REGISTER_OUT v1 [00FFFF00, 0000FF00, FF0000FF, FFFF00FF]
  #_ REGISTER_OUT v2 [FFFFFF00, 000000FF, FFFFFFFF, 000000FF]

test_vupklsh_8_GEN:
  #_ REGISTER_IN v1 [04040404, 04040404, 04040404, 04040404]
  vupklsh v2, v1
  blr
  #_ REGISTER_OUT v1 [04040404, 04040404, 04040404, 04040404]
  #_ REGISTER_OUT v2 [00000404, 00000404, 00000404, 00000404]

test_vupklsh_9_GEN:
  #_ REGISTER_IN v1 [07070707, 07070707, 07070707, 07070707]
  vupklsh v2, v1
  blr
  #_ REGISTER_OUT v1 [07070707, 07070707, 07070707, 07070707]
  #_ REGISTER_OUT v2 [00000707, 00000707, 00000707, 00000707]

test_vupklsh_10_GEN:
  #_ REGISTER_IN v1 [08080808, 08080808, 08080808, 08080808]
  vupklsh v2, v1
  blr
  #_ REGISTER_OUT v1 [08080808, 08080808, 08080808, 08080808]
  #_ REGISTER_OUT v2 [00000808, 00000808, 00000808, 00000808]

test_vupklsh_11_GEN:
  #_ REGISTER_IN v1 [12121212, 12121212, 12121212, 12121212]
  vupklsh v2, v1
  blr
  #_ REGISTER_OUT v1 [12121212, 12121212, 12121212, 12121212]
  #_ REGISTER_OUT v2 [00001212, 00001212, 00001212, 00001212]

test_vupklsh_12_GEN:
  #_ REGISTER_IN v1 [12345678, 87654321, 11223344, 55667788]
  vupklsh v2, v1
  blr
  #_ REGISTER_OUT v1 [12345678, 87654321, 11223344, 55667788]
  #_ REGISTER_OUT v2 [00001122, 00003344, 00005566, 00007788]

test_vupklsh_13_GEN:
  #_ REGISTER_IN v1 [3F800000, 3FC00000, 3F8CCCCD, 3FF33333]
  vupklsh v2, v1
  blr
  #_ REGISTER_OUT v1 [3F800000, 3FC00000, 3F8CCCCD, 3FF33333]
  #_ REGISTER_OUT v2 [00003F8C, FFFFCCCD, 00003FF3, 00003333]

test_vupklsh_14_GEN:
  #_ REGISTER_IN v1 [41200000, C1200000, 41700000, C1700000]
  vupklsh v2, v1
  blr
  #_ REGISTER_OUT v1 [41200000, C1200000, 41700000, C1700000]
  #_ REGISTER_OUT v2 [00004170, 00000000, FFFFC170, 00000000]

test_vupklsh_15_GEN:
  #_ REGISTER_IN v1 [7F800203, 04050607, 7F800A0B, 0C0D0E0F]
  vupklsh v2, v1
  blr
  #_ REGISTER_OUT v1 [7F800203, 04050607, 7F800A0B, 0C0D0E0F]
  #_ REGISTER_OUT v2 [00007F80, 00000A0B, 00000C0D, 00000E0F]

test_vupklsh_16_GEN:
  #_ REGISTER_IN v1 [80081010, 808F0000, 7FFFFFFF, 8FFFFFFF]
  vupklsh v2, v1
  blr
  #_ REGISTER_OUT v1 [80081010, 808F0000, 7FFFFFFF, 8FFFFFFF]
  #_ REGISTER_OUT v2 [00007FFF, FFFFFFFF, FFFF8FFF, FFFFFFFF]

test_vupklsh_17_GEN:
  #_ REGISTER_IN v1 [BF800000, BFC00000, BF8CCCCD, BFF33333]
  vupklsh v2, v1
  blr
  #_ REGISTER_OUT v1 [BF800000, BFC00000, BF8CCCCD, BFF33333]
  #_ REGISTER_OUT v2 [FFFFBF8C, FFFFCCCD, FFFFBFF3, 00003333]

test_vupklsh_18_GEN:
  #_ REGISTER_IN v1 [C1200000, 41A00000, C1A00000, 41F00000]
  vupklsh v2, v1
  blr
  #_ REGISTER_OUT v1 [C1200000, 41A00000, C1A00000, 41F00000]
  #_ REGISTER_OUT v2 [FFFFC1A0, 00000000, 000041F0, 00000000]

test_vupklsh_19_GEN:
  #_ REGISTER_IN v1 [CDCDCDCD, CDCDCDCD, CDCDCDCD, 04010203]
  vupklsh v2, v1
  blr
  #_ REGISTER_OUT v1 [CDCDCDCD, CDCDCDCD, CDCDCDCD, 04010203]
  #_ REGISTER_OUT v2 [FFFFCDCD, FFFFCDCD, 00000401, 00000203]

test_vupklsh_20_GEN:
  #_ REGISTER_IN v1 [F8F9FAFB, FCFDFEFF, 00010203, 04050607]
  vupklsh v2, v1
  blr
  #_ REGISTER_OUT v1 [F8F9FAFB, FCFDFEFF, 00010203, 04050607]
  #_ REGISTER_OUT v2 [00000001, 00000203, 00000405, 00000607]

test_vupklsh_21_GEN:
  #_ REGISTER_IN v1 [FEFEFEFE, FEFEFEFE, FEFEFEFE, FEFEFEFE]
  vupklsh v2, v1
  blr
  #_ REGISTER_OUT v1 [FEFEFEFE, FEFEFEFE, FEFEFEFE, FEFEFEFE]
  #_ REGISTER_OUT v2 [FFFFFEFE, FFFFFEFE, FFFFFEFE, FFFFFEFE]

test_vupklsh_22_GEN:
  #_ REGISTER_IN v1 [FFFCFFFD, FFFEFFFF, 00000001, 00020003]
  vupklsh v2, v1
  blr
  #_ REGISTER_OUT v1 [FFFCFFFD, FFFEFFFF, 00000001, 00020003]
  #_ REGISTER_OUT v2 [00000000, 00000001, 00000002, 00000003]

test_vupklsh_23_GEN:
  #_ REGISTER_IN v1 [FFFDFF7E, 00020081, FFFCFF7D, 00030082]
  vupklsh v2, v1
  blr
  #_ REGISTER_OUT v1 [FFFDFF7E, 00020081, FFFCFF7D, 00030082]
  #_ REGISTER_OUT v2 [FFFFFFFC, FFFFFF7D, 00000003, 00000082]

test_vupklsh_24_GEN:
  #_ REGISTER_IN v1 [FFFF0101, 7070FFFF, FFFFFFFF, 00000000]
  vupklsh v2, v1
  blr
  #_ REGISTER_OUT v1 [FFFF0101, 7070FFFF, FFFFFFFF, 00000000]
  #_ REGISTER_OUT v2 [FFFFFFFF, FFFFFFFF, 00000000, 00000000]

test_vupklsh_25_GEN:
  #_ REGISTER_IN v1 [FFFFFF80, 0000007F, FFFEFF7F, 00010080]
  vupklsh v2, v1
  blr
  #_ REGISTER_OUT v1 [FFFFFF80, 0000007F, FFFEFF7F, 00010080]
  #_ REGISTER_OUT v2 [FFFFFFFE, FFFFFF7F, 00000001, 00000080]

test_vupklsh_26_GEN:
  #_ REGISTER_IN v1 [FFFFFFFF, FFFFFFFF, FFFFFFFF, FFFFFFFF]
  vupklsh v2, v1
  blr
  #_ REGISTER_OUT v1 [FFFFFFFF, FFFFFFFF, FFFFFFFF, FFFFFFFF]
  #_ REGISTER_OUT v2 [FFFFFFFF, FFFFFFFF, FFFFFFFF, FFFFFFFF]

