using SolanaUE5.SDK.Database;

namespace SolanaUE5.SDK
{
    /// <summary>
    /// Encrypted Login Portal
    /// </summary>
    public class Portal
    {
        public static async Task RegisterAccount(string username, string gamertag, string email, string password, string sol_address)
        {
            GameAccount _BCAccount = new GameAccount(Guid.NewGuid().ToString(), username, gamertag, email, sol_address, 1, 1, 0, 0, 0, 0);
            _BCAccount.Password = password;
            await DatabaseClient.Add_Account(_BCAccount);
        }
        public static async Task<string?> LoginAccount(string username, string password)
        {
            string loginToken = "";
            string? _playerID = await DatabaseClient.CheckCredentials(username, password);
            if (_playerID != null && _playerID != String.Empty)
            {
                return _playerID;
            }
            Console.WriteLine(loginToken);
            return _playerID;
        }
    }
}
