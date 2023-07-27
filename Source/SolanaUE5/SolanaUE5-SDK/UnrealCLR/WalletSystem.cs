using System;
using System.Collections.Generic;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using UnrealEngine.Framework;

namespace SolanaUE5.SDK
{
    public class Main
    {
        public static void OnWorldPostBegin()
        {
            Debug.Log(LogLevel.Display, "Hello, Unreal Engine!");
            Actor alarmSound = new("AlarmSound");
            AudioComponent alarmAudioComponent = new(alarmSound);
            SoundWave alarmSoundWave = SoundWave.Load("/Game/Tests/AlarmSound");

            Debug.AddOnScreenMessage(-1, 5.0f, Color.PowderBlue, "Sound wave duration: " + alarmSoundWave.Duration + " seconds");

            alarmSoundWave.Loop = true;
            alarmAudioComponent.SetSound(alarmSoundWave);
            alarmAudioComponent.Play();

            Assert.IsTrue(alarmAudioComponent.IsPlaying);
        }

        public void OnEndPlay() => Debug.ClearOnScreenMessages();
    }
}
