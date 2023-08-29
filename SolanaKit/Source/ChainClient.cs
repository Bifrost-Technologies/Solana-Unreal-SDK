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
