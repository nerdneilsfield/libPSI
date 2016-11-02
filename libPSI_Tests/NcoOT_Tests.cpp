#include "OT_Tests.h"

#include "OT/TwoChooseOne/OTExtInterface.h"

#include "OT/Tools/Tools.h"
#include "OT/Tools/BchCode.h"
#include "Network/BtChannel.h"
#include "Network/BtEndpoint.h"
#include "Common/Log.h"
 
#include "OT/NChooseOne/KkrtNcoOtReceiver.h"
#include "OT/NChooseOne/KkrtNcoOtSender.h"

#include "OT/NChooseOne/Oos/OosNcoOtReceiver.h"
#include "OT/NChooseOne/Oos/OosNcoOtSender.h"

#include "Common.h"
#include <thread>
#include <vector>
#include <boost/multiprecision/cpp_int.hpp>

#ifdef GetMessage
#undef GetMessage
#endif

using namespace osuCrypto;
using namespace boost::multiprecision;




void KkrtNcoOt_Test_Impl()
{
    Log::setThreadName("Sender");

    PRNG prng0(_mm_set_epi32(4253465, 3434565, 234435, 23987045));

    u64 numOTs = 128;

    KkrtNcoOtSender sender;
    KkrtNcoOtReceiver recv;
    u64 codeSize, baseCount;
    sender.getParams(true,128, 40, 128, numOTs, codeSize, baseCount);
    
    std::vector<block> baseRecv(baseCount);
    std::vector<std::array<block, 2>> baseSend(baseCount);
    BitVector baseChoice(baseCount);
    baseChoice.randomize(prng0);

    prng0.get((u8*)baseSend.data()->data(), sizeof(block) * 2 * baseSend.size());
    for (u64 i = 0; i < baseCount; ++i)
    {
        baseRecv[i] = baseSend[i][baseChoice[i]];
    }

    std::string name = "n";
    BtIOService ios(0);
    BtEndpoint ep0(ios, "localhost", 1212, true, name);
    BtEndpoint ep1(ios, "localhost", 1212, false, name);


    auto &recvChl = ep1.addChannel(name, name);
    auto &sendChl = ep0.addChannel(name, name);




    //BitVector t0,t1,q, s = sender.mBaseChoiceBits;
    //for (u64 i = 0; i < recvMsgs[0].size(); ++i)
    //{
    //    t0.append((u8*)&recvMsgs[0][i][0], 8 * sizeof(block));
    //    t1.append((u8*)&recvMsgs[0][i][1], 8 * sizeof(block));
    //    q.append((u8*)&sendMsgs[0][i], 8 * sizeof(block));
    //}

    //auto exp = (t0 & ~s) | (t1 & s);

    //Log::out << Log::endl
    //    << exp << Log::endl
    //    << q << Log::endl
    //    << (q^exp) << Log::endl << Log::endl;

    sender.setBaseOts(baseRecv, baseChoice);
    recv.setBaseOts(baseSend);

    std::vector<block> codeword(codeSize), correction(codeSize);
    for (size_t j = 0; j < 10; j++)
    {
        sender.init(numOTs);

        recv.init(numOTs);

        for (u64 i = 0; i < numOTs; ++i)
        {
            prng0.get((u8*)codeword.data(), codeSize * sizeof(block));

            block encoding1, encoding2;
            recv.encode(i, codeword, encoding1);

            recv.sendCorrection(recvChl, 1);
            sender.recvCorrection(sendChl, 1);

            sender.encode(i, codeword,  encoding2);

            if (neq(encoding1, encoding2))
                throw UnitTestFail();

            prng0.get((u8*)codeword.data(), codeSize * sizeof(block));

            sender.encode(i, codeword,  encoding2);

            if (eq(encoding1, encoding2))
                throw UnitTestFail();
        }

    }

    sendChl.close();
    recvChl.close();

    ep0.stop();
    ep1.stop();
    ios.stop();
}

void OosNcoOt_Test_Impl()
{
    Log::setThreadName("Sender");

    PRNG prng0(_mm_set_epi32(4253465, 3434565, 234435, 23987045));

    u64 numOTs = 128;

    BchCode code;
    code.loadBinFile(std::string(SOLUTION_DIR) + "/libPSI/OT/Tools/bch511.bin");

    OosNcoOtSender sender(code);
    OosNcoOtReceiver recv(code);


    u64 ncoinputBlkSize, baseCount;
    sender.getParams(true, 128, 40, 128, numOTs, ncoinputBlkSize, baseCount);
    u64 codeSize = (baseCount + 127) / 128;

    std::vector<block> baseRecv(baseCount);
    std::vector<std::array<block, 2>> baseSend(baseCount);
    BitVector baseChoice(baseCount);
    baseChoice.randomize(prng0);

    prng0.get((u8*)baseSend.data()->data(), sizeof(block) * 2 * baseSend.size());
    for (u64 i = 0; i < baseCount; ++i)
    {
        baseRecv[i] = baseSend[i][baseChoice[i]];
    }



    sender.setBaseOts(baseRecv, baseChoice);
    sender.init(numOTs);

    recv.setBaseOts(baseSend);
    recv.init(numOTs);


    //BitVector t0, t1, q, s = sender.mBaseChoiceBits;
    //for (u64 i = 0; i < recvMsgs[0].size(); ++i)
    //{
    //    t0.append((u8*)&recvMsgs[0][i][0], 8 * sizeof(block));
    //    t1.append((u8*)&recvMsgs[0][i][1], 8 * sizeof(block));
    //    q.append((u8*)&sendMsgs[0][i], 8 * sizeof(block));
    //}

    //auto exp = (t0 & ~s) | (t1 & s);

    //Log::out << Log::endl
    //    << exp << Log::endl
    //    << q << Log::endl
    //    << (q^exp) << Log::endl << Log::endl;


    std::vector<block> choice(ncoinputBlkSize), correction(codeSize);
    for (size_t j = 0; j < 10; j++)
    {

        for (u64 i = 0; i < numOTs; ++i)
        {
            prng0.get((u8*)choice.data(), ncoinputBlkSize * sizeof(block));

            block encoding1, encoding2;
            recv.encode(i, choice,  encoding1);

            sender.encode(i, choice, encoding2);

            if (neq(encoding1, encoding2))
                throw UnitTestFail();

            prng0.get((u8*)choice.data(), ncoinputBlkSize * sizeof(block));

            sender.encode(i, choice, encoding2);

            if (eq(encoding1, encoding2))
                throw UnitTestFail();
        }

    }

}


void BchCode_Test_Impl()
{
    BchCode code;


    code.loadTxtFile(std::string(SOLUTION_DIR) + "/libPSI/OT/Tools/bch511.txt");
    code.writeBinFile(std::string(SOLUTION_DIR) + "/libPSI/OT/Tools/bch511.bin");
    code.loadBinFile(std::string(SOLUTION_DIR) + "/libPSI/OT/Tools/bch511.bin");
    //Log::out << code.codewordBitSize() << "  " << code.codewordBlkSize() <<
    //    "\n  " << code.plaintextBitSize() << "  " << code.plaintextBlkSize() << Log::endl;


    //for (u64 i = 0; i < code.mG.size(); ++i)
    //    Log::out << code.mG[i] << Log::endl;

    std::vector<block> 
        plainText(code.plaintextBlkSize(), AllOneBlock),
        codeword(code.codewordBlkSize());

    code.encode(plainText, codeword);

    BitVector cw((u8*)codeword.data(), code.codewordBitSize());

    // expect all ones
    for (size_t i = 0; i < cw.size(); i++)
    {
        if (cw[i] == 0)
            throw UnitTestFail();
    }

    BitVector pt("1111111111111111111111111111111111111111111111111101111111111101111111111111");
    memset(plainText.data(), 0, plainText.size() * sizeof(block));
    memcpy(plainText.data(), pt.data(), pt.sizeBytes());


    code.encode(plainText, codeword);
    cw.resize(0);
    cw.append((u8*)codeword.data(), code.codewordBitSize());


    BitVector expected("1111111111111111111111111111111111111111111111111101111111111101111111111111101000010001110100011100010110011111110010011010001010000111111001101101110101100000100010010101000110011001111101111100100111000101110000101000000011000100011110011100001101100111111001001011010100010010110001010011000011111010101010010010011101001001100001100010100101001100111000010110011110011110001110001011111101010001101000101010110100011000000011010011110101011001100011111111101001101111001111111101000010000011010111100011100");

    if (cw != expected)
    {
        Log::out << cw << Log::endl;
        Log::out << expected << Log::endl;
        throw UnitTestFail();
    }

}


