namespace SolanaUE5.SDK.Solana.Vault
{
    public class SolanaVault
    {
        private readonly Dictionary<string, byte[]>? NetworkAuthorityKeys;

        private readonly byte[]? EncryptedHandshake;
        public SolanaVault()
        {
            try
            {
                bool directoryExists = Directory.Exists(Directory.GetCurrentDirectory() + "/auth");
                bool handshakeExists = File.Exists(Path.Combine(Directory.GetCurrentDirectory(), "/auth/vault.cypher"));
                bool authvaultExists = File.Exists(Path.Combine(Directory.GetCurrentDirectory(), "/auth/auth.vault"));
                if (!directoryExists)
                {
                    Directory.CreateDirectory(Directory.GetCurrentDirectory()+"/auth");
                }
                if (handshakeExists && authvaultExists)
                {
                    EncryptedHandshake = Convert.FromBase64String(File.ReadAllText(Path.Combine(Directory.GetCurrentDirectory(), "/auth/vault.cypher")));
                    List<string> AuthKeys = File.ReadAllLines(Path.Combine(Directory.GetCurrentDirectory(), "/auth/auth.vault")).ToList();
                    Dictionary<string, byte[]> AuthorityKeys = new();
                    AuthKeys.ForEach(aKey => AuthorityKeys.Add(aKey.Split('|')[0], Convert.FromBase64String(aKey.Split('|')[1])));
                    NetworkAuthorityKeys = AuthorityKeys;
                }
                else
                {
                    throw new Exception("Create a network authority handshake with the network auth tool and place it in the auth folder!");
                }

            }
            catch (Exception ex)
            {
                Console.WriteLine(ex.Message);
                Console.WriteLine("Try creating a handshake with the network auth tool & placing it in the Auth folder. Run as admin as well");
            }


        }
    }
}
