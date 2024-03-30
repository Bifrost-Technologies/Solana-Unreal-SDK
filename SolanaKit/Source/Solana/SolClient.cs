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
