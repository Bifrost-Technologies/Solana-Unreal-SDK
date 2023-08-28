using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Security.Cryptography;
using System.Text;
using System.Threading.Tasks;
using MySql;
using MySql.Data.MySqlClient;
using SolanaUE5.SDK.Solana;
using Solnet.Metaplex.NFT.Library;
using Solnet.Wallet;

namespace SolanaUE5.SDK.Database
{
    public static class DatabaseClient
    {
        
        public static string DatabaseName = "SolanaUE5";

        //Development Connection String for the MySQL Database
        public const string connection = @"server=localhost;userid=root;password=$olanaGame;database=SolanaGameDatabase";
        public static MySqlConnection InitializeConnection()
        {

            return new MySqlConnection(connection);

        }
        public static async Task<string?> GetPlayerID(string player_nickname)
        {
            using var db_connection = new MySqlConnection(connection);
            await db_connection.OpenAsync();
            using var get_cmd = new MySqlCommand(SQL.SelectQuery(DatabaseName, DBTable.GameAccounts, DBGameAccountColumns.GamerTag) + SQL.WhereMatch(DBGameAccountColumns.GamerTag, player_nickname), db_connection);
            string? player_ID = (string?)await get_cmd.ExecuteScalarAsync();
            if (player_ID != null)
                return player_ID.ToString();
            await Task.CompletedTask;
            return player_ID;
        }
        public static async Task<GameAccount?> GetPlayerProfile(string player_ID)
        {

            using var db_connection = new MySqlConnection(connection);
            await db_connection.OpenAsync();
            using var get_cmd = new MySqlCommand(SQL.GetRowQuery(DatabaseName, DBTable.GameAccounts) + SQL.WhereMatch(DBGameAccountColumns.AccountID, player_ID), db_connection);
            using MySqlDataReader rdr = (MySqlDataReader)await get_cmd.ExecuteReaderAsync();
            while (rdr.Read())
            {
                return new GameAccount(
                     rdr.GetString(0),
                     rdr.GetString(1),
                     rdr.GetString(3),
                     rdr.GetString(4),
                     rdr.GetString(5),
                     rdr.GetInt32(6),
                     rdr.GetInt32(7),
                     rdr.GetInt32(8),
                     rdr.GetInt32(9),
                     rdr.GetInt32(10),
                     rdr.GetInt32(11));
            }
            return null;
        }
        public static async Task<StoreItem> GetStoreItem(string storeitemID)
        {
            using var db_connection = new MySqlConnection(connection);
            await db_connection.OpenAsync();
            using var get_cmd = new MySqlCommand(SQL.GetRowQuery(DatabaseName, DBTable.StoreItems) + SQL.WhereMatch(DBStoreItemColumns.StoreitemID, storeitemID), db_connection);
            using MySqlDataReader rdr = (MySqlDataReader)await get_cmd.ExecuteReaderAsync();
            while (rdr.Read())
            {
                return new StoreItem(
                     rdr.GetString(0),
                     rdr.GetString(1),
                     rdr.GetString(2),
                     rdr.GetString(3),
                     rdr.GetString(4),
                     rdr.GetDecimal(6)
                   );
            }
            return null;
        }
        public static async Task<GameItem> GetGameItem(string gameitemID)
        {
            using var db_connection = new MySqlConnection(connection);
            await db_connection.OpenAsync();
            using var get_cmd = new MySqlCommand(SQL.GetRowQuery(DatabaseName, DBTable.GameItems) + SQL.WhereMatch(DBGameItemColumns.GameitemID, gameitemID), db_connection);
            using MySqlDataReader rdr = (MySqlDataReader)await get_cmd.ExecuteReaderAsync();
            while (rdr.Read())
            {
                return new GameItem(
                     rdr.GetString(0),
                     rdr.GetString(1),
                     rdr.GetString(2),
                     rdr.GetString(3),
                     rdr.GetString(4)
                   );
            }
            return null;
        }
        public static async Task<InventoryItem> GetInventoryItem(string inventoryitemID)
        {
            using var db_connection = new MySqlConnection(connection);
            await db_connection.OpenAsync();
            using var get_cmd = new MySqlCommand(SQL.GetRowQuery(DatabaseName, DBTable.Inventory) + SQL.WhereMatch(DBInventoryItemColumns.InventoryitemID, inventoryitemID), db_connection);
            using MySqlDataReader rdr = (MySqlDataReader)await get_cmd.ExecuteReaderAsync();
            while (rdr.Read())
            {
                return new InventoryItem(
                     rdr.GetString(0),
                     rdr.GetString(1),
                     rdr.GetString(2),
                     rdr.GetInt32(3)
                   );
            }
            return null;
        }
        public static async Task<Dictionary<string, DigitalCollectible>> GetMetaplexDatabase()
        {

            var MetaplexCollectibles = new Dictionary<string, DigitalCollectible>();
            using var db_connection = new MySqlConnection(connection);
            await db_connection.OpenAsync();
            using var get_cmd = new MySqlCommand(SQL.GetRowQuery(DatabaseName, DBTable.CollectibleData), db_connection);
            using MySqlDataReader rdr = (MySqlDataReader)await get_cmd.ExecuteReaderAsync();
            while (rdr.Read())
            {
                DigitalCollectible _DigitalCollectibleData = new DigitalCollectible();
                _DigitalCollectibleData.CollectibleID = rdr.GetString(0);
                _DigitalCollectibleData.MintAuthority = new PublicKey(rdr.GetString(1));
                _DigitalCollectibleData.tokenStandard = (TokenStandard)rdr.GetInt32(2);
                _DigitalCollectibleData.Metadata = new Metadata
                {
                    name = rdr.GetString(3),
                    symbol = rdr.GetString(4),
                    uri = rdr.GetString(5),
                    //You can store several creators in a mysql column but for simplicity we will use one creator (game server)
                    creators = new List<Creator> { new Creator(new PublicKey(rdr.GetString(6)), 100, true) },
                    collection = null,
                    uses = null,
                    programmableConfig = null
                    
                };

                MetaplexCollectibles.Add(_DigitalCollectibleData.CollectibleID, _DigitalCollectibleData);
            }
            return MetaplexCollectibles;
        }

        public static async Task<DigitalCollectible> GetDigitalCollectible(string collectibleID)
        {

            var MetaplexCollectibles = new Dictionary<string, DigitalCollectible>();
            using var db_connection = new MySqlConnection(connection);
            await db_connection.OpenAsync();
            using var get_cmd = new MySqlCommand(SQL.GetRowQuery(DatabaseName, DBTable.CollectibleData) + SQL.WhereMatch(DBCollectibleDataColumns.CollectibleID, collectibleID), db_connection);
            using MySqlDataReader rdr = (MySqlDataReader)await get_cmd.ExecuteReaderAsync();
            while (rdr.Read())
            {
                DigitalCollectible _DigitalCollectibleData = new DigitalCollectible();
                _DigitalCollectibleData.CollectibleID = rdr.GetString(0);
                _DigitalCollectibleData.MintAuthority = new PublicKey(rdr.GetString(1));
                _DigitalCollectibleData.tokenStandard = (TokenStandard)rdr.GetInt32(2);
                _DigitalCollectibleData.Metadata = new Metadata
                {
                    name = rdr.GetString(3),
                    symbol = rdr.GetString(4),
                    uri = rdr.GetString(5),
                    //You can store several creators in a mysql column but for simplicity we will use one creator (game server)
                    creators = new List<Creator> { new Creator(new PublicKey(rdr.GetString(6)), 100, true) },
                    collection = null,
                    uses = null,
                    programmableConfig = null

                };

                  return _DigitalCollectibleData;
            }
            return null;
        }
        public static async Task<object?> GetPlayerData(string columnName, string player_ID)
        {
            using var db_connection = new MySqlConnection(connection);
            await db_connection.OpenAsync();
            using var get_cmd = new MySqlCommand(SQL.SelectQuery(DatabaseName, DBTable.GameAccounts, columnName) + SQL.WhereMatch(DBGameAccountColumns.AccountID, player_ID), db_connection);
            return await get_cmd.ExecuteScalarAsync();
        }
        public static async Task<string?> CheckCredentials(string username, string password)
        {           
            var sha512 = SHA512.Create();
            var HASHED_PW = Convert.ToBase64String(sha512.ComputeHash(Encoding.UTF8.GetBytes(password)));
            using var db_connection = new MySqlConnection(connection);
            await db_connection.OpenAsync();
            using var get_cmd = new MySqlCommand(SQL.SelectQuery(DatabaseName, DBTable.GameAccounts, DBGameAccountColumns.AccountID) + SQL.WhereUserPW(username, HASHED_PW), db_connection);
            var _player_ID = await get_cmd.ExecuteScalarAsync();

            if (_player_ID != null)
            {
                return _player_ID.ToString();
            }
            else
            {
                return null;
            }

        }
        public static async Task Add_Account(GameAccount newAccount)
        {
            using var db_connection = new MySqlConnection(connection);
            await db_connection.OpenAsync();
            using var cmd = new MySqlCommand(SQL.InsertNewBCAccount, db_connection);
            cmd.Parameters.AddWithValue("@acc_id", newAccount.AccID);
            cmd.Parameters.AddWithValue("@username", newAccount.Username);
            cmd.Parameters.AddWithValue("@password", newAccount.Password);
            cmd.Parameters.AddWithValue("@email", newAccount.Email);
            cmd.Parameters.AddWithValue("@gamertag", newAccount.Gamertag);
            cmd.Parameters.AddWithValue("@sol_address", newAccount.SolAddress);
            cmd.Parameters.AddWithValue("@account_level", newAccount.Account_level);
            cmd.Parameters.AddWithValue("@exp", newAccount.Experience);
            cmd.Parameters.AddWithValue("@gold", newAccount.Gold);
            cmd.Parameters.AddWithValue("@solarite", newAccount.Solarite);
            await cmd.PrepareAsync();
            await cmd.ExecuteNonQueryAsync();
        }
        public static async Task Update_AccountInfo(string target_accID, string column_name, string column_value)
        {
            using var db_connection = new MySqlConnection(connection);
            await db_connection.OpenAsync();

            string _paramname = "@t" + column_name;
            using var cmd = new MySqlCommand(SQL.UpdateQuery(DatabaseName, DBTable.GameAccounts, column_name) + SQL.WhereMatch(DBGameAccountColumns.AccountID, target_accID), db_connection);

            cmd.Parameters.AddWithValue(_paramname, column_value);

            await cmd.PrepareAsync();
            await cmd.ExecuteNonQueryAsync();
            Debug.WriteLine(column_name + " has been updated for " + target_accID);
        }
        public static async Task Update_AccountData(string target_accID, string column_name, int column_value, DBUpdateType updateType = DBUpdateType.Replace)
        {
            using var db_connection = new MySqlConnection(connection);
            await db_connection.OpenAsync();

            string _paramname = "@t" + column_name;
            using var get_cmd = new MySqlCommand(SQL.SelectQuery(DatabaseName, DBTable.GameAccounts, column_name) + SQL.WhereMatch(DBGameAccountColumns.AccountID, target_accID), db_connection);
            int originalValue = Convert.ToInt32(get_cmd.ExecuteScalar());
            using var cmd = new MySqlCommand(SQL.UpdateQuery(DatabaseName, DBTable.GameAccounts, column_name) + SQL.WhereMatch(DBGameAccountColumns.AccountID, target_accID), db_connection);

            if (updateType == DBUpdateType.Replace)
                cmd.Parameters.AddWithValue(_paramname, column_value);

            if (updateType == DBUpdateType.Add)
                cmd.Parameters.AddWithValue(_paramname, originalValue + column_value);

            if (updateType == DBUpdateType.Subtract)
                cmd.Parameters.AddWithValue(_paramname, originalValue - column_value);

            await cmd.PrepareAsync();
            await cmd.ExecuteNonQueryAsync();
            Debug.WriteLine(column_name + " has been updated for " + target_accID);
        }

        internal static Task<bool> CheckInventoryItemOwnership(string inventoryItemID)
        {
            throw new NotImplementedException();
        }

        internal static Task DeleteInventoryItem(string inventoryItemID)
        {
            throw new NotImplementedException();
        }
    }

}
