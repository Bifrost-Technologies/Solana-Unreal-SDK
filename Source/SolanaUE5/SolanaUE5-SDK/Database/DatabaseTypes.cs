using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SolanaUE5.SDK.Database
{
    public class GameAccount
    {
        public string AccID { get; set; }
        public string Username { get; set; }
        public string? Password { get; set; }
        public string Gamertag { get; set; }
        public string Email { get; set; }
        public string SolAddress { get; set; }
        public int Account_level { get; set; }
        public int Season_level { get; set; }
        public int Experience { get; set; }
        public int Season_exp { get; set; }
        public int Gold { get; set; }
        public int Solarite { get; set; }

        public GameAccount(string _AccID, string _username, string _gamertag, string _email, string _soladdy, int accLVL, int sLVL, int exp, int sEXP, int gold, int solarite)
        {
            AccID = _AccID;
            Username = _username;
            Gamertag = _gamertag;
            Email = _email;
            SolAddress = _soladdy;
            Account_level = accLVL;
            Season_level = sLVL;
            Experience = exp;
            Season_exp = sEXP;
            Gold = gold;
            Solarite = solarite;
        }
    }
    public class StoreItem
    {
        public string StoreItemID { get; set; }

        public string GameItemID { get; set; }
        public string ItemName { get; set; }
        public string? ClassName { get; set; }
        public string Image { get; set; }
        public decimal Price { get; set; }
        public StoreItem(string storeItemID, string gameItemID, string itemName, string? className, string image, decimal price)
        {
            StoreItemID = storeItemID;
            GameItemID = gameItemID;
            ItemName = itemName;
            ClassName = className;
            Image = image;
            Price = price;
        }
    }

        public class LoginRequest
    {
        public string? Username { get; set; }
        public string? Password { get; set; }
    }
    public class RequestStorePurchase
    {
        public string? UserToken { get; set; }
        public string? StoreItemID { get; set; }
    }
    public class JWTtoken
    {
        public string? Token { get; set; }
    }

}
