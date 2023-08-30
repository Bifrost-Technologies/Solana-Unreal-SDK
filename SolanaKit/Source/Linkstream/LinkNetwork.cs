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
using System.Net.Sockets;
using System.Net;
using System.Reflection.Metadata;
using Microsoft.AspNetCore.DataProtection;

using System.Diagnostics;

using System.Security.Cryptography;
using System.Drawing;

namespace LinkStream.Server
{
    public class SignRequestEventArgs : EventArgs
    {
        public SignRequestEventArgs(string _transactionMessage)
        {
            Message = _transactionMessage;
        }

        public string Message { get; set; }
    }
    public class LinkNetwork
    {
        public string? LinkServiceName { get; set; }
        public bool isOnline { get; set; }
        public bool isLocal { get; set; }

        public bool encryptionEnabled { get; set; }
        public Int32 LinkPort { get; set; }
        public TcpListener? LinkServer { get; set; }
        public TcpClient? LinkClient { get; set; }
        private IPAddress LinkServerIP { get; }
        private IDataProtector Protector { get; set; }

        public event EventHandler<SignRequestEventArgs> signatureRequestEvent;

        public LinkNetwork(Int32 _LinkPort, string _LinkServerIP = "127.0.0.1", string _LinkServerName = "", bool _encryptionEnabled = false)
        {
            LinkServerIP = IPAddress.Parse(_LinkServerIP);
            LinkPort = _LinkPort;
            LinkServiceName = _LinkServerName;
            encryptionEnabled = _encryptionEnabled;
            IDataProtectionProvider provider = DataProtectionProvider.Create("LinkStream");
            Protector = provider.CreateProtector("GateKeeper");
            //KEEP IT LOCAL for maximum security - Make sure ports being used are not open on your network.
            //If you are using LinkStream for a remote connection between dapps make sure to whitelist IP access to specific ports.
            if (_LinkServerIP == "127.0.0.1")
                isLocal= true;
            else
                isLocal = false;
        }
        public void TriggerSignRequest(string _transactionMessage)
        {
            SignRequestEventArgs requestArgs = new SignRequestEventArgs(_transactionMessage);
            SignRequestEvent(requestArgs);
        }
        protected virtual void SignRequestEvent(SignRequestEventArgs e)
        {
            EventHandler<SignRequestEventArgs> SignEvent = signatureRequestEvent;

            if (SignEvent != null)
                SignEvent(this, e);
        }
        public async void LinkStream()
        {
            try
            {
                LinkServer = new TcpListener(LinkServerIP, LinkPort);
                Byte[] bytes = new Byte[1400];
                isOnline = true;
                LinkServer.Start();
                UnrealEngine.Framework.Debug.AddOnScreenMessage(-1, 20.0f, Color.PowderBlue, "Linkstream Module online!");
               
            }
            catch (Exception ae)
            {
                Console.WriteLine(ae);
                UnrealEngine.Framework.Debug.AddOnScreenMessage(-1, 20.0f, Color.PowderBlue, " Linkstream Error: " +ae.Message);

            }

        }
    }
}
