using Microsoft.AspNetCore.DataProtection;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Security.Cryptography;
using System.Text;
using System.Threading.Tasks;

namespace UnrealSolana.SDK.Security
{
    public sealed class NetworkSecurity
    {
        public IDataProtector? Titan;

        private List<Handshake>? SecurityVault { get; set; }

        public  void InitializeNetworkVault()
        {
            SecurityVault = new List<Handshake>();
        }
        public void ActivateTitan()
        {
            IDataProtectionProvider provider = DataProtectionProvider.Create("SolanaUE5");
            Titan = provider.CreateProtector("GateKeeper");
        }
      
        public string GenerateHandshake()
        {
            string handshake = string.Empty;
            using (Aes aesAlg = Aes.Create())
            {
                var IVstring = Convert.ToBase64String(aesAlg.IV);
                var KEYstring = Convert.ToBase64String(aesAlg.Key);
                handshake = String.Join('|', (IVstring + KEYstring));
            }
            if (Titan != null && SecurityVault != null)
            {
                Handshake sessionHS = new Handshake(Titan.Protect(handshake));
                SecurityVault.Add(sessionHS);
            }
            else
                throw new Exception("Titan or SecurityVault is not activate!");
            return handshake;
        }

        public static byte[] EncryptString(string plainText, byte[] Key, byte[] IV)
        {
            if (plainText == null || plainText.Length <= 0)
                throw new ArgumentNullException("plainText");
            if (Key == null || Key.Length <= 0)
                throw new ArgumentNullException("Key");
            if (IV == null || IV.Length <= 0)
                throw new ArgumentNullException("IV");
            byte[] encrypted;

            using (Aes aesAlg = Aes.Create())
            {
                aesAlg.Key = Key;
                aesAlg.IV = IV;

                ICryptoTransform encryptor = aesAlg.CreateEncryptor(aesAlg.Key, aesAlg.IV);

                using (MemoryStream msEncrypt = new MemoryStream())
                {
                    using (CryptoStream csEncrypt = new CryptoStream(msEncrypt, encryptor, CryptoStreamMode.Write))
                    {
                        using (StreamWriter swEncrypt = new StreamWriter(csEncrypt))
                        {
                            swEncrypt.Write(plainText);
                        }
                        encrypted = msEncrypt.ToArray();
                    }
                }
            }

            return encrypted;
        }

        public static string DecryptString(byte[] cipherText, byte[] Key, byte[] IV)
        {
            if (cipherText == null || cipherText.Length <= 0)
                throw new ArgumentNullException("cipherText");
            if (Key == null || Key.Length <= 0)
                throw new ArgumentNullException("Key");
            if (IV == null || IV.Length <= 0)
                throw new ArgumentNullException("IV");

            string plaintext = string.Empty;

            using (Aes aesAlg = Aes.Create())
            {
                aesAlg.Key = Key;
                aesAlg.IV = IV;
              
                ICryptoTransform decryptor = aesAlg.CreateDecryptor(aesAlg.Key, aesAlg.IV);

                using (MemoryStream msDecrypt = new MemoryStream(cipherText))
                {
                    using (CryptoStream csDecrypt = new CryptoStream(msDecrypt, decryptor, CryptoStreamMode.Read))
                    {
                        using (StreamReader srDecrypt = new StreamReader(csDecrypt))
                        {
                            plaintext = srDecrypt.ReadToEnd();
                        }
                    }
                }
            }

            return plaintext;
        }
    }
    public class Handshake 
    {
        readonly DateTime HandshakeExpiration;
        readonly string SessionHandshake;
        public Handshake(string sessionHandshake) 
        {
            if (sessionHandshake != null && sessionHandshake != string.Empty)
            {
                SessionHandshake = sessionHandshake;
                HandshakeExpiration = DateTime.Now + TimeSpan.FromDays(1);
            } 
            else
               throw new Exception("Bad Handshake");
         
        }
    }
}
