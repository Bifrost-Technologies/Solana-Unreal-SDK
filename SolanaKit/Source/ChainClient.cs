/*
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
using UnrealSolana.SDK.Types;
using UnrealSolana.SDK.Wallet;
using Solnet.Extensions;
using Solnet.Programs.Abstract;
using Solnet.Rpc;
using Solnet.Wallet;

using UnrealEngine.Framework;

namespace UnrealSolana.SDK
{
    public partial class ChainClient : BaseClient
    {
        public UnrealSolana.SDK.Wallet.GameWallet AirlockWallet { get; set; }
        public Sequencer PacketChamber { get; set; }


        public ChainClient(IRpcClient rpcClient, IStreamingRpcClient streamingRpcClient, PublicKey programId) : base(rpcClient, streamingRpcClient, programId)
        {
           
        }

        public async void InitializeChainClient(string password)
        {

            PacketChamber = new Sequencer();
            try
            {
                AirlockWallet = new GameWallet(password);
                string eventMessage = "Successful!";
                int eventID = (int)EventID.Initialization;
                if (AirlockWallet.isLoaded())
                {

                    bool successfulLogin = AirlockWallet.validCredentials();
                    if (successfulLogin)
                    {
                        
                        
                        PacketChamber = new Sequencer();
                        AirlockWallet.tokenWallet = TokenWallet.Load(this.RpcClient, AirlockWallet.tokenMintDatabase, AirlockWallet.playerAddress);
                      
                        AirlockWallet.Balance = AirlockWallet.tokenWallet.Sol;
                        Debug.Log(LogLevel.Warning, AirlockWallet.playerAddress.Key);
                        Debug.Log(LogLevel.Warning, AirlockWallet.Balance.ToString());
 
                        Debug.Log(LogLevel.Warning, "Game Chain Client is initialized!");
                    }
                    else
                    {
                        eventMessage = "Failed login attempt!";
                    
                        Debug.Log(LogLevel.Warning, "Game Chain Client is initialization failed! Bad Login!");
                    }
                }
            }
            catch (Exception ex)
            {
                Debug.Log(LogLevel.Warning, ex.Message);

            }

        }

    }
}
