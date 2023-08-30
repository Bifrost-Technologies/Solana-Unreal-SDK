﻿/*
 *  Unreal Solana SDK
 *  Copyright (c) 2023 Bifrost Inc.
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
using Microsoft.AspNetCore.DataProtection;
using UnrealSolana.SDK.Errors;
using UnrealSolana.SDK.Security;
using UnrealSolana.SDK.Solana.Store;
using UnrealSolana.SDK.Solana.Vault;
using Solnet.Metaplex.NFT;
using Solnet.Metaplex.NFT.Library;
using Solnet.Metaplex.Utilities;
using Solnet.Programs;
using Solnet.Rpc;
using Solnet.Rpc.Builders;
using Solnet.Rpc.Core.Http;
using Solnet.Rpc.Messages;
using Solnet.Rpc.Models;
using Solnet.Wallet;

namespace UnrealSolana.SDK.Solana.NFT
{

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
                Account GameItemCreationAuthorityAccount = SolanaVault.FromSecretKey(_gameServer.NetworkSecurity.Titan.Unprotect(encryptedKey));

                return (await client.CreateNFT(GameItemCreationAuthorityAccount, freshMint, TokenStandard.FungibleAsset, digitalCollectible.Metadata, false, true)).Result;
            }
            else return null;
        }
        public static async Task<string> CraftMetaplexTokenPurchaseTransaction(GameServer _gameServer, decimal USDCprice, PublicKey playerWalletAddress, DigitalCollectible digitalCollectible)
        {
            var rpcClient = ClientFactory.GetClient(Cluster.MainNet);
            MetadataClient client = new MetadataClient(rpcClient);
            Account freshMint = new Account();
            if (_gameServer.AuthorityVault.NetworkAuthorityKeys != null)
            {
                string? encryptedKey = _gameServer.AuthorityVault.NetworkAuthorityKeys[AuthorityKeyNames.GameItemCreationAuthority];
                if (encryptedKey != null)
                {
                    Account GameItemCreationAuthorityAccount = SolanaVault.FromSecretKey(_gameServer.NetworkSecurity.Titan.Unprotect(encryptedKey));
                    byte[] transactionMessage = (await CraftNFTpurchaseTransaction(playerWalletAddress, USDCprice, GameItemCreationAuthorityAccount, freshMint, TokenStandard.FungibleAsset, digitalCollectible.Metadata, false, true));
                    //We build a light weight packet using break symbol as our discriminator character
                    //First position in the array is the transaction message. Anything after it is a signature or multiple signatures.
                    return Convert.ToBase64String(transactionMessage) + "|" + Convert.ToBase64String(GameItemCreationAuthorityAccount.Sign(transactionMessage));
                }
                else return ErrorResponses.BadRequest;
            }
            else return ErrorResponses.BadRequest;
        }
        public static async Task<byte[]> CraftNFTpurchaseTransaction(PublicKey playerAddress, decimal USDCprice, Account ownerAccount, Account mintAccount, TokenStandard tokenStandard, Metadata metaData, bool isMasterEdition, bool isMutable, MetadataVersion metadataVersion = MetadataVersion.V4, PublicKey _Authority = null, Account _UpdateAuthority = null, PublicKey _payerAddress = null, PublicKey delegateAddress = null, MetadataDelegateRole delegateRole = MetadataDelegateRole.Update, bool UpdateAuthorityIsSigner = true, ulong collectionDetails = 0, int maxSupply = 0)
        {
            var RpcClient = ClientFactory.GetClient(Cluster.MainNet);
            PublicKey Authority = ownerAccount.PublicKey;
            Account UpdateAuthority = ownerAccount;
            PublicKey PayerAddress = ownerAccount.PublicKey;
            PublicKey metadataAddress = PDALookup.FindMetadataPDA(mintAccount);
            PublicKey tokenRecord = PDALookup.FindTokenRecordPDA(ownerAccount, mintAccount);
            PublicKey masterEditionAddress = null;
            PublicKey delegateRecord = null;
            PublicKey rulesetAddress = null;
            List<Account> Signers = new() { ownerAccount, mintAccount };

            //Override default values if specific parameters are supplied
            if (delegateAddress != null)
            {
                delegateRecord = PDALookup.FindDelegateRecordPDA(UpdateAuthority, mintAccount.PublicKey, delegateAddress, delegateRole);
            }
            if (isMasterEdition == true)
            {
                masterEditionAddress = PDALookup.FindMasterEditionPDA(mintAccount.PublicKey);
            }
            if (Authority != _Authority && _Authority != null)
            {
                Authority = _Authority;
            }
            if (UpdateAuthority != _UpdateAuthority && _UpdateAuthority != null)
            {
                UpdateAuthority = _UpdateAuthority;

                Signers.Add(UpdateAuthority);
            }
            if (PayerAddress != _payerAddress && _payerAddress != null)
            {
                PayerAddress = _payerAddress;
            }
            if (metaData.programmableConfig != null)
            {
                rulesetAddress = metaData.programmableConfig.key;
            }


            ulong minBalanceForExemptionMint = (await RpcClient.GetMinimumBalanceForRentExemptionAsync(TokenProgram.MintAccountDataSize)).Result;
            TransactionInstruction NFTmetadata_instruction = MetadataProgram.CreateMetadataAccount(metadataAddress, mintAccount.PublicKey, Authority, PayerAddress, UpdateAuthority, metaData, tokenStandard, isMutable, UpdateAuthorityIsSigner, masterEditionAddress, maxSupply, collectionDetails);
            PublicKey USDC = new PublicKey(StoreAddresses.USDC_Token_Address);
            var _price = USDCprice * 1000000;
            RequestResult<ResponseValue<LatestBlockHash>> blockHash = await RpcClient.GetLatestBlockHashAsync();
            byte[] NFTrecycleTransactionMessage = new TransactionBuilder().
              SetRecentBlockHash(blockHash.Result.Value.Blockhash).
              SetFeePayer(ownerAccount).
              AddInstruction(SystemProgram.CreateAccount(ownerAccount, mintAccount, minBalanceForExemptionMint, TokenProgram.MintAccountDataSize, TokenProgram.ProgramIdKey)).
              AddInstruction(TokenProgram.InitializeMint(mintAccount.PublicKey, 0, ownerAccount.PublicKey, ownerAccount.PublicKey)).
              AddInstruction(AssociatedTokenAccountProgram.CreateAssociatedTokenAccount(ownerAccount.PublicKey, ownerAccount.PublicKey, mintAccount.PublicKey)).
              AddInstruction(TokenProgram.MintTo(mintAccount.PublicKey, ownerAccount.PublicKey, 1, ownerAccount.PublicKey)).
              AddInstruction(NFTmetadata_instruction).
              AddInstruction(MetadataProgram.SignMetadata(new PublicKey((string)metadataAddress), ownerAccount.PublicKey)).
              AddInstruction(AssociatedTokenAccountProgram.CreateAssociatedTokenAccount(ownerAccount, playerAddress, mintAccount)).
              AddInstruction(TokenProgram.Transfer(AssociatedTokenAccountProgram.DeriveAssociatedTokenAccount(ownerAccount, mintAccount), AssociatedTokenAccountProgram.DeriveAssociatedTokenAccount(playerAddress, mintAccount), 1, ownerAccount)).
              AddInstruction(TokenProgram.Transfer(AssociatedTokenAccountProgram.DeriveAssociatedTokenAccount(playerAddress, USDC), AssociatedTokenAccountProgram.DeriveAssociatedTokenAccount(ownerAccount, USDC), 1, ownerAccount)).

              CompileMessage();

            return NFTrecycleTransactionMessage;
        }
        public static async Task<string> CraftNFTrecycleTransaction(PublicKey playerAddress, Account ownerAccount, Account mintAccount, TokenStandard tokenStandard, Metadata metaData, bool isMasterEdition, bool isMutable, MetadataVersion metadataVersion = MetadataVersion.V4, PublicKey _Authority = null, Account _UpdateAuthority = null, PublicKey _payerAddress = null, PublicKey delegateAddress = null, MetadataDelegateRole delegateRole = MetadataDelegateRole.Update, bool UpdateAuthorityIsSigner = true, ulong collectionDetails = 0, int maxSupply = 0)
        {
            var RpcClient = ClientFactory.GetClient(Cluster.MainNet);
            PublicKey Authority = ownerAccount.PublicKey;
            Account UpdateAuthority = ownerAccount;
            PublicKey PayerAddress = ownerAccount.PublicKey;
            PublicKey metadataAddress = PDALookup.FindMetadataPDA(mintAccount);
            PublicKey tokenRecord = PDALookup.FindTokenRecordPDA(ownerAccount, mintAccount);
            PublicKey masterEditionAddress = null;
            PublicKey delegateRecord = null;
            PublicKey rulesetAddress = null;
            List<Account> Signers = new() { ownerAccount, mintAccount };

            //Override default values if specific parameters are supplied
            if (delegateAddress != null)
            {
                delegateRecord = PDALookup.FindDelegateRecordPDA(UpdateAuthority, mintAccount.PublicKey, delegateAddress, delegateRole);
            }
            if (isMasterEdition == true)
            {
                masterEditionAddress = PDALookup.FindMasterEditionPDA(mintAccount.PublicKey);
            }
            if (Authority != _Authority && _Authority != null)
            {
                Authority = _Authority;
            }
            if (UpdateAuthority != _UpdateAuthority && _UpdateAuthority != null)
            {
                UpdateAuthority = _UpdateAuthority;

                Signers.Add(UpdateAuthority);
            }
            if (PayerAddress != _payerAddress && _payerAddress != null)
            {
                PayerAddress = _payerAddress;
            }
            if (metaData.programmableConfig != null)
            {
                rulesetAddress = metaData.programmableConfig.key;
            }


            ulong minBalanceForExemptionMint = (await RpcClient.GetMinimumBalanceForRentExemptionAsync(TokenProgram.MintAccountDataSize)).Result;
            TransactionInstruction NFTmetadata_instruction = MetadataProgram.CreateMetadataAccount(metadataAddress, mintAccount.PublicKey, Authority, PayerAddress, UpdateAuthority, metaData, tokenStandard, isMutable, UpdateAuthorityIsSigner, masterEditionAddress, maxSupply, collectionDetails);
                      RequestResult<ResponseValue<LatestBlockHash>> blockHash = await RpcClient.GetLatestBlockHashAsync();

            byte[] NFTrecycleTransactionMessage = new TransactionBuilder().
                SetRecentBlockHash(blockHash.Result.Value.Blockhash).
                SetFeePayer(ownerAccount).
                AddInstruction(SystemProgram.CreateAccount(ownerAccount, mintAccount, minBalanceForExemptionMint, TokenProgram.MintAccountDataSize, TokenProgram.ProgramIdKey)).
                AddInstruction(TokenProgram.InitializeMint(mintAccount.PublicKey, 0, ownerAccount.PublicKey, ownerAccount.PublicKey)).
                AddInstruction(AssociatedTokenAccountProgram.CreateAssociatedTokenAccount(ownerAccount.PublicKey, ownerAccount.PublicKey, mintAccount.PublicKey)).
                AddInstruction(TokenProgram.MintTo(mintAccount.PublicKey, ownerAccount.PublicKey, 1, ownerAccount.PublicKey)).
                AddInstruction(NFTmetadata_instruction).
                AddInstruction(MetadataProgram.SignMetadata(new PublicKey((string)metadataAddress), ownerAccount.PublicKey)).
                AddInstruction(AssociatedTokenAccountProgram.CreateAssociatedTokenAccount(ownerAccount, playerAddress, mintAccount)).
                AddInstruction(TokenProgram.Transfer(AssociatedTokenAccountProgram.DeriveAssociatedTokenAccount(ownerAccount, mintAccount), AssociatedTokenAccountProgram.DeriveAssociatedTokenAccount(playerAddress, mintAccount), 1, ownerAccount)).
                CompileMessage();

            return Convert.ToBase64String(NFTrecycleTransactionMessage);
            

        }

    }
}
