using SolanaUE5.SDK.Database;
using SolanaUE5.SDK.Solana.Store;
using Solnet.Programs;
using Solnet.Rpc.Builders;
using Solnet.Rpc.Core.Http;
using Solnet.Rpc.Messages;
using Solnet.Rpc.Models;
using Solnet.Wallet;

namespace SolanaUE5.SDK.Solana
{
    public static class SolClient
    {
        public static async Task<string> GenerateStoreTransaction(GameAccount _gameAccount, StoreItem _storeItem)
        {
            return await StoreTransactions.BuildStoreTransaction(_gameAccount.SolAddress, _storeItem.StoreItemID, _storeItem.Price);
        }
        
    }
}
