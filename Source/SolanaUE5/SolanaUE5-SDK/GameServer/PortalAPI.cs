using Chaos.NaCl;
using MySqlX.XDevAPI.Common;
using SolanaUE5.SDK.Database;
using System.Security.Cryptography;
using System.Text;

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
            //Passwords are hashed in SHA512 and stored in the database
            SHA512 sha512 = SHA512.Create();
            var HASHED_PASS = sha512.ComputeHash(Encoding.UTF8.GetBytes(password));
            _BCAccount.Password = Convert.ToBase64String(HASHED_PASS);
            await DatabaseClient.Add_Account(_BCAccount);
        }
        public static async Task<string?> LoginAccount(string username, string password)
        {
            SHA512 sha512 = SHA512.Create();
            var HASHED_PASS = sha512.ComputeHash(Encoding.UTF8.GetBytes(password));
            string? _playerID = await DatabaseClient.CheckCredentials(username, Convert.ToBase64String(HASHED_PASS));
            if (_playerID != null && _playerID != String.Empty)
            {
                return _playerID;
            }
            return _playerID;
        }
    }
}
