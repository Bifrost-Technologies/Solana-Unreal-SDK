using UnrealSolana.SDK.Database;
using UnrealSolana.SDK.Solana.NFT;
using UnrealSolana.SDK.Solana.Store;
using Solnet.Programs;
using Solnet.Rpc.Builders;
using Solnet.Rpc.Core.Http;
using Solnet.Rpc.Messages;
using Solnet.Rpc.Models;
using Solnet.Wallet;

namespace UnrealSolana.SDK.Solana
{
    public static class SolClient
    {
        public static async Task<string> GenerateStoreTransaction(GameAccount _gameAccount, StoreItem _storeItem)
        {
            return await StoreTransactions.BuildStoreTransaction(_gameAccount.SolAddress, _storeItem.StoreItemID, _storeItem.Price);
        }
        public static async Task<string> GenerateStoreMintTransaction(GameServer _gameServer, GameAccount _gameAccount, StoreItem _storeItem)
        {
            DigitalCollectible collectible = await DatabaseClient.GetDigitalCollectible(_storeItem.CollectibleID);
            return await StoreTransactions.CraftandSignStoreMintTransaction(_gameServer, _storeItem.Price, new PublicKey(_gameAccount.SolAddress), collectible);
        }
        public static async Task<string> GenerateMetaplexTransaction(GameServer _gameServer, GameAccount _gameAccount, DigitalCollectible _collectibleItem)
        {
            return await MetaplexTransactions.CraftMetaplexToken(_gameServer, new PublicKey(_gameAccount.SolAddress), _collectibleItem);
        }
    }
}
