using SolanaUE5.SDK.Solana.NFT;
using Solnet.Programs;
using Solnet.Rpc.Builders;
using Solnet.Rpc.Core.Http;
using Solnet.Rpc.Messages;
using Solnet.Rpc.Models;
using Solnet.Wallet;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SolanaUE5.SDK.Solana.Store
{
    public class StoreTransactions
    {
        public static async Task<string> BuildStoreTransaction(string playerWalletAddress, string itemID, decimal price)
        {
            var rpcClient = Solnet.Rpc.ClientFactory.GetClient(Solnet.Rpc.Cluster.MainNet);

            //USDC Transaction
            PublicKey mint = new PublicKey(StoreAddresses.USDC_Token_Address);

            //Store PoS is the receiver
            PublicKey recipient = new PublicKey(StoreAddresses.Store_PoS_address);
            PublicKey walletOwner = new PublicKey(playerWalletAddress);

            RequestResult<ResponseValue<LatestBlockHash>> blockHash = await rpcClient.GetLatestBlockHashAsync();
            //convert to lamports based on the number of decimals
            var _price = price * 1000000;
            byte[] transferTx = new TransactionBuilder().
            SetRecentBlockHash(blockHash.Result.Value.Blockhash).
            SetFeePayer(walletOwner).
            AddInstruction(MemoProgram.NewMemoV2("In-Game Purchase ID: " + itemID)).
            AddInstruction(TokenProgram.Transfer(AssociatedTokenAccountProgram.DeriveAssociatedTokenAccount(walletOwner, mint), AssociatedTokenAccountProgram.DeriveAssociatedTokenAccount(recipient, mint), (ulong)_price, walletOwner)).
            CompileMessage();

            return Convert.ToBase64String(transferTx);
        }
        public static async Task<string> CraftandSignStoreMintTransaction(GameServer gameServer, decimal USDCprice, PublicKey playerAddress, DigitalCollectible digitalCollectible)
        {
            return await MetaplexTransactions.CraftMetaplexTokenPurchaseTransaction(gameServer, USDCprice, playerAddress, digitalCollectible);
        }
    }
}
