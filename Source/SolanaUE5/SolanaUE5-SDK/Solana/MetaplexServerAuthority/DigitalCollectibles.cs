using UnrealSolana.SDK.Database;
using Solnet.Metaplex.NFT;
using Solnet.Metaplex.NFT.Library;
using Solnet.Wallet;

namespace UnrealSolana.SDK.Solana
{
    public class DigitalCollectibles
    {
        public static Dictionary<string, DigitalCollectible> MetaplexDatabase { get; set; }

        public static async void InitializeMetaplexDatabase()
        {
            MetaplexDatabase = await DatabaseClient.GetMetaplexDatabase();
        }
    }
    public class DigitalCollectible
    {
        public string CollectibleID { get; set; }
        //Mint Authority
        public PublicKey MintAuthority { get; set; }

        public TokenStandard tokenStandard { get; set; }

        //OnChain Metadata
        public Metadata Metadata { get; set; }

    }
}
