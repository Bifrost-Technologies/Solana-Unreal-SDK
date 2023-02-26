using Solnet.Metaplex.NFT;
using Solnet.Metaplex.NFT.Library;
using Solnet.Rpc;
using Solnet.Wallet;

namespace SolanaUE5.SDK.Solana.NFT
{
    /// <summary>
    /// Coming soon
    /// </summary>
    public class MetaplexTransactions
    {
        public static async Task<string> CraftMetaplexToken(PublicKey playerWalletAddress, DigitalCollectible digitalCollectible)
        {
            var rpcClient = ClientFactory.GetClient(Cluster.MainNet);
            MetadataClient client = new MetadataClient(rpcClient);

            //Hopping over to server encryption and key storage to move forward from here tomorrow
           // var response = client.CreateNFT()
            
            await Task.CompletedTask;
            return "";
        }
    }
}
