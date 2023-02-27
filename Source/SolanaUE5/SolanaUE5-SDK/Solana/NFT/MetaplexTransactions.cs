using Microsoft.AspNetCore.DataProtection;
using SolanaUE5.SDK.Solana.Vault;
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
        public static async Task<string> CraftMetaplexToken(GameServer _gameServer, PublicKey playerWalletAddress, DigitalCollectible digitalCollectible)
        {
            var rpcClient = ClientFactory.GetClient(Cluster.MainNet);
            MetadataClient client = new MetadataClient(rpcClient);
            Account freshMint = new Account();
            
            string? encryptedKey = _gameServer.AuthorityVault.NetworkAuthorityKeys[AuthorityKeyNames.GameItemCreationAuthority];
            if (encryptedKey != null)
            {
                Account GameItemCreationAuthorityAccount = SolanaVault.FromSecretKey(_gameServer._protector.Unprotect(encryptedKey));

                return (await client.CreateNFT(GameItemCreationAuthorityAccount, freshMint, TokenStandard.FungibleAsset, digitalCollectible.Metadata, false, true)).Result;
            }
            else return null;
        }
    }
}
