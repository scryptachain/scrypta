// Copyright (c) 2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcoin developers
// Copyright (c) 2014-2015 The Dash developers
// Copyright (c) 2015-2018 The LYRA developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "chainparams.h"

#include "random.h"
#include "util.h"
#include "utilstrencodings.h"
#include "clientversion.h"
#include "streams.h"
#include <assert.h>

#include <boost/assign/list_of.hpp>

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
    (750, uint256("0x000000bf6ca733834f543433247145ad2451728e79bcc18f5b109a6bbd11cf87")) //Last POW Block
    (20925, uint256("0xd1e129981da28059d1d2484e8f098bab1f41e011631dee8771ab8bc16a00ee2c"))
    (302930, uint256("0xcc63411f5e6bac2d6e88c3347b6ed3baf0744a16466d7920225ac77edfd37414"))
    (475000, uint256("0xded790ed1b7158216d6e46206b2740a1ae9f3e1cea002d0f8cfe980ff8b3c457"));

static const Checkpoints::CCheckpointData data = {
    &mapCheckpoints,
    1574249693, // * UNIX timestamp of last checkpoint block
    1005815,    // * total number of transactions between genesis and last checkpoint
                //   (the tx=... number in the SetBestChain debug.log lines)
    2930        // * estimated number of transactions per day after checkpoint
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
        nTargetSpacing = 1 * 60;       // Scrypta Block Time: 1 Min
        nLastPOWBlock = 750;           //Last Pow Block
        nMaturity = 20;
        nMasternodeCountDrift = 20;
        nModifierUpdateBlock = 615800;
        nMasternodeCollateralAmt = 15000;
        nMaxMoneyOut = 50000000 * COIN; //Max coin supply 50 Millions Lyra

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

        // validation by-pass
        nLyraBadBlockTime = 1545446256; // Block 751
        nLyraBadBlockBits = 0x1e013b10; // Block 751

        // Make sure we get the correct genesis info.
        /*
        printf("genesis.nTime = %u \n", genesis.nTime);
        printf("genesis.nNonce = %u \n", genesis.nNonce);
        printf("genesis.GetHash = %s\n", genesis.GetHash().ToString().c_str());
        printf("genesis.MerkleRoot = %s\n", genesis.hashMerkleRoot.ToString().c_str());
        */
        hashGenesisBlock = genesis.GetHash();
        assert(hashGenesisBlock == uint256("0xe2aacf31ce196903e00157a50d207d04a152176b4eb83ecbb0b75b0c9455d1fd"));
        assert(genesis.hashMerkleRoot == uint256S("0x2346f9c436d961bf4f5a3818f60154e15ddd056d59f05b270240d0837d15daa5"));

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1, 48); // Mainnet lyra addresses start with 'L'
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1, 13); // Mainnet lyra script addresses start with '6'
        base58Prefixes[SECRET_KEY] = std::vector<unsigned char>(1, 174);
        base58Prefixes[EXT_PUBLIC_KEY] = boost::assign::list_of(0x2D)(0x02)(0x31)(0x33).convert_to_container<std::vector<unsigned char> >();
        base58Prefixes[EXT_SECRET_KEY] = boost::assign::list_of(0x02)(0x21)(0x25)(0x2B).convert_to_container<std::vector<unsigned char> >();
        // 	BIP44 coin type is from https://github.com/satoshilabs/slips/blob/master/slip-0044.md
        base58Prefixes[EXT_COIN_TYPE] = boost::assign::list_of(0x80)(0x00)(0x01)(0xF1).convert_to_container<std::vector<unsigned char> >();

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
        strBootstrapUrl = "https://bs.scryptachain.org/latest.zip";

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

        // validation by-pass
        nLyraBadBlockTime = 1545446256; // Block 751
        nLyraBadBlockBits = 0x1e013b10; // Block 751

        // Make sure we get the correct genesis info.
        /*
        printf("genesis.nTime = %u \n", genesis.nTime);
        printf("genesis.nNonce = %u \n", genesis.nNonce);
        printf("genesis.GetHash = %s\n", genesis.GetHash().ToString().c_str());
        printf("genesis.MerkleRoot = %s\n", genesis.hashMerkleRoot.ToString().c_str());
        */
        hashGenesisBlock = genesis.GetHash();
        assert(hashGenesisBlock == uint256("0xd07464ddcf6a6d7e7f48de12d9e824bc1a874965d5f52b300d9962ca489bb8e3"));
        assert(genesis.hashMerkleRoot == uint256S("0x2346f9c436d961bf4f5a3818f60154e15ddd056d59f05b270240d0837d15daa5"));

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
        strBootstrapUrl = "https://galilel.org/bootstrap/v3/testnet";
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

bool ParamsSelected()
{
    return pCurrentParams != nullptr;
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

uint64_t GetBlockChainSize()
{
    const uint64_t GB_BYTES = 1000000000LL;
    return 1LL * GB_BYTES;
}

bool VerifyGenesisBlock(const std::string& datadir, const uint256& genesisHash, std::string& err)
{
    const std::string path = strprintf("%s/blocks/blk00000.dat", datadir);
    FILE *fptr = fopen(path.c_str(), "rb");
    if (!fptr) {
        err = strprintf("Failed to open file: %s", path);
        return false;
    }

    CAutoFile filein(fptr, SER_DISK, CLIENT_VERSION);
    if (filein.IsNull()) {
        err = strprintf("Open block file failed: %s", path);
        return false;
    }

    char buf[MESSAGE_START_SIZE] = {0};
    filein.read(buf, MESSAGE_START_SIZE);
    if (memcmp(buf, Params().MessageStart(), MESSAGE_START_SIZE)) {
        err = strprintf("Invalid magic numer %s in the file: %s", HexStr(buf, buf + MESSAGE_START_SIZE), path);
        return false;
    }

    unsigned int nSize = 0;
    filein >> nSize;
    if (nSize < 80 || nSize > 2000000) {
        err = strprintf("Invalid block size %u in the file: %s", nSize, path);
        return false;
    }

    CBlock block;
    try {

        /** Read block */
        filein >> block;
    } catch (std::exception& e) {
        err = strprintf("Deserialize or I/O error: %s", e.what());
        return false;
    }

    /** Check block hash */
    if (block.GetHash() != genesisHash) {
        err = strprintf("Block hash %s does not match genesis block hash %s", block.GetHash().ToString(), genesisHash.ToString());
        return false;
    } else {
        return true;
    }
}
