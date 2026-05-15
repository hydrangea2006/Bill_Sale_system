#ifndef HASHSHA_H
#define HASHSHA_H

#include <QString>
#include <vector>
#include <string>

typedef uint8_t BYTE;
typedef uint32_t WORD;

class HashSha
{
public:
    HashSha();

    // 外部调用的接口，保持 QString 输入输出以兼容你的项目
    static QString hashSha256(const QString& input);

private:
    // 算法核心内部实现
    static void computeSha256(const std::string& input, BYTE hash[32]);

    // SHA-256 必备的 6 种位运算函数
    static inline WORD rotr(WORD x, WORD n) { return (x >> n) | (x << (32 - n)); }
    static inline WORD choose(WORD e, WORD f, WORD g) { return (e & f) ^ (~e & g); }
    static inline WORD majority(WORD a, WORD b, WORD c) { return (a & b) ^ (a & c) ^ (b & c); }
    static inline WORD sig0(WORD x) { return rotr(x, 7) ^ rotr(x, 18) ^ (x >> 3); }
    static inline WORD sig1(WORD x) { return rotr(x, 17) ^ rotr(x, 19) ^ (x >> 10); }
    static inline WORD SIG0(WORD x) { return rotr(x, 2) ^ rotr(x, 13) ^ rotr(x, 22); }
    static inline WORD SIG1(WORD x) { return rotr(x, 6) ^ rotr(x, 11) ^ rotr(x, 25); }
};

#endif