using Solnet.Rpc.Models;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace UnrealSolana.SDK.Types
{
    public class TransactionPacket
    {
        public bool Priority = false;
        public byte[] signedTransaction { get; init; }
        
        public TransactionPacket(bool _priority, byte[] _transaction) 
        {
            Priority = _priority;
            signedTransaction = _transaction;
        }
    }
}
