/*
 *  Unreal Solana SDK
 *  Copyright (c) 2024 Bifrost Inc.
 *  Author: Nathan Martell
 *
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in all
 *  copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *  SOFTWARE.
 */
using UnrealSolana.SDK.Solana.NFT;
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

namespace UnrealSolana.SDK.Solana.Store
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
