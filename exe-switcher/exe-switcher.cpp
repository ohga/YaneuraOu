// CPUID example
//
// see below for details:
//
// Intel Processor Identification and the CPUID Instruction
//     http://www.intel.com/content/www/us/en/processors/processor-identification-cpuid-instruction-note.html
//
// AMD : CPUID Specification
//     http://support.amd.com/us/Embedded_TechDocs/25481.pdf

// https://en.wikipedia.org/wiki/CPUID
// https://gist.github.com/t-mat/3769328
#include <array>
#include <string>
#include <bitset>
#include <vector>
#include <process.h>
#include <stdio.h>
#include <memory.h>

// int型のサイズが4バイトか確認する
static_assert(sizeof(int) == 4, "The size of integer must be 4byte.");

#if defined(_MSC_VER)
#include <intrin.h>

#define execvp  _execvp

// VCでcpuidの値を取得する
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

// gcc系でcpuidの値を取得する
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

// exeの拡張子を選択
#if defined(WIN32) || defined(_WIN32)
#define EXE_EXT ".exe"
#else
#define EXE_EXT ""
#endif

/// 参考: https://msdn.microsoft.com/ja-jp/library/hskdteyh.aspx
class InstructionSet
{
public:
    static std::string vendor() { return internal.vendor_; }
    static std::string brand()  { return internal.brand_; }

    static bool sse3() { return internal.f1_ecx_[0]; }
    static bool pclmulqdq() { return internal.f1_ecx_[1]; }
    static bool monitor() { return internal.f1_ecx_[3]; }
    static bool ssse3() { return internal.f1_ecx_[9]; }
    static bool fma() { return internal.f1_ecx_[12]; }
    static bool cmpxchg16b() { return internal.f1_ecx_[13]; }
    static bool sse41() { return internal.f1_ecx_[19]; }
    static bool sse42() { return internal.f1_ecx_[20]; }
    static bool movbe() { return internal.f1_ecx_[22]; }
    static bool popcnt() { return internal.f1_ecx_[23]; }
    static bool aes() { return internal.f1_ecx_[25]; }
    static bool xsave() { return internal.f1_ecx_[26]; }
    static bool osxsave() { return internal.f1_ecx_[27]; }
    static bool avx() { return internal.f1_ecx_[28]; }
    static bool f16c() { return internal.f1_ecx_[29]; }
    static bool rdrand() { return internal.f1_ecx_[30]; }

    static bool msr() { return internal.f1_edx_[5]; }
    static bool cx8() { return internal.f1_edx_[8]; }
    static bool sep() { return internal.f1_edx_[11]; }
    static bool cmov() { return internal.f1_edx_[15]; }
    static bool clfsh() { return internal.f1_edx_[19]; }
    static bool mmx() { return internal.f1_edx_[23]; }
    static bool fxsr() { return internal.f1_edx_[24]; }
    static bool sse() { return internal.f1_edx_[25]; }
    static bool sse2() { return internal.f1_edx_[26]; }

    static bool fsgsbase() { return internal.f7_ebx_[0]; }
    static bool bmi1() { return internal.f7_ebx_[3]; } 
    static bool avx2() { return internal.f7_ebx_[5]; }
    static bool bmi2() { return internal.f7_ebx_[8]; }
    static bool erms() { return internal.f7_ebx_[9]; }
    static bool invpcid() { return internal.f7_ebx_[10]; }
    static bool avx512f() { return internal.f7_ebx_[16]; }
    static bool rdseed() { return internal.f7_ebx_[18]; }
    static bool adx() { return internal.f7_ebx_[19]; }
    static bool avx512pf() { return internal.f7_ebx_[26]; }
    static bool avx512er() { return internal.f7_ebx_[27]; }
    static bool avx512cd() { return internal.f7_ebx_[28]; }
    static bool sha() { return internal.f7_ebx_[29]; }

private:
    class InstructionSet_Internal
    {
    public:
        InstructionSet_Internal()
        {
            // cpuid(0)の情報をまとめて取得
            auto last_id = get_last_id(0);
            auto data = get_data(0, last_id);

            // Capture vendor string
            char vendor[0x20] = { 0 };
            *reinterpret_cast<int *>(vendor + 0) = data[0][1];
            *reinterpret_cast<int *>(vendor + 4) = data[0][3];
            *reinterpret_cast<int *>(vendor + 8) = data[0][2];
            vendor_ = vendor;

            // load bitset with flags for function 0x00000001
            if (last_id >= 1) {
                f1_ecx_ = data[1][2];
                f1_edx_ = data[1][3];
            }

            // load bitset with flags for function 0x00000007
            if (last_id >= 7) {
                f7_ebx_ = data[7][1];
                f7_ecx_ = data[7][2];
            }

            // cpuid(0x80000000)の情報をまとめて取得
            auto last_id_ex = get_last_id(0x80000000);
            auto data_ex = get_data(0x80000000, last_id_ex);

            // Interpret CPU brand string if reported
            if (last_id_ex >= 0x80000004) {
                char brand[0x40] = { 0 };
                memcpy(brand + 0, data_ex[2].data(), sizeof(data_ex[0]));
                memcpy(brand + 16, data_ex[3].data(), sizeof(data_ex[0]));
                memcpy(brand + 32, data_ex[4].data(), sizeof(data_ex[0]));
                brand_ = brand;
            }
        };

        /// cpuidで取得可能なidの最後の数値を取得
        uint32_t get_last_id(int ecx) const
        {
            std::array<int, 4> cpuinfo;

            get_cpuid(cpuinfo.data(), ecx);
            return static_cast<uint32_t>(cpuinfo[0]);
        }

        /// cpuidによるデータをまとめて取得
        std::vector<std::array<int, 4>> get_data(int start_id, int last_id)
        {
            std::vector<std::array<int, 4>> data;
            std::array<int, 4> cpuinfo;

            for (int id = start_id; id <= last_id; ++id) {
                get_cpuidex(cpuinfo.data(), id, 0);
                data.push_back(cpuinfo);
            }

            return data;
        }

        std::string vendor_;
        std::string brand_;
        std::bitset<32> f1_ecx_;
        std::bitset<32> f1_edx_;
        std::bitset<32> f7_ebx_;
        std::bitset<32> f7_ecx_;
    };

    static const InstructionSet_Internal internal;
};

const InstructionSet::InstructionSet_Internal InstructionSet::internal;

static void exec_file(const std::string &filename, char *argv[])
{
#if 1
    std::vector<char> cloned(filename.begin(), filename.end());
    cloned.push_back('\0');
    argv[0] = cloned.data();

    printf("start");
    for (int i = 0; argv[i] != nullptr; ++i) {
        printf(" '%s'", argv[i]);
    }
    printf("\n");

    execvp(filename.c_str(), argv);

#else
    std::string command = filename;

    for (int i = 1; argv[i] != nullptr; ++i) {
        command += " ";
        command += (const char *)argv[i];
    }

    system(command.c_str());
#endif
}

int main(int argc, char *argv[])
{
    if (InstructionSet::avx2()) {
        exec_file("YaneuraOu-2017-early-godwhale" EXE_EXT, argv);
    }
    else if (InstructionSet::sse42()) {
        exec_file("YaneuraOu-2017-early-sse42-godwhale" EXE_EXT, argv);
    }
    else if (InstructionSet::sse41()) {
        exec_file("YaneuraOu-2017-early-sse41-godwhale" EXE_EXT, argv);
    }
    else if (InstructionSet::sse2()) {
        exec_file("YaneuraOu-2017-early-sse2-godwhale" EXE_EXT, argv);
    }
    else {
        fprintf(stderr, "fatal error: Not supported environment.");
    }

    return 0;
}
