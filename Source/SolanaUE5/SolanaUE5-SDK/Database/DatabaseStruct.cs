namespace SolanaUE5.SDK.Database
{
    public enum DBUpdateType
    {
        Add,
        Subtract,
        Replace
    }
    public struct DBGameAccountColumns
    {
        public const string AccountID = "acc_id";
        public const string Username = "username";
        public const string Password = "password";
        public const string Email = "email";
        public const string GamerTag = "gamertag";
        public const string Level = "account_level";
        public const string Experience = "experience";
        public const string SeasonLevel = "season_level";
        public const string SeasonExperience = "season_exp";
        public const string Gold = "gold";
        public const string Solarite = "solarite";
    }
    public struct DBStoreItemColumns
    {
        public const string StoreitemID = "storeitem_id";
        public const string GameitemID = "gameitem_id";
        public const string Image = "image";
        public const string Price = "price";

    }
    public struct DBInventoryItemColumns
    {
        public const string InventoryitemID = "storeitem_id";
        public const string WorlditemID = "worlditem_id";
        public const string GameitemID = "gameitem_id";
        public const string Slot = "slot";

    }
    public struct DBGameItemColumns
    {
        public const string GameitemID = "gameitem_id";
        public const string CollectibleID = "collectible_id";
        public const string Name = "name";
        public const string MeshClassName = "meshclass";
        public const string ObjectMaterials = "obj_materials";
    }
    public struct DBCollectibleDataColumns
    {
        public const string CollectibleID = "collectible_id";
        
    }
    public struct DBTable
    {
        public const string GameAccounts = "bc_accounts";
        public const string StoreItems = "store_items";
        public const string GameItems = "game_items";
        public const string Inventory = "inventory_items";
        public const string CollectibleData = "collectible_data";
        public const string WorldItems = "world_items";
    }
}
