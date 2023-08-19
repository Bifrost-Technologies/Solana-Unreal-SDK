using Microsoft.AspNetCore.DataProtection;
using Microsoft.AspNetCore.Mvc;
using Newtonsoft.Json;
using System.Diagnostics;
using UnrealSolana.SDK;
using UnrealSolana.SDK.Database;
using UnrealSolana.SDK.Errors;
using UnrealSolana.SDK.Solana;

namespace SolanaUE5.Controllers
{
    [ApiController]
    [Route("[controller]")]
    public class APIController : ControllerBase
    {

        private readonly ILogger<APIController> _logger;
        private GameServer GameServerSDK { get; set; }
        private string[] patchnotes { get; set; }
        public APIController(ILogger<APIController> logger)
        {
            _logger = logger;
            patchnotes = System.IO.File.ReadAllLines(Directory.GetCurrentDirectory() + "/assets/patchnotes.txt");
            GameServerSDK = new GameServer();
            DigitalCollectibles.InitializeMetaplexDatabase();
        }

        [HttpPost]
        [Route("/login")]
        public async Task<string> RequestLoginAuthorization([FromBody] LoginRequest login)
        {
            Console.WriteLine("Login request pinged!");
            string login_key = String.Empty;
            if (login != null && login.Username != null && login.Password != null && GameServerSDK.NetworkSecurity.Titan != null)
            {
                string? trade_id = await Portal.LoginAccount(login.Username, login.Password);
                Console.WriteLine("Login Found: " + trade_id);

                if (trade_id == null || trade_id == String.Empty)
                    login_key = "ERROR: Incorrect login information";
                else
                    login_key = GameServerSDK.NetworkSecurity.Titan.Protect(trade_id);
            }
            else
            {
                login_key = "ERROR: Incorrect login information";
            }

            Console.WriteLine($"{login_key}");
            var response = login_key;

            return response;
        }

        [HttpPost]
        [Route("/store/mintpurchase")]
        public async Task<string> RequestStoreMintPurchase([FromBody] RequestStorePurchase mintpurchaseDetails)
        {
            if (mintpurchaseDetails != null && mintpurchaseDetails.UserToken != null && mintpurchaseDetails.StoreItemID != null && GameServerSDK.NetworkSecurity.Titan != null)
                return await GameServerSDK.RequestStoreMintTransaction(GameServerSDK.NetworkSecurity.Titan.Unprotect(mintpurchaseDetails.UserToken), mintpurchaseDetails.StoreItemID);
            else
                return ErrorResponses.BadRequest;
        }

        [HttpPost]
        [Route("/store/purchase")]
        public async Task<string> RequestStorePurchase([FromBody] RequestStorePurchase purchaseDetails)
        {
            if (purchaseDetails != null && purchaseDetails.UserToken != null && purchaseDetails.StoreItemID != null && GameServerSDK.NetworkSecurity.Titan != null)
                return await GameServerSDK.RequestStoreTransaction(GameServerSDK.NetworkSecurity.Titan.Unprotect(purchaseDetails.UserToken), purchaseDetails.StoreItemID);
            else
                return ErrorResponses.BadRequest;
        }

        [HttpPost]
        [Route("/solara/recycler")]
        public async Task<string> RequestRecyclerOperation([FromBody] RequestRecycler recycleDetails)
        {
            if (recycleDetails != null && recycleDetails.UserToken != null && recycleDetails.InventoryItemID != null && GameServerSDK.NetworkSecurity.Titan != null)
                return await GameServerSDK.RequestRecyclerOperation(GameServerSDK.NetworkSecurity.Titan.Unprotect(recycleDetails.UserToken), recycleDetails.InventoryItemID);
            else
                return ErrorResponses.BadRequest;
        }

        [HttpGet]
        [Route("/getPatchNotes")]
        public string GetPatchNotes()
        {
            Console.WriteLine("Patch notes retrieved!");
            
            return string.Join(Environment.NewLine, patchnotes);

        }
        [HttpGet]
        [Route("/getGameVersion")]
        public string GetGameVersion()
        {
            Console.WriteLine("Game version retrieved!");
            return "1.0";
        }
        [HttpPost]
        [Route("/getProfile")]
        public async Task<string> GetUserProfile([FromBody] JWTtoken _token)
        {
            Console.WriteLine("server reached!");
            if (_token != null && _token.Token != null && GameServerSDK.NetworkSecurity.Titan != null)
            {
                var decrypted_token = GameServerSDK.NetworkSecurity.Titan.Unprotect(_token.Token);
                Debug.WriteLine("Decrypted Token:" + decrypted_token);

                var userdata = await DatabaseClient.GetPlayerProfile(decrypted_token);

                var user = JsonConvert.SerializeObject(userdata);
                Debug.WriteLine("Json Profile: " + user);
                return user;
            }
            else
            {
                return ErrorResponses.BadRequest;
            }
           
        }

    }
}