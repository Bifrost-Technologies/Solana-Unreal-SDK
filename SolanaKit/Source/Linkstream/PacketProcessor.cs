using Solnet.Programs;
using Solnet.Rpc.Models;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using UnrealEngine.Framework;

namespace LinkStream.Server
{
    public static class PacketProcessor
    {
        public static string ReadStreamRequest(LinkNetwork _LinkServer, string data)
        {
            try
            {
               
                    _LinkServer.TriggerSignRequest(data);

                    return "Transaction request received successfully";
               
            }
            catch (Exception hk)
            {
              
                return "Error occured during packet processing";
            }
           
        }
        public static string DecodeTransactionMessage(ReadOnlySpan<byte> messageData)
        {
            List<DecodedInstruction> ix = InstructionDecoder.DecodeInstructions(Message.Deserialize(messageData));

            string aggregate = ix.Aggregate(
                "",
                (s, instruction) =>
                {
                    s += $"\n\tProgram: {instruction.ProgramName}\n\t\t\t Instruction: {instruction.InstructionName}\n";
                    return instruction.Values.Aggregate(
                        s,
                        (current, entry) =>
                            current + $"\t\t\t\t{entry.Key} - {Convert.ChangeType(entry.Value, entry.Value.GetType())}\n");
                });
            System.Diagnostics.Debug.WriteLine(aggregate);

            return aggregate;
        }
    }
}
