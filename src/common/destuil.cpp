#include "desutil.h"
#include <QByteArray>

DESutil::DESutil() {}

bool DESutil::validateInput(const QString& input) {
    return !input.isEmpty() && input.length() <= 8; // 验证输入
}

// 2.1 密钥转换 (Key Transformation)
std::vector<uint64> DESutil::generateSubkeys(uint64 key) {
    std::vector<uint64> subkeys;
    // 步骤：丢弃每第8位，将64位转为56位
    uint64 key56 = 0;
    for (int i = 0; i < 64; ++i) {
        if ((i + 1) % 8 != 0) {
            key56 = (key56 << 1) | ((key >> (63 - i)) & 1);
        }
    }

    uint32 left = (uint32)((key56 >> 28) & 0x0FFFFFFF);
    uint32 right = (uint32)(key56 & 0x0FFFFFFF);

    for (int i = 1; i <= 16; ++i) {
        // 根据轮数决定循环左移位数
        int shift = (i == 1 || i == 2 || i == 9 || i == 16) ? 1 : 2;
        left = ((left << shift) | (left >> (28 - shift))) & 0x0FFFFFFF;
        right = ((right << shift) | (right >> (28 - shift))) & 0x0FFFFFFF;

        // 压缩置换：从56位中选出48位作为子密钥
        uint64 subkey48 = ((uint64)left << 24) | (right & 0xFFFFFF);
        subkeys.push_back(subkey48);
    }
    return subkeys;
}

// 2.2-2.5 轮函数 (Round Function)
uint32 DESutil::roundFunction(uint32 rpt, uint64 subkey) {
    // 1. 扩展置换 (Expansion Permutation): 32位 -> 48位
    uint64 expandedRPT = 0;
    // 逻辑：将32位分成8块，每块4位扩展为6位
    for (int i = 0; i < 8; ++i) {
        uint64 block = (rpt >> (28 - i * 4)) & 0xF;
        // 简化示意：取边缘位重复
        expandedRPT = (expandedRPT << 6) | (block | 0x20);
    }

    // 2. 与子密钥异或
    uint64 xored = expandedRPT ^ subkey;

    // 3. S盒替换 (S-box Substitution): 48位 -> 32位
    uint32 sboxOutput = 0;
    for (int i = 0; i < 8; ++i) {
        uint8_t sixBits = (xored >> (42 - i * 6)) & 0x3F;
        // 文档逻辑：第1、6位为行，中间4位为列
        uint8_t row = ((sixBits >> 4) & 0x2) | (sixBits & 0x1);
        uint8_t col = (sixBits >> 1) & 0xF;

        // 假设这里调用 S-box 表中的值（如文档示例中的 13/1101）
        uint8_t val = (row + col) % 16; // 实验简化，实际需查标准S盒表
        sboxOutput = (sboxOutput << 4) | val;
    }

    // 4. P盒置换
    return sboxOutput;
}

// 加密
QString DESutil::encrypt(const QString& plainText, const QString& key) {
    uint64 data = stringTo64Bit(plainText);
    uint64 k = stringTo64Bit(key);

    uint64 current = initialPermutation(data); // Step 1
    uint32 lpt = (current >> 32) & 0xFFFFFFFF;
    uint32 rpt = current & 0xFFFFFFFF;

    std::vector<uint64> subkeys = generateSubkeys(k);
    for (int i = 0; i < 16; ++i) { // Step 2: 16 Rounds
        uint32 nextRpt = lpt ^ roundFunction(rpt, subkeys[i]); // XOR
        lpt = rpt; // Swap
        rpt = nextRpt;
    }

    uint64 combined = ((uint64)rpt << 32) | lpt; // Step 3
    return QString::number(finalPermutation(combined), 16).toUpper();
}

// 解密
QString DESutil::decrypt(const QString& cipherText, const QString& key) {
    uint64 data = cipherText.toULongLong(nullptr, 16);
    uint64 k = stringTo64Bit(key);

    uint64 current = initialPermutation(data);
    uint32 lpt = (current >> 32) & 0xFFFFFFFF;
    uint32 rpt = current & 0xFFFFFFFF;

    std::vector<uint64> subkeys = generateSubkeys(k);
    for (int i = 15; i >= 0; --i) { // 解密子密钥逆序
        uint32 nextRpt = lpt ^ roundFunction(rpt, subkeys[i]);
        lpt = rpt;
        rpt = nextRpt;
    }

    uint64 combined = ((uint64)rpt << 32) | lpt;
    return sixtyFourBitToString(finalPermutation(combined));
}

// 辅助函数
uint64 DESutil::stringTo64Bit(const QString& str) {
    QByteArray ba = str.toUtf8().leftJustified(8, '\0');
    uint64 val = 0;
    for(int i=0; i<8; ++i) val = (val << 8) | (uint8_t)ba[i];
    return val;
}

QString DESutil::sixtyFourBitToString(uint64 val) {
    QByteArray ba;
    for(int i=0; i<8; ++i) ba.prepend((char)(val >> (i * 8)));
    return QString::fromUtf8(ba).trimmed();
}

uint64 DESutil::initialPermutation(uint64 input) { return input; } // 实验可保持原样
uint64 DESutil::finalPermutation(uint64 input) { return input; }

// 1. 初始化类内的静态常量成员（注意要带 DESutil::）
const QString DESutil::s_desKey = "hydrangea";

// 2. 实现类成员函数，去掉 static，加上 DESutil:: 前缀
QString DESutil::encryptWithDefaultKey(const QString& plainText) {
    return DESutil::encrypt(plainText, s_desKey);
}

QString DESutil::decryptWithDefaultKey(const QString& cipherText) {
    // 这里的逻辑是用默认 Key 调用你的解密函数
    return DESutil::decrypt(cipherText, s_desKey);
}