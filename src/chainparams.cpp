// Copyright (c) 2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcoin developers
// Copyright (c) 2014-2015 The Dash developers
// Copyright (c) 2017-2018 Scrypta Development Team
// Copyright (c) 2015-2018 The PIVX developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "chainparams.h"

#include "bignum.h"


#include "random.h"
#include "util.h"
#include "utilstrencodings.h"

#include <assert.h>

#include <boost/assign/list_of.hpp>


#include "crypto/scrypt.h"
//#include "util.h"
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <openssl/sha.h>


//#include <stdlib.h>
//#include <stdint.h>

static const int SCRYPT_SCRATCHPAD_SIZE = 131072 + 63;

void
PBKDF2_SHA256(const uint8_t *passwd, size_t passwdlen, const uint8_t *salt,
    size_t saltlen, uint64_t c, uint8_t *buf, size_t dkLen);

static inline uint32_t le32dec(const void *pp)
{
        const uint8_t *p = (uint8_t const *)pp;
        return ((uint32_t)(p[0]) + ((uint32_t)(p[1]) << 8) +
            ((uint32_t)(p[2]) << 16) + ((uint32_t)(p[3]) << 24));
}

static inline void le32enc(void *pp, uint32_t x)
{
        uint8_t *p = (uint8_t *)pp;
        p[0] = x & 0xff;
        p[1] = (x >> 8) & 0xff;
        p[2] = (x >> 16) & 0xff;
        p[3] = (x >> 24) & 0xff;
}


static inline uint32_t be32dec(const void *pp)
{
        const uint8_t *p = (uint8_t const *)pp;
        return ((uint32_t)(p[3]) + ((uint32_t)(p[2]) << 8) +
            ((uint32_t)(p[1]) << 16) + ((uint32_t)(p[0]) << 24));
}

static inline void be32enc(void *pp, uint32_t x)
{
        uint8_t *p = (uint8_t *)pp;
        p[3] = x & 0xff;
        p[2] = (x >> 8) & 0xff;
        p[1] = (x >> 16) & 0xff;
        p[0] = (x >> 24) & 0xff;
}

typedef struct HMAC_SHA256Context {
        SHA256_CTX ictx;
        SHA256_CTX octx;
} HMAC_SHA256_CTX;

/* Initialize an HMAC-SHA256 operation with the given key. */
static void
HMAC_SHA256_Init(HMAC_SHA256_CTX *ctx, const void *_K, size_t Klen)
{
        unsigned char pad[64];
        unsigned char khash[32];
        const unsigned char *K = (const unsigned char *)_K;
        size_t i;

        /* If Klen > 64, the key is really SHA256(K). */
        if (Klen > 64) {
                SHA256_Init(&ctx->ictx);
                SHA256_Update(&ctx->ictx, K, Klen);
                SHA256_Final(khash, &ctx->ictx);
                K = khash;
                Klen = 32;
        }

        /* Inner SHA256 operation is SHA256(K xor [block of 0x36] || data). */
        SHA256_Init(&ctx->ictx);
        memset(pad, 0x36, 64);
        for (i = 0; i < Klen; i++)
                pad[i] ^= K[i];
        SHA256_Update(&ctx->ictx, pad, 64);

        /* Outer SHA256 operation is SHA256(K xor [block of 0x5c] || hash). */
        SHA256_Init(&ctx->octx);
        memset(pad, 0x5c, 64);
        for (i = 0; i < Klen; i++)
                pad[i] ^= K[i];
        SHA256_Update(&ctx->octx, pad, 64);

        /* Clean the stack. */
        memset(khash, 0, 32);
}

/* Add bytes to the HMAC-SHA256 operation. */
static void
HMAC_SHA256_Update(HMAC_SHA256_CTX *ctx, const void *in, size_t len)
{
        /* Feed data to the inner SHA256 operation. */
        SHA256_Update(&ctx->ictx, in, len);
}

/* Finish an HMAC-SHA256 operation. */
static void
HMAC_SHA256_Final(unsigned char digest[32], HMAC_SHA256_CTX *ctx)
{
        unsigned char ihash[32];

        /* Finish the inner SHA256 operation. */
        SHA256_Final(ihash, &ctx->ictx);

        /* Feed the inner hash to the outer SHA256 operation. */
        SHA256_Update(&ctx->octx, ihash, 32);

        /* Finish the outer SHA256 operation. */
        SHA256_Final(digest, &ctx->octx);

        /* Clean the stack. */
        memset(ihash, 0, 32);
}

/**
 * PBKDF2_SHA256(passwd, passwdlen, salt, saltlen, c, buf, dkLen):
 * Compute PBKDF2(passwd, salt, c, dkLen) using HMAC-SHA256 as the PRF, and
 * write the output to buf.  The value dkLen must be at most 32 * (2^32 - 1).
 */
      ///////////////////////

#define ROTL(a, b) (((a) << (b)) | ((a) >> (32 - (b))))

static inline void xor_salsa8(uint32_t B[16], const uint32_t Bx[16])
{
        uint32_t x00,x01,x02,x03,x04,x05,x06,x07,x08,x09,x10,x11,x12,x13,x14,x15;
        int i;

        x00 = (B[ 0] ^= Bx[ 0]);
        x01 = (B[ 1] ^= Bx[ 1]);
        x02 = (B[ 2] ^= Bx[ 2]);
        x03 = (B[ 3] ^= Bx[ 3]);
        x04 = (B[ 4] ^= Bx[ 4]);
        x05 = (B[ 5] ^= Bx[ 5]);
        x06 = (B[ 6] ^= Bx[ 6]);
        x07 = (B[ 7] ^= Bx[ 7]);
        x08 = (B[ 8] ^= Bx[ 8]);
        x09 = (B[ 9] ^= Bx[ 9]);
        x10 = (B[10] ^= Bx[10]);
        x11 = (B[11] ^= Bx[11]);
        x12 = (B[12] ^= Bx[12]);
        x13 = (B[13] ^= Bx[13]);
        x14 = (B[14] ^= Bx[14]);
        x15 = (B[15] ^= Bx[15]);
        for (i = 0; i < 8; i += 2) {
                /* Operate on columns. */
                x04 ^= ROTL(x00 + x12,  7);  x09 ^= ROTL(x05 + x01,  7);
                x14 ^= ROTL(x10 + x06,  7);  x03 ^= ROTL(x15 + x11,  7);

                x08 ^= ROTL(x04 + x00,  9);  x13 ^= ROTL(x09 + x05,  9);
                x02 ^= ROTL(x14 + x10,  9);  x07 ^= ROTL(x03 + x15,  9);

                x12 ^= ROTL(x08 + x04, 13);  x01 ^= ROTL(x13 + x09, 13);
                x06 ^= ROTL(x02 + x14, 13);  x11 ^= ROTL(x07 + x03, 13);

                x00 ^= ROTL(x12 + x08, 18);  x05 ^= ROTL(x01 + x13, 18);
                x10 ^= ROTL(x06 + x02, 18);  x15 ^= ROTL(x11 + x07, 18);

                /* Operate on rows. */
                x01 ^= ROTL(x00 + x03,  7);  x06 ^= ROTL(x05 + x04,  7);
                x11 ^= ROTL(x10 + x09,  7);  x12 ^= ROTL(x15 + x14,  7);

                x02 ^= ROTL(x01 + x00,  9);  x07 ^= ROTL(x06 + x05,  9);
                x08 ^= ROTL(x11 + x10,  9);  x13 ^= ROTL(x12 + x15,  9);

                x03 ^= ROTL(x02 + x01, 13);  x04 ^= ROTL(x07 + x06, 13);
                x09 ^= ROTL(x08 + x11, 13);  x14 ^= ROTL(x13 + x12, 13);

                x00 ^= ROTL(x03 + x02, 18);  x05 ^= ROTL(x04 + x07, 18);
                x10 ^= ROTL(x09 + x08, 18);  x15 ^= ROTL(x14 + x13, 18);
        }
        B[ 0] += x00;
        B[ 1] += x01;
        B[ 2] += x02;
        B[ 3] += x03;
        B[ 4] += x04;
        B[ 5] += x05;
        B[ 6] += x06;
        B[ 7] += x07;
        B[ 8] += x08;
        B[ 9] += x09;
        B[10] += x10;
        B[11] += x11;
        B[12] += x12;
        B[13] += x13;
        B[14] += x14;
        B[15] += x15;
}

void scrypt_1024_1_1_256_sp_generic(const char *input, char *output, char *scratchpad)
{
        uint8_t B[128];
        uint32_t X[32];
        uint32_t *V;
        uint32_t i, j, k;

        V = (uint32_t *)(((uintptr_t)(scratchpad) + 63) & ~ (uintptr_t)(63));

        PBKDF2_SHA256((const uint8_t *)input, 80, (const uint8_t *)input, 80, 1, B, 128);

        for (k = 0; k < 32; k++)
                X[k] = le32dec(&B[4 * k]);

        for (i = 0; i < 1024; i++) {
                memcpy(&V[i * 32], X, 128);
                xor_salsa8(&X[0], &X[16]);
                xor_salsa8(&X[16], &X[0]);
        }
        for (i = 0; i < 1024; i++) {
                j = 32 * (X[16] & 1023);
                for (k = 0; k < 32; k++)
                        X[k] ^= V[j + k];
                xor_salsa8(&X[0], &X[16]);
                xor_salsa8(&X[16], &X[0]);
        }

        for (k = 0; k < 32; k++)
                le32enc(&B[4 * k], X[k]);

        PBKDF2_SHA256((const uint8_t *)input, 80, B, 128, 1, (uint8_t *)output, 32);
}

#if defined(USE_SSE2)
#if defined(_M_X64) || defined(__x86_64__) || defined(_M_AMD64) || (defined(MAC_OSX) && defined(__i386__))
/* Always SSE2 */
void scrypt_detect_sse2(unsigned int cpuid_edx)
{
    printf("scrypt: using scrypt-sse2 as built.\n");
}
#else
/* Detect SSE2 */
void (*scrypt_1024_1_1_256_sp)(const char *input, char *output, char *scratchpad);

void scrypt_detect_sse2(unsigned int cpuid_edx)
{
    if (cpuid_edx & 1<<26)
    {
        scrypt_1024_1_1_256_sp = &scrypt_1024_1_1_256_sp_sse2;
        printf("scrypt: using scrypt-sse2 as detected.\n");
    }
    else
    {
        scrypt_1024_1_1_256_sp = &scrypt_1024_1_1_256_sp_generic;
        printf("scrypt: using scrypt-generic, SSE2 unavailable.\n");
    }
}
#endif
#endif

void scrypt_1024_1_1_256(const char *input, char *output)
{
        char scratchpad[SCRYPT_SCRATCHPAD_SIZE];
#if defined(USE_SSE2)
        // Detection would work, but in cases where we KNOW it always has SSE2,
        // it is faster to use directly than to use a function pointer or conditional.
#if defined(_M_X64) || defined(__x86_64__) || defined(_M_AMD64) || (defined(MAC_OSX) && defined(__i386__))
        // Always SSE2: x86_64 or Intel MacOS X
        scrypt_1024_1_1_256_sp_sse2(input, output, scratchpad);
#else
        // Detect SSE2: 32bit x86 Linux or Windows
        scrypt_1024_1_1_256_sp(input, output, scratchpad);
#endif
#else
        // Generic scrypt
        scrypt_1024_1_1_256_sp_generic(input, output, scratchpad);
#endif
}




using namespace std;
using namespace boost::assign;

struct SeedSpec6 {
    uint8_t addr[16];
    uint16_t port;
};

#include "chainparamsseeds.h"

/**
 * Main network
 */

//! Convert the pnSeeds6 array into usable address objects.
static void convertSeed6(std::vector<CAddress>& vSeedsOut, const SeedSpec6* data, unsigned int count)
{
    // It'll only connect to one or two seed nodes because once it connects,
    // it'll get a pile of addresses with newer timestamps.
    // Seed nodes are given a random 'last seen time' of between one and two
    // weeks ago.
    const int64_t nOneWeek = 7 * 24 * 60 * 60;
    for (unsigned int i = 0; i < count; i++) {
        struct in6_addr ip;
        memcpy(&ip, data[i].addr, sizeof(ip));
        CAddress addr(CService(ip, data[i].port));
        addr.nTime = GetTime() - GetRand(nOneWeek) - nOneWeek;
        vSeedsOut.push_back(addr);
    }
}

//   What makes a good checkpoint block?
// + Is surrounded by blocks with reasonable timestamps
//   (no blocks before with a timestamp after, none after with
//    timestamp before)
// + Contains no strange transactions
static Checkpoints::MapCheckpoints mapCheckpoints =
      boost::assign::map_list_of
      (0, uint256("0xe2aacf31ce196903e00157a50d207d04a152176b4eb83ecbb0b75b0c9455d1fd"))
      (750, uint256("0x000000bf6ca733834f543433247145ad2451728e79bcc18f5b109a6bbd11cf87"))  //Last POW Block
      (20925, uint256("0xd1e129981da28059d1d2484e8f098bab1f41e011631dee8771ab8bc16a00ee2c"))
;

static const Checkpoints::CCheckpointData data = {
    &mapCheckpoints,
    1546655985, // * UNIX timestamp of last checkpoint block
    41348,    // * total number of transactions between genesis and last checkpoint
                //   (the tx=... number in the SetBestChain debug.log lines)
    2906        // * estimated number of transactions per day after checkpoint
};

static Checkpoints::MapCheckpoints mapCheckpointsTestnet =
    boost::assign::map_list_of
        (0, uint256("0xd07464ddcf6a6d7e7f48de12d9e824bc1a874965d5f52b300d9962ca489bb8e3"));
static const Checkpoints::CCheckpointData dataTestnet = {
    &mapCheckpointsTestnet,
    1546642871, // * UNIX timestamp of last TESTNET checkpoint block
    0,          // * total number of TESTNET transactions between genesis and last checkpoint
                //   (the tx=... number in the SetBestChain debug.log lines)
    250         // * estimated number TESTNET of transactions per day after checkpoint
};

static Checkpoints::MapCheckpoints mapCheckpointsRegtest =
    boost::assign::map_list_of(0, uint256("0x001"));
static const Checkpoints::CCheckpointData dataRegtest = {
    &mapCheckpointsRegtest,
    1454124731,
    0,
    100};

class CMainParams : public CChainParams
{
public:
    CMainParams()
    {
        networkID = CBaseChainParams::MAIN;
        strNetworkID = "main";
        /**
         * The message start string is designed to be unlikely to occur in normal data.
         * The characters are rarely used upper ASCII, not valid as UTF-8, and produce
         * a large 4-byte int at any alignment.
         */
        pchMessageStart[0] = 0x4c;
        pchMessageStart[1] = 0xaf;
        pchMessageStart[2] = 0x2c;
        pchMessageStart[3] = 0xe9;
        vAlertPubKey = ParseHex("02622dd9ed5fdbcd5a8fd63fd78fd7f88185c37835aec86ffd54ad6ed1f5ac53c5");

        nDefaultPort = 42222;
        bnProofOfWorkLimit = ~uint256(0) >> 20; // Scrypta starting difficulty is the lowest possible 1 / 2^12
        nSubsidyHalvingInterval = 210000;
        nMaxReorganizationDepth = 100;
        nEnforceBlockUpgradeMajority = 750;
        nRejectBlockOutdatedMajority = 950;
        nToCheckBlockUpgradeMajority = 1000;
        nMinerThreads = 0;
        nTargetTimespan = 1 * 60 * 60; // Scrypta 1 hours
        nTargetSpacing = 1 * 60;  // Scrypta Block Time: 1 Min
        nLastPOWBlock = 750;   //Last Pow Block
        nMaturity = 20;
        nMasternodeCountDrift = 20;
        nModifierUpdateBlock = 615800;
        nMaxMoneyOut = 50000000 * COIN;   //Max coin supply 50 Millions Lyra

        const char* pszTimestamp = "Trump Pushes Government Toward Shutdown Over Border Wall // From NYT 21th of Dec, 2018";
        CMutableTransaction txNew;
        txNew.vin.resize(1);
        txNew.vout.resize(1);
        txNew.vin[0].scriptSig = CScript() << 486604799 << CScriptNum(4) << vector<unsigned char>((const unsigned char*)pszTimestamp, (const unsigned char*)pszTimestamp + strlen(pszTimestamp));
        txNew.vout[0].nValue = 10 * COIN;
        txNew.vout[0].scriptPubKey = CScript() << ParseHex("046901033eeb013d794cbbc51a8b6bc5fff90c86bd35d07e3edac2ed065cc738d6bc51ab0214747a0d774fd32c260e01bf8c5d398e981d53cce04785dd20b32d84") << OP_CHECKSIG;
        genesis.vtx.push_back(txNew);
        genesis.hashPrevBlock = 0;
        genesis.hashMerkleRoot = genesis.BuildMerkleTree();
        genesis.nVersion = 1;
        genesis.nTime = 1545392792;
        genesis.nBits = 0x1e0ffff0;
        genesis.nNonce = 2546225;


        hashGenesisBlock = genesis.GetHash();
        assert(hashGenesisBlock == uint256("0xe2aacf31ce196903e00157a50d207d04a152176b4eb83ecbb0b75b0c9455d1fd"));

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1, 48);  // Mainnet lyra addresses start with 'L'
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1, 13); // Mainnet lyra script addresses start with '6'
        base58Prefixes[SECRET_KEY] = std::vector<unsigned char>(1, 174);
        base58Prefixes[EXT_PUBLIC_KEY] = boost::assign::list_of(0x2D)(0x02)(0x31)(0x33).convert_to_container<std::vector<unsigned char> >();
        base58Prefixes[EXT_SECRET_KEY] = boost::assign::list_of(0x02)(0x21)(0x25)(0x2B).convert_to_container<std::vector<unsigned char> >();
        // 	BIP44 coin type is from https://github.com/satoshilabs/slips/blob/master/slip-0044.md
        base58Prefixes[EXT_COIN_TYPE] = boost::assign::list_of(0x80)(0x00)(0x00)(0x77).convert_to_container<std::vector<unsigned char> >();

	convertSeed6(vFixedSeeds, pnSeed6_main, ARRAYLEN(pnSeed6_main));
	vSeeds.push_back(CDNSSeedData("seed01.scryptachain.org", "seed01.scryptachain.org"));
        vSeeds.push_back(CDNSSeedData("seed02.scryptachain.org", "seed02.scryptachain.org"));
        vSeeds.push_back(CDNSSeedData("seed03.scryptachain.org", "seed03.scryptachain.org"));
        vSeeds.push_back(CDNSSeedData("seed04.scryptachain.org", "seed04.scryptachain.org"));
        vSeeds.push_back(CDNSSeedData("seed05.scryptachain.org", "seed05.scryptachain.org"));
        vSeeds.push_back(CDNSSeedData("seed06.scryptachain.org", "seed06.scryptachain.org"));
        vSeeds.push_back(CDNSSeedData("seed07.scryptachain.org", "seed07.scryptachain.org"));
        vSeeds.push_back(CDNSSeedData("seed08.scryptachain.org", "seed08.scryptachain.org"));
        vSeeds.push_back(CDNSSeedData("seed09.scryptachain.org", "seed09.scryptachain.org"));
        vSeeds.push_back(CDNSSeedData("seed10.scryptachain.org", "seed10.scryptachain.org"));

        fRequireRPCPassword = true;
        fMiningRequiresPeers = true;
        fAllowMinDifficultyBlocks = false;
        fDefaultConsistencyChecks = false;
        fRequireStandard = true;
        fMineBlocksOnDemand = false;
        fSkipProofOfWorkCheck = false;
        fTestnetToBeDeprecatedFieldRPC = false;
        fHeadersFirstSyncingActive = false;

        nPoolMaxTransactions = 3;
        strSporkKey = "02381786ba02d6dd08229fd4f4d6ef285ac21f93833f6b108045bbf180fffedf51";
        strObfuscationPoolDummyAddress = "LTxuXP8eUcdu7dYZR7QP42AScrSdjTGQR2";
        nStartMasternodePayments = 1545129003; // Tuesday 18 December 2018 10:30:03
    }

    const Checkpoints::CCheckpointData& Checkpoints() const
     {
        return data;
    }
};
static CMainParams mainParams;

/**
 * Testnet (v3)
 */
class CTestNetParams : public CMainParams
{
public:
    CTestNetParams()
    {
        networkID = CBaseChainParams::TESTNET;
        strNetworkID = "test";
        pchMessageStart[0] = 0x45;
        pchMessageStart[1] = 0x76;
        pchMessageStart[2] = 0x65;
        pchMessageStart[3] = 0xba;
        vAlertPubKey = ParseHex("03cad31945cfc98a5f88d04fbc0140467644285c2b1d689864d1d403127eb54030");
        nDefaultPort = 32222;
        nEnforceBlockUpgradeMajority = 51;
        nRejectBlockOutdatedMajority = 75;
        nToCheckBlockUpgradeMajority = 100;
        nMinerThreads = 0;
        nTargetTimespan = 1 * 60; // Scrypta: 1 day
        nTargetSpacing = 1 * 60;  // Scrypta: 1 minute
        nLastPOWBlock = 100;
        nMaturity = 10;
        nMasternodeCountDrift = 4;
        nModifierUpdateBlock = 51197; //approx Mon, 17 Apr 2017 04:00:00 GMT
        nMaxMoneyOut = 50000000 * COIN;

        //! Modify the testnet genesis block so the timestamp is valid for a later start.
        genesis.nTime = 1546642871;
        genesis.nNonce = 3205836;

        hashGenesisBlock = genesis.GetHash();
        assert(hashGenesisBlock == uint256("0xd07464ddcf6a6d7e7f48de12d9e824bc1a874965d5f52b300d9962ca489bb8e3"));

        vFixedSeeds.clear();
        vSeeds.clear();

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1, 127); // Testnet lyra addresses start with 't'
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1, 19);  // Testnet lyra script addresses start with '8' or '9'
        base58Prefixes[SECRET_KEY] = std::vector<unsigned char>(1, 174);     // Testnet private keys start with '9' or 'c' (Bitcoin defaults)
        // Testnet lyra BIP32 pubkeys
        base58Prefixes[EXT_PUBLIC_KEY] = boost::assign::list_of(0x3a)(0x61)(0x60)(0xa0).convert_to_container<std::vector<unsigned char> >();
        // Testnet lyra BIP32 prvkeys
        base58Prefixes[EXT_SECRET_KEY] = boost::assign::list_of(0x3a)(0x88)(0x57)(0x37).convert_to_container<std::vector<unsigned char> >();
        // Testnet lyra BIP44
        base58Prefixes[EXT_COIN_TYPE] = boost::assign::list_of(0x80)(0x00)(0x00)(0x01).convert_to_container<std::vector<unsigned char> >();

        convertSeed6(vFixedSeeds, pnSeed6_test, ARRAYLEN(pnSeed6_test));
	vSeeds.push_back(CDNSSeedData("testseed01.scryptachain.org", "testseed01.scryptachain.org"));
        vSeeds.push_back(CDNSSeedData("testseed02.scryptachain.org", "testseed02.scryptachain.org"));
        vSeeds.push_back(CDNSSeedData("testseed03.scryptachain.org", "testseed03.scryptachain.org"));
        vSeeds.push_back(CDNSSeedData("testseed04.scryptachain.org", "testseed04.scryptachain.org"));
        vSeeds.push_back(CDNSSeedData("testseed05.scryptachain.org", "testseed05.scryptachain.org"));
        vSeeds.push_back(CDNSSeedData("testseed06.scryptachain.org", "testseed06.scryptachain.org"));

        fRequireRPCPassword = true;
        fMiningRequiresPeers = true;
        fAllowMinDifficultyBlocks = true;
        fDefaultConsistencyChecks = false;
        fRequireStandard = false;
        fMineBlocksOnDemand = false;
        fTestnetToBeDeprecatedFieldRPC = true;

        nPoolMaxTransactions = 2;
        strSporkKey = "0354b4ab1ec749189681540c0ce73c0c1c7f72f95d636e590026a3180f57cbe2ca";
        strObfuscationPoolDummyAddress = "t7afHrateavDhxKiYe4eLNr6zVSLHcLuaK";
        nStartMasternodePayments = 1545129003; // Tuesday 18 December 2018 10:30:03
    }
    const Checkpoints::CCheckpointData& Checkpoints() const
    {
        return dataTestnet;
    }
};
static CTestNetParams testNetParams;

/**
 * Regression test
 */
class CRegTestParams : public CTestNetParams
{
public:
    CRegTestParams()
    {
        networkID = CBaseChainParams::REGTEST;
        strNetworkID = "regtest";
        strNetworkID = "regtest";
        pchMessageStart[0] = 0xa1;
        pchMessageStart[1] = 0xcf;
        pchMessageStart[2] = 0x7e;
        pchMessageStart[3] = 0xac;
        nSubsidyHalvingInterval = 150;
        nEnforceBlockUpgradeMajority = 750;
        nRejectBlockOutdatedMajority = 950;
        nToCheckBlockUpgradeMajority = 1000;
        nMinerThreads = 1;
        nTargetTimespan = 24 * 60 * 60; // Scrypta: 1 day
        nTargetSpacing = 1 * 60;        // Scrypta: 1 minutes
        bnProofOfWorkLimit = ~uint256(0) >> 1;
        genesis.nTime = 1517076275;
        genesis.nBits = 0x207fffff;
        genesis.nNonce = 1517140;

        hashGenesisBlock = genesis.GetHash();
        nDefaultPort = 51476;
        //assert(hashGenesisBlock == uint256("0x"));

        vFixedSeeds.clear(); //! Testnet mode doesn't have any fixed seeds.
        vSeeds.clear();      //! Testnet mode doesn't have any DNS seeds.

        fRequireRPCPassword = false;
        fMiningRequiresPeers = false;
        fAllowMinDifficultyBlocks = true;
        fDefaultConsistencyChecks = true;
        fRequireStandard = false;
        fMineBlocksOnDemand = true;
        fTestnetToBeDeprecatedFieldRPC = false;
    }
    const Checkpoints::CCheckpointData& Checkpoints() const
    {
        return dataRegtest;
    }
};
static CRegTestParams regTestParams;

/**
 * Unit test
 */
class CUnitTestParams : public CMainParams, public CModifiableParams
{
public:
    CUnitTestParams()
    {
        networkID = CBaseChainParams::UNITTEST;
        strNetworkID = "unittest";
        nDefaultPort = 51478;
        vFixedSeeds.clear(); //! Unit test mode doesn't have any fixed seeds.
        vSeeds.clear();      //! Unit test mode doesn't have any DNS seeds.

        fRequireRPCPassword = false;
        fMiningRequiresPeers = false;
        fDefaultConsistencyChecks = true;
        fAllowMinDifficultyBlocks = false;
        fMineBlocksOnDemand = true;
    }

    const Checkpoints::CCheckpointData& Checkpoints() const
    {
        // UnitTest share the same checkpoints as MAIN
        return data;
    }

    //! Published setters to allow changing values in unit test cases
    virtual void setSubsidyHalvingInterval(int anSubsidyHalvingInterval) { nSubsidyHalvingInterval = anSubsidyHalvingInterval; }
    virtual void setEnforceBlockUpgradeMajority(int anEnforceBlockUpgradeMajority) { nEnforceBlockUpgradeMajority = anEnforceBlockUpgradeMajority; }
    virtual void setRejectBlockOutdatedMajority(int anRejectBlockOutdatedMajority) { nRejectBlockOutdatedMajority = anRejectBlockOutdatedMajority; }
    virtual void setToCheckBlockUpgradeMajority(int anToCheckBlockUpgradeMajority) { nToCheckBlockUpgradeMajority = anToCheckBlockUpgradeMajority; }
    virtual void setDefaultConsistencyChecks(bool afDefaultConsistencyChecks) { fDefaultConsistencyChecks = afDefaultConsistencyChecks; }
    virtual void setAllowMinDifficultyBlocks(bool afAllowMinDifficultyBlocks) { fAllowMinDifficultyBlocks = afAllowMinDifficultyBlocks; }
    virtual void setSkipProofOfWorkCheck(bool afSkipProofOfWorkCheck) { fSkipProofOfWorkCheck = afSkipProofOfWorkCheck; }
};
static CUnitTestParams unitTestParams;


static CChainParams* pCurrentParams = 0;

CModifiableParams* ModifiableParams()
{
    assert(pCurrentParams);
    assert(pCurrentParams == &unitTestParams);
    return (CModifiableParams*)&unitTestParams;
}

const CChainParams& Params()
{
    assert(pCurrentParams);
    return *pCurrentParams;
}

CChainParams& Params(CBaseChainParams::Network network)
{
    switch (network) {
    case CBaseChainParams::MAIN:
        return mainParams;
    case CBaseChainParams::TESTNET:
        return testNetParams;
    case CBaseChainParams::REGTEST:
        return regTestParams;
    case CBaseChainParams::UNITTEST:
        return unitTestParams;
    default:
        assert(false && "Unimplemented network");
        return mainParams;
    }
}

void SelectParams(CBaseChainParams::Network network)
{
    SelectBaseParams(network);
    pCurrentParams = &Params(network);
}

bool SelectParamsFromCommandLine()
{
    CBaseChainParams::Network network = NetworkIdFromCommandLine();
    if (network == CBaseChainParams::MAX_NETWORK_TYPES)
        return false;

    SelectParams(network);
    return true;
}
