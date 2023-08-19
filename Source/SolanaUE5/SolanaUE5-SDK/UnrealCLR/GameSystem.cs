using System;
using System.Collections.Generic;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using UnrealEngine.Framework;

namespace UnrealSolana.SDK
{
    public class Testing
    {
        public static void Test()
        {
            Debug.Log(LogLevel.Display, "Hello, Unreal Engine from C#!");
        

            Debug.AddOnScreenMessage(-1, 20.0f, Color.PowderBlue, "Hello, Unreal Engine from C#!");

          
        }

     
    }
}
