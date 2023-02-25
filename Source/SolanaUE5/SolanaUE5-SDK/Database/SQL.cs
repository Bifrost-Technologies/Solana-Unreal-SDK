using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SolanaUE5.SDK.Database
{
    public static class SQL
    {
        //Insert Statements
        public const string InsertNewBCAccount = "INSERT INTO `SolanaUE5`.`bc_accounts` (`acc_id`,`username`,`password`,`email`,'gamertag',`sol_address`,`account_level`,'experience','season_level','season_exp',`gold`,`solarite`) VALUES (@acc_id, @username, @password, @email, @gamertag, @sol_address, @account_level, @exp, @season_level, @season_exp, @gold, @solarite); ";
        //Select Statements
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
