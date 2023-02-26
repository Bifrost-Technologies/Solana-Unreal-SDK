using SolanaUE5.SDK.Database;
using SolanaUE5.SDK.Solana;

namespace SolanaUE5.SDK
{
    /// <summary>
    /// GameServer API
    /// </summary>
    public class GameServer
    {

        public static Dictionary<int, int> ExperienceChart = Build_ExperienceChart();
        public static Dictionary<int, int> Build_ExperienceChart()
        {
            int level = 1;
            int snowball_XP = 1000;
           var _ExperienceChart = new Dictionary<int, int>();
            while (level <= 50)
            {
                if (level != 1)
                    snowball_XP =+ Convert.ToInt32((snowball_XP / 100) * 61.08);

                _ExperienceChart.Add(level, snowball_XP);
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
                {
                    await DatabaseClient.Update_AccountData(player_ID, DBGameAccountColumns.Experience, amount, DBUpdateType.Add);
                }
            }
            else
            {
                Console.WriteLine("Account Retrieval failed! Failed to update EXP");
            }
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
                {
                    await DatabaseClient.Update_AccountData(player_ID, DBGameAccountColumns.SeasonExperience, amount, DBUpdateType.Add);
                }
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
        public static async Task RequestStoreTransaction(string playerID, string itemID)
        {
            GameAccount? playerdata = await DatabaseClient.GetPlayerProfile(playerID);
            StoreItem storeItem = await DatabaseClient.GetStoreItem(itemID);

            await SolClient.GenerateStoreTransaction(playerdata, storeItem);
        }
        public static async Task RequestRecyclerOperation(string playerID, string InventoryItemID)
        {
            GameAccount? playerdata = await DatabaseClient.GetPlayerProfile(playerID);

           // InventoryItem inventoryItem = await DatabaseClient.GetInventoryItem(InventoryItemID);
        }
    }
}