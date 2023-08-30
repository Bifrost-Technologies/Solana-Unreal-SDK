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
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace UnrealSolana.SDK.Database
{
    public static class SQL
    {
        public const string InsertNewBCAccount = "INSERT INTO `UnrealSolana`.`bc_accounts` (`acc_id`,`username`,`password`,`email`,'gamertag',`sol_address`,`account_level`,'experience','season_level','season_exp',`gold`,`solarite`) VALUES (@acc_id, @username, @password, @email, @gamertag, @sol_address, @account_level, @exp, @season_level, @season_exp, @gold, @solarite); ";
       
        public static string SelectQuery(string _databaseName, string _tableName, string _columnName)
        {
            return "SELECT `" + _columnName + "` FROM `" + _databaseName + "`.`" + _tableName + "`";
        }
        public static string UpdateQuery(string _databaseName, string _tableName, string _columnName)
        {
            return "UPDATE `" + _databaseName + "`.`" + _tableName + "` SET `" + _columnName + "` = @t" + _columnName + "";
        }
        public static string GetRowQuery(string _databaseName, string _tableName)
        {
            return "SELECT * FROM `" + _databaseName + "`.`" + _tableName + "`";
        }

        public static string WhereMatch(string _columnName, string _value)
        {
            return " WHERE `" + _columnName + "` = '" + _value + "'";
        }
        public static string WhereUserPW(string username, string password)
        {
            return " WHERE `username` = '" + username + "'" + " AND " + "`password` = '" + password + "'";
        }
    }
}
