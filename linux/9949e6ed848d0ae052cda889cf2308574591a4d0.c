// general protection fault in xt_rateest_put
// https://syzkaller.appspot.com/bug?id=9949e6ed848d0ae052cda889cf2308574591a4d0
// status:open
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <endian.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>

uint64_t r[1] = {0xffffffffffffffff};

int main(void)
{
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 3ul, 0x32ul, -1, 0);
  intptr_t res = 0;
  res = syscall(__NR_socket, 2ul, 2ul, 0x88ul);
  if (res != -1)
    r[0] = res;
  memcpy((void*)0x200008c0, "filter\000\000\000\000\000\000\000\000\000\000\000"
                            "\000\000\000\000\000\000\000\000\000\000\000\000"
                            "\000\000\000",
         32);
  *(uint32_t*)0x200008e0 = 0xc001;
  *(uint32_t*)0x200008e4 = 4;
  *(uint32_t*)0x200008e8 = 0x3e0;
  *(uint32_t*)0x200008ec = 0x1e8;
  *(uint32_t*)0x200008f0 = 0x1e8;
  *(uint32_t*)0x200008f4 = 0;
  *(uint32_t*)0x200008f8 = 0x2f8;
  *(uint32_t*)0x200008fc = 0x2f8;
  *(uint32_t*)0x20000900 = 0x2f8;
  *(uint32_t*)0x20000904 = 4;
  *(uint64_t*)0x20000908 = 0;
  *(uint8_t*)0x20000910 = 0;
  *(uint8_t*)0x20000911 = 0;
  *(uint8_t*)0x20000912 = 0;
  *(uint8_t*)0x20000913 = 0;
  *(uint8_t*)0x20000914 = 0;
  *(uint8_t*)0x20000915 = 0;
  *(uint8_t*)0x20000916 = 0;
  *(uint8_t*)0x20000917 = 0;
  *(uint8_t*)0x20000918 = 0;
  *(uint8_t*)0x20000919 = 0;
  *(uint8_t*)0x2000091a = 0;
  *(uint8_t*)0x2000091b = 0;
  *(uint8_t*)0x2000091c = 0;
  *(uint8_t*)0x2000091d = 0;
  *(uint8_t*)0x2000091e = 0;
  *(uint8_t*)0x2000091f = 0;
  *(uint8_t*)0x20000920 = 0;
  *(uint8_t*)0x20000921 = 0;
  *(uint8_t*)0x20000922 = 0;
  *(uint8_t*)0x20000923 = 0;
  *(uint8_t*)0x20000924 = 0;
  *(uint8_t*)0x20000925 = 0;
  *(uint8_t*)0x20000926 = 0;
  *(uint8_t*)0x20000927 = 0;
  *(uint8_t*)0x20000928 = 0;
  *(uint8_t*)0x20000929 = 0;
  *(uint8_t*)0x2000092a = 0;
  *(uint8_t*)0x2000092b = 0;
  *(uint8_t*)0x2000092c = 0;
  *(uint8_t*)0x2000092d = 0;
  *(uint8_t*)0x2000092e = 0;
  *(uint8_t*)0x2000092f = 0;
  *(uint8_t*)0x20000930 = 0;
  *(uint8_t*)0x20000931 = 0;
  *(uint8_t*)0x20000932 = 0;
  *(uint8_t*)0x20000933 = 0;
  *(uint8_t*)0x20000934 = 0;
  *(uint8_t*)0x20000935 = 0;
  *(uint8_t*)0x20000936 = 0;
  *(uint8_t*)0x20000937 = 0;
  *(uint8_t*)0x20000938 = 0;
  *(uint8_t*)0x20000939 = 0;
  *(uint8_t*)0x2000093a = 0;
  *(uint8_t*)0x2000093b = 0;
  *(uint8_t*)0x2000093c = 0;
  *(uint8_t*)0x2000093d = 0;
  *(uint8_t*)0x2000093e = 0;
  *(uint8_t*)0x2000093f = 0;
  *(uint8_t*)0x20000940 = 0;
  *(uint8_t*)0x20000941 = 0;
  *(uint8_t*)0x20000942 = 0;
  *(uint8_t*)0x20000943 = 0;
  *(uint8_t*)0x20000944 = 0;
  *(uint8_t*)0x20000945 = 0;
  *(uint8_t*)0x20000946 = 0;
  *(uint8_t*)0x20000947 = 0;
  *(uint8_t*)0x20000948 = 0;
  *(uint8_t*)0x20000949 = 0;
  *(uint8_t*)0x2000094a = 0;
  *(uint8_t*)0x2000094b = 0;
  *(uint8_t*)0x2000094c = 0;
  *(uint8_t*)0x2000094d = 0;
  *(uint8_t*)0x2000094e = 0;
  *(uint8_t*)0x2000094f = 0;
  *(uint8_t*)0x20000950 = 0;
  *(uint8_t*)0x20000951 = 0;
  *(uint8_t*)0x20000952 = 0;
  *(uint8_t*)0x20000953 = 0;
  *(uint8_t*)0x20000954 = 0;
  *(uint8_t*)0x20000955 = 0;
  *(uint8_t*)0x20000956 = 0;
  *(uint8_t*)0x20000957 = 0;
  *(uint8_t*)0x20000958 = 0;
  *(uint8_t*)0x20000959 = 0;
  *(uint8_t*)0x2000095a = 0;
  *(uint8_t*)0x2000095b = 0;
  *(uint8_t*)0x2000095c = 0;
  *(uint8_t*)0x2000095d = 0;
  *(uint8_t*)0x2000095e = 0;
  *(uint8_t*)0x2000095f = 0;
  *(uint8_t*)0x20000960 = 0;
  *(uint8_t*)0x20000961 = 0;
  *(uint8_t*)0x20000962 = 0;
  *(uint8_t*)0x20000963 = 0;
  *(uint8_t*)0x20000964 = 0;
  *(uint8_t*)0x20000965 = 0;
  *(uint8_t*)0x20000966 = 0;
  *(uint8_t*)0x20000967 = 0;
  *(uint8_t*)0x20000968 = 0;
  *(uint8_t*)0x20000969 = 0;
  *(uint8_t*)0x2000096a = 0;
  *(uint8_t*)0x2000096b = 0;
  *(uint8_t*)0x2000096c = 0;
  *(uint8_t*)0x2000096d = 0;
  *(uint8_t*)0x2000096e = 0;
  *(uint8_t*)0x2000096f = 0;
  *(uint8_t*)0x20000970 = 0;
  *(uint8_t*)0x20000971 = 0;
  *(uint8_t*)0x20000972 = 0;
  *(uint8_t*)0x20000973 = 0;
  *(uint8_t*)0x20000974 = 0;
  *(uint8_t*)0x20000975 = 0;
  *(uint8_t*)0x20000976 = 0;
  *(uint8_t*)0x20000977 = 0;
  *(uint8_t*)0x20000978 = 0;
  *(uint8_t*)0x20000979 = 0;
  *(uint8_t*)0x2000097a = 0;
  *(uint8_t*)0x2000097b = 0;
  *(uint8_t*)0x2000097c = 0;
  *(uint8_t*)0x2000097d = 0;
  *(uint8_t*)0x2000097e = 0;
  *(uint8_t*)0x2000097f = 0;
  *(uint8_t*)0x20000980 = 0;
  *(uint8_t*)0x20000981 = 0;
  *(uint8_t*)0x20000982 = 0;
  *(uint8_t*)0x20000983 = 0;
  *(uint8_t*)0x20000984 = 0;
  *(uint8_t*)0x20000985 = 0;
  *(uint8_t*)0x20000986 = 0;
  *(uint8_t*)0x20000987 = 0;
  *(uint8_t*)0x20000988 = 0;
  *(uint8_t*)0x20000989 = 0;
  *(uint8_t*)0x2000098a = 0;
  *(uint8_t*)0x2000098b = 0;
  *(uint8_t*)0x2000098c = 0;
  *(uint8_t*)0x2000098d = 0;
  *(uint8_t*)0x2000098e = 0;
  *(uint8_t*)0x2000098f = 0;
  *(uint8_t*)0x20000990 = 0;
  *(uint8_t*)0x20000991 = 0;
  *(uint8_t*)0x20000992 = 0;
  *(uint8_t*)0x20000993 = 0;
  *(uint8_t*)0x20000994 = 0;
  *(uint8_t*)0x20000995 = 0;
  *(uint8_t*)0x20000996 = 0;
  *(uint8_t*)0x20000997 = 0;
  *(uint8_t*)0x20000998 = 0;
  *(uint8_t*)0x20000999 = 0;
  *(uint8_t*)0x2000099a = 0;
  *(uint8_t*)0x2000099b = 0;
  *(uint8_t*)0x2000099c = 0;
  *(uint8_t*)0x2000099d = 0;
  *(uint8_t*)0x2000099e = 0;
  *(uint8_t*)0x2000099f = 0;
  *(uint8_t*)0x200009a0 = 0;
  *(uint8_t*)0x200009a1 = 0;
  *(uint8_t*)0x200009a2 = 0;
  *(uint8_t*)0x200009a3 = 0;
  *(uint8_t*)0x200009a4 = 0;
  *(uint8_t*)0x200009a5 = 0;
  *(uint8_t*)0x200009a6 = 0;
  *(uint8_t*)0x200009a7 = 0;
  *(uint8_t*)0x200009a8 = 0;
  *(uint8_t*)0x200009a9 = 0;
  *(uint8_t*)0x200009aa = 0;
  *(uint8_t*)0x200009ab = 0;
  *(uint8_t*)0x200009ac = 0;
  *(uint8_t*)0x200009ad = 0;
  *(uint8_t*)0x200009ae = 0;
  *(uint8_t*)0x200009af = 0;
  *(uint8_t*)0x200009b0 = 0;
  *(uint8_t*)0x200009b1 = 0;
  *(uint8_t*)0x200009b2 = 0;
  *(uint8_t*)0x200009b3 = 0;
  *(uint16_t*)0x200009b4 = 0xc0;
  *(uint16_t*)0x200009b6 = 0x100;
  *(uint32_t*)0x200009b8 = 0;
  *(uint64_t*)0x200009c0 = 0;
  *(uint64_t*)0x200009c8 = 0;
  *(uint16_t*)0x200009d0 = 0x40;
  memcpy((void*)0x200009d2, "RATEEST\000\000\000\000\000\000\000\000\000\000"
                            "\000\000\000\000\000\000\000\000\000\000\000\000",
         29);
  *(uint8_t*)0x200009ef = 0;
  memcpy((void*)0x200009f0,
         "syz0\000\000\000\000\000\000\000\000\000\000\000\000", 16);
  *(uint8_t*)0x20000a00 = 0;
  *(uint8_t*)0x20000a01 = 0;
  *(uint64_t*)0x20000a08 = 0;
  *(uint8_t*)0x20000a10 = 0;
  *(uint8_t*)0x20000a11 = 0;
  *(uint8_t*)0x20000a12 = 0;
  *(uint8_t*)0x20000a13 = 0;
  *(uint8_t*)0x20000a14 = 0;
  *(uint8_t*)0x20000a15 = 0;
  *(uint8_t*)0x20000a16 = 0;
  *(uint8_t*)0x20000a17 = 0;
  *(uint8_t*)0x20000a18 = 0;
  *(uint8_t*)0x20000a19 = 0;
  *(uint8_t*)0x20000a1a = 0;
  *(uint8_t*)0x20000a1b = 0;
  *(uint8_t*)0x20000a1c = 0;
  *(uint8_t*)0x20000a1d = 0;
  *(uint8_t*)0x20000a1e = 0;
  *(uint8_t*)0x20000a1f = 0;
  *(uint8_t*)0x20000a20 = 0;
  *(uint8_t*)0x20000a21 = 0;
  *(uint8_t*)0x20000a22 = 0;
  *(uint8_t*)0x20000a23 = 0;
  *(uint8_t*)0x20000a24 = 0;
  *(uint8_t*)0x20000a25 = 0;
  *(uint8_t*)0x20000a26 = 0;
  *(uint8_t*)0x20000a27 = 0;
  *(uint8_t*)0x20000a28 = 0;
  *(uint8_t*)0x20000a29 = 0;
  *(uint8_t*)0x20000a2a = 0;
  *(uint8_t*)0x20000a2b = 0;
  *(uint8_t*)0x20000a2c = 0;
  *(uint8_t*)0x20000a2d = 0;
  *(uint8_t*)0x20000a2e = 0;
  *(uint8_t*)0x20000a2f = 0;
  *(uint8_t*)0x20000a30 = 0;
  *(uint8_t*)0x20000a31 = 0;
  *(uint8_t*)0x20000a32 = 0;
  *(uint8_t*)0x20000a33 = 0;
  *(uint8_t*)0x20000a34 = 0;
  *(uint8_t*)0x20000a35 = 0;
  *(uint8_t*)0x20000a36 = 0;
  *(uint8_t*)0x20000a37 = 0;
  *(uint8_t*)0x20000a38 = 0;
  *(uint8_t*)0x20000a39 = 0;
  *(uint8_t*)0x20000a3a = 0;
  *(uint8_t*)0x20000a3b = 0;
  *(uint8_t*)0x20000a3c = 0;
  *(uint8_t*)0x20000a3d = 0;
  *(uint8_t*)0x20000a3e = 0;
  *(uint8_t*)0x20000a3f = 0;
  *(uint8_t*)0x20000a40 = 0;
  *(uint8_t*)0x20000a41 = 0;
  *(uint8_t*)0x20000a42 = 0;
  *(uint8_t*)0x20000a43 = 0;
  *(uint8_t*)0x20000a44 = 0;
  *(uint8_t*)0x20000a45 = 0;
  *(uint8_t*)0x20000a46 = 0;
  *(uint8_t*)0x20000a47 = 0;
  *(uint8_t*)0x20000a48 = 0;
  *(uint8_t*)0x20000a49 = 0;
  *(uint8_t*)0x20000a4a = 0;
  *(uint8_t*)0x20000a4b = 0;
  *(uint8_t*)0x20000a4c = 0;
  *(uint8_t*)0x20000a4d = 0;
  *(uint8_t*)0x20000a4e = 0;
  *(uint8_t*)0x20000a4f = 0;
  *(uint8_t*)0x20000a50 = 0;
  *(uint8_t*)0x20000a51 = 0;
  *(uint8_t*)0x20000a52 = 0;
  *(uint8_t*)0x20000a53 = 0;
  *(uint8_t*)0x20000a54 = 0;
  *(uint8_t*)0x20000a55 = 0;
  *(uint8_t*)0x20000a56 = 0;
  *(uint8_t*)0x20000a57 = 0;
  *(uint8_t*)0x20000a58 = 0;
  *(uint8_t*)0x20000a59 = 0;
  *(uint8_t*)0x20000a5a = 0;
  *(uint8_t*)0x20000a5b = 0;
  *(uint8_t*)0x20000a5c = 0;
  *(uint8_t*)0x20000a5d = 0;
  *(uint8_t*)0x20000a5e = 0;
  *(uint8_t*)0x20000a5f = 0;
  *(uint8_t*)0x20000a60 = 0;
  *(uint8_t*)0x20000a61 = 0;
  *(uint8_t*)0x20000a62 = 0;
  *(uint8_t*)0x20000a63 = 0;
  *(uint8_t*)0x20000a64 = 0;
  *(uint8_t*)0x20000a65 = 0;
  *(uint8_t*)0x20000a66 = 0;
  *(uint8_t*)0x20000a67 = 0;
  *(uint8_t*)0x20000a68 = 0;
  *(uint8_t*)0x20000a69 = 0;
  *(uint8_t*)0x20000a6a = 0;
  *(uint8_t*)0x20000a6b = 0;
  *(uint8_t*)0x20000a6c = 0;
  *(uint8_t*)0x20000a6d = 0;
  *(uint8_t*)0x20000a6e = 0;
  *(uint8_t*)0x20000a6f = 0;
  *(uint8_t*)0x20000a70 = 0;
  *(uint8_t*)0x20000a71 = 0;
  *(uint8_t*)0x20000a72 = 0;
  *(uint8_t*)0x20000a73 = 0;
  *(uint8_t*)0x20000a74 = 0;
  *(uint8_t*)0x20000a75 = 0;
  *(uint8_t*)0x20000a76 = 0;
  *(uint8_t*)0x20000a77 = 0;
  *(uint8_t*)0x20000a78 = 0;
  *(uint8_t*)0x20000a79 = 0;
  *(uint8_t*)0x20000a7a = 0;
  *(uint8_t*)0x20000a7b = 0;
  *(uint8_t*)0x20000a7c = 0;
  *(uint8_t*)0x20000a7d = 0;
  *(uint8_t*)0x20000a7e = 0;
  *(uint8_t*)0x20000a7f = 0;
  *(uint8_t*)0x20000a80 = 0;
  *(uint8_t*)0x20000a81 = 0;
  *(uint8_t*)0x20000a82 = 0;
  *(uint8_t*)0x20000a83 = 0;
  *(uint8_t*)0x20000a84 = 0;
  *(uint8_t*)0x20000a85 = 0;
  *(uint8_t*)0x20000a86 = 0;
  *(uint8_t*)0x20000a87 = 0;
  *(uint8_t*)0x20000a88 = 0;
  *(uint8_t*)0x20000a89 = 0;
  *(uint8_t*)0x20000a8a = 0;
  *(uint8_t*)0x20000a8b = 0;
  *(uint8_t*)0x20000a8c = 0;
  *(uint8_t*)0x20000a8d = 0;
  *(uint8_t*)0x20000a8e = 0;
  *(uint8_t*)0x20000a8f = 0;
  *(uint8_t*)0x20000a90 = 0;
  *(uint8_t*)0x20000a91 = 0;
  *(uint8_t*)0x20000a92 = 0;
  *(uint8_t*)0x20000a93 = 0;
  *(uint8_t*)0x20000a94 = 0;
  *(uint8_t*)0x20000a95 = 0;
  *(uint8_t*)0x20000a96 = 0;
  *(uint8_t*)0x20000a97 = 0;
  *(uint8_t*)0x20000a98 = 0;
  *(uint8_t*)0x20000a99 = 0;
  *(uint8_t*)0x20000a9a = 0;
  *(uint8_t*)0x20000a9b = 0;
  *(uint8_t*)0x20000a9c = 0;
  *(uint8_t*)0x20000a9d = 0;
  *(uint8_t*)0x20000a9e = 0;
  *(uint8_t*)0x20000a9f = 0;
  *(uint8_t*)0x20000aa0 = 0;
  *(uint8_t*)0x20000aa1 = 0;
  *(uint8_t*)0x20000aa2 = 0;
  *(uint8_t*)0x20000aa3 = 0;
  *(uint8_t*)0x20000aa4 = 0;
  *(uint8_t*)0x20000aa5 = 0;
  *(uint8_t*)0x20000aa6 = 0;
  *(uint8_t*)0x20000aa7 = 0;
  *(uint8_t*)0x20000aa8 = 0;
  *(uint8_t*)0x20000aa9 = 0;
  *(uint8_t*)0x20000aaa = 0;
  *(uint8_t*)0x20000aab = 0;
  *(uint8_t*)0x20000aac = 0;
  *(uint8_t*)0x20000aad = 0;
  *(uint8_t*)0x20000aae = 0;
  *(uint8_t*)0x20000aaf = 0;
  *(uint8_t*)0x20000ab0 = 0;
  *(uint8_t*)0x20000ab1 = 0;
  *(uint8_t*)0x20000ab2 = 0;
  *(uint8_t*)0x20000ab3 = 0;
  *(uint16_t*)0x20000ab4 = 0xc0;
  *(uint16_t*)0x20000ab6 = 0xe8;
  *(uint32_t*)0x20000ab8 = 0;
  *(uint64_t*)0x20000ac0 = 0;
  *(uint64_t*)0x20000ac8 = 0;
  *(uint16_t*)0x20000ad0 = 0x28;
  memcpy((void*)0x20000ad2, "\000\000\000\000\000\000\000\000\000\000\000\000"
                            "\000\000\000\000\000\000\000\000\000\000\000\000"
                            "\000\000\000\000\000",
         29);
  *(uint8_t*)0x20000aef = 0;
  *(uint32_t*)0x20000af0 = 0x1e8;
  *(uint8_t*)0x20000af8 = 0;
  *(uint8_t*)0x20000af9 = 0;
  *(uint8_t*)0x20000afa = 0;
  *(uint8_t*)0x20000afb = 0;
  *(uint8_t*)0x20000afc = 0;
  *(uint8_t*)0x20000afd = 0;
  *(uint8_t*)0x20000afe = 0;
  *(uint8_t*)0x20000aff = 0;
  *(uint8_t*)0x20000b00 = 0;
  *(uint8_t*)0x20000b01 = 0;
  *(uint8_t*)0x20000b02 = 0;
  *(uint8_t*)0x20000b03 = 0;
  *(uint8_t*)0x20000b04 = 0;
  *(uint8_t*)0x20000b05 = 0;
  *(uint8_t*)0x20000b06 = 0;
  *(uint8_t*)0x20000b07 = 0;
  *(uint8_t*)0x20000b08 = 0;
  *(uint8_t*)0x20000b09 = 0;
  *(uint8_t*)0x20000b0a = 0;
  *(uint8_t*)0x20000b0b = 0;
  *(uint8_t*)0x20000b0c = 0;
  *(uint8_t*)0x20000b0d = 0;
  *(uint8_t*)0x20000b0e = 0;
  *(uint8_t*)0x20000b0f = 0;
  *(uint8_t*)0x20000b10 = 0;
  *(uint8_t*)0x20000b11 = 0;
  *(uint8_t*)0x20000b12 = 0;
  *(uint8_t*)0x20000b13 = 0;
  *(uint8_t*)0x20000b14 = 0;
  *(uint8_t*)0x20000b15 = 0;
  *(uint8_t*)0x20000b16 = 0;
  *(uint8_t*)0x20000b17 = 0;
  *(uint8_t*)0x20000b18 = 0;
  *(uint8_t*)0x20000b19 = 0;
  *(uint8_t*)0x20000b1a = 0;
  *(uint8_t*)0x20000b1b = 0;
  *(uint8_t*)0x20000b1c = 0;
  *(uint8_t*)0x20000b1d = 0;
  *(uint8_t*)0x20000b1e = 0;
  *(uint8_t*)0x20000b1f = 0;
  *(uint8_t*)0x20000b20 = 0;
  *(uint8_t*)0x20000b21 = 0;
  *(uint8_t*)0x20000b22 = 0;
  *(uint8_t*)0x20000b23 = 0;
  *(uint8_t*)0x20000b24 = 0;
  *(uint8_t*)0x20000b25 = 0;
  *(uint8_t*)0x20000b26 = 0;
  *(uint8_t*)0x20000b27 = 0;
  *(uint8_t*)0x20000b28 = 0;
  *(uint8_t*)0x20000b29 = 0;
  *(uint8_t*)0x20000b2a = 0;
  *(uint8_t*)0x20000b2b = 0;
  *(uint8_t*)0x20000b2c = 0;
  *(uint8_t*)0x20000b2d = 0;
  *(uint8_t*)0x20000b2e = 0;
  *(uint8_t*)0x20000b2f = 0;
  *(uint8_t*)0x20000b30 = 0;
  *(uint8_t*)0x20000b31 = 0;
  *(uint8_t*)0x20000b32 = 0;
  *(uint8_t*)0x20000b33 = 0;
  *(uint8_t*)0x20000b34 = 0;
  *(uint8_t*)0x20000b35 = 0;
  *(uint8_t*)0x20000b36 = 0;
  *(uint8_t*)0x20000b37 = 0;
  *(uint8_t*)0x20000b38 = 0;
  *(uint8_t*)0x20000b39 = 0;
  *(uint8_t*)0x20000b3a = 0;
  *(uint8_t*)0x20000b3b = 0;
  *(uint8_t*)0x20000b3c = 0;
  *(uint8_t*)0x20000b3d = 0;
  *(uint8_t*)0x20000b3e = 0;
  *(uint8_t*)0x20000b3f = 0;
  *(uint8_t*)0x20000b40 = 0;
  *(uint8_t*)0x20000b41 = 0;
  *(uint8_t*)0x20000b42 = 0;
  *(uint8_t*)0x20000b43 = 0;
  *(uint8_t*)0x20000b44 = 0;
  *(uint8_t*)0x20000b45 = 0;
  *(uint8_t*)0x20000b46 = 0;
  *(uint8_t*)0x20000b47 = 0;
  *(uint8_t*)0x20000b48 = 0;
  *(uint8_t*)0x20000b49 = 0;
  *(uint8_t*)0x20000b4a = 0;
  *(uint8_t*)0x20000b4b = 0;
  *(uint8_t*)0x20000b4c = 0;
  *(uint8_t*)0x20000b4d = 0;
  *(uint8_t*)0x20000b4e = 0;
  *(uint8_t*)0x20000b4f = 0;
  *(uint8_t*)0x20000b50 = 0;
  *(uint8_t*)0x20000b51 = 0;
  *(uint8_t*)0x20000b52 = 0;
  *(uint8_t*)0x20000b53 = 0;
  *(uint8_t*)0x20000b54 = 0;
  *(uint8_t*)0x20000b55 = 0;
  *(uint8_t*)0x20000b56 = 0;
  *(uint8_t*)0x20000b57 = 0;
  *(uint8_t*)0x20000b58 = 0;
  *(uint8_t*)0x20000b59 = 0;
  *(uint8_t*)0x20000b5a = 0;
  *(uint8_t*)0x20000b5b = 0;
  *(uint8_t*)0x20000b5c = 0;
  *(uint8_t*)0x20000b5d = 0;
  *(uint8_t*)0x20000b5e = 0;
  *(uint8_t*)0x20000b5f = 0;
  *(uint8_t*)0x20000b60 = 0;
  *(uint8_t*)0x20000b61 = 0;
  *(uint8_t*)0x20000b62 = 0;
  *(uint8_t*)0x20000b63 = 0;
  *(uint8_t*)0x20000b64 = 0;
  *(uint8_t*)0x20000b65 = 0;
  *(uint8_t*)0x20000b66 = 0;
  *(uint8_t*)0x20000b67 = 0;
  *(uint8_t*)0x20000b68 = 0;
  *(uint8_t*)0x20000b69 = 0;
  *(uint8_t*)0x20000b6a = 0;
  *(uint8_t*)0x20000b6b = 0;
  *(uint8_t*)0x20000b6c = 0;
  *(uint8_t*)0x20000b6d = 0;
  *(uint8_t*)0x20000b6e = 0;
  *(uint8_t*)0x20000b6f = 0;
  *(uint8_t*)0x20000b70 = 0;
  *(uint8_t*)0x20000b71 = 0;
  *(uint8_t*)0x20000b72 = 0;
  *(uint8_t*)0x20000b73 = 0;
  *(uint8_t*)0x20000b74 = 0;
  *(uint8_t*)0x20000b75 = 0;
  *(uint8_t*)0x20000b76 = 0;
  *(uint8_t*)0x20000b77 = 0;
  *(uint8_t*)0x20000b78 = 0;
  *(uint8_t*)0x20000b79 = 0;
  *(uint8_t*)0x20000b7a = 0;
  *(uint8_t*)0x20000b7b = 0;
  *(uint8_t*)0x20000b7c = 0;
  *(uint8_t*)0x20000b7d = 0;
  *(uint8_t*)0x20000b7e = 0;
  *(uint8_t*)0x20000b7f = 0;
  *(uint8_t*)0x20000b80 = 0;
  *(uint8_t*)0x20000b81 = 0;
  *(uint8_t*)0x20000b82 = 0;
  *(uint8_t*)0x20000b83 = 0;
  *(uint8_t*)0x20000b84 = 0;
  *(uint8_t*)0x20000b85 = 0;
  *(uint8_t*)0x20000b86 = 0;
  *(uint8_t*)0x20000b87 = 0;
  *(uint8_t*)0x20000b88 = 0;
  *(uint8_t*)0x20000b89 = 0;
  *(uint8_t*)0x20000b8a = 0;
  *(uint8_t*)0x20000b8b = 0;
  *(uint8_t*)0x20000b8c = 0;
  *(uint8_t*)0x20000b8d = 0;
  *(uint8_t*)0x20000b8e = 0;
  *(uint8_t*)0x20000b8f = 0;
  *(uint8_t*)0x20000b90 = 0;
  *(uint8_t*)0x20000b91 = 0;
  *(uint8_t*)0x20000b92 = 0;
  *(uint8_t*)0x20000b93 = 0;
  *(uint8_t*)0x20000b94 = 0;
  *(uint8_t*)0x20000b95 = 0;
  *(uint8_t*)0x20000b96 = 0;
  *(uint8_t*)0x20000b97 = 0;
  *(uint8_t*)0x20000b98 = 0;
  *(uint8_t*)0x20000b99 = 0;
  *(uint8_t*)0x20000b9a = 0;
  *(uint8_t*)0x20000b9b = 0;
  *(uint16_t*)0x20000b9c = 0xc0;
  *(uint16_t*)0x20000b9e = 0x110;
  *(uint32_t*)0x20000ba0 = 0;
  *(uint64_t*)0x20000ba8 = 0;
  *(uint64_t*)0x20000bb0 = 0;
  *(uint16_t*)0x20000bb8 = 0x50;
  memcpy((void*)0x20000bba, "mangle\000\000\000\000\000\000\000\000\000\000\000"
                            "\000\000\000\000\000\000\000\000\000\000\000\000",
         29);
  *(uint8_t*)0x20000bd7 = 0;
  *(uint8_t*)0x20000bd8 = 0;
  *(uint8_t*)0x20000bd9 = 0;
  *(uint8_t*)0x20000bda = 0;
  *(uint8_t*)0x20000bdb = 0;
  *(uint8_t*)0x20000bdc = 0;
  *(uint8_t*)0x20000bdd = 0;
  *(uint8_t*)0x20000be8 = 0xaa;
  *(uint8_t*)0x20000be9 = 0xaa;
  *(uint8_t*)0x20000bea = 0xaa;
  *(uint8_t*)0x20000beb = 0xaa;
  *(uint8_t*)0x20000bec = 0xaa;
  *(uint8_t*)0x20000bed = 0;
  *(uint32_t*)0x20000bf8 = htobe32(0);
  *(uint32_t*)0x20000bfc = htobe32(0xe0000001);
  *(uint8_t*)0x20000c00 = 0;
  *(uint32_t*)0x20000c04 = 0;
  *(uint8_t*)0x20000c08 = 0;
  *(uint8_t*)0x20000c09 = 0;
  *(uint8_t*)0x20000c0a = 0;
  *(uint8_t*)0x20000c0b = 0;
  *(uint8_t*)0x20000c0c = 0;
  *(uint8_t*)0x20000c0d = 0;
  *(uint8_t*)0x20000c0e = 0;
  *(uint8_t*)0x20000c0f = 0;
  *(uint8_t*)0x20000c10 = 0;
  *(uint8_t*)0x20000c11 = 0;
  *(uint8_t*)0x20000c12 = 0;
  *(uint8_t*)0x20000c13 = 0;
  *(uint8_t*)0x20000c14 = 0;
  *(uint8_t*)0x20000c15 = 0;
  *(uint8_t*)0x20000c16 = 0;
  *(uint8_t*)0x20000c17 = 0;
  *(uint8_t*)0x20000c18 = 0;
  *(uint8_t*)0x20000c19 = 0;
  *(uint8_t*)0x20000c1a = 0;
  *(uint8_t*)0x20000c1b = 0;
  *(uint8_t*)0x20000c1c = 0;
  *(uint8_t*)0x20000c1d = 0;
  *(uint8_t*)0x20000c1e = 0;
  *(uint8_t*)0x20000c1f = 0;
  *(uint8_t*)0x20000c20 = 0;
  *(uint8_t*)0x20000c21 = 0;
  *(uint8_t*)0x20000c22 = 0;
  *(uint8_t*)0x20000c23 = 0;
  *(uint8_t*)0x20000c24 = 0;
  *(uint8_t*)0x20000c25 = 0;
  *(uint8_t*)0x20000c26 = 0;
  *(uint8_t*)0x20000c27 = 0;
  *(uint8_t*)0x20000c28 = 0;
  *(uint8_t*)0x20000c29 = 0;
  *(uint8_t*)0x20000c2a = 0;
  *(uint8_t*)0x20000c2b = 0;
  *(uint8_t*)0x20000c2c = 0;
  *(uint8_t*)0x20000c2d = 0;
  *(uint8_t*)0x20000c2e = 0;
  *(uint8_t*)0x20000c2f = 0;
  *(uint8_t*)0x20000c30 = 0;
  *(uint8_t*)0x20000c31 = 0;
  *(uint8_t*)0x20000c32 = 0;
  *(uint8_t*)0x20000c33 = 0;
  *(uint8_t*)0x20000c34 = 0;
  *(uint8_t*)0x20000c35 = 0;
  *(uint8_t*)0x20000c36 = 0;
  *(uint8_t*)0x20000c37 = 0;
  *(uint8_t*)0x20000c38 = 0;
  *(uint8_t*)0x20000c39 = 0;
  *(uint8_t*)0x20000c3a = 0;
  *(uint8_t*)0x20000c3b = 0;
  *(uint8_t*)0x20000c3c = 0;
  *(uint8_t*)0x20000c3d = 0;
  *(uint8_t*)0x20000c3e = 0;
  *(uint8_t*)0x20000c3f = 0;
  *(uint8_t*)0x20000c40 = 0;
  *(uint8_t*)0x20000c41 = 0;
  *(uint8_t*)0x20000c42 = 0;
  *(uint8_t*)0x20000c43 = 0;
  *(uint8_t*)0x20000c44 = 0;
  *(uint8_t*)0x20000c45 = 0;
  *(uint8_t*)0x20000c46 = 0;
  *(uint8_t*)0x20000c47 = 0;
  *(uint8_t*)0x20000c48 = 0;
  *(uint8_t*)0x20000c49 = 0;
  *(uint8_t*)0x20000c4a = 0;
  *(uint8_t*)0x20000c4b = 0;
  *(uint8_t*)0x20000c4c = 0;
  *(uint8_t*)0x20000c4d = 0;
  *(uint8_t*)0x20000c4e = 0;
  *(uint8_t*)0x20000c4f = 0;
  *(uint8_t*)0x20000c50 = 0;
  *(uint8_t*)0x20000c51 = 0;
  *(uint8_t*)0x20000c52 = 0;
  *(uint8_t*)0x20000c53 = 0;
  *(uint8_t*)0x20000c54 = 0;
  *(uint8_t*)0x20000c55 = 0;
  *(uint8_t*)0x20000c56 = 0;
  *(uint8_t*)0x20000c57 = 0;
  *(uint8_t*)0x20000c58 = 0;
  *(uint8_t*)0x20000c59 = 0;
  *(uint8_t*)0x20000c5a = 0;
  *(uint8_t*)0x20000c5b = 0;
  *(uint8_t*)0x20000c5c = 0;
  *(uint8_t*)0x20000c5d = 0;
  *(uint8_t*)0x20000c5e = 0;
  *(uint8_t*)0x20000c5f = 0;
  *(uint8_t*)0x20000c60 = 0;
  *(uint8_t*)0x20000c61 = 0;
  *(uint8_t*)0x20000c62 = 0;
  *(uint8_t*)0x20000c63 = 0;
  *(uint8_t*)0x20000c64 = 0;
  *(uint8_t*)0x20000c65 = 0;
  *(uint8_t*)0x20000c66 = 0;
  *(uint8_t*)0x20000c67 = 0;
  *(uint8_t*)0x20000c68 = 0;
  *(uint8_t*)0x20000c69 = 0;
  *(uint8_t*)0x20000c6a = 0;
  *(uint8_t*)0x20000c6b = 0;
  *(uint8_t*)0x20000c6c = 0;
  *(uint8_t*)0x20000c6d = 0;
  *(uint8_t*)0x20000c6e = 0;
  *(uint8_t*)0x20000c6f = 0;
  *(uint8_t*)0x20000c70 = 0;
  *(uint8_t*)0x20000c71 = 0;
  *(uint8_t*)0x20000c72 = 0;
  *(uint8_t*)0x20000c73 = 0;
  *(uint8_t*)0x20000c74 = 0;
  *(uint8_t*)0x20000c75 = 0;
  *(uint8_t*)0x20000c76 = 0;
  *(uint8_t*)0x20000c77 = 0;
  *(uint8_t*)0x20000c78 = 0;
  *(uint8_t*)0x20000c79 = 0;
  *(uint8_t*)0x20000c7a = 0;
  *(uint8_t*)0x20000c7b = 0;
  *(uint8_t*)0x20000c7c = 0;
  *(uint8_t*)0x20000c7d = 0;
  *(uint8_t*)0x20000c7e = 0;
  *(uint8_t*)0x20000c7f = 0;
  *(uint8_t*)0x20000c80 = 0;
  *(uint8_t*)0x20000c81 = 0;
  *(uint8_t*)0x20000c82 = 0;
  *(uint8_t*)0x20000c83 = 0;
  *(uint8_t*)0x20000c84 = 0;
  *(uint8_t*)0x20000c85 = 0;
  *(uint8_t*)0x20000c86 = 0;
  *(uint8_t*)0x20000c87 = 0;
  *(uint8_t*)0x20000c88 = 0;
  *(uint8_t*)0x20000c89 = 0;
  *(uint8_t*)0x20000c8a = 0;
  *(uint8_t*)0x20000c8b = 0;
  *(uint8_t*)0x20000c8c = 0;
  *(uint8_t*)0x20000c8d = 0;
  *(uint8_t*)0x20000c8e = 0;
  *(uint8_t*)0x20000c8f = 0;
  *(uint8_t*)0x20000c90 = 0;
  *(uint8_t*)0x20000c91 = 0;
  *(uint8_t*)0x20000c92 = 0;
  *(uint8_t*)0x20000c93 = 0;
  *(uint8_t*)0x20000c94 = 0;
  *(uint8_t*)0x20000c95 = 0;
  *(uint8_t*)0x20000c96 = 0;
  *(uint8_t*)0x20000c97 = 0;
  *(uint8_t*)0x20000c98 = 0;
  *(uint8_t*)0x20000c99 = 0;
  *(uint8_t*)0x20000c9a = 0;
  *(uint8_t*)0x20000c9b = 0;
  *(uint8_t*)0x20000c9c = 0;
  *(uint8_t*)0x20000c9d = 0;
  *(uint8_t*)0x20000c9e = 0;
  *(uint8_t*)0x20000c9f = 0;
  *(uint8_t*)0x20000ca0 = 0;
  *(uint8_t*)0x20000ca1 = 0;
  *(uint8_t*)0x20000ca2 = 0;
  *(uint8_t*)0x20000ca3 = 0;
  *(uint8_t*)0x20000ca4 = 0;
  *(uint8_t*)0x20000ca5 = 0;
  *(uint8_t*)0x20000ca6 = 0;
  *(uint8_t*)0x20000ca7 = 0;
  *(uint8_t*)0x20000ca8 = 0;
  *(uint8_t*)0x20000ca9 = 0;
  *(uint8_t*)0x20000caa = 0;
  *(uint8_t*)0x20000cab = 0;
  *(uint16_t*)0x20000cac = 0xc0;
  *(uint16_t*)0x20000cae = 0xe8;
  *(uint32_t*)0x20000cb0 = 0;
  *(uint64_t*)0x20000cb8 = 0;
  *(uint64_t*)0x20000cc0 = 0;
  *(uint16_t*)0x20000cc8 = 0x28;
  memcpy((void*)0x20000cca, "\000\000\000\000\000\000\000\000\000\000\000\000"
                            "\000\000\000\000\000\000\000\000\000\000\000\000"
                            "\000\000\000\000\000",
         29);
  *(uint8_t*)0x20000ce7 = 0;
  *(uint32_t*)0x20000ce8 = 0xfffffffe;
  syscall(__NR_setsockopt, r[0], 0xa02000000000000ul, 0x60ul, 0x200008c0ul,
          0x430ul);
  return 0;
}
