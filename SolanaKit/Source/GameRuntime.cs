using Solnet.Rpc;
using UnrealEngine.Framework;
using Solnet.Extensions;
using UnrealSolana.SDK;
using System.Drawing;
using LinkStream.Server;
using Solnet.Rpc.Models;
using System.Net.Sockets;
using System.Net;

namespace Game
{
    public class Main
    {
        public static ChainClient ChainClient { get; set; }
       public static LinkNetwork _LinkNetwork { get; set; }
        public static async void OnWorldBegin()
        {
            Debug.AddOnScreenMessage(-1, 20.0f, Color.PowderBlue, "UnrealSOLNET is initialized");
            Debug.AddOnScreenMessage(-1, 20.0f, Color.PowderBlue, "Solana SDK is ready for use!");

            await Task.CompletedTask;
        }
        public static async void OnWorldPostBegin()
        {
            _LinkNetwork = new LinkNetwork(50510, _LinkServerName: "NET Runtime Listener");
            var ip =IPAddress.Parse("127.0.0.1");
            _LinkNetwork.LinkServer = new TcpListener(ip, _LinkNetwork.LinkPort);
            Byte[] bytes = new Byte[1400];
            _LinkNetwork.isOnline = true;
            _LinkNetwork.LinkServer.Start();
            Debug.AddOnScreenMessage(-1, 20.0f, Color.PowderBlue, "Linkstream Module online!");
        }
        public static void OnWorldEnd()
        {
            _LinkNetwork.isOnline = false;
            _LinkNetwork.LinkServer.Stop();
            _LinkNetwork = null;
        }
        //Client transaction railgun
        public static async void OnWorldDuringPhysicsTick(float deltaTime)
        {
            try
            {
                Byte[] bytes = new Byte[1400];
                if (ChainClient != null && ChainClient.PacketChamber != null)
                {
                    RailGun();
                }
                bool? hasPending = _LinkNetwork.LinkServer.Pending();
               
                if (hasPending != null && hasPending == true)
                {
                    try
                    {
                        //Blueprint is talking to NET runtime
                        _LinkNetwork.LinkClient = await _LinkNetwork.LinkServer.AcceptTcpClientAsync();
                        NetworkStream stream = _LinkNetwork.LinkClient.GetStream();

                        int i = await stream.ReadAsync(bytes, 0, bytes.Length);
                        string data = System.Text.Encoding.ASCII.GetString(bytes, 0, i);
                        string response = string.Empty;
                        string data_decrypted = string.Empty;
                       

                        response = HandleRequestEvent(data);
                        Byte[] response_data = System.Text.Encoding.ASCII.GetBytes(response);
                        await stream.WriteAsync(response_data, 0, response_data.Length);
                        stream.Close();
                        _LinkNetwork.LinkClient.Close();
                        stream.Dispose();
                        _LinkNetwork.LinkClient.Dispose();
                        _LinkNetwork.LinkClient = null;
                    }
                    catch (Exception packetIssues)
                    {
                        Debug.AddOnScreenMessage(-1, 7.0f, Color.PowderBlue, "Read client error from TCP: " +packetIssues.ToString());
                    }

                }
            }
            catch(Exception ex) 
            {

            }
        }
       public static void InitializeGameClient()
        {
            try
            {
                var rpc1 = ClientFactory.GetClient("https://side-virulent-butterfly.solana-devnet.quiknode.pro/c14f8529f9bbaef2a55542c7e3fbc5eb06e5acf0/");
               var rpc2 = ClientFactory.GetStreamingClient("wss://side-virulent-butterfly.solana-devnet.quiknode.pro/c14f8529f9bbaef2a55542c7e3fbc5eb06e5acf0/");
                ChainClient = new ChainClient(rpc1, rpc2, null);
                ChainClient.InitializeChainClient("test");

            }
            catch (Exception ex)
            {
                Debug.Log(LogLevel.Warning, ex.Message);

            }

        }
        public static void RetrieveUpdatedWalletBalance()
        {
            ChainClient.AirlockWallet.tokenWallet = TokenWallet.Load(ChainClient.RpcClient, ChainClient.AirlockWallet.tokenMintDatabase, ChainClient.AirlockWallet.playerAddress);
            ChainClient.AirlockWallet.UpdateBalance(ChainClient.AirlockWallet.tokenWallet.Sol);
        }
        public static void RailGun()
        {
            try
            {
                if (ChainClient.PacketChamber != null && ChainClient.PacketChamber.ToArray().Count() > 0)
                {
                    int delay = 200;
                    if (ChainClient.PacketChamber.Count() > 5)
                        delay = 50;
                    foreach (var tpkt in ChainClient.PacketChamber.ToArray())
                    {
                      
                        var tx = ChainClient.RpcClient.SendTransaction(tpkt.signedTransaction, true, commitment: Solnet.Rpc.Types.Commitment.Finalized);
                        Debug.AddOnScreenMessage(-1, 7.0f, Color.PowderBlue, "Sending game transaction! - "+ tx.RawRpcResponse);
                        Debug.Log(LogLevel.Warning, "Sending a game transaction! - " + Convert.ToBase64String(tpkt.signedTransaction) + " | " + tx.RawRpcResponse);
                        var tpktRef = ChainClient.PacketChamber.Find(p => p == tpkt);
                        _ = ChainClient.PacketChamber.Remove(tpktRef);
                        //await Task.Delay(delay);
                    }
                }
            }catch (Exception ex)
            {
                Debug.Log(LogLevel.Warning, ex.Message);
            }
        }
        public static string HandleRequestEvent(string message)
        {
           // Debug.AddOnScreenMessage(-1, 7.0f, Color.PowderBlue, "Request received by Linkstream!" + message);

            if (message != null && message.Contains('|'))
            {
                try
                {
                    //Data can be parsed and encoded anyway you want. In this example we use a break symbol to separate data in our requests. JSON can be used as well
                    string[] parsedData = message.Split('|');
                    int requestID = Convert.ToInt32(parsedData[0]);

                    if (requestID == 0)
                    {
                        Debug.AddOnScreenMessage(-1, 20.0f, Color.PowderBlue, "Message readable!");
                        return "AIRLOCK WALLET INFO <br> Sol Address: " + ChainClient.AirlockWallet.playerAddress + "<br> Balance: " + ChainClient.AirlockWallet.Balance + "<br><br> CHAIN INFO<br> Latest Blockhash:" + ChainClient.RpcClient.GetLatestBlockHashAsync().Result.Result.Value.Blockhash.ToString() + "<br>" + "Block #: " + ChainClient.RpcClient.GetBlockHeight().Result.ToString();
                    }
                }
                catch (Exception ex)
                {
                    Debug.AddOnScreenMessage(-1, 20.0f, Color.PowderBlue, ex.Message);
                }
                return Math.Round(ChainClient.AirlockWallet.Balance, 3).ToString();
            } return Math.Round(ChainClient.AirlockWallet.Balance, 3).ToString();
        }
     
        
    }
}


