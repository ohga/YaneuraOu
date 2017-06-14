// CPUID example
//
// see below for details:
//
// Intel Processor Identification and the CPUID Instruction
//     http://www.intel.com/content/www/us/en/processors/processor-identification-cpuid-instruction-note.html
//
// AMD : CPUID Specification
//     http://support.amd.com/us/Embedded_TechDocs/25481.pdf

// http://d.hatena.ne.jp/shiku_otomiya/20091016/p1
// https://en.wikipedia.org/wiki/CPUID
// https://gist.github.com/t-mat/3769328
#include <stdio.h>
#include <stdint.h>
#include <string>

#if defined(_MSC_VER)
#include <intrin.h>
static void get_cpuid(int *p, int i)
{
    __cpuid(p, i);
}
static void get_cpuidex(int *p, int i, int c)
{
    __cpuidex(p, i, c);
}

#elif defined(__GNUC__)
#include <cpuid.h>
static void get_cpuid(int *p, int i)
{
  __cpuid(i, p[0], p[1], p[2], p[3]);
}
static void get_cpuidex(int *p, int i, int c)
{
  __cpuid_count(i, c, p[0], p[1], p[2], p[3]);
}

#else
#error "This platform is not supported"
#endif


struct CpuInfo
{
    uint32_t eax, ebx, ecx, edx; // Do not change member order.

    CpuInfo(int infoType) { get_cpuid(&eax, infoType); }
    CpuInfo(int infoType, uint32_t ecxValue) { get_cpuidex(&eax, infoType, ecxValue); }
};


int main()
{
    char vendor[12+1] = { 0 };
    {
        CpuInfo f0(0);
        *(uint32_t *)&vendor[4*0] = f0.ebx;
        *(uint32_t *)&vendor[4*1] = f0.edx;
        *(uint32_t *)&vendor[4*2] = f0.ecx;
    }
    vendor[sizeof(vendor)-1] = 0;

    char brand[16*3+1] = { 0 };
    for(int i = 0; i < 3; ++i) {
        CpuInfo e(i + 0x80000002);
        *(uint32_t *)&brand[16*i + 4*0] = e.eax;
        *(uint32_t *)&brand[16*i + 4*1] = e.ebx;
        *(uint32_t *)&brand[16*i + 4*2] = e.ecx;
        *(uint32_t *)&brand[16*i + 4*3] = e.edx;
    };
    brand[sizeof(brand)-1] = 0;

    printf("Vendor       : [%s]\n", vendor);
    printf("Brand String : [%s]\n", brand);

    CpuInfo f1(1);
    printf("CMOV         : %d\n", (f1.edx >> 15) & 1);
    printf("FMA          : %d\n", (f1.ecx >> 12) & 1);
    printf("AVX          : %d\n", (f1.ecx >> 28) & 1);
    printf("RDRAND       : %d\n", (f1.ecx >> 30) & 1);
}
