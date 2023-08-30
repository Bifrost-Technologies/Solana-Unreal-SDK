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
using Chaos.NaCl;
using MySqlX.XDevAPI.Common;
using UnrealSolana.SDK.Database;
using System.Security.Cryptography;
using System.Text;

namespace UnrealSolana.SDK
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
