﻿/*
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
using Microsoft.AspNetCore.DataProtection;
using UnrealSolana.SDK.Database;
using UnrealSolana.SDK.Errors;
using UnrealSolana.SDK.Security;
using UnrealSolana.SDK.Solana;
using UnrealSolana.SDK.Solana.Vault;

namespace UnrealSolana.SDK
{
    /// <summary>
    /// GameServer API
    /// </summary>
    public class GameServer
    {
        public SolanaVault AuthorityVault { get; set; }
        public NetworkSecurity NetworkSecurity { get; set; }

        public GameServer()
        {
            AuthorityVault = new SolanaVault();
            NetworkSecurity = new NetworkSecurity();
            NetworkSecurity.InitializeNetworkVault();
            NetworkSecurity.ActivateTitan();
        }

        public static Dictionary<int, int> ExperienceChart = Build_ExperienceChart();
        public static Dictionary<int, int> Build_ExperienceChart()
        {
            int level = 1;
            int snowball_XP = 1000;
            var _ExperienceChart = new Dictionary<int, int>();
            while (level <= 50)
            {
                if (level != 1)
                    snowball_XP = +Convert.ToInt32((snowball_XP / 100) * 61.08);

                _ExperienceChart.Add(level, snowball_XP);
                level++;
            }
            return _ExperienceChart;
        }
        public static async Task Level_UP(string player_ID)
        {
            await DatabaseClient.Update_AccountData(player_ID, DBGameAccountColumns.Level, 1, DBUpdateType.Add);
            await DatabaseClient.Update_AccountData(player_ID, DBGameAccountColumns.Experience, 0, DBUpdateType.Replace);
        }
        public static async Task SeasonLevel_UP(string player_ID)
        {
            await DatabaseClient.Update_AccountData(player_ID, DBGameAccountColumns.SeasonLevel, 1, DBUpdateType.Add);
            await DatabaseClient.Update_AccountData(player_ID, DBGameAccountColumns.SeasonExperience, 0, DBUpdateType.Replace);
        }
        public static async Task Earn_Experience(string player_ID, int amount)
        {
            GameAccount? playerdata = await DatabaseClient.GetPlayerProfile(player_ID);
            if (playerdata != null)
            {
                int _playerlevel = playerdata.Account_level;
                int _currentEXP = playerdata.Experience;
                int expCAP = ExperienceChart[_playerlevel];
                if ((_currentEXP + amount) >= expCAP)
                {
                    await Level_UP(player_ID);
                    int rolloverXP = amount - (expCAP - _currentEXP);
                    await DatabaseClient.Update_AccountData(player_ID, DBGameAccountColumns.Experience, rolloverXP, DBUpdateType.Add);
                }
                else
                    await DatabaseClient.Update_AccountData(player_ID, DBGameAccountColumns.Experience, amount, DBUpdateType.Add);
            }
            else
                Console.WriteLine("Account Retrieval failed! Failed to update EXP");
            
        }
        public static async Task Earn_SeasonExperience(string player_ID, int amount)
        {
            GameAccount? playerdata = await DatabaseClient.GetPlayerProfile(player_ID);
            if (playerdata != null)
            {
                int _playerlevel = playerdata.Season_level;
                int _currentEXP = playerdata.Season_exp;
                int expCAP = ExperienceChart[_playerlevel];
                if ((_currentEXP + amount) >= expCAP)
                {
                    await SeasonLevel_UP(player_ID);
                    int rolloverXP = amount - (expCAP - _currentEXP);
                    await DatabaseClient.Update_AccountData(player_ID, DBGameAccountColumns.SeasonExperience, rolloverXP, DBUpdateType.Add);
                }
                else
                    await DatabaseClient.Update_AccountData(player_ID, DBGameAccountColumns.SeasonExperience, amount, DBUpdateType.Add);
            }
            else
            {
                Console.WriteLine("Account Retrieval failed! Failed to update Season EXP");
            }

        }
        public static async Task Earn_solarite(string player_ID, int amount)
        {
            await DatabaseClient.Update_AccountData(player_ID, DBGameAccountColumns.Solarite, amount, DBUpdateType.Add);
        }
        public static async Task Spend_solarite(string player_ID, int amount)
        {
            await DatabaseClient.Update_AccountData(player_ID, DBGameAccountColumns.Solarite, amount, DBUpdateType.Subtract);
        }
        public async Task<string> RequestStoreTransaction(string playerID, string itemID)
        {
            GameAccount? playerAccount = await DatabaseClient.GetPlayerProfile(playerID);
            StoreItem? storeItem = await DatabaseClient.GetStoreItem(itemID);

            if (playerAccount != null && storeItem != null)
                return await SolClient.GenerateStoreTransaction(playerAccount, storeItem);
            else
                return ErrorResponses.AccountRetrievalError;
        }
        public async Task<string> RequestStoreMintTransaction(string playerID, string itemID)
        {
            GameAccount? playerAccount = await DatabaseClient.GetPlayerProfile(playerID);
            StoreItem? storeItem = await DatabaseClient.GetStoreItem(itemID);

            if (playerAccount != null && storeItem != null)
                return await SolClient.GenerateStoreMintTransaction(this, playerAccount, storeItem);
            else
                return ErrorResponses.AccountRetrievalError;
        }
        public async Task<string> RequestRecyclerOperation(string playerID, string InventoryItemID)
        {
            GameAccount? playerAccount = await DatabaseClient.GetPlayerProfile(playerID);
            InventoryItem? inventoryItem = await DatabaseClient.GetInventoryItem(InventoryItemID);
            if (playerAccount != null && inventoryItem != null && inventoryItem.GameItemID != null)
            {
                GameItem? gameItem = await DatabaseClient.GetGameItem(inventoryItem.GameItemID);
                if (gameItem != null && gameItem.CollectibleID != null)
                {
                    DigitalCollectible? digitalCollectible = await DatabaseClient.GetDigitalCollectible(gameItem.CollectibleID);
                    bool playerOwnsItem = await DatabaseClient.CheckInventoryItemOwnership(InventoryItemID);
                    if (playerOwnsItem)
                    {
                        string response = await SolClient.GenerateMetaplexTransaction(this, playerAccount, digitalCollectible);
                        await DatabaseClient.DeleteInventoryItem(InventoryItemID);

                        return response;
                    }
                }
            }
            return ErrorResponses.AccountRetrievalError;
        }
    }
}