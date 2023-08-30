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
namespace UnrealSolana.SDK.Database
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
