using System;
using System.Collections.Generic;
using System.Linq;
using System.Security.Cryptography;
using System.Text;
using System.Threading.Tasks;
using Microsoft.AspNetCore.DataProtection;
using UnrealEngine.Framework;

namespace UnrealSolana.SDK.Security
{
    public class Titan
    {
       public IDataProtector Shield { get; }

        public Titan(string password)
        {
                SHA512 sha512 = SHA512.Create();
                var HASHED_PASS = sha512.ComputeHash(Encoding.UTF8.GetBytes(password));
                IDataProtectionProvider protectionProvider = DataProtectionProvider.Create("UnrealSolana.SDK");
                Shield = protectionProvider.CreateProtector(Convert.ToBase64String(HASHED_PASS));
        }

    }
}
