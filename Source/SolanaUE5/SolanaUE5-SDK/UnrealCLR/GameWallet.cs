using Frontier.Security;
using Microsoft.AspNetCore.DataProtection;
using Solnet.Wallet;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using UnrealEngine.Framework;

namespace UnrealSolana.Wallet
{
    public class GameWallet
    {
        private string sessionKey { get; set; }
        private Titan playerTitan { get; set; }

        public PublicKey playerAddress { get; set; }
        public decimal Balance { get; set; }

        public bool freshWallet { get; set; }

        public GameWallet(string password) 
        {
            playerTitan = new Titan(password);
        }
        public void CreateWallet()
        {
            try
            {
                if (sessionKey == null && freshWallet == true)
                {
                    Account newAccount = new Account();
                    sessionKey = playerTitan.Shield.Protect(newAccount.PrivateKey.Key);
                    SaveWallet();
                }
            }
            catch (Exception ex) 
            { 
                UnrealEngine.Framework.Debug.Log(LogLevel.Error, ex.ToString()); 
            }
        }
        public void SyncWallet(string secretKey)
        {
            try
            {
                if (sessionKey == null && freshWallet == true)
                {
                    sessionKey = playerTitan.Shield.Protect(secretKey);
                    SaveWallet();
                }
            }catch(Exception ex)
            {
                UnrealEngine.Framework.Debug.Log(LogLevel.Error, ex.ToString());
            }
        }
        public void LoadWallet()
        {
            try
            {
                if(File.Exists(@"C:\\Program Files (x86)\Bifrost\Frontier\sealed_airlock.wallet"))
                {
                    sessionKey = File.ReadAllText(@"C:\\Program Files (x86)\Bifrost\Frontier\sealed_airlock.wallet");
                    freshWallet = false;
                }
                else
                {
                    freshWallet = true;
                }
               
            }catch(Exception ex) 
            {
                UnrealEngine.Framework.Debug.Log(LogLevel.Error, ex.ToString()); 
            }
        }
        public void SaveWallet()
        {
            try
            { 
                File.WriteAllText(@"C:\\Program Files (x86)\Bifrost\Frontier\sealed_airlock.wallet", sessionKey);
            }
            catch (Exception ex) 
            {
                UnrealEngine.Framework.Debug.Log(LogLevel.Error, ex.ToString());
            }
        }

    }
}
