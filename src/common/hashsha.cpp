#include "hashsha.h"
#include <iomanip>
#include <sstream>

// SHA-256 算法常量表
const WORD k[64] = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

HashSha::HashSha() {}

QString HashSha::hashSha256(const QString& input) {
    BYTE hash[32];
    // 转换为 std::string 处理
    computeSha256(input.toStdString(), hash);

    // 将 32 字节结果转为十六进制 QString
    std::stringstream ss;
    for (int i = 0; i < 32; ++i)
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];

    return QString::fromStdString(ss.str());
}

void HashSha::computeSha256(const std::string& input, BYTE hash[32]) {
    // 初始哈希值（H0）
    WORD h[] = { 0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a, 0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19 };

    std::vector<BYTE> data(input.begin(), input.end());
    uint64_t bit_len = data.size() * 8;

    // 1. 补位逻辑 (Padding)
    data.push_back(0x80); // 补一个 '1'
    while ((data.size() * 8) % 512 != 448)
        data.push_back(0x00); // 补 '0' 直到长度符合要求

    // 2. 附加长度信息 (64-bit length)
    for (int i = 7; i >= 0; --i)
        data.push_back((bit_len >> (i * 8)) & 0xFF);

    // 3. 分块处理 (每个块 512 bit / 64 byte)
    for (size_t i = 0; i < data.size(); i += 64) {
        WORD w[64];
        // 将块拆分为 16 个 32-bit 字
        for (int j = 0; j < 16; ++j) {
            w[j] = (data[i + j * 4] << 24) | (data[i + j * 4 + 1] << 16) |
                   (data[i + j * 4 + 2] << 8) | (data[i + j * 4 + 3]);
        }
        // 扩展为 64 个字
        for (int j = 16; j < 64; ++j) {
            w[j] = sig1(w[j - 2]) + w[j - 7] + sig0(w[j - 15]) + w[j - 16];
        }

        // 初始化工作变量
        WORD a = h[0], b = h[1], c = h[2], d = h[3];
        WORD e = h[4], f = h[5], g = h[6], h_val = h[7];

        // 核心压缩循环 (64次)
        for (int j = 0; j < 64; ++j) {
            WORD t1 = h_val + SIG1(e) + choose(e, f, g) + k[j] + w[j];
            WORD t2 = SIG0(a) + majority(a, b, c);
            h_val = g; g = f; f = e;
            e = d + t1;
            d = c; c = b; b = a;
            a = t1 + t2;
        }

        // 更新哈希值
        h[0] += a; h[1] += b; h[2] += c; h[3] += d;
        h[4] += e; h[5] += f; h[6] += g; h[7] += h_val;
    }

    // 4. 输出最终 256-bit 哈希结果
    for (int i = 0; i < 8; ++i) {
        hash[i * 4] = (h[i] >> 24) & 0xFF;
        hash[i * 4 + 1] = (h[i] >> 16) & 0xFF;
        hash[i * 4 + 2] = (h[i] >> 8) & 0xFF;
        hash[i * 4 + 3] = h[i] & 0xFF;
    }
}